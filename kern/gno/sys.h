/*	$Id: sys.h,v 1.1 1998/02/02 08:19:10 taubert Exp $ */

#if 0
int createProc(word stack, word dp, longword process);
int _ready(int pid, int resch);
int isbadpid( int pid );
/*void *mmemcpy(const void *dest, void *src, size_t len);  */
#endif
#define Kgetpid() kp->truepid

typedef struct pgrp {
   word pgrpref; /* number of references to this pgrp - tty and process */
} pgrp;

fdentryPtr allocFD(int *fdn);
void copygsstr(void *,void*);
void nfree(void *);

#define PROC procPtr
extern struct pentry *procPtr;
