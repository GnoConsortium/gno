/*	$Id: signal.c,v 1.2 1998/02/22 05:05:49 taubert Exp $ */

/*  Signals work like this.
  The following paragraphs only describe calling a handler routine, as
  ignore and default are trivial operations in all situations. kill(x,9)
  supercedes all this and just blasts the process to hell.
  Mutex should of course be in effect during any of this.

  1) A process sends a signal to itself
     Set the sigmasks appropriately, then pushes the context on the stack
     and jsl's the handler routine.  The handler RTLs to the context restore
     routine, and another rtl occurs to resume execution right after the sig
     call.
  2) A process sends a signal to another process
     a) if there are signals pending, just set the bits and what-not and 
        return (the context restore routine will run the next handler if
        necessary)
     b) get the process context from the process table, and build stuff
        on the process' stack, so that the next time the proc is executed,
        it will be executing the signal handler.
     c) in either case, the restore context routine checks to see whether
        there are any more signals pending, and if so, sets it up immediately.

  The only difference between 1 and 2 is that 1 gets the 'current context'
  by shoving registers on the stack, and 2 gets it from the target process'

  Note that the signal code need not worry about the various machine states
  (shadow, statereg, etc) because those would only be modified by routines
  in emulation mode, which should by all accounts be mutexed.
*/
#pragma optimize 79
segment "KERN2     ";

#include "conf.h"
#include "kernel.h"
#include "proc.h"
#include "sys.h"
#include "gno.h"
#include "/lang/orca/libraries/orcacdefs/stdio.h"
#include "/lang/orca/libraries/orcacdefs/string.h"
#include "/lang/orca/libraries/orcacdefs/stdlib.h"
#include <memory.h>
#include <loader.h>
#include <gsos.h>
#include <misctool.h>
#include <sys/errno.h>
#include <sys/wait.h>

extern kernelStructPtr kp;
extern void selwakeup(int col_flag, int pid);
extern void selwait(void);

enqueueWait(int targetpid, int pid, union wait status)
{
chldInfoPtr walk;
chldInfoPtr nwait;

/*  printf("enqueueWait: tpid %d pid %d status %04X\n",targetpid,pid,status);
*/
    if (targetpid == 0) return; /* don't queue up for Null Process */
    nwait = malloc(sizeof(chldInfo));
    nwait->next = NULL;
    nwait->pid = pid;
    nwait->status = status;

    walk = kp->procTable[targetpid].waitq;
    if (walk == NULL) { kp->procTable[targetpid].waitq = nwait; return; }
    while (walk->next != NULL) walk = walk->next;
    walk->next = nwait;
}

word numInWaitQueue(int pid)
{
chldInfoPtr w;
word x;

    w = kp->procTable[pid].waitq;
    while (w != NULL) {
    	w = w->next;
	x++;
    }
    return x;
}

int dequeueWait(chldInfoPtr status, int pid)
{
chldInfoPtr item;

  /*printf("dequeueWait: %06lX, pid %d ",kp->procTable[pid].waitq,
            pid);   */
    if ((item = kp->procTable[pid].waitq) == NULL)  return -1;
    if (status != NULL)
        memcpy(status,item,sizeof(chldInfo));
    kp->procTable[pid].waitq = item->next;
 /* printf("status: %04X, tpid: %d\n",item->status,item->pid);  */
    nfree(item);
    return 0;
}

#pragma databank 1
#pragma toolparms 1

extern void addsig(int, int);

/*
 * This MUST be in a function by itself so the kp-> dereference will have
 * an allocated DP to use.
 */
static void proc_free(struct pentry *tosig)
{
	if (tosig->flags & FL_FORKED) {
		DisposeAll(tosig->userID); /* process was fork()ed, don't USD */
		DeleteID(tosig->userID);
	} else {
		UserShutDown(tosig->userID,
		    ((tosig->flags & FL_RESTART) &&
		    !(tosig->flags & FL_NORESTART)) ? 0x4000 : 0);
	}

        kp->numProcs--;
        if (kp->numProcs == 1) kp->shutdown = 1;
}

int KERNkill(int *ERRNO, int signum, int pid)
{
struct pentry *tosig;
struct sigrec *sig;
int i,j,mpid;
longword wait;
word ClosePB[2];
union wait status;
extern int OLDGSOSST(word callnum, void *pBlock);
extern void ctxtRestore(void);
extern void enableBuf(void);
extern pgrp pgrpInfo[];
extern void disposevar(int);

    if (kp->gsosDebug & 8)
    	kern_printf("kill (-%d):pid %d\n\r",signum,pid);
/* $$$ if (pid == 0) pid = -(kp->procTable[Kgetpid()].pgrp); */
    if (pid == 0) pid = -(PROC->pgrp);
    if (pid < 0) {
    	pid = 0-pid;
    	for (i = 0; i < NPROC; i++) {
            if ((kp->procTable[i].processState) &&
                (kp->procTable[i].pgrp == pid))
            	addsig(i,signum);
	}
	_resched(); /* allow signals to be processed before we go on */
	return 0;
    }
    mpid = mapPID(pid);
    if (mpid == 0) return -1; /* can't signal the kernel null process */
    if (mpid == -1) { *ERRNO = ESRCH; return -1; }
    if (!signum) return 0;
    if ((signum < 1) || (signum > 32)) { *ERRNO = EINVAL; return -1; }
    tosig = &(kp->procTable[mpid]);
    sig = tosig->siginfo;
    disableps();
    if (sig->signalmask & sigmask(signum)) {
	sig->sigpending |= sigmask(signum);
	enableps();
	return 0;
    }

    /* if we were BLOCKED, then restart the operation */
    if (tosig->processState == procBLOCKED) {
#if 1
        /* if (sig interrupt bit is set) */
	if ((sig->v_signal[signum] != SIG_DFL) &&
            (sig->v_signal[signum] != SIG_IGN))
	{ tosig->waitdone = -1; tosig->processState = procREADY; }
#else
	{ tosig->waitdone = 1; tosig->processState = procREADY; }
#endif

	/* Semaphore cleanup for preceive(), swait() */
	if (tosig->psem && tosig->psem != -1)
		semINTR(tosig->psem, mpid);
    }

    /* if we were waiting for a signal, restart the process after we
       execute any signal handler (including killing the puppy if necessary) */
    if (tosig->processState == procPAUSED)
	tosig->processState = procREADY;
    else if (tosig->processState == procWAITSIGCH) {
	if (signum == SIGCHLD) tosig->waitdone = 1;
	else {
            if (sig->v_signal[signum] != SIG_DFL)
        	tosig->waitdone = -1;
	}
	tosig->processState = procREADY; /* restart the process, bloke! */
    }

    /* ignore the signal? */
    if (sig->v_signal[signum] == SIG_IGN) { enableps(); return 0; }
    else if (sig->v_signal[signum] == SIG_DFL) {   /* default actions */
	switch (signum) {
            case SIGCONT:
            	if (tosig->processState != procSUSPENDED)
        	  { enableps(); return 0; }
		if ((tosig->stoppedState == procRUNNING) || 
                    (tosig->stoppedState == procBLOCKED))
                    tosig->processState = procREADY;
        	else tosig->processState = tosig->stoppedState;
	   /*     if (tosig->stoppedState == procBLOCKED)
        	  tosig->irq_A = 0xFF; *//* restart I/O operation */

        	enableps();
        	return 0;
            case SIGURG:
            case SIGCHLD: enableps(); return 0;
            case SIGSTOP:
            case SIGTSTP:
            case SIGTTIN:
            case SIGTTOU:
 /* if the process is already suspended, ignore these signals */

        	if (tosig->processState != procSUSPENDED) {
                    status.w_stopsig = signum;
                    status.w_stopval = WSTOPPED;
                    enqueueWait(tosig->parentpid,pid,status);
   /* this seems to be a definite no-no; the parent will restart on receipt
   of the SIGCHLD */
   /*  kp->procTable[tosig->parentpid].processState = procREADY; */
                    tosig->stoppedState = tosig->processState;
                    tosig->processState = procSUSPENDED;
                    addsig(tosig->parentpid,SIGCHLD);
        	}
        	enableps();
        	if (mpid == Kgetpid()) _resched();
        	return 0;
	}

/* this is the old 'kill' code
   This portion is repsponsible for terminating processes
*/

/*printf("kill (-%d):pid %d (userID %04X)",signum,pid,tosig->userID);*/

#if 0
	if (kp->gsosDebug & 8) {
	int i;
		for (i=0;i<sizeof(struct pentry);i++)
			fprintf(stderr, "%02x ", ((unsigned char *)tosig)[i]);
		fprintf(stderr, "\n");
	}
#endif

	if (tosig->parentpid != 0) {
            if (tosig->flags & FL_NORMTERM) {
		status.w_termsig = 0;
		status.w_coredump = 0;
		status.w_retcode = tosig->exitCode;
            } else {
		status.w_termsig = signum;
		status.w_coredump = 0;
		status.w_retcode = 0;
            }
            enqueueWait(tosig->parentpid,pid,status);
	    addsig(tosig->parentpid,SIGCHLD);
           /* update children time accounting stuff for times() */
	    if (tosig->parentpid)
		kp->procTable[tosig->parentpid].childTicks +=
                    tosig->ticks + tosig->childTicks;
       	}
       	if (!(tosig->flags & FL_COMPLIANT)) {
            enableBuf();
       	}
        /* If the process was in a sleep queue, remove it - do not ready */
        if (tosig->p_waitvec) k_remove(tosig->p_waitvec, mpid, 0);

        if (tosig->flags & FL_QDSTARTUP) *((byte *)0xE0C029l) &= 0x7F;

   /* if the process has active children, have INIT (pid 0) inherit them */
   /*   printf("reassigning active children\n"); */
        for (i = 1; i < NPROC; i++) {
          if (kp->procTable[i].processState != procUNUSED)
            if (kp->procTable[i].parentpid == mpid)
       		kp->procTable[i].parentpid = 0;
       	}
       	disposevar(mpid);
        /*printf("dequeueing all wait info\n"); */
       	while (dequeueWait(NULL,mpid) != -1); /* unallocate all the wait info */
       	
        /*printf("freeing prefix recs\n");*/
       	for (i = 0; i < 33; i++)
            if (tosig->prefix[i] != NULL)
                nfree(tosig->prefix[i]);
       	
        /*printf("freeing prefix rec\n");*/
       	nfree(tosig->prefix);
       	/*printf("freeing args\n");*/
       	if (tosig->args) nfree(tosig->args);
	   /*printf("freeing sig\n");*/
       	nfree(sig);	/* dealloc the signal record */
       	/*printf("deallocating pgrp\n"); */
       	if (tosig->pgrp != 0) pgrpInfo[tosig->pgrp-2].pgrpref--;
       	tosig->alarmCount = 0l;
       
       	switch (tosig->processState) {
           case procRUNNING: /* suicide */
               tosig->openFiles->fdLevel =
                 tosig->openFiles->fdLevelMode = 0;
               ClosePB[0] = 1; ClosePB[1] = 0;
               CloseGS(ClosePB);
               nfree(tosig->openFiles); /* since we alloc'ed this- fix this later */
               tosig->processState = procUNUSED;

		/*
		 * Switch to the nullproc's stack so we won't be running on a
		 * deallocated stack.
		 */
		{
		Word nullproc_S = kp->procTable[0].irq_S;

			asm {
				lda nullproc_S
				tcs
			}
		}
		/* STACK REPAIR MUST BE OFF TO CALL proc_free() */
		proc_free(tosig);
		/* DON'T USE ANY DP AFTER THIS until _resched() */

#if 1
               /* do this in case program crashed in the kernel */
               asm { lda #0
                     sta 0xE100FF
               }
#else
               enableps();
#endif
               _resched();
               PANIC("KILL OVERRUN #1");
               /* not reached */
           default:
           {
           fdtablePtr tmpof;
	    /* $$$  tmpof = kp->procTable[Kgetpid()].openFiles; */
               tmpof = PROC->openFiles;
            /* $$$  kp->procTable[Kgetpid()].openFiles = tosig->openFiles; */
               PROC->openFiles = tosig->openFiles;
	       /* make sure we close _all_ files */
               tosig->openFiles->fdLevel =
                 tosig->openFiles->fdLevelMode = 0;
               ClosePB[0] = 1; ClosePB[1] = 0;
               CloseGS(ClosePB);
               /* $$$ kp->procTable[Kgetpid()].openFiles = tmpof; */
               PROC->openFiles = tmpof;
               tosig->processState = procUNUSED;

               nfree(tosig->openFiles); /* since we alloc'ed this- fix this later */
               break;
           }
     }
     proc_free(tosig);
     enableps();
     return SYSOK;
   }

   if (mpid == Kgetpid()) {
    int tmpwaitdone;

     /* this is implemented as a function call- is there a reason not to? */
       sig->signalmask |= sigmask(signum); /* block the signal */
       tmpwaitdone = tosig->waitdone;
       enableps();
       (*sig->v_signal[signum])(signum,0);
       tosig->waitdone = tmpwaitdone;
       Ksigsetmask(ERRNO, sig->signalmask & ~sigmask(signum));
       return 0;
   } /* process signalling itself */
   else {  /* signalling another process */
       sig->signalmask |= sigmask(signum); /* block the signal */

/* fake the RTL address of the 'jsl' to the signal handler */
       *((longword *) (tosig->irq_S-30)) = (longword) (((byte *)ctxtRestore)-1);

/* fake the two parameters - first 'code', then 'signum' */
       *((longword *) (tosig->irq_S-27)) = (longword) signum;

/* fake the parameter to cSignalHook */
       *((word *) (tosig->irq_S-23)) = (word) signum;

/* store the waitdone flag */
       *((word *) (tosig->irq_S-21)) = (word) tosig->waitdone;

/* copy the process' context record to the stack for restoration in 
   CTXTRESTORE */
       memcpy((void *) ((tosig->irq_S-19)),((byte *) tosig)+8,20l);

/* use the stack pointer context field to store the old process state,
   since the Stack is implicit */

	/* Interrupt select() */
#if 0
	/* selwakeup(1, mpid2KToff(mpid));
	FIXME: why does this break init? */
#else
        if (tosig->p_waitvec == (unsigned long)selwait)
		k_remove(tosig->p_waitvec, mpid, 1);
#endif
	switch(i = tosig->processState) {
	  /* case procRUNNING: */
	  case procBLOCKED:	/* read(), preceive(), swait(), procreceive() */
		/*
		 * We need to stay awake after the signal handler so
		 * the system call can return EINTR
		 */
		i = procREADY;
		break;
	}

	*((word *) (tosig->irq_S-13)) = i;

/* we want the signal handler to run even if the process was not READY */
       tosig->processState = procREADY;

	/* a->i1 = a->i2 = b BROKEN in C 2.0.3 */
#if 0
       tosig->irq_B = tosig->irq_B1 =
           tosig->irq_K = (sig->v_signal[signum] >> 16);
#else
	tosig->irq_K = (sig->v_signal[signum] >> 16);
	tosig->irq_B1 = tosig->irq_K;
	tosig->irq_B = tosig->irq_B1;
#endif

  /* the PC field isn't used right now. fix it */
       tosig->irq_PC = (word) sig->v_signal[signum];
       tosig->irq_P = 0x04; /* leave interrupts off you idiot */

       /* this RTI info simulation goes away at the next context switch,
          and the other, actual interrupt info from the last context
          switch out of the process gets RTId in ctxtRestore */
       *((word *) (tosig->irq_S-33)) = (word) tosig->irq_PC;
       *((byte *) (tosig->irq_S-34)) = 0; /*(byte) tosig->irq_P;*/ /* handler should run with interrupts on */
       *((byte *) (tosig->irq_S-31)) = (byte) tosig->irq_K;

       tosig->irq_S -= 35;
       enableps();
       return 0;
   }
/* we won't actually get here ... */
   PANIC("we shouldn't be here in signal");
   enableps();
   return 0;
}

void *KERNsignal(int *ERRNO, void (*func)(), int sig )
{
	return Ksignal(ERRNO, func, sig);
}

longword KERNsigsetmask(int *ERRNO, longword mask)
{
	return Ksigsetmask(ERRNO, mask);
}

longword KERNsigblock(int *ERRNO, longword mask)
{
	return Ksigblock(ERRNO, mask);
}

/* wait() code- use with caution, this stuff is heinous! */

int KERNwait(int *ERRNO, union wait *stat)
{
/* $$$ struct pentry *p; */
unsigned i;
chldInfo waitinfo;


    disableps();
    for (i = 0; i < NPROC; i++) {
        if (kp->procTable[i].processState) {
            if (kp->procTable[i].parentpid == Kgetpid()) break;
	}
    }
    if ((i == NPROC) && (PROC->waitq == NULL)) {
        *ERRNO = ECHILD; enableps(); return -1;
    }

   /* $$$ p = &(kp->procTable[Kgetpid()]); */
    while (dequeueWait(&waitinfo,Kgetpid()) == -1) {
        PROC->waitdone = 0;

        /* WAITSIGCH returns when a SIGCHLD or caught signal is sent
           to the process */
      /* $$$  kp->procTable[Kgetpid()].processState = procWAITSIGCH; */
        PROC->processState = procWAITSIGCH;
        enableps();
        _resched();
        if (PROC->waitdone == -1) { /* interrupted by a caught signal */
            *ERRNO = EINTR; return -1;
        }
        disableps();
    }
    enableps();
    if (stat != NULL)
        *stat = waitinfo.status;
    return (waitinfo.pid);
}

longword KERNalarm(int *ERRNO, longword seconds)
{
longword old;

    asm {
       php
       sei
    }
    /* $$$ old = kp->procTable[Kgetpid()].alarmCount; */
    old = PROC->alarmCount;
    /* $$$ kp->procTable[Kgetpid()].alarmCount = seconds * 10; */
    PROC->alarmCount = seconds * 10;
    asm { plp }
    return (old - (old % 10)) / 10;
}

longword KERNalarm10(int *ERRNO, longword seconds10)
{
longword old;
    asm {
       php
       sei
    }
    /* $$$ old = kp->procTable[Kgetpid()].alarmCount; */
    old = PROC->alarmCount;
    /* $$$ kp->procTable[Kgetpid()].alarmCount = seconds * 10; */
    PROC->alarmCount = seconds10;
    asm { plp }
    return old;
}

int KERNsigpause(int *ERRNO, longword mask)
{
longword oldmask;

    disableps();
    oldmask = Ksigsetmask(ERRNO, mask);
    /* $$$ kp->procTable[Kgetpid()].processState = procPAUSED; */
    PROC->processState = procPAUSED;
    enableps();
    _resched();
    Ksigsetmask(ERRNO, oldmask);
    return -1;
}

#pragma toolparms 0

#if 0
longword Kreceive(int *ERRNO)
{
longword tmp;
struct pentry *p;
extern void sleepbusy(void);

    disableps();
    p = PROC;

    p->waitdone = BLOCKED_RECEIVE;
    if (!(p->flags & FL_MSGRECVD)) {
	p->processState = procBLOCKED;
        sleepbusy();
    }
    if (p->waitdone == BLOCKED_RECEIVE) {
	tmp = p->msg;
        p->flags &= ~FL_MSGRECVD;
    }
    else tmp = -1l;

    enableps();
    return tmp; /* return the value of the message */
}
#else
extern longword Kreceive(int *);
#endif

void rcvttrap(int sig, int code)
{
  /* foobar! */
}

#pragma toolparms 1

longword KERNreceive(int *ERRNO)
{
	return Kreceive(ERRNO);
}

longword KERNrecvclr(int *ERRNO)
{
longword tmp;
struct pentry *p;
    
    p = PROC;
    disableps();

    if (p->flags & FL_MSGRECVD) tmp = p->msg;
    else tmp = -1l;
    p->flags &= ~FL_MSGRECVD;

    enableps();
    return tmp;
}

longword KERNrecvtim(int *ERRNO, int timeout)
{
void *oldsig;
longword oldalrm, oldmask, tmp;
struct pentry *p;

    oldmask = Ksigblock(ERRNO, SIGALRM);
    oldsig = Ksignal(ERRNO, rcvttrap, SIGALRM);
   /* $$$ p = &(kp->procTable[Kgetpid()]);  */
    p = PROC;
    oldalrm = p->alarmCount;
    p->alarmCount = timeout;
    Ksigsetmask(ERRNO, oldmask & ~sigmask(SIGALRM));
    tmp = Kreceive(ERRNO);
    Ksigsetmask(ERRNO, oldmask);
    p->alarmCount = oldalrm;
    Ksignal(ERRNO, oldsig, SIGALRM);
    return tmp;
}

int KERNsend(int *ERRNO, longword msg, int pid)
{
struct pentry *targetp;
int mpid;

    mpid = mapPID(pid);
    if ((mpid == 0) || (mpid == -1)) {
	*ERRNO = ESRCH; return -1;
    }
    targetp = &(kp->procTable[mpid]);
    disableps();
    if (targetp->flags & FL_MSGRECVD) {
	*ERRNO = EIO; enableps(); return -1;
    }
    targetp->msg = msg;
    targetp->flags |= FL_MSGRECVD;
    if ((targetp->waitdone == BLOCKED_RECEIVE) &&
	(targetp->processState == procBLOCKED)) {
        targetp->processState = procREADY;
    }
    enableps();
    return 0;
}

#pragma toolparms 0

void cSignalHook(int signum)
{
struct sigrec *siginf;

    disableps();
  /* $$$  siginf = kp->procTable[Kgetpid()].siginfo; */
    siginf = PROC->siginfo;
    Ksigsetmask(&errno, siginf->signalmask &= ~sigmask(signum));
    enableps();
}

#pragma databank 0

void *Ksignal(int *ERRNO, void (*func)(), int sig )
{
void (*old)();
struct sigrec *siginf;

   if (kp->gsosDebug & 16)
   	kern_printf("signal(sig: %d, func:%06lX)\n\r",sig,func);
/* $$$  siginf = kp->procTable[Kgetpid()].siginfo; */
    siginf = PROC->siginfo;
/*   printf("siginf: %08lX\n",siginf);  */
   old = siginf->v_signal[sig];
   /* silently enforce restriction on signal handling */
   if ((sig != SIGKILL) && (sig != SIGSTOP)) {
       if (func == SIG_IGN) siginf->sigpending &= ~sigmask(sig);
/*       siginf->sigpending |= sigmask(sig);  */
       siginf->v_signal[sig] = func;
   }
   return old;
}

longword Ksigblock(int *ERRNO, longword mask)
{
struct sigrec *siginf;
longword oldmask;

  /* $$$  siginf = kp->procTable[Kgetpid()].siginfo; */
   siginf = PROC->siginfo;
   oldmask = siginf->signalmask;
/* install new mask, but don't allow blocking of SIGKILL, SIGSTOP, or SIGCONT */
   siginf->signalmask |= (mask & 0xFFFAFEFFl); 
   return oldmask;
}

longword Ksigsetmask(int *ERRNO, longword mask)
{
struct sigrec *siginf;
longword oldmask,ready;
int i;

   disableps();
   /* $$$ siginf = kp->procTable[Kgetpid()].siginfo; */
   siginf = PROC->siginfo;
   oldmask = siginf->signalmask;
   mask &= 0xFFFAFEFFl;/* don't allow blocking of SIGKILL, SIGSTOP, or SIGCONT */
   ready = siginf->sigpending & ~mask;
   if (ready) {
       for (i = 1; i < 32; i++)
         if (ready & sigmask(i)) {
           addsig(Kgetpid(), i);
           siginf->sigpending &= ~sigmask(i);
         }
   }
   siginf->signalmask = mask;
   enableps();
   return oldmask;
}

