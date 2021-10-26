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

static volatile sig_atomic_t is_running = 1;

int main(void)
{
	Queue *ready_queue;
	sem_t *sem_start_queue;
	void *shm_start_queue_ptr;
	Process *curr_proc = NULL;
	IOProcess *io_proc;
	SList *io_proc_list;
	unsigned long int local_pid = 0;
	int shm_start_queue_fd, stat_loc;
	struct timeval start, now;
	struct sigaction s_action;

	s_action.sa_handler = sig_handler;
	sigemptyset(&s_action.sa_mask);
	s_action.sa_flags = 0;

	sigaction(SIGINT, &s_action, NULL);
	sigaction(SIGTERM, &s_action, NULL);

	ready_queue = queue_create();
	if (!ready_queue) {
		fprintf(stderr, "ERRO: Não foi possível criar fila de prontos.\nAbortando programa.\n");
		exit(EXIT_FAILURE);
	}

	io_proc_list = slist_create(&slist_ioproc_ordering);
	if (!io_proc_list) {
		fprintf(stderr, "ERRO: Não foi possível criar fila de IO.\nAbortando programa.\n");
		exit(EXIT_FAILURE);
	}

	sem_start_queue = sem_open(SEM_NAME, O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, 0);
	shm_start_queue_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
	ftruncate(shm_start_queue_fd, 4096);
	shm_start_queue_ptr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, shm_start_queue_fd, 0);

	while (is_running) {
		if (*(unsigned char*)shm_start_queue_ptr) {
			const char *proc_name;
			Process *proc = (Process *)malloc(sizeof(Process));
			pid_t pid;
			sem_wait(sem_start_queue);
			proc_name = ((char *)shm_start_queue_ptr) + 1;
			proc->name = (char *)malloc(sizeof(char) * (strlen(proc_name) + 1));
			strcpy(proc->name, proc_name);
			*(unsigned char*)shm_start_queue_ptr = 0;
			sem_post(sem_start_queue);
			pid = fork();
			if (!pid) {
				execlp(proc_name, proc_name, NULL);
				fprintf(stderr, "ERRO: não foi possível iniciar o processo \"%s\"\n", proc_name);
				exit(EXIT_FAILURE);
			}
			kill(pid, SIGSTOP);
			proc->pid = pid;
			proc->local_pid = local_pid++;
			queue_enqueue(ready_queue, (void *)proc);
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
				queue_enqueue(ready_queue, tmp->proc);
				print_ready_queue(curr_proc, ready_queue);
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
#ifdef DEBUG
			printf("[INFO] Executando processo \"%s\" de PID %d.\tPID local: %lu\n", curr_proc->name, curr_proc->pid, curr_proc->local_pid);
#endif
			print_ready_queue(curr_proc, ready_queue);
		}

		if (waitpid(curr_proc->pid, &stat_loc, WNOHANG | WUNTRACED)) {
			if (WIFEXITED(stat_loc)) {
#ifdef DEBUG
				printf("[INFO] Processo \"%s\" de PID %d finalizou.\tPID local: %lu\n", curr_proc->name, curr_proc->pid, curr_proc->local_pid);
#endif
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

				curr_proc = NULL;
			}
			continue;
		}

		gettimeofday(&now, NULL);
		if (timevaldiff(start, now) > QUANTUM) {
			kill(curr_proc->pid, SIGSTOP);
#ifdef DEBUG
			printf("[INFO] Pausando processo \"%s\" de PID %d.\tPID local: %lu\n", curr_proc->name, curr_proc->pid, curr_proc->local_pid);
#endif
			queue_enqueue(ready_queue, (void *)curr_proc);
			curr_proc = NULL;
			print_ready_queue(curr_proc, ready_queue);
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

void print_ready_queue(Process *curr_proc, Queue *queue)
{
	Node *node = queue->start;

	if (curr_proc)
		printf("%lu\t[ ", curr_proc->local_pid);
	else
		printf("-1\t[ ");
	while(node) {
		printf("%lu\t", ((Process *)node->val)->local_pid);
		node = node->next;
	}
	printf("]\n");
}

int slist_ioproc_ordering(void *proc1, void *proc2)
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

void sig_handler (int sig)
{
	is_running = 0;
}
