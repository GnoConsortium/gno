/*	$Id: proc.h,v 1.1 1998/02/02 08:18:43 taubert Exp $ */

/*
         Kernel Process table structure
         Copyright 1991-1998 Procyon, Inc.
*/

#ifndef PROC_KERN
#define PROC_KERN

#include <types.h>
#include <sys/types.h>
#include <sys/signal.h>

/* the various process states are defined here */

#define procUNUSED 0
#define procRUNNING 1
#define procREADY 2
#define procBLOCKED 3
#define procNEW 4
#define procSUSPENDED 5
#define procWAIT 6
#define procWAITSIGCH 7
#define procPAUSED 8
#define procSLEEP 9

#define BLOCKED_RECEIVE		2
#define BLOCKED_PRECEIVE	3
#define BLOCKED_SWAIT		4

#define SYSERR -1
#define SYSOK 0

#define rtGSOS 0
#define rtPIPE 1
#define rtTTY 2
#define rtSOCKET 3

#define rfPIPEREAD 1    /* read end of the pipe */
#define rfPIPEWRITE 2   /* write end of the pipe */
#define rfCLOSEEXEC 4   /* close this file on an exec() */
#define rfP16NEWL 8     /* special prodos-16 newline mode */

typedef struct fdentry {
    word refNum;        /* refNum, pipeNum, ttyID, or sockNum */
    word refType;       /* 0 = GS/OS refnum, 1 = pipe, 2 = tty, 3 = socket */
    word refLevel;      /* "file level" of the refnum */
    word refFlags;      /* see flags above */
    word NLenableMask;  /* these three fields are for newline handling */
    word NLnumChars;
    void *NLtable;
} fdentry, *fdentryPtr;

typedef struct fdtable {
    word fdCount;
    word fdLevel;
    word fdLevelMode;
    word fdTableSize;
    fdentry fds[1];
} fdtable, *fdtablePtr;

#define FD_SIZE 32

typedef struct quitStack {
  struct quitStack *next;
  char data[1];
} quitStack;

/* these flags are set by execve() and fork() during process creation. */
   
#define FL_RESOURCE 1      /* does the process have and use a resource fork? */
#define FL_FORKED 2        /* was the process started with fork() ? */
#define FL_COMPLIANT 4     /* is the process fully GNO compliant? */
#define FL_NORMTERM 8      /* did the program terminate via exit()? 1=yes */
#define FL_RESTART 16      /* is the program restartable? (set by QuitGS) */
#define FL_NORESTART 32    /* don't allow this code to restart */
#define FL_QDSTARTUP 64    /* flag set if QDStartUp was called */
#define FL_MSGRECVD 128	   /* flag set if there's a send() msg waiting */
#define FL_SELECTING 256   /* this process is 'selecting' */

struct pentry {
    int parentpid;      /* pid of this process' parent */
    int processState;
    int userID;         /* a GS/OS UserID, used to keep track of memory */
    int ttyID;          /* driver (not GS/OS) number of i/o port */
    word irq_A;          /* context information for the process */
    word irq_X;
    word irq_Y;
    word irq_S;
    word irq_D;
    byte irq_B;
    byte irq_B1;
    word irq_P;
    word irq_state;
    word irq_PC;
    word irq_K;
    int psem;           /* semaphoreID process is blocked on */
    char **prefix;      /* cwd's (GS/OS prefixes 0,1, and 9 */
    char *args;         /* the command line that invoked the process program */
    char **env;         /* environment variables for the program */
    struct sigrec *siginfo; /* global mask of which signals are blocked */
    byte irq_SLTROM;
    byte irq_STATEREG;
    word lastTool;
    longword ticks;
    word flags;
    fdtablePtr openFiles;
    word pgrp;
    word exitCode;
    void *LInfo;
    word stoppedState;  /* process state before stoppage */
    longword alarmCount;
    void *executeHook; /* for a good time call... */
    word queueLink;
#ifdef KERNEL
    chldInfoPtr waitq;   /* where waits wait to be processed */
#else
    void *waitq;
#endif
    int waitdone;
    int flpid;
    quitStack *returnStack;
    word t2StackPtr;
    word p_uid, p_gid;
    word p_euid, p_egid;
    word SANEwap;
    longword msg;
    longword childTicks;
    unsigned long p_waitvec;
    unsigned p_slink;
    struct pentry *p_link,*p_rlink;
    int p_prio;
    /* no unused entries */
};
typedef struct pentry procState, *procStatePtr;

#ifdef KERNEL
struct kernelStruct {
    procState procTable[32];
    int curProcInd;
    int userID;
    int mutex;
    int timeleft;
    int numProcs;
    word truepid;
    word shutdown;
    word gsosDebug;
    int floatingPID;
};
typedef struct kernelStruct kernelStruct, *kernelStructPtr;

#endif /* KERNEL */
#endif /* PROC_KERN */
