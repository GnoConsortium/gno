/*
 * gno/gno.h	This collection of declarations are for routines that
 *		reside in libc, but are Apple IIgs or GNO specific.
 *
 * $Id: gno.h,v 1.5 1998/10/31 18:50:44 gdr-ftp Exp $
 */

#ifndef _GNO_GNO_H_
#define _GNO_GNO_H_

#ifndef _SYS_CDEFS_H_
#include <sys/cdefs.h>
#endif

#ifndef _SYS_TYPES_H_
#include <sys/types.h>
#endif

#ifndef __TYPES__
#include <types.h>
#endif

#ifndef _STDLIB_H_
#include <stdlib.h>
#endif

/*
 * Don't use va_list in these prototypes.   Va_list is typedef'd in two
 * places (<machine/varargs.h> and <machine/stdarg.h>), so if we include one
 * of them here we may collide with the utility's includes.  It's unreasonable
 * for utilities to have to include one of them to include err.h, so we get
 * _BSD_VA_LIST_ from <machine/ansi.h> and use it.
 */

#ifndef _MACHINE_ANSI_H_
#include <machine/ansi.h>
#endif

#ifndef udispatch
#define udispatch  0xE10008
#endif

/* Arguments to setdebug(2) -- unverified */
#define	dbgGSOS		0x0001
#define dbgPATH		0x0002
#define dbgERROR	0x0004
#define dbgSIG		0x0008
#define dbgSYSCALL	0x0010
#define dbgPBLOCK	0x0020

/* Environment Information */
extern char *	__progname;
char *		__prognameGS __P((void));
char *		buildCmd __P((char * const *));
int		buildEnv __P((char * const *));
char *		buildPath __P((const char *));
int		isRootPath __P((const char *));
int		needsgno __P((void));
int		setdebug __P((int));
void *		setsystemvector __P((void *));
int		InstallNetDriver __P((caddr_t netcore, short domain));
void		SetGNOQuitRec __P((word pCount, GSStringPtr pathname,
				   word flags));
#if defined(__ORCAC__) && !defined(__KERN_STATUS)
#define __KERN_STATUS			/* conflict with <gno/kerntool.h> */
pascal int	kernVersion(void)	inline(0x0403, udispatch);
pascal int	kernStatus(void)	inline(0x0603, udispatch);
#endif

/* Stack Checking */
void		_assertStack __P((unsigned int, int, const char *));
void		_beginStackCheck __P((void));
int		_endStackCheck __P((void));
unsigned int	_getStackBottom __P((void));
void		_reportStack __P((void));
#ifdef __STACK_CHECK__
#define		__REPORT_STACK	_reportStack
#else
#define		__REPORT_STACK()
#endif

/* String Conversions */
#define		GIfree(a) free(a)
#define		GOfree(a) free(a)

GSStringPtr	__C2GS __P((const char *, GSStringPtr));
GSStringPtr	__C2GSMALLOC __P((const char *));
char *		__GS2C __P((char *, const GSStringPtr));
char *		__GS2CMALLOC __P((const GSStringPtr));
GSStringPtr	GIinit __P((word, const char *));
GSStringPtr	GIchange __P((GSStringPtr, word, const char *));
ResultBufPtr	GOinit __P((word, const char *));
ResultBufPtr	GOchange __P((ResultBufPtr, word, const char *));

/* Mapping */
char *		_mapPath __P((char *));
GSStringPtr	_mapPathGS __P((GSStringPtr));
int		_getPathMapping __P((void));
int		_setPathMapping __P((int));
mode_t		_mapMode2GS __P((mode_t));
mode_t		_mapMode2Unix __P((mode_t));
int		_getModeEmulation __P((void));
int		_setModeEmulation __P((int));
int		_mapErr __P((int));

/* Low-level multithread safe routines */
char *		sprintmt __P((char *, size_t, const char *, ...));
char *		vsprintmt __P((char *, size_t, const char *, _BSD_VA_LIST_));

/* Message Passing IPC */
int		procsend __P((pid_t, unsigned long));
unsigned long	procreceive __P((void));
unsigned long	procrecvtim __P((short));
unsigned long	procrecvclr __P((void));

/* Job Control */
int		tcnewpgrp __P((int));
int		tctpgrp __P((int, pid_t));
int		settpgrp __P((int));

/* Semaphores */
int		scount __P((int));
int		screate __P((int));
int		sdelete __P((int));
int		ssignal __P((int));
int		swait __P((int));

/* Timers */
unsigned long	alarm10 __P((unsigned long tenths));

/* Network */
int		InstallNetDriver __P((void *pr_usrreq, int domain));

/* Kernel Virtual Memory */
#ifdef KERNEL
kvm_t *		kvm_open __P((void));			/* non-BSD */
int		kvm_close __P((kvm_t *));
struct pentry *	kvm_getproc __P((kvm_t *, pid_t));
struct pentry *	kvm_nextproc __P((kvm_t *));
int		kvm_setproc __P((kvm_t *));
#endif

#endif /* _GNO_GNO_H_ */
