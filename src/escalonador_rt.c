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
#define STEP_TIME 1000.0f /* ms */

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
static void print_ready_queue(Process *curr_proc, Process **ready_queue);
static int slist_ready_queue_ordering(void *proc1, void *proc2);
static void sig_handler (int sig);

static volatile sig_atomic_t is_running = 1;

int main(void)
{
	SList *io_proc_list;
	Process **ready_queue;
	unsigned long int ready_queue_size;
	sem_t *sem_start_queue;
	void *shm_start_queue_ptr;
	Process *curr_proc = NULL;
	IOProcess *io_proc;
	unsigned long int local_pid = 0, curr_index;
	int shm_start_queue_fd, stat_loc;
	float time;
	struct timeval period_start, now;
	struct sigaction s_action;

	s_action.sa_handler = sig_handler;
	sigemptyset(&s_action.sa_mask);
	s_action.sa_flags = 0;

	sigaction(SIGINT, &s_action, NULL);
	sigaction(SIGTERM, &s_action, NULL);

	ready_queue_size = ceil(PERIOD_TIME / STEP_TIME);
	ready_queue = (Process **)calloc(ready_queue_size, sizeof(Process *));
	if (!ready_queue) {
		fprintf(stderr, "ERRO: Não foi possível criar fila de prontos.\nAbortando programa.\n");
		exit(EXIT_FAILURE);
	}

	io_proc_list = slist_create(&slist_ready_queue_ordering);
	if (!io_proc_list) {
		fprintf(stderr, "ERRO: Não foi possível criar fila de IO.\nAbortando programa.\n");
		exit(EXIT_FAILURE);
	}

	sem_start_queue = sem_open(SEM_NAME, O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, 0);
	shm_start_queue_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
	ftruncate(shm_start_queue_fd, MSG_BUFF_LEN);
	shm_start_queue_ptr = mmap(NULL, MSG_BUFF_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, shm_start_queue_fd, 0);

	gettimeofday(&period_start, NULL);
	while (is_running) {
		if (*(unsigned char*)shm_start_queue_ptr) {
			unsigned long int i;
			Msg *msg;
			Process *proc = (Process *)malloc(sizeof(Process));
			pid_t pid;
			sem_wait(sem_start_queue);
			msg = ((Msg *)((unsigned char *)shm_start_queue_ptr + 1));
			proc->name = (char *)malloc(sizeof(char) * (strlen(msg->proc_name) + 1));
			strcpy(proc->name, msg->proc_name);
			proc->burst_time = msg->tempo_duracao;
			proc->is_blocked = 0;

			if (msg->is_init_absolute) {
				proc->init_time = msg->proc_init_abs;
			} else {
				unsigned char found = 0;
				const char *proc_init = msg->proc_init_rel;

				for (i = 0; !found && i < ready_queue_size; ++i) {
					if (!ready_queue[i])
						continue;
					if (strcmp(ready_queue[i]->name, proc_init) == 0) {
						proc->init_time = ready_queue[i]->init_time + ready_queue[i]->burst_time;
						found = 1;
					}
				}

				if (!found) {
					fprintf(stderr, "Não existe processo anterior\n");
					exit(EXIT_FAILURE);
				}
			}

			if ((proc->init_time + proc->burst_time) * STEP_TIME > PERIOD_TIME)
				exit(EXIT_FAILURE);

			for (i = 0; i < proc->burst_time; ++i) {
				if (ready_queue[proc->init_time + i]) {
					fprintf(stderr, "Novo processo conflita com processos anteriores.\n");
					exit(EXIT_FAILURE);
				}
			}

			*(unsigned char*)shm_start_queue_ptr = 0;
			sem_post(sem_start_queue);
			pid = fork();
			if (!pid) {
				execlp(proc->name, proc->name, NULL);
				fprintf(stderr, "ERRO: não foi possível iniciar o processo \"%s\"\n", proc->name);
				exit(EXIT_FAILURE);
			}
			kill(pid, SIGSTOP);
			proc->pid = pid;
			proc->local_pid = local_pid++;

			for (i = 0; i < proc->burst_time; ++i) {
				ready_queue[proc->init_time + i] = proc;
			}
#ifdef DEBUG
			printf("[INFO] Criando novo processo \"%s\" de PID %d.\tPID local: %lu\n", proc->name, proc->pid, proc->local_pid);
#endif
			print_ready_queue(curr_proc, ready_queue);
		}

		gettimeofday(&now, NULL);
		io_proc = slist_iterator_head(io_proc_list);
		while(io_proc) {
			if (timevaldiff(io_proc->io_start, now) > io_proc->io_time) {
				IOProcess *tmp = (IOProcess *)slist_iterator_remove(io_proc_list);
				tmp->proc->is_blocked = 0;
				print_ready_queue(curr_proc, ready_queue);
				free(tmp);
			}
			io_proc = (IOProcess *)slist_iterator_next(io_proc_list);
		}

		gettimeofday(&now, NULL);
		time = timevaldiff(period_start, now);
		if (time >= PERIOD_TIME) {
			gettimeofday(&period_start, NULL);
			time = timevaldiff(period_start, now);
		}
		curr_index = time / STEP_TIME;

		if (!curr_proc) {
			if (!ready_queue[curr_index] || ready_queue[curr_index]->is_blocked)
				continue;

			curr_proc = ready_queue[curr_index];
			kill(curr_proc->pid, SIGCONT);
#ifdef DEBUG
			printf("[INFO] Executando processo \"%s\" de PID %d.\tPID local: %lu\n", curr_proc->name, curr_proc->pid, curr_proc->local_pid);
#endif
			print_ready_queue(curr_proc, ready_queue);
		}

		if (waitpid(curr_proc->pid, &stat_loc, WNOHANG | WUNTRACED)) {
			if (WIFEXITED(stat_loc)) {
				unsigned int i;
#ifdef DEBUG
				printf("[INFO] Processo \"%s\" de PID %d finalizou.\tPID local: %lu\n", curr_proc->name, curr_proc->pid, curr_proc->local_pid);
#endif

				for (i = 0; i < curr_proc->burst_time; ++i) {
					ready_queue[curr_proc->init_time + i] = NULL;
				}
				free(curr_proc->name);
				free(curr_proc);
				curr_proc = NULL;
			} else if (WIFSTOPPED(stat_loc)) {
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

		if (ready_queue[curr_index] != curr_proc) {
			kill(curr_proc->pid, SIGSTOP);
#ifdef DEBUG
			printf("[INFO] Pausando processo \"%s\" de PID %d.\tPID local: %lu\n", curr_proc->name, curr_proc->pid, curr_proc->local_pid);
#endif
			curr_proc = NULL;
			print_ready_queue(curr_proc, ready_queue);
		}


	}

	printf("[INFO] Finalizando escalonador.\n");

	curr_proc = NULL;
	for (unsigned int i = 0; i < ready_queue_size; ++i) {
		if (!ready_queue[i])
			continue;

		if (curr_proc != ready_queue[i]) {
			curr_proc = ready_queue[i];
			free(curr_proc->name);
			free(curr_proc);
		}
	}
	free(ready_queue);


	while (!slist_is_empty(io_proc_list)) {
		IOProcess *io_process = slist_remove(io_proc_list);
		free(io_process);
	}
	slist_destroy(io_proc_list);

	sem_close(sem_start_queue);
	close(shm_start_queue_fd);
	sem_unlink(SEM_NAME);
	shm_unlink(SHM_NAME);

	return 0;
}

float timevaldiff(struct timeval start, struct timeval end)
{
	return (end.tv_sec - start.tv_sec) * 1000.0f + (end.tv_usec - start.tv_usec) / 1000.0f;
}

static void print_ready_queue(Process *curr_proc, Process **ready_queue)
{
	unsigned long int i, size = ceil(PERIOD_TIME / STEP_TIME);

	if (curr_proc)
		printf("\n curr_proc: %lu\t schedule: | ", curr_proc->local_pid);
	else
		printf("-1\t| ");

	for (i = 0; i < size; ++i) {
		Process *proc = ready_queue[i];
		if (proc)
			printf("%lu: %lu | ", i, ready_queue[i]->local_pid);
		else
			printf("%lu: x | ", i);
	}
	printf("\n");
}

static int slist_ready_queue_ordering(void *proc1, void *proc2)
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

void sig_handler (int sig)
{
	is_running = 0;
}

