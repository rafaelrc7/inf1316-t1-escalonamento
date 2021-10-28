#include <errno.h>
#include <fcntl.h>
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
#define QUANTUM 1000.0f /* ms */

#define SEM_NAME "/sem_escalonador_rr"
#define SHM_NAME "/shm_escalonador_rr"

typedef struct _process {
	char *name;
	pid_t pid;
	unsigned long int local_pid;
} Process;

typedef struct _io_process {
	Process *proc;
	float io_time;
	struct timeval io_start;
} IOProcess;

static float timevaldiff(struct timeval start, struct timeval end);
static void print_ready_queue(Process *curr_proc, Queue *queue);
static int slist_ioproc_ordering(void *proc1, void *proc2);
static void sig_handler (int sig);
static void cleanup_sem(void);
static void cleanup_shm(void);

static Process *create_process(Queue *ready_queue);

static volatile sig_atomic_t is_running = 1;
static sem_t *sem_message;
static int shm_message_fd;
static void *shm_message_ptr;

int main(void)
{
	Queue *ready_queue;
	Process *curr_proc = NULL;
	IOProcess *io_proc;
	SList *io_proc_list;
	int stat_loc, ret;
	struct timeval start, now;
	struct sigaction s_action;

	s_action.sa_handler = sig_handler;
	sigemptyset(&s_action.sa_mask);
	s_action.sa_flags = 0;
	sigaction(SIGINT, &s_action, NULL);
	sigaction(SIGTERM, &s_action, NULL);

	ready_queue = queue_create();
	if (!ready_queue) {
		fprintf(stderr, "ERRO: queue_create() falhou\n");
		exit(EXIT_FAILURE);
	}

	io_proc_list = slist_create(&slist_ioproc_ordering);
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

	ret = ftruncate(shm_message_fd, 4096);
	if (ret == -1) {
		perror("ftruncate()");
		exit(EXIT_FAILURE);
	}

	shm_message_ptr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, shm_message_fd, 0);
	if (shm_message_ptr == MAP_FAILED) {
		perror("mmap()");
		exit(EXIT_FAILURE);
	}

	sem_wait(sem_message);
	*(unsigned char*)shm_message_ptr = 0;
	sem_post(sem_message);

	while (is_running) {
		/* Checa se existe processo pendente para ser criado */
		if (*(unsigned char*)shm_message_ptr) {
			Process *proc;
			if ((proc = create_process(ready_queue))) {
				print_ready_queue(curr_proc, ready_queue);
				printf("Processo %lu criado e adicionado ao fim da fila.\n", proc->local_pid);
			}
		}

		gettimeofday(&now, NULL);
		io_proc = slist_iterator_head(io_proc_list);
		while(io_proc) {
			if (timevaldiff(io_proc->io_start, now) > io_proc->io_time) {
				IOProcess *tmp = (IOProcess *)slist_iterator_remove(io_proc_list);
				queue_enqueue(ready_queue, tmp->proc);
				print_ready_queue(curr_proc, ready_queue);
				printf("Processo %lu finaliza I/O, é desbloqueado e vai para o fim da fila.\n", tmp->proc->local_pid);
				free(tmp);
			}
			io_proc = (IOProcess *)slist_iterator_next(io_proc_list);
		}

		if (!curr_proc) {
			if (queue_is_empty(ready_queue))
				continue;

			curr_proc = queue_dequeue(ready_queue);
			gettimeofday(&start, NULL);
			kill(curr_proc->pid, SIGCONT);
			print_ready_queue(curr_proc, ready_queue);
			printf("Processo %lu entra em execução.\n", curr_proc->local_pid);
		}

		if (waitpid(curr_proc->pid, &stat_loc, WNOHANG | WUNTRACED)) {
			if (WIFEXITED(stat_loc)) {
				Process *proc = curr_proc;
				curr_proc = NULL;
				print_ready_queue(curr_proc, ready_queue);
				printf("*Processo %lu finalizou.\n", proc->local_pid);
				free(proc->name);
				free(proc);
			} else if (WIFSTOPPED(stat_loc)) {
				IOProcess *io_proc = (IOProcess *)malloc(sizeof(IOProcess));
				io_proc->proc = curr_proc;
				gettimeofday(&io_proc->io_start, NULL);
				io_proc->io_time = IO_TIME;
				slist_insert(io_proc_list, io_proc);
				print_ready_queue(curr_proc, ready_queue);
				printf("Processo %lu é interrompido por I/O e é bloqueado.\n", curr_proc->local_pid);
				curr_proc = NULL;
			}
			continue;
		}

		gettimeofday(&now, NULL);
		if (timevaldiff(start, now) > QUANTUM) {
			Process *proc;
			kill(curr_proc->pid, SIGSTOP);
			queue_enqueue(ready_queue, (void *)curr_proc);
			proc = curr_proc;
			curr_proc = NULL;
			print_ready_queue(curr_proc, ready_queue);
			printf("Processo %lu sofre preempção e vai para o fim da fila.\n", proc->local_pid);
		}


	}

	printf("[INFO] Finalizando escalonador.\n");

	if (curr_proc) {
		free(curr_proc->name);
		free(curr_proc);
	}

	while (!queue_is_empty(ready_queue)) {
		Process *process = queue_dequeue(ready_queue);
		free(process->name);
		free(process);
	}
	queue_destroy(ready_queue);


	while (!slist_is_empty(io_proc_list)) {
		IOProcess *io_process = slist_remove(io_proc_list);
		Process *process = io_process->proc;
		free(process->name);
		free(process);
		free(io_process);
	}
	slist_destroy(io_proc_list);

	return 0;
}

static Process *create_process(Queue *ready_queue)
{
	static unsigned long local_pid = 0;

	const char *proc_name;
	int pipedes[2];
	Process *proc;
	pid_t pid;

	proc = (Process *)malloc(sizeof(Process));
	if (!proc) {
		perror("malloc()");
		exit(EXIT_FAILURE);
	}

	sem_wait(sem_message);
	proc_name = ((char *)shm_message_ptr) + 1;
	proc->name = (char *)malloc(sizeof(char) * (strlen(proc_name) + 1));
	if (!proc->name) {
		perror("malloc()");
		exit(EXIT_FAILURE);
	}

	strcpy(proc->name, proc_name);
	*(unsigned char*)shm_message_ptr = 0;
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
	queue_enqueue(ready_queue, (void *)proc);

	return proc;

erro1:
	*(unsigned char *)shm_message_ptr = 0;
	sem_post(sem_message);
	free(proc->name);
	free(proc);
	return NULL;
}

static float timevaldiff(struct timeval start, struct timeval end)
{
	return (end.tv_sec - start.tv_sec) * 1000.0f + (end.tv_usec - start.tv_usec) / 1000.0f;
}

static void print_ready_queue(Process *curr_proc, Queue *queue)
{
	Node *node = queue->start;

	if (curr_proc)
		printf("curr_proc: %lu\tready_queue: [ ", curr_proc->local_pid);
	else
		printf("curr_proc: -1\tready_queue: [ ");
	while(node) {
		printf("%lu ", ((Process *)node->ptr)->local_pid);
		node = node->next;
	}
	printf("]\t\t");
}

static int slist_ioproc_ordering(void *proc1, void *proc2)
{
	unsigned long int local_pid1 = ((IOProcess *)proc1)->proc->local_pid;
	unsigned long int local_pid2 = ((IOProcess *)proc2)->proc->local_pid;

	if (local_pid1 > local_pid2)
		return 1;
	else if (local_pid1 < local_pid2)
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

