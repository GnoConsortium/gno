/*
 * $Id: fdopen2.c,v 1.1 1997/09/05 06:46:30 gdr Exp $
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
#ifdef FROM_FILE
	if ((fp1 = fopen("data1", "rb")) == NULL) {
#else
#if 0
	fd1 = dup(STDIN_FILENO);
	fd2 = dup(STDOUT_FILENO);
#endif
	if ((fd1 < 0) || (fd2 < 0)) {
		perror("dup failed");
		exit(1);
	}
	if ((fp1 = fdopen(fd1, "rb")) == NULL) {
#endif
		perror("failed to do fdopen on stdin");
		exit(1);
	}
#ifdef FROM_FILE
	if ((fp2 = fopen("data5", "wb")) == NULL) {
#else
	if ((fp2 = fdopen(fd2, "wb")) == NULL) {
#endif
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
