/*
 * Test for exec* routines written by Devin Reade
 *
 * $Id: exectest.c,v 1.1 1997/02/28 05:12:57 gdr Exp $
 *
 * This file is formatted for tab stops every 8 characters.
 */

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>

#pragma debug 0

char *argarray[4];
char *envarray[2];                        /* for execve test */
char *envstring="bork=thingamadoo";
char *filestr1="now2";
   
int
main (int argc, char **argv) {
	int i;
	
	if (argc!=5) {
		printf("usage: exectest case prog opt1 opt2\n");
		printf("\tcase is one of:\n");
		printf("\t\t0 - execl\n");
		printf("\t\t1 - execlp\n");
		printf("\t\t2 - execve\n");
		printf("\t\t3 - execv\n");
		printf("\t\t4 - execvp\n");
		exit (-1);
	}

	i = atoi(argv[1]);
	switch (i) {
	case 0:
		execl(argv[2], argv[2], argv[3], argv[4], (char *) NULL);
		perror("execl() failed");
		break;
	case 1:
		execlp(argv[2], argv[2], argv[3], argv[4], (char *) NULL);
		perror("execlp() failed");
		break;
	case 2:
		envarray[0]=envstring;
		envarray[1]=NULL;

		argarray[0]=argv[2];
		argarray[1]=argv[3];
		argarray[2]=argv[4];
		argarray[3]=NULL;
		execve(argarray[0], argarray, envarray);
		perror("execve failed");
		break;
	case 3:
		argarray[0]=argv[2];
		argarray[1]=argv[3];
		argarray[2]=argv[4];
		argarray[3]=NULL;

		execv(argarray[0], argarray);
		perror("execv failed");
		break;
	case 4:
		argarray[0]=argv[2];
		argarray[1]=argv[3];
		argarray[2]=argv[4];
		argarray[3]=NULL;

		execvp(argarray[0], argarray);
		perror("execvp failed");
		break;
	default:
		printf("bad case value: %d\n", i);
	}
	return -1;
}
