/*
 * This program exhibits a problem with the wait(2) system call;
 * If you do a wait and have no children, you will hang in the wait.
 * The man page says that -1 should be returned and errno set to
 * ECHILD.
 *
 * $Id: wait.c,v 1.1 1997/07/27 23:36:25 gdr Exp $
 */

#include <stdio.h>
#include <sys/wait.h>

int main (int argc, char **argv) {
	union wait wstat;
	int retval;

	printf("starting wait\n");
	retval = wait(&wstat);
	printf("wait returned %d\n", retval);
	return 0;
}
