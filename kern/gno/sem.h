/*	$Id: sem.h,v 1.1 1998/02/02 08:18:54 taubert Exp $ */

/* sem.h - semaphore definitions */

#if NSEM
#define SFREE		1	/* free semaphore */
#define SUSED		2	/* semaphore in use */

struct sentry {			/* semaphore table entry */
	char sstate;		/* SFREE or SUSED */
	short semcnt;		/* semaphore count, (i.e. value) */
        unsigned squeue;	/* process queue id */
};
extern struct sentry *_semaph;
/*extern struct sentry _semaph[];*/

#define isbadsem(s)	(s<0 || s>=NSEM)

void semINTR(int sem, int mpid);
#endif
