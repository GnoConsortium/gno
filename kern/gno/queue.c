/*	$Id: queue.c,v 1.1 1998/02/02 08:18:48 taubert Exp $ */

#pragma optimize 79

#ifdef KERNEL
#include "proc.h"
#include "sys.h"
#include "conf.h"
#include "kernel.h"
#include "q.h"
#include "/lang/orca/libraries/orcacdefs/stdlib.h"

extern kernelStructPtr kp;
#else
#include "tests/testqueue.c"
#endif

void initQ(void)
{
int i;

	for (i=0; i<NPROC; i++) {
		kp->procTable[i].p_link = NULL;
		kp->procTable[i].p_rlink = NULL;
	}

	if (!(nq = calloc(NQS,sizeof(struct qstruct))))
		PANIC("Could not allocate Q entries");
	for (i=0; i<NQS-1; i++)
		nq[i].head = (struct pentry *) &nq[i+1];
	nq[NQS-1].head = NULL;
	q_free = nq;
}

int Qalloc(void)
{
struct qstruct *newfree,*newQ;

	if (!q_free)
		PANIC("Qalloc failed - ran out of free Q entries");
	newQ = q_free;
	q_free = (struct qstruct *) newQ->head;
	newQ->head = NULL;
	newQ->tail = NULL;
	return (newQ - nq);
}

void Qdispose(int qnum)
{
struct qstruct *qn;

	if (isbadqnum(qnum))
		PANIC("Bogus qnum passed to Qdispose");
	qn = &nq[qnum];
	qn->head = (struct pentry *) q_free;
	q_free = qn;
}

int _enqueue(int item, int qnum)
/* insert item at the tail of a list */
{
struct pentry *tptr;      /* points to tail entry */
struct pentry *mptr;      /* points to item entry */

	if (isbadqnum(qnum))
		PANIC("Bogus qnum passed to Qdispose");
	mptr = &kp->procTable[item];
	if (mptr->p_link || mptr->p_rlink) {
		asm {brk 0xee};
		PANIC("Attempt to _enqueue pentry on multiple queues");
	}

	tptr = nq[qnum].tail;
	mptr->p_rlink = tptr;
	mptr->p_link = NULL;
	if (tptr)
		tptr->p_link = mptr;
	else
		nq[qnum].head = mptr; /* if there was nothing in list... */
	nq[qnum].tail = mptr;
	return item;
}

int _getfirst(int qnum)
/* remove item from the front of a queue and return it */
{
struct pentry *mptr;

	if (isbadqnum(qnum))
		PANIC("Bogus qnum passed to _getfirst");
        mptr = nq[qnum].head;
	if (!mptr) return SYSERR;
	nq[qnum].head = mptr->p_link;
	if (!mptr->p_link) nq[qnum].tail = NULL;
	else mptr->p_link->p_rlink = NULL;

	mptr->p_link = NULL;
	mptr->p_rlink = NULL;

	return (mptr - kp);
}

/* we don't use _getlast anywhere */

int _dequeueitem(int item, int qnum)
/* remove an item from a list and return it */
{
struct pentry *mptr;		/* pointer to q entry for item */
struct pentry *test;

	if (isbadqnum(qnum))
		PANIC("Bogus qnum passed to _dequeueitem");
	mptr = &kp->procTable[item];

	test = nq[qnum].head;
	while (test != mptr) {
		if (!test) return SYSERR; /* item not found */
		test = test->p_link;
        }

	if (!mptr->p_link) nq[qnum].tail = mptr->p_rlink;
	else mptr->p_link->p_rlink = mptr->p_rlink;
	if (!mptr->p_rlink) nq[qnum].head = mptr->p_link;
	else mptr->p_rlink->p_link = mptr->p_link;

	mptr->p_link = NULL;
	mptr->p_rlink = NULL;

	return(item);
}

int _insert(int proc, int qnum, int key)
/* insert a process into a q list in key order */
/* int proc;	process to insert */
/* int qnum;	q index of head of list */
/* int key;	key to use for this process */
{
struct pentry *mptr, *next, *prev;

	if (isbadqnum(qnum))
		PANIC("Bogus qnum passed to _insert");
        mptr = &kp->procTable[proc];
	if (mptr->p_link || mptr->p_rlink) {
		asm {brk 0xee};
		PANIC("Attempt to _insert pentry on multiple queues");
	}

	prev = NULL;
	for (next=nq[qnum].head;next&&(next->p_prio<key);next=next->p_link)
		prev = next;
	if (mptr->p_rlink = prev) prev->p_link = mptr;
	else nq[qnum].head = mptr;
	if (mptr->p_link = next) next->p_rlink = mptr;
	else nq[qnum].tail = mptr;
	return OK;
}

