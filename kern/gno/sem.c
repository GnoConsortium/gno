/*	$Id: sem.c,v 1.1 1998/02/02 08:18:51 taubert Exp $ */

/* _sem.c - semaphore functions

SYSCALL swait() - make current process wait on a semaphore
SYSCALL ssignal() - signal a semaphore, releasing one waiting process
SYSCALL screate() - create and initialize a semaphore, returning its id
SYSCALL sdelete() - delete a semaphore by releasing its table entry

_seminit() - initialize semaphore system

newsem() - allocate an unused semaphore and return its index
*/

#pragma optimize 79
segment "KERN3     ";

#include "conf.h"
#if NSEM
#include "kernel.h"
#include "proc.h"
#include "sys.h"
#include "q.h"
#include "sem.h"
#include "gno.h"
#include "/lang/orca/libraries/orcacdefs/stdio.h"
#include "/lang/orca/libraries/orcacdefs/stdlib.h"
#include <sys/errno.h>

static int nextsem = NSEM-1;
extern kernelStructPtr kp;
extern void sleepbusy(void);

static int newsem()
{
int sem;
int i;

	for (i=0;i<NSEM;i++) {
		sem = nextsem--;
		if (nextsem < 0) nextsem = NSEM-1;
		if (_semaph[sem].sstate == SFREE) {
			_semaph[sem].sstate = SUSED;
                        _semaph[sem].squeue = Qalloc();
			return sem;
		}
	}
	return SYSERR;
}

/* initialize semaphores */
void _seminit()
{
int i;
struct sentry *sptr;

        initQ();
        if (!(_semaph = calloc(NSEM,sizeof(struct sentry))))
		PANIC("Could not allocate sempahore entries");
        for ( i=0; i < NSEM; i++ ) /* initialize semaphores */
	{
	    (sptr = &_semaph[i])->sstate = SFREE;
	    /* sptr->sqtail = 1 + ( sptr->sqhead = _newqueue() ); */
            /* don't allocate queues until we need them, but init them
               here */
	}
}

/* make current process wait on a semaphore */
SYSCALL commonSwait(int *ERRNO, int sem, int blockas, int waitdone)
{
struct sentry *sptr;
struct pentry *pptr;

	if (blockas != procBLOCKED)
		PANIC("commonSwait() only supports procBLOCKED now");

    disableps();
    if (isbadsem(sem) || (sptr = &_semaph[sem])->sstate == SFREE) {
	enableps();
	*ERRNO = EINVAL;
	return SYSERR;
    }
    if (--(sptr->semcnt) < 0) {
	PROC->processState = blockas;
	if (blockas == procBLOCKED) PROC->waitdone = waitdone;
	PROC->psem = sem;
	_enqueue(Kgetpid(),sptr->squeue);

        sleepbusy();

	if (blockas == procBLOCKED && PROC->waitdone == waitdone) goto gotit;
	if (sptr->sstate != SFREE) {
	    if (_dequeueitem(Kgetpid(),sptr->squeue) != SYSERR)
		PANIC("pentry still on queue in commonSwait()");
	} /* else sem was deallocated */

	enableps();
	*ERRNO = EINTR;
	return SYSERR;
    }
gotit:
    PROC->psem = 0;
    enableps();
    return OK;
}

/* Cleanup semaphore due to signal causing EINTR */
void semINTR(int sem, int mpid)
{
struct sentry *sptr;

	if (isbadsem(sem) || (sptr = &_semaph[sem])->sstate == SFREE)
		return;
	_dequeueitem(mpid, sptr->squeue);
	sptr->semcnt++;
	PROC->psem = -1;
}

/* create and initialize a semaphore, returning its id */
/* initial count (>=0) */
SYSCALL Kscreate(int *ERRNO, int count)
{
int sem;

    disableps();
    if (count < 0 || (sem=newsem()) == SYSERR) {
	enableps();
	*ERRNO = ENOMEM;
	return SYSERR;
    }
    _semaph[sem].semcnt = count;
    enableps();
    return sem;
}

SYSCALL Kscount(int *ERRNO, int sem)
{
struct sentry *sptr;
int c;

    disableps();
    if (isbadsem(sem) || (sptr = &_semaph[sem])->sstate == SFREE) {
	enableps();
	*ERRNO = EINVAL;
	return SYSERR;
    }
    c = sptr->semcnt;
    enableps();
    return c;	/* BAH!  This could be == SYSERR */
}

#pragma databank 1
#pragma toolparms 1

SYSCALL KERNswait(int *ERRNO,int sem)
{
	return commonSwait(ERRNO,sem,procBLOCKED,BLOCKED_SWAIT);
}

SYSCALL KERNscreate(int *ERRNO, int count)
{
	return Kscreate(ERRNO, count);
}

SYSCALL KERNscount(int *ERRNO, int sem)
{
	return Kscount(ERRNO, sem);
}

/* signal a semaphore, releasing one waiting process */
SYSCALL KERNssignal(int *ERRNO, int sem)
{
struct sentry *sptr;
int mpid;

    disableps();
    if (isbadsem(sem) || (sptr = &_semaph[sem])->sstate == SFREE) {
	enableps();
	*ERRNO = EINVAL;
	return SYSERR;
    }
    if ((sptr->semcnt++) < 0) {
	if ((mpid = _getfirst(sptr->squeue)) == SYSERR)
		PANIC("ssignal: _getfirst FAILED!");

	/* _ready(kp->procTable[mpid].flpid,RESCHNO); */
	if (kp->procTable[mpid].processState != procUNUSED)
		kp->procTable[mpid].processState = procREADY;

    }
    enableps();
    return OK;
}

/* delete a semaphore by releasing its table entry */
SYSCALL KERNsdelete(int *ERRNO, int sem)
{
int mpid;
struct sentry *sptr;	/* address of sem to free */

    disableps();
    if (isbadsem(sem) || (sptr = &_semaph[sem])->sstate == SFREE) {
	enableps();
	*ERRNO = EINVAL;
        return SYSERR;
    }
    sptr->sstate = SFREE;
    if (nonempty(sptr->squeue)) {
	while ((mpid = _getfirst(sptr->squeue)) != SYSERR) {
		/* _ready(kp->procTable[mpid].flpid,RESCHNO); */
		if (kp->procTable[mpid].processState != procUNUSED)
			kp->procTable[mpid].processState = procREADY;
	}
        Qdispose(sptr->squeue);
        enableps();
        _resched();
        return OK;
    }
    Qdispose(sptr->squeue);
    enableps();
    return OK;
}

#pragma databank 0
#pragma toolparms 0
#endif

