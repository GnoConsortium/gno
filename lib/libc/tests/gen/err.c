/*
 * Tests for gen/err.c
 * Written by Devin Reade
 *
 * $Id: err.c,v 1.1 1997/02/28 05:12:52 gdr Exp $
 */

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

void
custom_exit(int val) {
	printf("in custom_exit with val %d\n", val);
	exit(val);
}

int main(int argc, char **argv) {
        int i;

	if (argc != 2) {
		printf("usage!\n");
		exit(1);
	}

	printf("testing err_set_file\n");
	err_set_file(stdout);
	printf("testing err_set_exit\n");
	err_set_exit(custom_exit);

	errno = ENOMEM;

	printf("testing warn and vwarn\n");
	warn("\ttest of %s", "warn");
	printf("testing warnx and vwarnx\n");
	warnx("\ttest of warnx");

	i = atoi(argv[1]);
	switch (i) {
	case 0:
		printf("testing err and verr\n");
		err(1, "\ttest of %s", "err");
		break;

	case 1:
		printf("testing errx and verrx\n");
		errx(2, "\ttest of %s", "verrx"); /* bogus */
		break;

	default:
		printf("unknown case: %d\n", i);
		exit(1);
	}
	return 0;
}
