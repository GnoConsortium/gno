/*
 * ToolBox interface file for the GNO version 1.0.6 Kernel.
 *
 * Copyright 1991-1996, Procyon Inc.
 *
 * $Id: kerntool.h,v 1.3 1998/04/29 03:05:40 gdr-ftp Exp $
 *
 * This file is formatted for tab stops every 8 columns.
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

#define __INLINE(arg) inline(arg,udispatch)

/*
 * There are no kerntool traps for the following:
 *	Ksetruid	- implemented in terms of Ksetreuid()
 *	Kseteuid	- implemented in terms of Ksetreuid()
 *	Ksetrgid	- implemented in terms of Ksetregid()
 *	Ksetegid	- implemented in terms of Ksetregid()
 *	Kvfork		- implemented in terms of Kfork()
 */

pascal void	kernBootInit(void)			__INLINE(0x0103);
pascal void	kernStartUp(void)			__INLINE(0x0203);
pascal void	kernShutDown(void)			__INLINE(0x0303);
pascal void	kernReset(void)				__INLINE(0x0503);

#if defined(__ORCAC__) && !defined(__KERN_STATUS)
#define __KERN_STATUS			/* conflict with <gno/gno.h> */
pascal int	kernVersion(void)			__INLINE(0x0403);
pascal int	kernStatus(void)			__INLINE(0x0603);
#endif

/* 0x0703, 0x0803 undocumented */
pascal int	Kgetpid(void)				__INLINE(0x0903);
pascal int	Kkill(int pid, int sig, int *errno)	__INLINE(0x0A03);
pascal int	Kfork(void *subr, int *errno)		__INLINE(0x0B03);
/*		Kexec() no longer available in this interface	(0x0C03) */
pascal int	Kswait(int sem, int *errno)		__INLINE(0x0D03);
pascal int	Kssignal(int sem, int *errno)		__INLINE(0x0E03);
pascal int	Kscreate(int count, int *errno)		__INLINE(0x0F03);
pascal int	Ksdelete(int sem, int *errno)		__INLINE(0x1003);

#ifdef _GNO_KVM_H_
pascal int	Kkvm_open(int *errno)			__INLINE(0x1103);
pascal int	Kkvm_close(kvmt *k, int *errno)		__INLINE(0x1203);
pascal struct pentry *Kkvm_getproc(kvmt *kd, int pid, int *errno)
							__INLINE(0x1303);
pascal struct pentry *Kkvm_nextproc(kvmt *kd, int *err)	__INLINE(0x1403);
pascal int	Kkvm_setproc(kvmt *kd)			__INLINE(0x1503);
#endif

#ifdef _SYS_SIGNAL_H_
pascal void *	Ksignal(int sig, sig_t func, int *err)	__INLINE(0x1603);
#endif

#ifdef _SYS_WAIT_H_
pascal int	Kwait(union wait *status, int *errno)	__INLINE(0x1703);
#endif

pascal int	Ktcnewpgrp(int fdtty, int *errno)	__INLINE(0x1803);
pascal int	Ksettpgrp(int fdtty, int *errno)	__INLINE(0x1903);
pascal int	Ktctpgrp(int fdtty, int pid, int *err)	__INLINE(0x1A03);
pascal longword	Ksigsetmask(longword mask, int *errno)	__INLINE(0x1B03);
pascal longword	Ksigblock(longword mask, int *errno)	__INLINE(0x1C03);
pascal int	K_execve(char *file, char *cmdline, int *err)
							__INLINE(0x1D03);
pascal longword	Kalarm(longword seconds, int *errno)	__INLINE(0x1E03);
pascal int	Ksetdebug(int code)			__INLINE(0x1F03);
pascal void *	Ksetsystemvector(void *vect)		__INLINE(0x2003);
pascal int	Ksigpause(longword mask, int *errno)	__INLINE(0x2103);
pascal int	Kdup(int oldfd, int *errno)		__INLINE(0x2203);
pascal int	Kdup2(int oldfd, int newfd, int *errno)	__INLINE(0x2303);
pascal int	Kpipe(int *fildes, int *errno)		__INLINE(0x2403);
pascal pid_t	K_getpgrp(pid_t pid, int *errno)	__INLINE(0x2503);
pascal int	Kioctl(int d, unsigned long req, void *ptr, int *errno)
							__INLINE(0x2603);

#ifdef _SYS_STAT_H_
pascal int	Kstat(const char *path, struct stat *sbuf, int *errno)
							__INLINE(0x2703);
pascal int	Kfstat(int fd, struct stat *sbuf, int *errno)
							__INLINE(0x2803);
pascal int	Klstat(const char *path, struct stat *sbuf, int *errno)
							__INLINE(0x2903);
#endif	/* _SYS_STAT_H_ */

pascal int	Kgetuid(int *errno)			__INLINE(0x2A03);
pascal int	Kgetgid(int *errno)			__INLINE(0x2B03);
pascal int	Kgeteuid(int *errno)			__INLINE(0x2C03);
pascal int	Kgetegid(int *errno)			__INLINE(0x2D03);
pascal int	Ksetuid(uid_t uid, int *errno)		__INLINE(0x2E03);
pascal int	Ksetgid(gid_t gid, int *errno)		__INLINE(0x2F03);
pascal int	Kprocsend(pid_t pid, unsigned long msg, int *errno)
							__INLINE(0x3003);
pascal unsigned long Kprocreceive(int *errno)		__INLINE(0x3103);
pascal unsigned long Kprocrecvclr(int *errno)		__INLINE(0x3203);
pascal unsigned long Kprocrecvtim(short timeout, int *errno)
							__INLINE(0x3303);
pascal int	Ksetpgrp(pid_t pid, pid_t pgrp, int *errno)
							__INLINE(0x3403);

#ifdef _SYS_TIMES_
pascal clock_t	Ktimes(struct tms *tp, int *errno)	__INLINE(0x3503);
#endif

pascal int	Kpcreate(int count, int *errno)		__INLINE(0x3603);
pascal int	Kpsend(int portid, int *errno)		__INLINE(0x3703);
pascal long	Kpreceive(int portid, int *errno)	__INLINE(0x3803);
pascal int	Kpdelete(int portid, int (*dispose)(long), int *errno)
							__INLINE(0x3903);
pascal int	Kpreset(int portid, int (*dispose)(long), int *errno)
							__INLINE(0x3A03);
pascal int	Kpbind(int portid, const char *name, int *errno)
							__INLINE(0x3B03);
pascal int	Kpgetport(const char *name, int *errno)	__INLINE(0x3C03);
pascal int	Kpgetcount(int portid, int *errno)	__INLINE(0x3D03);
pascal int	Kscount(int sem, int *errno)		__INLINE(0x3E03);
/*		Kfork2() not available in this interface	(0x3F03) */
pascal int	Kgetppid(int *errno)			__INLINE(0x4003);
pascal void	KSetGNOQuitRec(word pCount, GSStringPtr path, word flags,
			       int *errno)		__INLINE(0x4103);
pascal longword	Kalarm10(longword seconds, int *errno)	__INLINE(0x4203);
pascal int	Kselect(int nfds, fd_set *readfds, fd_set *writefds,
			fd_set *exceptfds, struct timeval *timeout)
							__INLINE(0x4303);
pascal int	KInstallNetDriver(void *netcore, unsigned int domain,
				  int *errno)		__INLINE(0x4403);
pascal int	Ksocket(int domain, int type, int protocol, int *errno)
							__INLINE(0x4503);

#ifdef _SYS_SOCKET_H_
pascal int	Kbind(int s, struct sockaddr *name, int namelen, int *errno)
							__INLINE(0x4603);
pascal int	Kconnect(int s, struct sockaddr *name, int namelen, int *errno)
							__INLINE(0x4703);
pascal int	Klisten(int s, int backlog, int *errno)	__INLINE(0x4803);
pascal int	Kaccept(int s, struct sockaddr *addr, int *addrlen, int *errno)
							__INLINE(0x4903);
pascal int	Krecvfrom(int s, void *buf, size_t len, unsigned int flags,
			  struct sockaddr *from, int *fromlen, int *errno)
							__INLINE(0x4A03);
pascal int	Ksendto(int s, const void *msg, size_t len, int flags,
			const struct sockaddr *to, unsigned int tolen,
			int *errno)			__INLINE(0x4B03);
pascal int	Krecv(int s, void *buf, size_t len, unsigned int flags,
		      int *errno)			__INLINE(0x4C03);
pascal int	Ksend(int s, const void *msg, size_t len, unsigned int flags,
		      int *errno)			__INLINE(0x4D03);
pascal int	Kgetpeername(int s, struct sockaddr *name, int *namelen,
			     int *errno)		__INLINE(0x4E03);
pascal int	Kgetsockname(int s, struct sockaddr *name, int *namelen,
			     int *errno)		__INLINE(0x4F03);
pascal int	Kgetsockopt(int s, int level, int optname, void *optval,
			    int *optlen, int *errno)	__INLINE(0x5003);
pascal int	Ksetsockopt(int s, int level, int optname, const void *optval,
			    int optlen, int *errno)	__INLINE(0x5103);
#endif	/* _SYS_SOCKET_H_ */

pascal int	Kshutdown(int s, int how, int *errno)	__INLINE(0x5203);
pascal int	Ksetreuid(uid_t uid, uid_t euid, int *errno) __INLINE(0x5303);
pascal int	Ksetregid(gid_t gid, gid_t egid, int *errno) __INLINE(0x5403);


/* Not yet implemented:
pascal int	Ksendmsg (int s, const struct msghdr *msg, unsigned int flags);
pascal int	Krecvmsg(int s, struct msghdr *msg, unsigned int flags);
 */

#undef __INLINE
#endif /* _GNO_KERNTOOL_H_ */


