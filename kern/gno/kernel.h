/*	$Id: kernel.h,v 1.1 1998/02/02 08:18:30 taubert Exp $ */

/* kernel.h - disable, enable, halt, restore */

/* Symbolic constants used throughout Xinu */

#ifndef NULL
#define NULL (void*)0l
#endif

typedef char		Bool;		/* Boolean type */
#define FALSE		0		/* Boolean constants */
#define TRUE		1
#define SYSCALL		int		/* system call */
#define LOCAL		static		/* local procedure */
#define INTPROC		int		/* interrupt procedure */
#define PROCESS		int		/* process declaration */
#define RESCHYES	1		/* tell ready to reschedule */
#define RESCHNO		0		/* tell not ready to reschedule */
#define MININT		0100000		/* minimum short integer (-32768) */
#define MAXINT		0077777		/* maximum short integer */
#define MINSTK		40		/* minimum process stack size */
#define OK		0		/* returned when system call ok */
#define SYSERR		-1		/* returned when system call fails */

/* actually these just fiddle with the interrupt from the timer */

#define disableps() asm { jsl 0xE10064 }
#define enableps() asm { jsl 0xE10068 }
#define _resched()  asm { cop 0x7f }

#define disable(oldmask)	disableps()
#define enable()		(void) KERNsigsetmask(&errno, 0)
#define restore(oldmask)	enableps()

extern int _rdyhead, _rdytail;
extern int mapPID(int);
extern int allocPID(void);
#define mpid2KToff(__m) ((__m) << 7)

