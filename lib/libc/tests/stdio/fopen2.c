/*
 * This program exhibits a problem when opening a file for reading in
 * binary mode.
 *
 * $Id: fopen2.c,v 1.1 1997/09/05 06:46:31 gdr Exp $
 */

#pragma keep "fopen2"
#pragma debug 25
#pragma lint  -1

#include <stdio.h>

#define BUFFERSIZE 256
#undef  DO_READ

int main (int argc, char **argv) {
	FILE *fp;
#ifdef DO_READ
	static char buffer[BUFFERSIZE];
        int i;
#endif

	fp = fopen(*argv, "rb");
	if (fp == NULL) {
		perror("open failed");
		return 1;
	}

#ifdef DO_READ
	i = fread(buffer, 1, BUFFERSIZE, fp);
	printf("fread returned %d\n", i);
#endif

	if (fclose(fp) != 0) {
		perror("fclose failed");
		return 1;
	}
	printf("done\n");
	return 0;
}
