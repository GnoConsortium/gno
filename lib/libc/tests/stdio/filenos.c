#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv) {

	printf("stdin fileno expected %d got %d\n", STDIN_FILENO, fileno(stdin));
	printf("stdout fileno expected %d got %d\n", STDOUT_FILENO, fileno(stdout));
	printf("stderr fileno expected %d got %d\n", STDERR_FILENO, fileno(stderr));
	return 0;
}
