#include <signal.h>

int main(void)
{
	unsigned long int i, j, max, ios;
	max = 1000000000;
	ios = 3;

	for (j = 0; j < ios; ++j) {
		for (i = 0; i < max; ++i);
		raise(SIGSTOP);
	}

	return 0;
}

