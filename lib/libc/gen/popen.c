/*
 * This is a complete rewrite of the popen/pclose routines.
 * Devin Reade, 1997.
 *
 * $Id: popen.c,v 1.2 1997/09/21 06:04:01 gdr Exp $
 *
 * This file is formatted for tab stops every 8 columns.
 */

#ifdef __ORCAC__
segment "libc_gen__";
#endif

#include <sys/param.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <paths.h>
#include <sys/queue.h>
#ifdef __GNO__
#include <gno/gno.h>
#endif

#include <assert.h>	/* shouldn't be necessary in final code */

/*
 * This #define makes us create a temporary file containing the
 * commands to execute rather than giving them on the gsh command
 * line.  Gsh has a problem with accepting command line arguments ...
 */
#define USE_SCRIPT 1

/*
 * We maintain state information by having a linked list of these
 * structs.
 */

static SLIST_HEAD(popenStateHead, popenEntry_t) popenHead;
static struct popenStateHead *popenHeadPtr = NULL;

typedef struct popenEntry_t {
	SLIST_ENTRY(popenEntry_t) pr_link;	/* link */
        FILE *pr_fp;				/* the FILE pointer */
        pid_t pr_pid;				/* pid of the child */
#ifdef USE_SCRIPT
        char *pr_name;				/* name of the "script" file */
#endif
} popenEntry_t;

#ifdef USE_SCRIPT

/*
 * _popen_script -- create a temporary file of type EXEC that contains
 *		    the set of commands that we will execute.  This
 *		    is a Real Kludge to get around a problem with gsh
 *		    ignoring it's command line flags (-c in particular).
 */
static char *
_popen_script (const char *command) {
	/* the number of chars in prefix comes from "popen<pid>":
         *	"popen"	5
         *	<pid>	up to 6	(for a 16-bit pid)
         *	null	1
         */
	char prefix[12];
	char *result;
        FILE *fp;
	int i;
#ifdef __GNO__
        int oldEmulate;
#endif

        sprintf(prefix, "popen%u", getpid());
        if ((result = tempnam(NULL, prefix)) == NULL) {
	        return NULL;
        }
        if ((fp = fopen(result, "w")) == NULL) {
	        free(result);
                return NULL;
        }
        fprintf(fp, "%s\n", command);
        fclose(fp);

        /* change file/aux type to SRC/EXEC */
#ifdef __GNO__
        oldEmulate = _setModeEmulation (1);
#endif
        i = chmod(result, S_IRWXU);
#ifdef __GNO__
        _setModeEmulation(oldEmulate);
#endif
        if (i == -1) {
	        i = errno;
                free(result);
	        errno = i;
                result = NULL;
        }
	return result;
}

#endif	/* USE_SCRIPT */

#pragma optimize 78	/* bits 3 and 6, minimum */
#pragma debug 0
                  
/* change this when we have an sh(1) for GNO */
#ifdef __GNO__
#define SHELL_PATH _PATH_GSHELL
#define SHELL_NAME "gsh"
#else
#define SHELL_PATH _PATH_BSHELL
#define SHELL_NAME "sh"
#endif

#ifdef USE_SCRIPT
#define SH_CFLAG
#else
#define SH_CFLAG "-c",
#error this is broken
#endif

#ifdef __GNO__
#pragma databank 1
static void 
_popen_child(int *pdes, char *type, char *command)
{
	char buffer[128];
	strcpy(buffer, "/bin/gsh ");
	strcat(buffer, command);

	if (*type == 'r') {
		if (pdes[1] != STDOUT_FILENO) {
			if (dup2(pdes[1], STDOUT_FILENO) == -1) {
				_exit(126);
			}
			close(pdes[1]);
		}
		close(pdes[0]);              
	} else {          
		if (pdes[0] != STDIN_FILENO) {
			if (dup2(pdes[0], STDIN_FILENO) == -1 ) {
				_exit(126);
			}
			close(pdes[0]);
		}
		close(pdes[1]);
	}
#if 0
	execl(SHELL_PATH, SHELL_NAME, SH_CFLAG command, (char *) 0);
#else
	_execve("/bin/gsh", buffer);
#endif
	{
		char *p;
		p = strerror(errno);
		write(STDERR_FILENO, p, strlen(p));
		write(STDERR_FILENO, "\r", 1);
	}
	_exit(127);                          
}
#pragma databank 0

#endif /* __GNO__ */

FILE *
popen(const char *command, const char *type)
{
	static int pdes[2];
	popenEntry_t *entry = NULL;
	int err;

	/* initialize the status pointer if necessary */
	if (popenHeadPtr == NULL) {
		popenHeadPtr = &popenHead;
		SLIST_INIT(popenHeadPtr);
	}
	
	/* verify legality of access mode */
	if ((*type != 'r' && *type != 'w') || type[1]) {
		errno = EINVAL;
		goto fail;
	}

	/* get a new status element */
	if ((entry = malloc(sizeof(popenEntry_t))) == NULL) {
		goto fail;
	}
	entry->pr_fp = NULL;
	entry->pr_pid = 0;
#ifdef USE_SCRIPT
	entry->pr_name = NULL;
#endif

	/* create the pipes */
	if (pipe(pdes) < 0) {
		goto fail;
	}

#ifdef USE_SCRIPT
	if ((command = _popen_script(command)) == NULL) {
		goto fail;
	}
	entry->pr_name = (char *) command;
#endif

	
#ifdef __GNO__
	entry->pr_pid = fork2(_popen_child, 4096, 0, 
			      "forked child of popen", 6, pdes, type,
			      command);
	switch (entry->pr_pid) {
#else
	switch (entry->pr_pid = vfork()) {
#endif
	case -1:                    /* error */
		err = errno;
		close(pdes[0]);
		close(pdes[1]);
		errno = err;
		goto fail;
		/* NOTREACHED */

#ifndef __GNO__
	case 0:                     /* child */
		if (*type == 'r') {
			if (pdes[1] != STDOUT_FILENO) {
				if (dup2(pdes[1], STDOUT_FILENO) == -1) {
					_exit(126);
				}
				close(pdes[1]);
			}
			close(pdes[0]);
		} else {
			if (pdes[0] != STDIN_FILENO) {
				if (dup2(pdes[0], STDIN_FILENO) == -1 ) {
					_exit(126);
				}
				close(pdes[0]);
			}
			close(pdes[1]);
		}
		execl(SHELL_PATH, SHELL_NAME, SH_CFLAG command, (char *) 0);
		_exit(127);
		/* NOTREACHED */
#endif /* ! __GNO__ */
	}

	/* parent */
	if (*type == 'r') {
		if ((entry->pr_fp = fdopen(pdes[0], type)) == NULL) {
	                close(pdes[0]);
                }
		close(pdes[1]);
	} else {
		if ((entry->pr_fp = fdopen(pdes[1], type)) == NULL) {
	                close(pdes[1]);
                }
		close(pdes[0]);
	}

        if (entry->pr_fp != NULL) {
		SLIST_INSERT_HEAD(popenHeadPtr, entry, pr_link);
		return entry->pr_fp;
	}
	/*FALLTHROUGH*/

    fail:
	/* 
	 * OK, so goto's are passe' -- but it reduces the size of the 
	 * generated code.
	 */
	if (entry) {
		err = errno;
		if (entry->pr_fp == NULL) {
			if (entry->pr_pid > 0) {
				kill(entry->pr_pid, SIGKILL);
			}
		}
		if (entry->pr_name) {
			unlink(entry->pr_name);
			free(entry->pr_name);
		}
		free(entry);
		errno = err;
	}
	return NULL;
}

#ifdef __GNO__
#define WAITSTAT_TYPE union wait *
#else
#define WAITSTAT_TYPE int *
#endif

/*
 * pclose returns -1 if stream is not associated with a
 * `popened' command, if already `pclosed', or waitpid
 * returns an error.
 */

int 
pclose(FILE * iop)
{
	union wait pstat;
	pid_t pid;
	popenEntry_t *np;

        if (iop == NULL || popenHeadPtr == NULL) {
        	errno = EINVAL;
                return -1;
        }

	for (np = popenHeadPtr->slh_first; np != NULL;
	     np = np->pr_link.sle_next) {
		if (np->pr_fp == iop) {
			break;
		}
	}
	if (np == NULL) {
		/* iop not opened by popen(3) */
		errno = EINVAL;
		return -1;
	}
	assert(np->pr_name != NULL);
	assert(np->pr_name[0] != '\0');
	SLIST_REMOVE(popenHeadPtr, np, popenEntry_t, pr_link);

	/* Close it, and wait for the child to exit. */
	fclose(iop);
#if 1
	pid = wait((WAITSTAT_TYPE) &pstat);
#else
	do {
		pid = waitpid(np->pr_pid, (WAITSTAT_TYPE) &pstat, 0);
	} while (pid == -1 && errno == EINTR);
#endif

	/* clean up the structures */
#ifdef USE_SCRIPT
	unlink(np->pr_name);
	free(np->pr_name);
#endif
	free(np);
	return (pid == -1 ? -1 : pstat.w_retcode);
}
