/*
 * $Id: fopen.c,v 1.1 1997/09/05 06:46:31 gdr Exp $
 */
#include <stdlib.h>
#include <stdio.h>

int main (int argc, char **argv) {
	FILE *fp;
        int i, j;

	fp = fopen("/tmp/testfile", "w+");
	if (fp == NULL) {
		perror("open of /tmp/testfile failed");
		exit(1);
	}
	i = fprintf(fp, "now is the time\n");
	j = fclose(fp);
	printf("fprintf returned %d\nfclose returned %d\n", i, j);
	exit(0);
}
