/*	$Id: sleep.c,v 1.1 1998/02/02 08:19:00 taubert Exp $ */

/*
 *	sleep.c
 *
 *	sleep/wakeup handling routines
 */

extern void sleepbusy(void);

#ifndef KERNEL
#include "tests/testsleep.c"
#else

/* Building it into the kernel */

#pragma databank 1
#pragma optimize 79

/*segment "KERN2     ";*/
#include "proc.h"
#include "sys.h"
#include "kernel.h"
#include "/lang/orca/libraries/orcacdefs/stdio.h"
extern kernelStructPtr kp;

void dosleep(int pid)
{
    kp->procTable[pid].processState = procSLEEP;
}

static void ready(int pid, int resch)
{
    kp->procTable[pid].processState = procREADY;
    if (resch) _resched();
}

#endif

typedef struct hash_entry {
    unsigned		pid;		/* first pool entry on the wait list */
} hash_entry;

#define NUM_HVENTRIES 64
hash_entry hv_tab[NUM_HVENTRIES];

#if 0
void init_sleep_hash(void)
{
unsigned i;

    for (i = 0; i < NUM_HVENTRIES; i++) {
        hv_tab[i].vector = 0l;
        hv_tab[i].pid = 0;
    }
}
#endif

static unsigned hash_vector(unsigned long vec)
{
    return (unsigned)(vec & 0x3F);
}

#define BUSY_FLAG ((byte *)0xE100FFl)

/* no longer used; this code is now in assembly language */
#if 0
int k_sleepold(unsigned long vec, int pri, int pid)
{
unsigned hv;
struct proc *p;
unsigned last;
unsigned cur;
byte oldBusy;

    disableps();
    hv = hash_vector(vec);
    /*PROC->p_pri = pri;*/
    /*PROC->p_waitvec = vec;*/
    kp->procTable[pid].p_waitvec = vec;

    /* Add this process to the END of this wait queue.  This gives us a
       FIFO action on the sleep queues */
    last = hv_tab[hv].pid;
    if (last == 0) {
        hv_tab[hv].pid = pid;
    } else {
        while (kp->procTable[last].p_slink != 0) {
            last = kp->procTable[last].p_slink;
        }
        kp->procTable[last].p_slink = pid;
    }
    kp->procTable[pid].p_slink = 0;
    /*fprintf(stderr,"process %d going to sleep on vec %06lX\n",pid,vec);*/
    dosleep(pid);
    sleepbusy();
    enableps();
}
#endif

/* Remove a single process from a sleep vector */
void k_remove(unsigned long vec, int pid, int readyq)
{
unsigned last = 0;
unsigned hv;
unsigned cur,link;

    /*fprintf(stderr,"removing %d from %06lX\n",pid,vec);*/
    disableps();
    hv = hash_vector(vec);
    cur = hv_tab[hv].pid;
    while (cur != 0) {
	link = kp->procTable[cur].p_slink;
	if ((kp->procTable[cur].p_waitvec == vec) && (cur == pid)) {
	    /* Remove an item from the list. If the item is the first,
               last == 0, and we set the hash vector to the follower
               of the item */
            if (last == 0) {
        	hv_tab[hv].pid = link;
	    }
            /* Otherwise, the item is not the first; last != 0; we set
               last's link to the current link, removing the curproc
               from the list */
            else {
        	kp->procTable[last].p_slink = kp->procTable[cur].p_slink;
        	last = cur;
	    }
            if (readyq) ready(cur,0);
            kp->procTable[cur].p_waitvec = 0l;
            cur = link;
        } else {
	    last = cur;
            cur = link;
        }
    }
    enableps();
    /* if priority of a proc we awakened was higher than current priority
       we need to _resched() */
}

/* Remove all processes from a sleep vector */
void k_wakeup(unsigned long vec)
{
unsigned last = 0;
unsigned hv;
unsigned cur,link;

    /*fprintf(stderr,"waking up %06lX\n",vec);*/
    disableps();
    hv = hash_vector(vec);
    cur = hv_tab[hv].pid;
    while (cur != 0) {
	link = kp->procTable[cur].p_slink;
	if (kp->procTable[cur].p_waitvec == vec) {
	    /* Remove an item from the list. If the item is the first,
               last == 0, and we set the hash vector to the follower
               of the item */
            if (last == 0) {
        	hv_tab[hv].pid = link;
	    }
            /* Otherwise, the item is not the first; last != 0; we set
               last's link to the current link, removing the curproc
               from the list */
            else {
        	kp->procTable[last].p_slink = kp->procTable[cur].p_slink;
        	last = cur;
	    }
            ready(cur,0);
            kp->procTable[cur].p_waitvec = 0l;
            cur = link;
        } else {
	    last = cur;
            cur = link;
        }
    }
    enableps();
    /* if priority of a proc we awakened was higher than current priority
       we need to _resched() */
}

