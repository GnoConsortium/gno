#include <stdio.h>
#include <unistd.h>

#define  DURATION 3

int main (int argc, char **argv) {
	printf("sleeping for %d seconds\n", DURATION);
	sleep(DURATION);
	printf("now awake, starting pause\n");
	pause();
	return 0;
}
