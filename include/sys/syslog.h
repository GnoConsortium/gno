/*
 * Copyright (c) 1982, 1986, 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
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
 *	@(#)syslog.h	8.1 (Berkeley) 6/2/93
 * $Id: syslog.h,v 1.4 1999/01/04 05:09:04 gdr-ftp Exp $
 */

#ifndef _SYS_SYSLOG_H_
#define _SYS_SYSLOG_H_

#ifdef __appleiigs__
#define	_PATH_LOG	".syslog"
#else
#define	_PATH_LOG	"/dev/log"
#endif

/*
 * priorities/facilities are encoded into a single 32-bit quantity, where the
 * bottom 3 bits are the priority (0-7) and the top 28 bits are the facility
 * (0-big number).  Both the priorities and the facilities map roughly
 * one-to-one to strings in the syslogd(8) source code.  This mapping is
 * included in this file.
 *
 * priorities (these are ordered)
 */
#define	LOG_EMERG	0	/* system is unusable */
#define	LOG_ALERT	1	/* action must be taken immediately */
#define	LOG_CRIT	2	/* critical conditions */
#define	LOG_ERR		3	/* error conditions */
#define	LOG_WARNING	4	/* warning conditions */
#define	LOG_NOTICE	5	/* normal but significant condition */
#define	LOG_INFO	6	/* informational */
#define	LOG_DEBUG	7	/* debug-level messages */

#define	LOG_PRIMASK	0x07	/* mask to extract priority part (internal) */
				/* extract priority */
#define	LOG_PRI(p)	((p) & LOG_PRIMASK)
#define	LOG_MAKEPRI(fac, pri)	(((fac) << 3) | (pri))

#ifdef SYSLOG_NAMES
#define	INTERNAL_NOPRI	0x10	/* the "no priority" priority */
				/* mark "facility" */
#define	INTERNAL_MARK	LOG_MAKEPRI(LOG_NFACILITIES, 0)
typedef struct _code {
	char	*c_name;
	long	c_val;
} CODE;

CODE prioritynames[] = {
	"alert",	LOG_ALERT,
	"crit",		LOG_CRIT,
	"debug",	LOG_DEBUG,
	"emerg",	LOG_EMERG,
	"err",		LOG_ERR,
	"error",	LOG_ERR,		/* DEPRECATED */
	"info",		LOG_INFO,
	"none",		INTERNAL_NOPRI,		/* INTERNAL */
	"notice",	LOG_NOTICE,
	"panic", 	LOG_EMERG,		/* DEPRECATED */
	"warn",		LOG_WARNING,		/* DEPRECATED */
	"warning",	LOG_WARNING,
	NULL,		-1,
};
#endif

/*
 * The only code that should ever define __SYSLOG_INTERNALS is syslogd(8) and
 * the syslog(3) implementation in libc.
 */
#ifdef __SYSLOG_INTERNALS

/*
 * Identifier used by the kernel ports interface.  Only the first 32
 * characters are significant!
 */
#define __SYSLOG_PORT_NAME "syslogd"

/*
 * _SYSLOG_BUFFERLEN is the max number of characters that syslog(3) and
 *	vsyslog(3) will log per call.
 * _SYSLOG_BUFFERLEN_MT is the max number of characters that syslogmt(3) and
 *	vsyslogmt(3) will log per call.  This one cannot be large because,
 *	to ensure multithreading, a buffer of this size must be allocated on
 *	the stack.
 */
#define _SYSLOG_BUFFERLEN	1024
#define _SYSLOG_BUFFERLEN_MT	128

/*
 * This _MUST_ be updated any time the SyslogDataBuffer_t definition
 * (or the layout of the rest of the allocated handle region) is changed.
 */
#define _SYSLOG_PROTOCOL_VERSION	0

/*
 * For communcation between user processes and the syslog daemon, a handle
 * is passed between processes, and ownership of that handle (and memory)
 * is transferred to syslogd.
 *
 * The memory region referred to by the passed handle looks like this:
 *
 * This is a structure approximating the structure passed for version zero
 * of the syslog daemon (by Phil Vandry).  It remains here for legacy 
 * purposes.
 *
 * The memory region actually looks like this:
 *	int	version
 *	int	facility/priority
 *	int	number of strings
 *	int	offset of string 1, relative to string 1 (always zero)
 *	int	offset of string 2, relative to string 1 [optional]
 *	...
 *	int	offset of string N, relative to string 1 [optional]
 *	int	(number of bytes to this point? -- apparently unused)
 *	int	length of string 1			\_ Class 1 GS String
 *	char(s)	string 1				/
 *	int	length of string 2 [optional]		\_ Class 1 GS String
 *	char(s)	string 2 [optional]			/
 *	...
 *	int	length of string N [optional]		\_ Class 1 GS String
 *	char(s)	string N [optional]			/
 *
 * Unfortunately, due to the variable nature of the defined memory region,
 * we can't define a struct for the whole thing.  Therefore, we only include
 * the first three words in the structure.
 */

typedef struct SyslogDataBuffer_t {
	int sdb0_version;              /* __SYSLOG_PROTOCOL_VERSION */
	int sdb0_prio;                 /* priority and facility */
	int sdb0_numstrings;           /* number of strings sent to syslogd */
} SyslogDataBuffer_t;

#endif	/* __SYSLOG_INTERNALS */

/* facility codes */
#define	LOG_KERN	0x00	/* (0<<3)	kernel messages */
#define	LOG_USER	0x08	/* (1<<3)	random user-level messages */
#define	LOG_MAIL	0x10	/* (2<<3)	mail system */
#define	LOG_DAEMON	0x18	/* (3<<3)	system daemons */
#define	LOG_AUTH	0x20	/* (4<<3)	security/authorization messages */
#define	LOG_SYSLOG	0x28	/* (5<<3)	messages generated internally by syslogd */
#define	LOG_LPR		0x30	/* (6<<3)	line printer subsystem */
#define	LOG_NEWS	0x38	/* (7<<3)	network news subsystem */
#define	LOG_UUCP	0x40	/* (8<<3)	UUCP subsystem */
#define	LOG_CRON	0x48	/* (9<<3)	clock daemon */
#define	LOG_AUTHPRIV	0x50	/* (10<<3)	security/authorization messages (private) */
#define	LOG_FTP		0x58	/* (11<<3)	ftp daemon */

	/* other codes through 15 reserved for system use */
#define	LOG_LOCAL0	0x80	/* (16<<3)	reserved for local use */
#define	LOG_LOCAL1	0x88	/* (17<<3)	reserved for local use */
#define	LOG_LOCAL2	0x90	/* (18<<3)	reserved for local use */
#define	LOG_LOCAL3	0x98	/* (19<<3)	reserved for local use */
#define	LOG_LOCAL4	0xA0	/* (20<<3)	reserved for local use */
#define	LOG_LOCAL5	0xA8	/* (21<<3)	reserved for local use */
#define	LOG_LOCAL6	0xB0	/* (22<<3)	reserved for local use */
#define	LOG_LOCAL7	0xB8	/* (23<<3)	reserved for local use */

#define	LOG_NFACILITIES	24	/* current number of facilities */
#define	LOG_FACMASK	0x03f8	/* mask to extract facility part */
				/* facility of pri */
#define	LOG_FAC(p)	(((p) & LOG_FACMASK) >> 3)

#ifdef SYSLOG_NAMES
CODE facilitynames[] = {
	"auth",		LOG_AUTH,
	"authpriv",	LOG_AUTHPRIV,
	"cron", 	LOG_CRON,
	"daemon",	LOG_DAEMON,
	"ftp",		LOG_FTP,
	"kern",		LOG_KERN,
	"lpr",		LOG_LPR,
	"mail",		LOG_MAIL,
	"mark", 	INTERNAL_MARK,		/* INTERNAL */
	"news",		LOG_NEWS,
	"security",	LOG_AUTH,		/* DEPRECATED */
	"syslog",	LOG_SYSLOG,
	"user",		LOG_USER,
	"uucp",		LOG_UUCP,
	"local0",	LOG_LOCAL0,
	"local1",	LOG_LOCAL1,
	"local2",	LOG_LOCAL2,
	"local3",	LOG_LOCAL3,
	"local4",	LOG_LOCAL4,
	"local5",	LOG_LOCAL5,
	"local6",	LOG_LOCAL6,
	"local7",	LOG_LOCAL7,
	NULL,		-1,
};
#endif

#ifdef KERNEL
#define	LOG_PRINTF	-1	/* pseudo-priority to indicate use of printf */
#endif

/*
 * arguments to setlogmask.
 */
#define	LOG_MASK(pri)	(1 << (pri))		/* mask for one priority */
#define	LOG_UPTO(pri)	((1 << ((pri)+1)) - 1)	/* all priorities through pri */

/*
 * Option flags for openlog.
 *
 * LOG_ODELAY no longer does anything.
 * LOG_NDELAY is the inverse of what it used to be.
 */
#define	LOG_PID		0x01	/* log the pid with each message */
#define	LOG_CONS	0x02	/* log on the console if errors in sending */
#define	LOG_ODELAY	0x04	/* delay open until first syslog() (default) */
#define	LOG_NDELAY	0x08	/* don't delay open */
#define	LOG_NOWAIT	0x10	/* don't wait for console forks: DEPRECATED */
#define	LOG_PERROR	0x20	/* log to stderr as well */

#ifdef KERNEL

#else /* not KERNEL */

/*
 * Don't use va_list in the vsyslog() prototype.   Va_list is typedef'd in two
 * places (<machine/varargs.h> and <machine/stdarg.h>), so if we include one
 * of them here we may collide with the utility's includes.  It's unreasonable
 * for utilities to have to include one of them to include syslog.h, so we get
 * _BSD_VA_LIST_ from <machine/ansi.h> and use it.
 */
#ifndef _MACHINE_ANSI_H_
#include <machine/ansi.h>
#endif
#ifndef _SYS_CDEFS_H_
#include <sys/cdefs.h>
#endif

__BEGIN_DECLS
void	closelog __P((void));
void	openlog __P((const char *, int, int));
int	setlogmask __P((int));
void	syslog __P((int, const char *, ...));
void	vsyslog __P((int, const char *, _BSD_VA_LIST_));
#ifndef _POSIX_SOURCE
void	old_syslog __P((char **dataHandle));		/* GNO-specific */
void	syslogmt __P((int, const char *, ...));	/* GNO-specific */
void	vsyslogmt __P((int, const char *, _BSD_VA_LIST_)); /* GNO-specific */
#endif
__END_DECLS

#endif /* !KERNEL */

#endif
