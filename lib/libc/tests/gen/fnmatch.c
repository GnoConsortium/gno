/*
 * Test written by Devin Reade
 *
 * This one doesn't test the flags parameter yet.
 *
 * $Id: fnmatch.c,v 1.1 1997/02/28 05:12:52 gdr Exp $
 */

#include <fnmatch.h>
#include <stdlib.h>
#include <stdio.h>

int
main (int argc, char **argv) {
	int result;

	if (argc != 3) {
		printf("usage: %s pattern filename\n", argv[0]);
		exit(1);
	}

	result = fnmatch (argv[1], argv[2], FNM_CASEFOLD);
	printf("result is %d\n", result);
	return 0;
}
