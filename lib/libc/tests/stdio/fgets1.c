#pragma lint -1
#pragma debug 25

#include <stdio.h>

#define BUFFERSIZE 1024

char buffer[BUFFERSIZE];
int main(int argc, char **argv) {
	printf("hello, world\n");
	fprintf(stderr, "please enter a string: ");
	if (fgets(buffer, BUFFERSIZE, stdin) == NULL) {
		printf("fgets failed\n");
	} else {
		printf("input string was \"%s\"\n", buffer);
	}
	return 0;
}
