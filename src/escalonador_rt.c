#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <semaphore.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

#include "queue.h"
#include "slist.h"

#define IO_TIME 3000.0f	/* ms */
#define PERIOD_TIME 60000.0f /* ms */
#define SLICE_TIME 1000.0f /* ms */

#define SEM_NAME "/sem_escalonador_rt"
#define SHM_NAME "/shm_escalonador_rt"

#define NAME_BUFF_LEN 1024
typedef struct _msg {
	unsigned long int tempo_duracao;
	char proc_name[NAME_BUFF_LEN];
	unsigned char is_init_absolute;
	union {
		unsigned long int proc_init_abs;
		char proc_init_rel[NAME_BUFF_LEN];
	};
} Msg;
#define MSG_BUFF_LEN (sizeof(Msg))

typedef struct _process {
	char *name;
	unsigned long int local_pid, init_time, burst_time;
	pid_t pid;
	unsigned char is_blocked;
} Process;

typedef struct _io_process {
	Process *proc;
	float io_time;
	struct timeval io_start;
} IOProcess;

static float timevaldiff(struct timeval start, struct timeval end);
static void print_schedule(Process *curr_proc, Process **schedule);
static int slist_schedule_ordering(void *proc1, void *proc2);
static void sig_handler (int sig);
static void cleanup_sem(void);
static void cleanup_shm(void);

static int create_process(Process **schedule, unsigned long int schedule_size);

static volatile sig_atomic_t is_running = 1;
static sem_t *sem_message;
static int shm_message_fd;
static void *shm_message_ptr;

int main(void)
{
	SList *io_proc_list;
	Process **schedule;
	Process *curr_proc = NULL;
	IOProcess *io_proc;
	unsigned long int curr_index, schedule_size;
	int stat_loc, ret;
	unsigned int process_count = 0;
	float time;
	struct timeval period_start, now;
	struct sigaction s_action;

	/* Definindo handlers para sinais */
	s_action.sa_handler = sig_handler;
	sigemptyset(&s_action.sa_mask);
	s_action.sa_flags = 0;
	sigaction(SIGINT, &s_action, NULL);
	sigaction(SIGTERM, &s_action, NULL);

	/* Alocação de estruturas de dados */
	schedule_size = ceil(PERIOD_TIME / SLICE_TIME);
	schedule = (Process **)calloc(schedule_size, sizeof(Process *));
	if (!schedule) {
		perror("calloc()");
		exit(EXIT_FAILURE);
	}

	io_proc_list = slist_create(&slist_schedule_ordering);
	if (!io_proc_list) {
		fprintf(stderr, "ERRO: slist_create() falhou.\n");
		exit(EXIT_FAILURE);
	}

	sem_message = sem_open(SEM_NAME, O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, 1);
	if (sem_message == SEM_FAILED) {
		perror("sem_open()");
		exit(EXIT_FAILURE);
	}
	atexit(&cleanup_sem);

	shm_message_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
	if (shm_message_fd == -1) {
		perror("shm_open()");
		exit(EXIT_FAILURE);
	}
	atexit(&cleanup_shm);

	ret = ftruncate(shm_message_fd, MSG_BUFF_LEN);
	if (ret == -1) {
		perror("ftruncate()");
		exit(EXIT_FAILURE);
	}

	shm_message_ptr = mmap(NULL, MSG_BUFF_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, shm_message_fd, 0);
	if (shm_message_ptr == MAP_FAILED) {
		perror("mmap()");
		exit(EXIT_FAILURE);
	}

	sem_wait(sem_message);
	*(unsigned char*)shm_message_ptr = 0;
	sem_post(sem_message);

	/* LOOP PRINCIPAL DO ESCALONADOR */
	while (is_running) {
		/* Checa se interpretador enviou novo processo para iniciar */
		if (*(unsigned char*)shm_message_ptr) {
			if (create_process(schedule, schedule_size) == 0) {
				if (process_count++ == 0)
					gettimeofday(&period_start, NULL);
				print_schedule(curr_proc, schedule);
			}
		}

		/* pula loop se não tiver processos em execução */
		if (!process_count)
			continue;

		/* Checa se algum processo bloqueado em I/O pode voltar a estar pronto */
		gettimeofday(&now, NULL);
		io_proc = slist_iterator_head(io_proc_list);
		while(io_proc) {
			if (timevaldiff(io_proc->io_start, now) > io_proc->io_time) {
				IOProcess *tmp = (IOProcess *)slist_iterator_remove(io_proc_list);
				tmp->proc->is_blocked = 0;
				print_schedule(curr_proc, schedule);
				free(tmp);
			}
			io_proc = (IOProcess *)slist_iterator_next(io_proc_list);
		}


		/* Avança o index da agenda atual de acordo com o tempo. Reseta
		 * o tempo para zero caso o período definido passe */
		gettimeofday(&now, NULL);
		time = timevaldiff(period_start, now);
		if (time >= PERIOD_TIME) {
			gettimeofday(&period_start, NULL);
			time = timevaldiff(period_start, now);
		}
		curr_index = time / SLICE_TIME;

		/* Se não tiver algum processo em execução, checa se está na hora de
		 * executar algum */
		if (!curr_proc) {
			if (!schedule[curr_index] || schedule[curr_index]->is_blocked)
				continue;

			curr_proc = schedule[curr_index];
			kill(curr_proc->pid, SIGCONT);
#ifdef DEBUG
			printf("[INFO] Executando processo \"%s\" de PID %d.\tPID local: %lu\n", curr_proc->name, curr_proc->pid, curr_proc->local_pid);
#endif
			print_schedule(curr_proc, schedule);
		}

		/* Checa se o processo atual em execução entrou em I/O ou finalizou */
		if (waitpid(curr_proc->pid, &stat_loc, WNOHANG | WUNTRACED)) {
			if (WIFEXITED(stat_loc)) {
				/* finalizou */
				unsigned int i;
#ifdef DEBUG
				printf("[INFO] Processo \"%s\" de PID %d finalizou.\tPID local: %lu\n", curr_proc->name, curr_proc->pid, curr_proc->local_pid);
#endif

				for (i = 0; i < curr_proc->burst_time; ++i) {
					schedule[curr_proc->init_time + i] = NULL;
				}
				free(curr_proc->name);
				free(curr_proc);
				--process_count;
				curr_proc = NULL;
			} else if (WIFSTOPPED(stat_loc)) {
				/* I/O */
#ifdef DEBUG
				printf("[INFO] Processo \"%s\" de PID %d entrou em I/O.\tPID local: %lu\n", curr_proc->name, curr_proc->pid, curr_proc->local_pid);
#endif
				IOProcess *io_proc = (IOProcess *)malloc(sizeof(IOProcess));
				io_proc->proc = curr_proc;
				gettimeofday(&io_proc->io_start, NULL);
				io_proc->io_time = IO_TIME;
				slist_insert(io_proc_list, io_proc);
				curr_proc->is_blocked = 1;

				curr_proc = NULL;
			}
			continue;
		}

		/* Checa se o tempo agendando do processo atual passou, se sim, faz
		 * preempção */
		if (schedule[curr_index] != curr_proc) {
			kill(curr_proc->pid, SIGSTOP);
#ifdef DEBUG
			printf("[INFO] Preempção no processo \"%s\" de PID %d.\tPID local: %lu\n", curr_proc->name, curr_proc->pid, curr_proc->local_pid);
#endif
			curr_proc = NULL;
			print_schedule(curr_proc, schedule);
		}


	}

	/* Limpeza e finalização */
	printf("[INFO] Finalizando escalonador.\n");

	curr_proc = NULL;
	for (unsigned int i = 0; i < schedule_size; ++i) {
		if (!schedule[i])
			continue;

		if (curr_proc != schedule[i]) {
			curr_proc = schedule[i];
			free(curr_proc->name);
			free(curr_proc);
		}
	}
	free(schedule);


	while (!slist_is_empty(io_proc_list)) {
		IOProcess *io_process = slist_remove(io_proc_list);
		free(io_process);
	}
	slist_destroy(io_proc_list);

	return 0;

}

static int create_process(Process **schedule, unsigned long int schedule_size)
{
	static unsigned long local_pid = 0;

	unsigned long int i;
	int pipedes[2];
	Process *proc;
	Msg *msg;
	pid_t pid;

	proc = (Process *)malloc(sizeof(Process));
	if (!proc) {
		perror("malloc()");
		exit(EXIT_FAILURE);
	}

	sem_wait(sem_message);
	msg = ((Msg *)((unsigned char *)shm_message_ptr + 1));
	proc->name = (char *)malloc(sizeof(char) * (strlen(msg->proc_name) + 1));
	if (!proc->name) {
		perror("malloc()");
		exit(EXIT_FAILURE);
	}

	strcpy(proc->name, msg->proc_name);
	proc->burst_time = msg->tempo_duracao;
	proc->is_blocked = 0;

	if (msg->is_init_absolute) {
		proc->init_time = msg->proc_init_abs;
	} else {
		const char *proc_init = msg->proc_init_rel;
		unsigned int found = 0;

		for (i = 0; !found && i < schedule_size; ++i) {
			if (!schedule[i])
				continue;
			if (strcmp(schedule[i]->name, proc_init) == 0) {
				proc->init_time = schedule[i]->init_time + schedule[i]->burst_time;
				found = 1;
			}
		}

		if (!found) {
			fprintf(stderr, "[ERRO] Novo Processo \"%s\" depende de \"%s\" que"
							" não foi econtrado. Ignorando novo processo.\n",
							proc->name, msg->proc_init_rel);
			goto erro1;
		}
	}

	if ((proc->init_time + proc->burst_time) * SLICE_TIME > PERIOD_TIME) {
		fprintf(stderr, "[ERRO] Novo Processo não se encaixa no escalonamento."
						" Ignorando novo processo.\n");
		goto erro1;
	}

	for (i = 0; i < proc->burst_time; ++i) {
		if (schedule[proc->init_time + i]) {
			fprintf(stderr, "[ERRO] Novo Processo conflita com outro processo"
							" escalonado. Ignorando novo processo.\n");
			goto erro1;
		}
	}

	*(unsigned char *)shm_message_ptr = 0;
	sem_post(sem_message);

	if (pipe(pipedes) == -1) {
		perror("pipe()");
		exit(EXIT_FAILURE);
	}

	pid = fork();
	if (pid == -1) {
		/* FORK FAILED */
		close(pipedes[0]);
		close(pipedes[1]);
		perror("fork()");
		exit(EXIT_FAILURE);
	} else if (pid == 0 ) {
		/* SON PROCESS */
		close(pipedes[0]);
		if (fcntl(pipedes[1], F_SETFD, FD_CLOEXEC) != -1)
			execlp(proc->name, proc->name, NULL);
		write(pipedes[1], &errno, sizeof(errno));
		close(pipedes[1]);
		_exit(EXIT_FAILURE);
	} else {
		/* PARENT PROCESS */
		close(pipedes[1]);
		if (read(pipedes[0], &errno, sizeof(errno)) > 0) {
			perror("exec()");
			goto erro1;
		}
		close(pipedes[0]);
	}

	kill(pid, SIGSTOP);
	proc->pid = pid;
	proc->local_pid = local_pid++;

	for (i = 0; i < proc->burst_time; ++i)
		schedule[proc->init_time + i] = proc;

#ifdef DEBUG
	printf("[INFO] Criado novo processo \"%s\" de PID %d.\tPID local: %lu\n", proc->name, proc->pid, proc->local_pid);
#endif

	return 0;

	/* tratamento e limpeza de erro */
erro1:
	*(unsigned char *)shm_message_ptr = 0;
	sem_post(sem_message);
	free(proc->name);
	free(proc);
	return -1;
}

float timevaldiff(struct timeval start, struct timeval end)
{
	return (end.tv_sec - start.tv_sec) * 1000.0f + (end.tv_usec - start.tv_usec) / 1000.0f;
}

static void print_schedule(Process *curr_proc, Process **schedule)
{
	unsigned long int i, size = ceil(PERIOD_TIME / SLICE_TIME);

	if (curr_proc)
		printf("\n curr_proc: %lu\t schedule: | ", curr_proc->local_pid);
	else
		printf("-1\t| ");

	for (i = 0; i < size; ++i) {
		Process *proc = schedule[i];
		if (proc)
			printf("%lu: %lu | ", i, schedule[i]->local_pid);
		else
			printf("%lu: x | ", i);
	}
	printf("\n");
}

static int slist_schedule_ordering(void *proc1, void *proc2)
{
	unsigned long int init_time1 = ((IOProcess *)proc1)->proc->init_time;
	unsigned long int init_time2 = ((IOProcess *)proc2)->proc->init_time;

	if (init_time1 > init_time2)
		return 1;
	else if (init_time1 < init_time2)
		return -1;
	else
		return 0;
}

static void sig_handler (int sig)
{
	is_running = 0;
}

static void cleanup_sem(void)
{
	sem_close(sem_message);
	sem_unlink(SEM_NAME);
}

static void cleanup_shm(void)
{
	close(shm_message_fd);
	shm_unlink(SHM_NAME);
}

