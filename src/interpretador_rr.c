#include <fcntl.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <unistd.h>

#define EXEC_FILE "exec.txt"

float timevaldiff(struct timeval start, struct timeval end);

int main(void)
{
	sem_t *sem_start_queue;
	void *shm_start_queue_ptr;
	int shm_start_queue_fd;
	struct timeval start, now;

	FILE *exec_file_fd = fopen(EXEC_FILE, "r");

	sem_start_queue = sem_open("/sem_escalonador_rr", O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, 0);
	shm_start_queue_fd = shm_open("/shm_escalonador_rr", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
	ftruncate(shm_start_queue_fd, 4096);
	shm_start_queue_ptr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, shm_start_queue_fd, 0);
	*(unsigned char *)shm_start_queue_ptr = 0;
	sem_post(sem_start_queue);
	sem_post(sem_start_queue);

	while(!feof(exec_file_fd)) {
		int ret;
		char buff[4095];
		ret = fscanf(exec_file_fd, "Run %[^\n]\n", buff);

		while(*(unsigned char *)shm_start_queue_ptr);
		gettimeofday(&start, NULL);
		if (ret) {
			sem_wait(sem_start_queue);
			*(unsigned char *)shm_start_queue_ptr = 1;
			strcpy(((char *)shm_start_queue_ptr) + 1, buff);
			sem_post(sem_start_queue);
		}

		printf("[INTERPRETADOR] Proc %s iniciado.\n", buff);

		do {
			gettimeofday(&now, NULL);
		} while(timevaldiff(start, now) < 1000.0f);

	}

	*(unsigned char *)shm_start_queue_ptr = 2;

	fclose(exec_file_fd);
	return 0;
}

float timevaldiff(struct timeval start, struct timeval end)
{
	return (end.tv_sec - start.tv_sec) * 1000.0f + (end.tv_usec - start.tv_usec) / 1000.0f;
}

