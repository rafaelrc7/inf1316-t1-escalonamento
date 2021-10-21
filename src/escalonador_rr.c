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

#define IO_TIME 3000.0f

float timevaldiff(struct timeval start, struct timeval end);
void print_ready_queue(Queue *queue);

typedef struct _process {
	char *name;
	pid_t pid;
} Process;

typedef struct _io_process {
	Process *proc;
	float io_time;
	struct timeval io_start;
	struct _io_process *next, *prev;
} IO_Process;

#define QUANTUM 1000.0f /* ms */

int main(void)
{
	Queue *ready_queue;
	sem_t *sem_start_queue;
	void *shm_start_queue_ptr;
	Process *curr_proc = NULL;
	IO_Process *io_proc_llist = NULL, *io_proc_it;
	int shm_start_queue_fd, stat_loc;
	struct timeval start, now;

	ready_queue = create_queue();
	if (!ready_queue) {
		fprintf(stderr, "ERRO: Não foi possível criar fila de prontos.\nAbortando programa.\n");
		exit(EXIT_FAILURE);
	}

	sem_start_queue = sem_open("/sem_escalonador_rr", O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, 0);
	sem_wait(sem_start_queue);
	shm_start_queue_fd = shm_open("/shm_escalonador_rr", O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
	shm_start_queue_ptr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, shm_start_queue_fd, 0);

	while (1) {
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
			if (!pid)
				execlp(proc_name, proc_name, NULL);
			kill(pid, SIGSTOP);
			proc->pid = pid;
			enqueue(ready_queue, (void *)proc);
#ifdef DEBUG
			printf("[INFO] Criando novo processo \"%s\" de PID %d.\n", proc->name, proc->pid);
#endif
			print_ready_queue(ready_queue);
		}

		gettimeofday(&now, NULL);
		io_proc_it = io_proc_llist;
		while(io_proc_it) {
			IO_Process *next = io_proc_it->next;
			if (timevaldiff(io_proc_it->io_start, now) > io_proc_it->io_time) {
				enqueue(ready_queue, io_proc_it->proc);
				if (io_proc_it->prev)
					io_proc_it->prev->next = io_proc_it->next;

				if (io_proc_it->next)
					io_proc_it->next->prev = io_proc_it->prev;

				if (io_proc_llist == io_proc_it)
					io_proc_llist = io_proc_it->next;

				free(io_proc_it);
				print_ready_queue(ready_queue);
			}

			io_proc_it = next;
		}

		if (!curr_proc) {
			if (is_queue_empty(ready_queue))
				continue;

			curr_proc = dequeue(ready_queue);
			gettimeofday(&start, NULL);
			kill(curr_proc->pid, SIGCONT);
#ifdef DEBUG
			printf("[INFO] Executando processo \"%s\" de PID %d.\n", curr_proc->name, curr_proc->pid);
#endif
			print_ready_queue(ready_queue);
		}

		if (waitpid(curr_proc->pid, &stat_loc, WNOHANG | WUNTRACED)) {
			if (WIFEXITED(stat_loc)) {
#ifdef DEBUG
				printf("[INFO] Processo \"%s\" de PID %d finalizou.\n", curr_proc->name, curr_proc->pid);
#endif
				curr_proc = NULL;
			} else if (WIFSTOPPED(stat_loc)) {
#ifdef DEBUG
				printf("[INFO] Processo \"%s\" de PID %d entrou em I/O.\n", curr_proc->name, curr_proc->pid);
#endif
				IO_Process *io_proc = (IO_Process *)malloc(sizeof(IO_Process));
				io_proc->proc = curr_proc;
				gettimeofday(&io_proc->io_start, NULL);
				io_proc->io_time = IO_TIME;
				io_proc->next = io_proc_llist;
				if (io_proc->next)
					io_proc->next->prev = io_proc;
				io_proc->prev = NULL;

				io_proc_llist = io_proc;

				curr_proc = NULL;
			}
			continue;
		}

		gettimeofday(&now, NULL);
		if (timevaldiff(start, now) > QUANTUM) {
			kill(curr_proc->pid, SIGSTOP);
#ifdef DEBUG
			printf("[INFO] Pausando processo \"%s\" de PID %d.\n", curr_proc->name, curr_proc->pid);
#endif
			enqueue(ready_queue, (void *)curr_proc);
			curr_proc = NULL;
			print_ready_queue(ready_queue);
		}


	}

	sem_close(sem_start_queue);
	sem_unlink("escalonador_rr");

	return 0;
}

float timevaldiff(struct timeval start, struct timeval end)
{
	return (end.tv_sec - start.tv_sec) * 1000.0f + (end.tv_usec - start.tv_usec) / 1000.0f;
}

void print_ready_queue(Queue *queue)
{
	Node *node = queue->start;

	printf("[ ");
	while(node) {
		printf("%d\t", ((Process *)node->val)->pid);
		node = node->next;
	}
	printf("]\n");
}

