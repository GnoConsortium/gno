/*	$Id: data.c,v 1.1 1998/02/02 08:18:22 taubert Exp $ */

/* data.c - nulluser, sysinit
     global data structures
 */
#pragma optimize 79

#define INITSTK 2000 /* stack size of initial process */
#define INITPRIO 20	/* priority of initial process */
#define INITNAME "user_main" /* name of initial process */

#include "conf.h"
#include "kernel.h"
#include "proc.h"
#include "q.h"

#if defined(NSEM)
#include "sem.h"
#endif /* defined(NSEM) */

/* declarations of major kernel variables */
/* Not needed- we store our processTable elsewhere */
/* struct pentry _proctab[NPROC];*/ /* process table */

#if defined(NSEM)
struct sentry *_semaph;
/*struct sentry _semaph[NSEM]; /* semaphore table */
int _nextsem; /* next semaphore slot to use in screate */
#endif /* defined(NSEM) */

/*struct qstruct nq[NQS];*/
struct qstruct *nq;
struct qstruct *q_free;

#ifdef NOTDEFINED
/* active system status */
int _numproc; /* number of live user processes */
int _currpid; /* id of currently running process */

/* real-time clock variables and sleeping process queue pointers */
int _slnempty; /* FALSE if the sleep queue is empty */
int *_sltop; /* address of key part of top entry in */

/* the sleep queue of sleeping processes */
int _clockq; /* head of queue of sleeping processes */

int _rdyhead; /* head of ready list (q indices) ... */
int _rdytail; /* ... and the tail */
#endif

