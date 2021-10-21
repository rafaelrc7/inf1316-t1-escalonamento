#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct prog {
	char *name;
	unsigned int init_time;
	unsigned int duration;
};

int main(void)
{
	char line[256];
	FILE *file = fopen("exec.txt", "r");

	while (!feof(file)) {
		fgets(line, 256, file);
		if (line[strlen(line)] != '\n')
			exit(1);
	}

	fclose(file);

	return 0;
}
