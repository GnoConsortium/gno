/*
 * echoparams: echo the arguments passed as parameters
 *
 * Written by Dave Tribby to test gsh  *  June 1998
 * $Id: echoparams.c,v 1.1 1999/11/30 20:28:24 tribby Exp $
 */

#include <stdio.h>
#include <orca.h>

/* Interface to subroutine that re-parses parameters */
extern int reparse(char *argv[], char *commandline);

int main(int argc, char *argv[])
{
	int i;
        int argc1;
        char **argv1;

	printf("Command line: %s\n", commandline());
	for (i = 0; i < argc; i++)
		printf(" parameter %2d: '%s'\n", i,*argv++);

	printf("Reparsing...\n");
        argc1 = reparse(&argv1, commandline());
	for (i = 0; i < argc1; i++)
		printf(" parameter %2d: '%s'\n", i,*argv1++);

	return 0;                              
}
