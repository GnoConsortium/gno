/*
 * Copyright (c) 1982, 1986, 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are /*
 * Copyright (c) 1982, 1986, 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are y the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)errno.h	8.5 (Berkeley) 1/21/94
 * $Id: errno.h,v 1.1 1997/02/28 04:42:14 gdr Exp $
 */

#ifndef _SYS_ERRNO_H_
#define _SYS_ERRNO_H_

#ifndef KERNEL
extern int errno;			/* global error number */
#endif

/* These are provided by both the GNO kernel and Orca/Shell */
#define	EDOM		1		/* Numerical argument out of domain */
#define	ERANGE		2		/* Result too large */
#define	ENOMEM		3		/* Cannot allocate memory */
#define	ENOENT		4		/* No such file or directory */
#define	EIO		5		/* Input/output error */
#define	EINVAL		6		/* Invalid argument */
#define	EBADF		7		/* Bad file descriptor */
#define	EMFILE		8		/* Too many open files */
#define	EACCES		9		/* Permission denied -- POSIX	*/
					/* The Orca/C <errno.h> uses	*/
					/*    EACCESS for this value,	*/
					/*    but that macro conflicts	*/
					/*    with <arpa/tftp.h>.	*/
#define	EEXIST		10		/* File exists */
#define	ENOSPC		11		/* No space left on device */

/* These are only provided by the GNO kernel. */
#define	EPERM		12		/* Operation not permitted */
#define	ESRCH		13		/* No such process */
#define	EINTR		14		/* Interrupted system call */
#define	E2BIG		15		/* Argument list too long */
#define	ENOEXEC		16		/* Exec format error */
#define	ECHILD		17		/* No child processes */
#define	EAGAIN		18		/* Resource temporarily unavailable */
#define	ENOTDIR		19		/* Not a directory */
#define	ENOTTY		20		/* Inappropriate ioctl for device */
#define	EPIPE		21		/* Broken pipe */
#define	ESPIPE		22		/* Illegal seek */
#ifndef _POSIX_SOURCE
#define	ENOTBLK		23		/* Block device required */
#endif
#define	EISDIR		24		/* Is a directory */

/* ipc/network software -- argument errors */
#ifndef _POSIX_SOURCE
#define	ENOTSOCK	25		/* Socket operation on non-socket */
#define	EDESTADDRREQ	26		/* Destination address required */
#define	EMSGSIZE	27		/* Message too long */
#define	EPROTOTYPE	28		/* Protocol wrong type for socket */
#define	ENOPROTOOPT	29		/* Protocol not available */
#define	EPROTONOSUPPORT	30		/* Protocol not supported */
#define	ESOCKTNOSUPPORT	31		/* Socket type not supported */
#define	EOPNOTSUPP	32		/* Operation not supported on socket */
#define	EPFNOSUPPORT	33		/* Protocol family not supported */
#define	EAFNOSUPPORT	34		/* Address family not supported by protocol family */
#define	EADDRINUSE	35		/* Address already in use */
#define	EADDRNOTAVAIL	36		/* Can't assign requested address */

/* ipc/network software -- operational errors */
#define	ENETDOWN	37		/* Network is down */
#define	ENETUNREACH	38		/* Network is unreachable */
#define	ENETRESET	39		/* Network dropped connection on reset */
#define	ECONNABORTED	40		/* Software caused connection abort */
#define	ECONNRESET	41		/* Connection reset by peer */
#define	ENOBUFS		42		/* No buffer space available */
#define	EISCONN		43		/* Socket is already connected */
#define	ENOTCONN	44		/* Socket is not connected */
#define	ESHUTDOWN	45		/* Can't send after socket shutdown */
#define	ETOOMANYREFS	46		/* Too many references: can't splice */
#define	ETIMEDOUT	47		/* Connection timed out */
#define	ECONNREFUSED	48		/* Connection refused */

/* non-blocking and interrupt i/o */
#define EWOULDBLOCK	49		/* Operation would block */
#define EINPROGRESS	50		/* Operation now in progress */
#define EALREADY	51		/* Operation already in progress */
#endif	/* _POSIX_SOURCE */

#define EFAULT		52		/* this one should be up top */
#define ENODEV		53		/* this one should be up top */

#ifndef _POSIX_SOURCE
#define EHOSTDOWN	54		/* Host is down */
#define EHOSTUNREACH	55		/* No route to host */
#define	ELAST		55		/* Must be equal largest errno */
#endif

/*
 * The remainder of these values are not currently used by GNO.
 */
#if 0
#define	ENXIO		56		/* Device not configured */
#define	EDEADLK		57		/* Resource deadlock avoided */
#define	EBUSY		58		/* Device busy */
#define	EXDEV		59		/* Cross-device link */
#define	ENFILE		60		/* Too many open files in system */
#ifndef _POSIX_SOURCE
#define	ETXTBSY		61		/* Text file busy */
#endif
#define	EFBIG		62		/* File too large */
#define	EROFS		63		/* Read-only file system */
#define	EMLINK		64		/* Too many links */
#ifndef _POSIX_SOURCE
#define	ELOOP		65		/* Too many levels of symbolic links */
#endif
#define	ENAMETOOLONG	66		/* File name too long */
#define	ENOTEMPTY	67		/* Directory not empty */
#define	ENOLCK		68		/* No locks available */
#define	ENOSYS		69		/* Function not implemented */

/* quotas & mush */
#ifndef _POSIX_SOURCE
#define	EPROCLIM	70		/* Too many processes */
#define	EUSERS		71		/* Too many users */
#define	EDQUOT		72		/* Disc quota exceeded */

/* Network File System */
#define	ESTALE		73		/* Stale NFS file handle */
#define	EREMOTE		74		/* Too many levels of remote in path */
#define	EBADRPC		75		/* RPC struct is bad */
#define	ERPCMISMATCH	76		/* RPC version wrong */
#define	EPROGUNAVAIL	77		/* RPC prog. not avail */
#define	EPROGMISMATCH	78		/* Program version wrong */
#define	EPROCUNAVAIL	79		/* Bad procedure for program */

#define	EFTYPE		80		/* Inappropriate file type or format */
#define	EAUTH		81		/* Authentication error */
#define	ENEEDAUTH	82		/* Need authenticator */
#define	ELAST		82		/* Must be equal largest errno */
#endif	/* _POSIX_SOURCE */

#ifdef KERNEL
/* pseudo-errors returned inside kernel to modify return to process */
#define	ERESTART	-1		/* restart syscall */
#define	EJUSTRETURN	-2		/* don't modify regs, just return */
#endif

#endif /* 0 */
#endif /* _SYS_ERRNO_H_ */
