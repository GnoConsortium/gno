/*
 * Test by Devin Reade.
 *
 * $Id: scandir.c,v 1.1 1997/02/28 05:12:52 gdr Exp $
 */

#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>

#define FAIL(arg) { fprintf(stderr,"%s (errno==%d)\n", arg, errno); exit(1); }

int main (int argc, char **argv) {
	struct dirent **namelist;
	char *path;
	int count, i, loop;

	if (argc != 2) {
		printf("usage: %s dir_name\n", argv[0]);
		exit(1);
	}
	path = argv[1];

	for (loop = 0; loop < 2; loop++) {
		switch(loop) {
		case 0:
			printf("sorting %s\n", path);
			count = scandir(path, &namelist, NULL, alphasort);
			break;
		case 1:
			printf("sorting %s (case insensitive)\n", path);
			count = scandir(path, &namelist, NULL, alphacasesort);
			break;
		default:
			assert(0);
		}	
		if (count == -1) {
			FAIL("scandir failed");
		}
		for (i=0; i<count; i++) {
			printf("\t%s\n", namelist[i]->d_name);
		}
		printf("freeing pointers: ");
		for (i=0; i<count; i++) {
			printf(" %d", i);
			free(namelist[i]);
		}
     printf("\n");
		free(namelist);
	}
	return 0;
}
