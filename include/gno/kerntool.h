/*
 * ToolBox interface file for the GNO version 1.0.6 Kernel.
 *
 * Copyright 1991-1996, Procyon Inc.
 *
 * $Id: kerntool.h,v 1.1 1997/02/28 04:42:06 gdr Exp $
 */

#ifndef _GNO_KERNTOOL_H_
#define _GNO_KERNTOOL_H_

#ifndef _SYS_TYPES_H_
#include <sys/types.h>
#endif

#ifndef __TYPES__
#include <types.h>
#endif

#ifndef udispatch
#define udispatch  0xE10008
#endif

pascal void	kernBootInit(void)	inline(0x0103, udispatch);
pascal void	kernStartUp(void)	inline(0x0203, udispatch);
pascal void	kernShutDown(void)	inline(0x0303, udispatch);
pascal void	kernReset(void)		inline(0x0503, udispatch);

#if defined(__ORCAC__) && !defined(__KERN_STATUS)
#define __KERN_STATUS			/* conflict with <gno/kerntool.h> */
pascal int	kernVersion(void)	inline(0x0403, udispatch);
pascal int	kernStatus(void)	inline(0x0603, udispatch);
#endif

/* 0x0703, 0x0803 undocumented */
pascal int	Kgetpid(void)		inline(0x0903, udispatch);
pascal int	Kkill(int pid, int sig) inline (0x0A03, udispatch);
pascal int	Kfork(void *subr) inline(0x0B03, udispatch);
pascal int	Kexec(char *filename,char *cmdline) inline(0x0C03, udispatch);
pascal int	Kswait(int sem) inline(0x0D03, udispatch);
pascal int	Kssignal(int sem) inline(0x0E03, udispatch);
pascal int	Kscreate(int count) inline(0x0F03, udispatch);
pascal int	Ksdelete(int sem) inline(0x1003, udispatch);
/* kvm         1103 thru 1503 */
pascal void *	Ksignal(int sig, void (*func)()) inline(0x1603, udispatch);
pascal int	Kwait(union wait *status) inline(0x1703, udispatch);
pascal int	Ktcnewpgrp(int fdtty) inline(0x1803, udispatch);
pascal int	Ksettpgrp(int fdtty) inline(0x1903, udispatch);
pascal int	Ktctpgrp(int fdtty, int pid) inline(0x1A03, udispatch);
pascal longword	Ksigsetmask(longword mask) inline(0x1B03, udispatch);
pascal longword	Ksigblock(longword mask) inline(0x1C03, udispatch);
# pascal int	K_execve(char *filename,char *cmdline)
			 inline(0x1D03,udispatch);
#pascal longword	Kalarm(longword seconds) inline(0x1E03,udispatch);
pascal int	Ksetdebug(int code) inline(0x1F03,udispatch);
pascal void *	Ksetsystemvector(void *vect) inline(0x2003,udispatch);
pascal int	Ksigpause(longword mask) inline(0x2103,udispatch);

pascal int	Kdup(int filedes)
		Kdup2
		Kpipe
		Kgetpgrp
		Kioctl
		Kstat
		Kfstat
		Klstat
		Kgetuid
		Kgetgid
		Kgeteuid
		Kgetegid
		Ksetuid
		Ksetgid
		Kprocsend
		Kprocreceive
		Krecvclr
		Krecvtim
		Ksetpgrp
		Ktimes
		Kpcreate
		Kpsend
		Kpreceive
		Kpdelete
		Kpreset
		Kpbind
		Kpgetport
		Kpgetcount
		Kscount
		Kfork2

#endif /* _GNO_KERNTOOL_H_ */
