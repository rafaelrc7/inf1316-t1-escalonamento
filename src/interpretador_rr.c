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
#define SEM_NAME "/sem_escalonador_rr"
#define SHM_NAME "/shm_escalonador_rr"

float timevaldiff(struct timeval start, struct timeval end);

int main(void)
{
	sem_t *sem_start_queue;
	void *shm_start_queue_ptr;
	int shm_start_queue_fd;
	struct timeval start, now;

	FILE *exec_file_fd = fopen(EXEC_FILE, "r");
	if (!exec_file_fd) {
		perror("fopen()");
		goto erro1;
	}

	sem_start_queue = sem_open(SEM_NAME, O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, 1);
	if (sem_start_queue == SEM_FAILED) {
		perror("sem_start_queue()");
		goto erro2;
	}
	while((shm_start_queue_fd = shm_open(SHM_NAME, O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)) < 0);
	if (ftruncate(shm_start_queue_fd, 4096) == -1) {
		perror("ftruncate()");
		goto erro3;
	}

	shm_start_queue_ptr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, shm_start_queue_fd, 0);
	if (shm_start_queue_ptr == MAP_FAILED) {
		perror("mmap()");
		goto erro4;
	}

	*(unsigned char *)shm_start_queue_ptr = 0;

	while(!feof(exec_file_fd)) {
		int ret;
		char buff[4095];
		ret = fscanf(exec_file_fd, "Run %[^\n]\n", buff);
		if (ret != 3) {
			fscanf(stderr, "Arquivo no formato errado.\n");
			goto erro4;
		}

		while(*(unsigned char *)shm_start_queue_ptr);
		gettimeofday(&start, NULL);
		if (ret) {
			sem_wait(sem_start_queue);
			*(unsigned char *)shm_start_queue_ptr = 1;
			strcpy(((char *)shm_start_queue_ptr) + 1, buff);
			sem_post(sem_start_queue);
		}

		printf("[INTERPRETADOR] Proc %s iniciado.\n", buff);

		do gettimeofday(&now, NULL);
		while(timevaldiff(start, now) < 1000.0f);

	}

	fclose(exec_file_fd);
	sem_close(sem_start_queue);
	close(shm_start_queue_fd);

	return 0;

	/* limpeza de erro */
erro4:
	close(shm_start_queue_fd);
erro3:
	sem_close(sem_start_queue);
erro2:
	fclose(exec_file_fd);
erro1:
	return EXIT_FAILURE;
}

float timevaldiff(struct timeval start, struct timeval end)
{
	return (end.tv_sec - start.tv_sec) * 1000.0f + (end.tv_usec - start.tv_usec) / 1000.0f;
}

