/*-
 * Copyright (c) 1982, 1986, 1990, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
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
 *	@(#)ioctl.h	8.6 (Berkeley) 3/28/94
 * $Id: ioctl.h,v 1.1 1997/02/28 04:42:14 gdr Exp $
 */

#ifndef	_SYS_IOCTL_H_
#define	_SYS_IOCTL_H_

#ifndef _SYS_TTYCOM_H_
#include <sys/ttycom.h>
#endif

/*
 * Pun for SunOS prior to 3.2.  SunOS 3.2 and later support TIOCGWINSZ
 * and TIOCSWINSZ (yes, even 3.2-3.5, the fact that it wasn't documented
 * nonwithstanding).
 */
struct ttysize {
	unsigned short	ts_lines;
	unsigned short	ts_cols;
	unsigned short	ts_xxx;
	unsigned short	ts_yyy;
};
#define	TIOCGSIZE	TIOCGWINSZ
#define	TIOCSSIZE	TIOCSWINSZ

#ifndef _SYS_IOCCOM_H_
#include <sys/ioccom.h>
#endif

#ifndef _SYS_FILIO_H_
#include <sys/filio.h>
#endif

#ifndef _SYS_SOCKIO_H_
#include <sys/sockio.h>
#endif

#ifndef KERNEL

#ifndef _SYS_CDEFS_H_
#include <sys/cdefs.h>
#endif

__BEGIN_DECLS
/*
 * non-BSD:  uses "..." instead of "void *"
 */
int	ioctl __P((int, unsigned long, void *));
__END_DECLS
#endif /* !KERNEL */

/*
 * This section appears to be GNO-specific
 */
#if defined(__GNO__) && !defined(_POSIX_SOURCE)

#define FIOCDIROK       _IOW('f', 104, int)
#define TIOCSETK	_IOW('t',19,unsigned int) /* set keyboard map flags */
#define TIOCGETK	_IOR('t',20,unsigned int) /* get keyboard map flags */
#define		OAMAP		0001		/* map OA-key to some sequence */
#define		OA2META		0002		/* map OA-key to meta-key */
#define		OA2HIBIT	0004		/* map OA-key to key|0x80 */
#define		VT100ARROW	0010		/* map arrows to vt100 arrows */
#define TIOCSHUP	_IOW('t',21,unsigned int) /* set sighup control flags */
#define TIOCGHUP	_IOR('t',22,unsigned int) /* get sighup control flags */
#define		CDxNIL		0x00
#define		CDxGPI		0x08		
#define		CDxCTS          0x20
#define TIOCSVECT	_IOW('t',23,unsigned long *) /* set console i/o vectors */
#define TIOCGVECT	_IOR('t',24,unsigned long *) /* get console i/o vectors */

/* CS_RPAUSE */
/* Disk space resource pause per-file control */
#define FIOCNOSPC       _IOWR('f', 106, int)
#define FIOCNOSPC_SAME  0                       /* no change */
#define FIOCNOSPC_ERROR 1                       /* prohibit pause */
#define FIOCNOSPC_PAUSE 2                       /* allow pause */
/* CS_RPAUSE */

#endif /* __GNO__ && !_POSIX_SOURCE */

#endif /* !_SYS_IOCTL_H_ */

/*
 * Keep outside _SYS_IOCTL_H_
 * Compatability with old terminal driver
 *
 * Source level -> define USE_OLD_TTY
 * Kernel level -> options COMPAT_43 or COMPAT_SUNOS
 */
#if defined(USE_OLD_TTY) || defined(COMPAT_43) || defined(COMPAT_SUNOS)
#ifndef _SYS_IOCTL_COMPAT_H_
#include <sys/ioctl.compat.h>
#endif
#endif
