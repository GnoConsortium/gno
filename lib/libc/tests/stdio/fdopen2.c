/*
 * This test attaches a FILE stream to each of the descriptors STDIN_FILENO
 * and STDOUT_FILENO, then reads from the former and writes to the latter
 * in binary mode until EOF is reached.
 *
 * Devin Reade, 1997.
 *
 * $Id: fdopen2.c,v 1.2 1997/09/21 16:36:18 gdr Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFERSIZE 1024

char buffer[BUFFERSIZE];

int
main(int argc, char **argv) {
	FILE *fp1, *fp2;
	int fd1 = STDIN_FILENO;
	int fd2 = STDOUT_FILENO;

	fprintf(stderr,"starting\n");
	if ((fp1 = fdopen(fd1, "rb")) == NULL) {
		perror("failed to do fdopen on stdin");
		exit(1);
	}
	if ((fp2 = fdopen(fd2, "wb")) == NULL) {
		perror("failed to do fdopen on stdout");
		exit(1);
	}
	while (fgets(buffer, BUFFERSIZE, fp1) != NULL) {
		if (fputs(buffer, fp2) == EOF) {
			perror("fputs failed for stdout");
			exit(1);
		}
	}
	if (ferror(fp1)) {
		perror("error while reading from stdin");
		exit(1);
	}
	fclose(fp1);
	fclose(fp2);
	fprintf(stderr,"done\n");
	return 0;
}        
