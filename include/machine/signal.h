/*
 * Copyright (c) 1992-1996, Procyon, Inc.
 * This file was pieced together from the BSD manpages and my own
 * implementation.  The definition of sigmask (with a modification to 
 * make ANSI happy) was borrowed straight from the BSD signal.h file.
 */

#ifndef _MACHINE_SIGNAL_H_
#define _MACHINE_SIGNAL_H_

typedef int sig_atomic_t;

#ifndef _SYS_WAIT_H_
#include <sys/wait.h>
#endif

typedef struct chldInfo {
   struct chldInfo *next;
   int pid;                       /* pid of terminated process */
   union wait status;           /* exit status of terminated process */
} chldInfo, *chldInfoPtr;

#ifdef KERNEL

#ifndef	_SYS_SIGNAL_H_
#include <sys/signal.h>
#endif

struct sigrec {
   unsigned long signalmask;
   unsigned long sigpending;	/* describes which signals are pending */
   void (*v_signal[NSIG])(int, int);
   unsigned long u_sigmask[NSIG];
   unsigned long u_oldmask;	/* for sigpause & sigblock */
   int sigindex;		/* which signal to check next for processing */
};
#endif	/* KERNEL */

#endif
