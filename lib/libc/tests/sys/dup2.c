/*
 * This tests whether or not a dup2() of STDOUT_FILENO in a child
 * process makes STDOUT_FILENO unusable in the parent process.  It should
 * not.
 *
 * Indirectly, it also tests screate(2), ssignal(2), and swait(2).
 *
 * Devin Reade, 1997
 *
 * $Id: dup2.c,v 1.1 1997/09/21 18:11:42 gdr Exp $
 */

#pragma optimize 72
#pragma debug 0

#include <sys/types.h>
#include <gno/gno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define TEST_MSG     "this is the test message\r"
#define PARENT_MSG_1 "doing fork\r"
#define PARENT_MSG_2 "parent waking up\r"
#define CHILD_MSG_1  "child sleeping\r"
#define CHILD_MSG_2  "child doing sema up\r"
#define CHILD_FAIL_1 "dup2() failed in child\r"

#if 1
#define LENGTH(str) (sizeof(str) -1)
#else
#include <string.h>
#define LENGTH(str) strlen(str)
#endif

int sema;

static void
_child (void) {
	write(STDOUT_FILENO, CHILD_MSG_1, LENGTH(CHILD_MSG_1));
	sleep(1);
	if (dup2(STDERR_FILENO, STDOUT_FILENO) == -1) {
		write(STDOUT_FILENO, CHILD_FAIL_1, LENGTH(CHILD_FAIL_1));
	} else {
		write(STDOUT_FILENO, CHILD_MSG_2, LENGTH(CHILD_MSG_2));
	}
	ssignal(sema);
	_exit(0);
}                                                             


int
main (int argc, char **argv) {

	if ((sema = screate(0)) == -1) {
		perror("couldn't create semaphore");
		exit(1);
	}

	write(STDOUT_FILENO, PARENT_MSG_1, LENGTH(PARENT_MSG_1));
	sleep(1);

	switch(fork(_child)) {
	case -1:
		perror("fork failed");
		exit(1);
	default:
		swait(sema);
		write(STDOUT_FILENO, PARENT_MSG_2, LENGTH(PARENT_MSG_2));
		write(STDOUT_FILENO, TEST_MSG, LENGTH(TEST_MSG));
	}
	return 0;	
}
