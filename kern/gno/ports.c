/*	$Id: ports.c,v 1.1 1998/02/02 08:18:41 taubert Exp $ */

#include "conf.h"
#include "kernel.h"
#include "proc.h"
#include "gno.h"
#include "/lang/orca/libraries/orcacdefs/stdlib.h"
#include "/lang/orca/libraries/orcacdefs/string.h"
#include <sys/errno.h>
#include <sys/ports.h>

#pragma optimize 79

struct	ptnode	*ptfree;	/* list of free queue nodes */
struct	pt	ports[NPORTS];
int	ptnextp;

extern void PANIC(char *s);

#ifdef KERNEL
segment "KERN2     ";
#else
#include "tests/testports.c"
#endif

/*
 *	pinit - initialize all ports
 */

SYSCALL pinit(int maxmsgs)
{
int i;
struct ptnode *next,*prev;

    if ( (ptfree=malloc(maxmsgs*sizeof(struct ptnode)))==NULL )
        PANIC("pinit - insufficient memory");
    for (i = 0; i < NPORTS; i++) {
	ports[i].ptstate = PTFREE;
        ports[i].ptseq = 0;
    }
    ptnextp = NPORTS - 1;

    /* link up free list of message pointer nodes */
    for (prev = next = ptfree; --maxmsgs > 0; prev = next)
	prev->ptnext = ++next;
    prev->ptnext = NULL;
    return(OK);
}

#pragma databank 1
#pragma toolparms 1
/*
 *	pcreate - create a port that allows "count" outstanding messages
 */
pascal SYSCALL KERNpcreate(int count, int *ERRNO)
{
int ps;
int i,p;
struct pt *ptptr;

    if (count < 0) return SYSERR;
    disableps();
    for (i = 0; i < NPORTS; i++) {
	if ((p = ptnextp--) < 0)
	    ptnextp = NPORTS - 1;
        if ((ptptr = &ports[p])->ptstate == PTFREE) {
	    ptptr->ptstate = PTALLOC;
            ptptr->ptname = PTUNNAMED;
            ptptr->ptssem = Kscreate(ERRNO, count);
            if (ptptr->ptssem == SYSERR) {
      bad:      ptptr->ptstate = PTFREE;
                *ERRNO =  ENOMEM;
                enableps();
                return SYSERR;
            }
            ptptr->ptrsem = Kscreate(ERRNO, 0);
	    if (ptptr->ptrsem == SYSERR) {
	        Ksdelete(ERRNO, ptptr->ptssem);
	        goto bad;
            }
	    /* a->i1 = a->i2 = b BROKEN in C 2.0.3 */
#if 0
	    ptptr->pthead = ptptr->pttail = NULL;
#else
	    ptptr->pthead = NULL;
	    ptptr->pttail = NULL;
#endif
            ptptr->ptseq++;
            ptptr->ptmaxcnt = count;
            enableps();
            return p;
        }
    }
    enableps();
    *ERRNO = ENOMEM;
    return SYSERR;
}

/*
 *	psend - send a message to a port by enqueueing it
 */

pascal SYSCALL KERNpsend(int portid, long int msg, int *ERRNO)
{
int ps;
struct pt *ptptr;
int seq;
struct ptnode *freenode;

    disableps();
    if (isbadport(portid) ||
        (ptptr = &ports[portid])->ptstate != PTALLOC) {
            enableps();
            return SYSERR;
    }
    /* wait for space and verify port is still allocated */
    seq = ptptr->ptseq;
    if (commonSwait(ERRNO,ptptr->ptssem,procBLOCKED,BLOCKED_PRECEIVE) == SYSERR) {
	enableps();
	/* *ERRNO set in commonSwait() */
	return SYSERR;
    }
    if (ptptr->ptstate != PTALLOC || ptptr->ptseq != seq) {
	enableps();
	return SYSERR;
    }
    if (ptfree == NULL)
	PANIC("Ports - out of nodes");
    freenode = ptfree;
    ptfree = freenode->ptnext;
    freenode->ptnext = NULL;
    freenode->ptmsg = msg;
    if (ptptr->pttail == NULL) {
	/* a->i1 = a->i2 = b BROKEN in C 2.0.3 */
#if 0
	ptptr->pthead = ptptr->pttail = freenode;
#else
	ptptr->pthead = freenode;
	ptptr->pttail = freenode;
#endif
    } else {
	(ptptr->pttail)->ptnext = freenode;
        ptptr->pttail = freenode;
    }
    Kssignal(ERRNO, ptptr->ptrsem);
    enableps();
    return OK;
}

/*
 *	preceive - receive a message from a port, blocking if port empty
 */

pascal long SYSCALL KERNpreceive(int portid, int *ERRNO)
{
int ps;
struct pt *ptptr;
int seq;
long int msg;
struct ptnode *nxtnode;

    disableps();
    if (isbadport(portid) ||
        (ptptr = &ports[portid])->ptstate != PTALLOC) {
            enableps();
            return SYSERR;
    }
    /* wait for message and verify that the port is still allocated */
    seq = ptptr->ptseq;
    /* sleep, and return EINTR/SYSERR if we were interrupted */
    if (commonSwait(ERRNO,ptptr->ptrsem,procBLOCKED,BLOCKED_PRECEIVE) == SYSERR) {
        enableps();
	/* *ERRNO set in commonSwait() */
        return SYSERR;
    }
    if (ptptr->ptstate != PTALLOC || ptptr->ptseq != seq) {
            enableps();
            return SYSERR;
    }
    /* dequeue first message that is waiting in the port */

    nxtnode = ptptr->pthead;
    msg = nxtnode->ptmsg;
    if (ptptr->pthead == ptptr->pttail) { /* delete last item */
	/* a->i1 = a->i2 = b BROKEN in C 2.0.3 */
#if 0
	ptptr->pthead = ptptr->pttail = NULL;
#else
	ptptr->pthead = NULL;
	ptptr->pttail = NULL;
#endif
    } else ptptr->pthead = nxtnode->ptnext;
    nxtnode->ptnext = ptfree; /* return to free list */
    ptfree = nxtnode;
    Kssignal(ERRNO, ptptr->ptssem);
    enableps();
    return msg;
}

#pragma toolparms 0

/*
 *	_ptclear - used by pdelete and preset to clear a port
 */

_ptclear(struct pt *ptptr, int newstate, int (*dispose)(long int))
{
struct ptnode *p;

    /* put port in limbo until done freeing processes */
    ptptr->ptstate = PTLIMBO;
    ptptr->ptseq++;
    if ((p=ptptr->pthead) != NULL) {
        for (; p != NULL; p=p->ptnext)
            /* only do this if they specified a disposition */
	    if (dispose != NULL) (*dispose)(p->ptmsg);
        (ptptr->pttail)->ptnext = ptfree;
        ptfree = ptptr->pthead;
    }
    if (newstate == PTALLOC) {
	/* a->i1 = a->i2 = b BROKEN in C 2.0.3 */
#if 0
	ptptr->pthead = ptptr->pttail = NULL;
#else
	ptptr->pthead = NULL;
	ptptr->pttail = NULL;
#endif
        /* sreset(ptptr->ptssem, ptptr->ptmaxcnt);
        sreset(ptptr->ptrsem, 0); */
    } else {
        enableps();
        Ksdelete(&errno, ptptr->ptssem);
        Ksdelete(&errno, ptptr->ptrsem);
        disableps();
    }
    ptptr->ptstate = newstate;
}

#pragma toolparms 1
/*
 *	pdelete - delete a port, freeing waiting processes and messages
 */

pascal SYSCALL KERNpdelete(int portid, int (*dispose)(long int), int *ERRNO)
{
int ps;
struct pt *ptptr;

    disableps();
    if (isbadport(portid) ||
        (ptptr = &ports[portid])->ptstate != PTALLOC) {
        enableps();
        return SYSERR;
    }
    _ptclear(ptptr,PTFREE,dispose);
    /* dispose of the port's name */
    if (ptptr->ptname != NULL) {
        free(ptptr->ptname);
        ptptr->ptname = NULL;
    }
    enableps();
    return OK;
}

/*
 *	preset - reset a port, freeing waiting processes and messages
 */

pascal SYSCALL KERNpreset(int portid, int (*dispose)(long int), int *ERRNO)
{
int ps;
struct pt *ptptr;

    disableps();
    if (isbadport(portid) ||
        (ptptr = &ports[portid])->ptstate != PTALLOC) {
        enableps();
        return SYSERR;
    }
    _ptclear(ptptr, PTALLOC, dispose);
    enableps();
    return OK;
}

/*
 *	pbind - binds a port to a name
 */

pascal SYSCALL KERNpbind(int portid, char *name, int *ERRNO)
{
struct pt *ptptr;

    disableps();
    if (isbadport(portid) ||
        (ptptr = &ports[portid])->ptstate != PTALLOC) {
        enableps();
        return SYSERR;
    }
    if (ptptr->ptname != PTUNNAMED) {
        enableps();
        return SYSERR;
    }
    ptptr->ptname = malloc(33);
    strncpy(ptptr->ptname,name,32);
    enableps();
    return OK;
}

pascal SYSCALL KERNpgetport(char *name, int *ERRNO)
{
struct pt *ptptr;
unsigned i;

    disableps();
    for (i = 0; i < NPORTS; i++)
        if ((ports[i].ptstate == PTALLOC) && (ports[i].ptname != NULL)
            && (!strncmp(ports[i].ptname,name,32))) {
            enableps();                      
            return i;
        }
    enableps();
    return SYSERR;
}

pascal SYSCALL KERNpgetcount(int portid, int *ERRNO)
{
struct pt *ptptr;
int c,d;

    disableps();
    if (isbadport(portid) ||
        (ptptr = &ports[portid])->ptstate != PTALLOC) {
        enableps();
        return SYSERR;
    }
    c = Kscount(ERRNO, ptptr->ptssem);
    d = (c == SYSERR) ? ptptr->ptmaxcnt : ptptr->ptmaxcnt - c;
    enableps();
    return d;
}

