#include <sys/types.h>
#include <sys/wait.h>
#include <gno/gno.h>
#include <stdarg.h>
#include <syslog.h>
#include <unistd.h>

#define MAX_PROC 10
#define LOOP_COUNT 5

#pragma lint -1
#pragma optimize 78
#pragma debug 0

int lock;

void
child_process (void) {
	int i;
	pid_t pid;
	
	/* wait for everyone else */
	pid = getpid();
	swait(lock);
	for (i=0; i<LOOP_COUNT; i++) {
		syslogmt(LOG_NOTICE, "child %d loop %d of %d",
			 pid, i, LOOP_COUNT);
	}
	_exit(0);
}

int
main (int argc, char **argv) {
	int i;
	union wait status;
	pid_t pid;

	openlog("syslogmt-test", LOG_NDELAY|LOG_PID, LOG_DAEMON);
	syslogmt(LOG_NOTICE, "creating semaphore lock");
	
	if ((lock = screate(1)) == -1) {
		syslogmt(LOG_NOTICE, "screate failed: %m");
		exit(1);
	}

	/* create the children */
	for (i=0; i<MAX_PROC; i++) {
		syslogmt(LOG_NOTICE, "forking child %d", i);
		fork(child_process);
	}

	/* release the children */
	syslogmt(LOG_NOTICE, "doing a raise of %d", MAX_PROC);
	for (i=0; i<MAX_PROC; i++) {
		ssignal(lock);
	}

	/* wait for the children */
	while((pid = wait(&status)) != -1) {
		syslogmt(LOG_NOTICE, "parent: wait returned for child %d",
			 pid);
	}
	syslogmt(LOG_NOTICE, "parent: done, closing log");
	closelog();
	return 0;
}
