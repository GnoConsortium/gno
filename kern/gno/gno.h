/*	$Id: gno.h,v 1.1 1998/02/02 08:18:29 taubert Exp $ */

/*
         gno.h

         libgno interface file for GNO Kernel
         v1.0b2
         Copyright 1991-1998, Procyon Inc.
*/

#include <types.h>

#ifndef udispatch
#define udispatch  0xE10008
#endif
extern pascal int   kernVersion() inline(0x0403, udispatch);
extern pascal int   kernStatus() inline(0x0603, udispatch);

#ifndef KERNEL
int   getppid(void);
int   fork(void *subr);
int   exec(char *filename,char *cmdline);
int   tcnewpgrp(int fdtty);
int   settpgrp(int fdtty);
int   tctpgrp(int fdtty, int pid);
int   setdebug(int code);
void *setsystemvector(void *vect);
int   pipe(int filedes[2]);
int   getpgrp(int pid);
int   setpgrp(int pid,int pgrp);
int   ioctl(int d, unsigned long request, void *argp);
/* 'dup()' appears in fcntl.h, dup2() should too, but I'm not rewriting
   any more O**A C header files */
int   dup2(int filedes, int filedes2);

void SetGNOQuitRec(word,void *,word);
unsigned int sleep(unsigned int);

#ifdef __stdio__
FILE *fdopen(int,char*);
#endif

int   execve(char *filename,char *cmdline);
int   fork2(void *subr, int stack, int prio, char *name, int nargs, ...);
int   screate(int count);
int   ssignal(int sem);
int   swait(int sem);
int   scount(int sem);
int   sdelete(int sem);
int   getpid(void);

int   kill(int pid, int sig);
int   wait(union wait *status);
void  *signal(int sig, void (*func)());
longword sigblock(longword mask);
longword sigsetmask(longword mask);
longword alarm(longword seconds);
longword alarm10(longword tenths);
int   sigpause(longword mask);
longword procrecvclr(void);
longword procreceive(void);
longword procrecvtim(int timeout);
int   procsend(int pid, unsigned long msg);

#else
int kern_printf(const char *, ...);

int KERNexecve(int *ERRNO, char *cmdline, char *filename);
int Kscreate(int *ERRNO, int count);
int KERNssignal(int *ERRNO, int sem);
int Kscount(int *ERRNO, int sem);
int KERNsdelete(int *ERRNO, int sem);
int KERNkill(int *ERRNO, int signum, int pid);
void *Ksignal(int *ERRNO, void (*func)(), int sig );
longword Ksigblock(int *ERRNO, longword mask);
longword Ksigsetmask(int *ERRNO, longword mask);
int KERNkvmsetproc(int *ERRNO, struct kvmt *kd);

#define Kexecve(__e, __p1, __p2)	\
	{ asm { pha } KERNexecve(__e, __p1, __p2); asm { pla } }
#define Kssignal(__e, __p1)		\
	{ asm { pha } KERNssignal(__e, __p1); asm { pla } }
#define Ksdelete(__e, __p1)		\
	{ asm { pha } KERNsdelete(__e, __p1); asm { pla } }
#define Kkill(__e, __p1, __p2)		\
	{ asm { pha } KERNkill(__e, __p1, __p2); asm { pla } }
#define Kkvmsetproc(__e, __p1)		\
	{ asm { pha } KERNkvmsetproc(__e, __p1); asm { pla } }

extern int errno;

#endif
