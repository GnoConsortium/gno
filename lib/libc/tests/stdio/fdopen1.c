/*
 * $Id: fdopen1.c,v 1.1 1997/09/05 06:46:30 gdr Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main (int argc, char **argv) {
	int fd, i;
	FILE *fp;

	if ((fd = open(*argv, O_RDONLY)) == -1) {
		perror("open");
		exit(1);
	}
	printf("open succeeded\n");
	if ((fp = fdopen(fd, "r")) == NULL) {
		perror("fdopen");
		exit(1);
	}
	printf("fopen succeeded\n");
#if 0
	i = fclose(fp);
	printf("fclose returned %d\n", i);
	i = close(fd);
	printf("close returned %d\n", i);
#endif
	exit(0);                   
}
