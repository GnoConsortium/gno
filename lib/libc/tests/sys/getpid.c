#include <stdio.h>
#include <unistd.h>

int
main(int argc, char **argv) {
	pid_t pid;

	pid = getpid();
	printf("pid is %d\n", pid);
	return 0;
}
