/*	$Id: q.h,v 1.1 1998/02/02 08:18:46 taubert Exp $ */

/* q.h - firstid, firstkey, isempty, lastkey, nonempty */
#if 0
#ifndef NQNET
#define NQENT		NPROC + NSEM + NSEM + 4 /* for ready & sleep */
#endif

struct qent {		/* one for each process plust two for each list */
	int qkey;	/* key on which the queue is ordered */
	int qnext;	/* pointer to next process or tail */
	int qdata;      /* extra data to be stored with the key */
        int qprev;	/* pointer to previous process or head */
};

extern struct qent _q[];

/* list manipulation macros */
#define isempty(list)	(_q[(list)].qnext >= NPROC)
#define nonempty(list)	(_q[(list)].qnext < NPROC)
#define firstkey(list)	(_q[_q[(list)].qnext].qkey)
#define lastkey(tail)	(_q[_q[(tail)].qprev].qkey)
#define firstid(list)	(_q[(list)].qnext)

#define EMPTY	-1		/* equivalent of null pointer */

#else

struct qstruct {	/* one for each process plust two for each list */
	struct pentry *head;	/* head of the q */
	struct pentry *tail;	/* tail of the q */
};

#define NQS NSEM+2	/* ready, sleep, and # of semaphores */
#define isbadqnum(__qn)	(((__qn) < 0) || ((__qn) >= NQS))

extern struct qstruct *nq;
extern struct qstruct *q_free;

#define nonempty(q) (nq[(q)].head != NULL)

#endif
