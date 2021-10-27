#include <ctype.h>
#include <errno.h>
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

static float timevaldiff(struct timeval start, struct timeval end);
static unsigned char is_string_number(const char *str);

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
	if (ftruncate(shm_start_queue_fd, MSG_BUFF_LEN) == -1) {
		perror("ftruncate()");
		goto erro3;
	}

	shm_start_queue_ptr = mmap(NULL, MSG_BUFF_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, shm_start_queue_fd, 0);
	if (shm_start_queue_ptr == MAP_FAILED) {
		perror("mmap()");
		goto erro4;
	}

	*(unsigned char *)shm_start_queue_ptr = 0;

	while(!feof(exec_file_fd)) {
		int ret;
		Msg msg;
		ret = fscanf(exec_file_fd, "Run %s I=%s D=%lu\n", msg.proc_name, msg.proc_init_rel, &msg.tempo_duracao);
		if (ret != 3) {
			fscanf(stderr, "Arquivo no formato errado.\n");
			goto erro4;
		}

		msg.is_init_absolute = is_string_number(msg.proc_init_rel);
		if (msg.is_init_absolute)
			msg.proc_init_abs = strtoul(msg.proc_init_rel, NULL, 10);

		while(*(unsigned char *)shm_start_queue_ptr);
		gettimeofday(&start, NULL);
		if (ret) {
			sem_wait(sem_start_queue);
			*(unsigned char *)shm_start_queue_ptr = 1;
			memcpy(((unsigned char *)shm_start_queue_ptr) + 1, &msg, sizeof(Msg));
			sem_post(sem_start_queue);
		}

		printf("[INTERPRETADOR] Proc %s iniciado.\n", msg.proc_name);

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

static float timevaldiff(struct timeval start, struct timeval end)
{
	return (end.tv_sec - start.tv_sec) * 1000.0f + (end.tv_usec - start.tv_usec) / 1000.0f;
}

static unsigned char is_string_number(const char *str)
{
	do {
		if (!isdigit(*str)) return 0;
	} while (*++str);
	return 1;
}

