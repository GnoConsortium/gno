/*
 * callsystem.c: see what happens when system() is called
 *
 * Written by Dave Tribby to test gsh  *  June 1998
 * $Id: callsystem.c,v 1.1 1999/11/30 20:28:24 tribby Exp $
 */

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	int i, rtnval;
	printf("Test of system() function\n");
	*argv++;
        if (argc == 1)   {
		rtnval = system((char *)0L);
		printf(" Null parameter returns value of %d\n", rtnval);
	}
	else for (i = 1; i < argc; i++) {
		rtnval = system(*argv);
		printf(" `%s`  returns value = %d\n", *argv,rtnval);
		*argv++;
		}
	return rtnval;
}
