/*
 * Copyright (c) 1983, 1988, 1993
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
 */

#ifdef __ORCAC__
segment "libc_gen__";
#endif

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)syslog.c	8.4 (Berkeley) 3/18/94";
#endif /* LIBC_SCCS and not lint */

#define USE_PORTS		/* use ports mech rather than TCP/IP */
#define USE_VZERO		/* use syslogd v0 protocol a la GNO 2.0.4 */
#define __SYSLOG_INTERNALS	/* needed for the ports interface */

#ifndef USE_PORTS
#include <sys/types.h>
#include <sys/socket.h>
#endif

#include <sys/syslog.h>
#include <sys/uio.h>

#ifndef USE_PORTS
#include <netdb.h>
#endif

#include <errno.h>
#include <fcntl.h>
#include <paths.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifdef __GNO__
#include <types.h>
#include <gno/gno.h>
#include <misctool.h>
#include <memory.h>
#endif

#if __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#ifdef __ORCAC__
#define STATIC static
#else
#define STATIC
#endif

static int	LogFile = -1;		/* fd for log */
static int	connected;		/* have done connect */
static int	LogStat = 0;		/* status bits, set by openlog() */
static const char *LogTag = NULL;	/* string to tag the entry with */
static int	LogFacility = LOG_USER;	/* default facility code */
static int	LogMask = 0xff;		/* mask of priorities to be logged */
#ifndef __GNO__	/* get __prognameGS() from <gno/gno.h>
extern char	*__progname;		/* Program name, from crt0. */
#endif

#ifdef USE_PORTS
#include	<sys/ports.h>
static int	openPort(void);
static int	sendPort(int port, int pri, const void *buf, int len);
static void	closePort(int port);
#endif

/*
 * Format of the magic cookie passed through the stdio hook
 */
struct bufcookie {
	char	*base;	/* start of buffer */
	int	left;
};

/*
 * stdio write hook for writing to a static string buffer
 * XXX: Maybe one day, dynamically allocate it so that the line length
 *      is `unlimited'.
 */
static ssize_t writehook(
	void	*cookie,	/* really [struct bufcookie *] */
	const char *buf,	/* characters to copy */
	size_t	len)		/* length to copy */
{
	struct bufcookie *h;	/* private `handle' */

	h = (struct bufcookie *)cookie;
	if (len > h->left) {
		/* clip in case of wraparound */
		len = h->left;
	}
	if (len > 0) {
		(void)memcpy(h->base, buf, len); /* `write' it. */
		h->base += len;
		h->left -= len;
	}
	return 0;
}

/*
 * syslog, vsyslog --
 *	print message on log file; output is intended for syslogd(8).
 * syslogmt, vsyslogmt --
 *	These are GNO-specific multithreading safe versions of the
 *	above two routines.  Note that the allowed format specifier
 *	set is more limited.  See the man page for details.
 */

void
vsyslog(int pri, const char *fmt, va_list ap)
{
	register int cnt;
	register char ch, *p, *t;
#ifndef USE_VZERO
	time_t now;
#endif
	int fd, saved_errno;
	char *stdp;
	STATIC char tbuf[2048], fmt_cpy[1024];
	FILE *fp, *fmt_fp;
	struct bufcookie tbuf_cookie;
	struct bufcookie fmt_cookie;

#define	INTERNALLOG	LOG_ERR|LOG_CONS|LOG_PERROR|LOG_PID
	/* Check for invalid bits. */
	if (pri & ~(LOG_PRIMASK|LOG_FACMASK)) {
		syslog(INTERNALLOG,
		    "syslog: unknown facility/priority: %x", pri);
		pri &= LOG_PRIMASK|LOG_FACMASK;
	}

	/* Check priority against setlogmask values. */
	if (!(LOG_MASK(LOG_PRI(pri)) & LogMask))
		return;

	saved_errno = errno;

	/* Set default facility if none specified. */
	if ((pri & LOG_FACMASK) == 0)
		pri |= LogFacility;

	/* Create the primary stdio hook */
	tbuf_cookie.base = tbuf;
	tbuf_cookie.left = sizeof(tbuf);
	fp = fwopen(&tbuf_cookie, writehook);
	if (fp == NULL)
		return;

	/* Build the message. */
#ifndef USE_VZERO
	/* 
	 * If we're using Phil's syslogd, we don't need to prepend the
	 * <facpri> or date stamp.  The facpri is provided separately and
	 * the date stamp is determined by the daemon.
	 */
	(void)time(&now);
	(void)fprintf(fp, "<%d>", pri);
	(void)fprintf(fp, "%.15s ", ctime(&now) + 4);
#endif
	if (LogStat & LOG_PERROR) {
		/* Transfer to string buffer */
		(void)fflush(fp);
		stdp = tbuf + (sizeof(tbuf) - tbuf_cookie.left);
	}
	if (LogTag == NULL)
#ifdef __GNO__
		LogTag = __prognameGS();
#else
		LogTag = __progname;
#endif
	if (LogTag != NULL)
		(void)fprintf(fp, "%s", LogTag);
	if (LogStat & LOG_PID)
		(void)fprintf(fp, "[%d]", getpid());
	if (LogTag != NULL) {
		(void)fprintf(fp, ": ");
	}

	/* Check to see if we can skip expanding the %m */
	if (strstr(fmt, "%m")) {

		/* Create the second stdio hook */
		fmt_cookie.base = fmt_cpy;
		fmt_cookie.left = sizeof(fmt_cpy) - 1;
		fmt_fp = fwopen(&fmt_cookie, writehook);
		if (fmt_fp == NULL) {
			fclose(fp);
			return;
		}

		/* Substitute error message for %m. */
		for ( ; ch = *fmt; ++fmt)
			if (ch == '%' && fmt[1] == 'm') {
				++fmt;
				fputs(strerror(saved_errno), fmt_fp);
			} else
				fputc(ch, fmt_fp);

		/* Null terminate if room */
		fputc(0, fmt_fp);
		fclose(fmt_fp);

		/* Guarantee null termination */
		fmt_cpy[sizeof(fmt_cpy) - 1] = '\0';

		fmt = fmt_cpy;
	}

	(void)vfprintf(fp, fmt, ap);
	(void)fclose(fp);

	cnt = sizeof(tbuf) - tbuf_cookie.left;

	/* Output to stderr if requested. */
	if (LogStat & LOG_PERROR) {
#ifdef BROKEN_WRITEV
		write(STDERR_FILENO, stdp, cnt - (stdp - tbuf));
		write(STDERR_FILENO, "\r", 1);
#else
		struct iovec iov[2];
		register struct iovec *v = iov;

		v->iov_base = stdp;
		v->iov_len = cnt - (stdp - tbuf);
		++v;
#ifdef __appleiigs__
		v->iov_base = "\r";
#else
		v->iov_base = "\n";
#endif
		v->iov_len = 1;
		(void)writev(STDERR_FILENO, iov, 2);
#endif	/* BROKEN_WRITEV */
	}

	/* Get connected, output the message to the local logger. */
	if (!connected)
		openlog(LogTag, LogStat | LOG_NDELAY, 0);
#ifdef USE_PORTS
	if (sendPort(LogFile, pri, tbuf, cnt) >= 0)
		return;
#else
	if (send(LogFile, tbuf, cnt, 0) >= 0)
		return;
#endif

	/*
	 * Output the message to the console; don't worry about blocking,
	 * if console blocks everything will.  Make sure the error reported
	 * is the one from the syslogd failure.
	 */
	 
#ifdef __ORCAC__	/* watch for stack trashing */
#define	OPEN(path, flags, mode) open(path, flags)
#else
#define OPEN(path, flags, mode) open(path, flags, mode)
#endif
	if (LogStat & LOG_CONS &&
	    (fd = OPEN(_PATH_CONSOLE, O_WRONLY, 0)) >= 0) {
#undef OPEN
#ifdef BROKEN_WRITEV
		p = strchr(tbuf, '>') + 1;
		write(fd, p, cnt - (p - tbuf));
		write(fd, "\r", 1);
#else
		struct iovec iov[2];
		register struct iovec *v = iov;

		p = strchr(tbuf, '>') + 1;
		v->iov_base = p;
		v->iov_len = cnt - (p - tbuf);
		++v;
#ifdef __appleiigs__
		v->iov_base = "\r";
		v->iov_len = 1;
#else
		v->iov_base = "\r\n";
		v->iov_len = 2;
#endif
		(void)writev(fd, iov, 2);
#endif	/* BROKEN_WRITEV */
		(void)close(fd);
	}
}

#ifdef __GNO__
/*
 * Copy a maximum of <maxcnt> characters from <src> into the buffer
 * pointed to by mt_bufptr.  Decrement the variable mt_bytesLeft by
 * the number of characters copied.  The copy will stop either when
 * <maxcnt> characters have been copied, a NULL-terminator is found
 * in <src>, or when mt_bytesLeft reaches zero.
 *
 * mt_bufptr is left pointing at the (inserted) NULL-terminator at
 * the end of the copied string.
 *
 * Implicitly uses the variables mt_bufptr, mt_bytesLeft, p, and i,
 * which are local to vsyslogmt().
 */
#define STRNCPY2BUF(src,maxcnt) \
{ \
	i = maxcnt; \
	p = (src); \
	while ((i>0) && (mt_bytesLeft > 0) && (*p != '\0')) { \
		*mt_bufptr++ = *p++; \
		i--; \
		mt_bytesLeft--; \
	} \
	*mt_bufptr = '\0'; \
}

/*
 * STRCPY2BUF is to STRNCPY2BUF like strcpy() is to strncpy().
 * Does not use the local variable 'i'.
 */
#define STRCPY2BUF(src) \
{ \
	p = (src); \
	while ((mt_bytesLeft > 0) && (*p != '\0')) { \
		*mt_bufptr++ = *p++; \
		mt_bytesLeft--; \
	} \
	*mt_bufptr = '\0'; \
}

void
vsyslogmt(int pri, const char *fmt, va_list ap)
{
	char mt_buffer[_SYSLOG_BUFFERLEN_MT];	/* defined in <sys/syslog.h> */
	char *mt_bufptr;
	int mt_bytesLeft;
	int i;
	char *local_LogTag;
	int local_LogFile;
	char *p;
	char *stdp;
#ifndef USE_VZERO
	time_t now;
#endif
	int fd, saved_errno;
#define MT_BYTES_USED	(_SYSLOG_BUFFERLEN_MT - mt_bytesLeft)

	/* Check for invalid bits. */
	if (pri & ~(LOG_PRIMASK|LOG_FACMASK)) {
		syslogmt(INTERNALLOG,
		    "syslog: unknown facility/priority: %x", pri);
		pri &= LOG_PRIMASK|LOG_FACMASK;
	}

	/* Check priority against setlogmask values. */
	if (!(LOG_MASK(LOG_PRI(pri)) & LogMask))
		return;

	saved_errno = errno;

	/* Set default facility if none specified. */
	if ((pri & LOG_FACMASK) == 0)
		pri |= LogFacility;

	/* Build the message. */
	mt_bufptr = mt_buffer;
	mt_bytesLeft = _SYSLOG_BUFFERLEN_MT;	/* not off-by-one */
#ifndef USE_VZERO
	/* 
	 * If we're using Phil's syslogd, we don't need to prepend the
	 * <facpri> or date stamp.  The facpri is provided separately and
	 * the date stamp is determined by the daemon.
	 */
	time(&now);
	p = sprintmt(mt_bufptr, mt_bytesLeft, "<%d>", pri);
	mt_bytesLeft -= (p - mt_bufptr);
	mt_bufptr = p;
#if 0
/* %%% ctime is not thread safe %%% */
	STRNCPY2BUF(ctime(&now)+4, ((mt_bytesLeft < 15) ? mt_bytesLeft : 15));
	STRCPY2BUF(" ");
#endif
#endif	/* USE_VZERO */

	/* mark the beginning of the string for stderr, if necessary */
	if (LogStat & LOG_PERROR) {
		stdp = mt_bufptr;
	}

	/*
	 * Unlike this section of code in vsyslog(), we cannot set
	 * LogTag if it is NULL.  In fact, we can't even call __prognameGS(),
	 * because that routine is not thread safe.  We _can_ reference
	 * the string __progname, but it may not be initialized to anything
	 * but "(unknown)" at this point.
	 *
	 * Note that if someone called openlog() in a non-multithreaded
	 * portion of their code prior to this point, that LogTag will
	 * be suitably initialized, anyway.
	 */
	local_LogTag = LogTag;
	if (local_LogTag == NULL) {
		local_LogTag = __progname;
	}
	if (local_LogTag != NULL) {
		STRCPY2BUF(local_LogTag);
	}
	if (LogStat & LOG_PID) {
		p = sprintmt(mt_bufptr, mt_bytesLeft, "[%d]", getpid());
		mt_bytesLeft -= (p - mt_bufptr);
		mt_bufptr = p;
	}
	if (local_LogTag != NULL) {
		STRCPY2BUF(": ");
	}

	/*
	 * Unlike the code in vsyslog(), we don't bother to check for "%m"
	 * here, because vsprintmt will expand that one for us.
	 *
	 * We do, however, have to restore errno, since that may have been
	 * corrupted in our code above.
	 */
	errno = saved_errno;

	/* 
	 * expand the user's format string 
	 */
	p = vsprintmt(mt_bufptr, mt_bytesLeft, fmt, ap);
	mt_bytesLeft -= (p - mt_bufptr);
	mt_bufptr = p;

	/* Output to stderr if requested. */
	if (LogStat & LOG_PERROR) {
#ifdef BROKEN_WRITEV
		write(STDERR_FILENO, stdp, MT_BYTES_USED - (stdp - mt_buffer));
		write(STDERR_FILENO, "\r", 1);
#else
		struct iovec iov[2];
		register struct iovec *v = iov;

		v->iov_base = stdp;
		v->iov_len = MT_BYTES_USED - (stdp - mt_buffer);
		++v;
#ifdef __appleiigs__
		v->iov_base = "\r";
#else
		v->iov_base = "\n";
#endif
		v->iov_len = 1;
		(void)writev(STDERR_FILENO, iov, 2);
#endif	/* BROKEN_WRITEV */
	}

	/* Get connected, output the message to the local logger. */
#ifdef USE_PORTS
	local_LogFile = LogFile;
	if (local_LogFile == -1) {	/* an openlog wasn't done? */
		local_LogFile = openPort();
	}
	if (sendPort(local_LogFile, pri, mt_buffer, MT_BYTES_USED) >= 0) {
		return;
	}
#else	/* not USE_PORTS -- no output if someone forgot to openlog() */
	if (!connected)
		openlog(LogTag, LogStat | LOG_NDELAY, 0);
	if (send(LogFile, mt_buffer, MT_BYTES_USED, 0) >= 0)
		return;
#endif

	/*
	 * Output the message to the console; don't worry about blocking,
	 * if console blocks everything will.  Make sure the error reported
	 * is the one from the syslogd failure.
	 */
	 
#ifdef __ORCAC__	/* watch for stack trashing */
#define	OPEN(path, flags, mode) open(path, flags)
#else
#define OPEN(path, flags, mode) open(path, flags, mode)
#endif
	if (LogStat & LOG_CONS &&
	    (fd = OPEN(_PATH_CONSOLE, O_WRONLY, 0)) >= 0) {
#undef OPEN
#ifdef BROKEN_WRITEV
		p = strchr(mt_buffer, '>') + 1;
		write(fd, p, MT_BYTES_USED - (p - mt_buffer));
		write(fd, "\r", 1);
#else
		struct iovec iov[2];
		register struct iovec *v = iov;

		p = strchr(mt_buffer, '>') + 1;
		v->iov_base = p;
		v->iov_len = MT_BYTES_USED - (p - mt_buffer);
		++v;
#ifdef __appleiigs__
		v->iov_base = "\r";
		v->iov_len = 1;
#else
		v->iov_base = "\r\n";
		v->iov_len = 2;
#endif
		(void)writev(fd, iov, 2);
#endif	/* BROKEN_WRITEV */
		(void)close(fd);
	}
}
#endif /* __GNO__ */


#ifndef USE_PORTS
static struct sockaddr SyslogAddr;	/* AF_UNIX address of local logger */
#endif

void
openlog(const char *ident, int logstat, int logfac)
{
	if (ident != NULL)
		LogTag = ident;
	LogStat = logstat;
	if (logfac != 0 && (logfac &~ LOG_FACMASK) == 0)
		LogFacility = logfac;

	if (LogFile == -1) {
#ifdef USE_PORTS
		if (LogStat & LOG_NDELAY) {
			if ((LogFile = openPort()) == -1) {
				return;
			}
		}
#else	/* not USE_PORTS */
		SyslogAddr.sa_family = AF_UNIX;
		(void)strncpy(SyslogAddr.sa_data, _PATH_LOG,
		    sizeof(SyslogAddr.sa_data));
		if (LogStat & LOG_NDELAY) {
			if ((LogFile = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
				return;
			(void)fcntl(LogFile, F_SETFD, 1);
		}
#endif	/* not USE_PORTS */
	}
	if (LogFile != -1 && !connected)
#ifdef USE_PORTS
		connected = 1;
#else
		if (connect(LogFile, &SyslogAddr, sizeof(SyslogAddr)) == -1) {
			(void)close(LogFile);
			LogFile = -1;
		} else
			connected = 1;
#endif
}

void
closelog(void)
{
#ifdef USE_PORTS
	closePort(LogFile);
#else
	(void)close(LogFile);
#endif
	LogFile = -1;
	connected = 0;
}

/* setlogmask -- set the log mask level */
int
setlogmask(int pmask)
{
	int omask;

	omask = LogMask;
	if (pmask != 0)
		LogMask = pmask;
	return (omask);
}

#ifdef USE_PORTS
static int
openPort(void) {
	/* may return -1 */
	return pgetport(__SYSLOG_PORT_NAME);
}

static int
sendPort(int port, int pri, const void *buf, int len)
{
	Handle datahand;
	Word memID;
	char *buffer, *strptr;
	int saved_errno, saved_toolerr;
	SyslogDataBuffer_t *header;
	int *lenptr, *offptr, *magicptr, length;

	if (port == -1) {
		/* not a valid port; silent error */
		return 0;
	}

	/* 
	 * We don't have to worry about this process exiting before the
	 * syslogd has a chance to change the ownership of the allocated
	 * memory because we do a busy-wait before returning.  See the end
	 * of this routine.
	 *
	 * In order to avoid the busy wait, we would have to allocate a new
	 * user id for every message and ensure that the temporary user id
	 * was deleted after use.  That is probably more expensive than just
	 * doing the busy-wait.
	 *
	 * We could probably increase concurrency, though, if in syslogd we
	 * queued up all incoming buffers after changing their ownership, 
	 * then writing out the buffers only after no more processes are
	 * attempting to send messages to syslogd.  With such a mechanism
	 * in place, one could also optimize out a bunch of extraneous open(2)
	 * and close(2) calls on the log files.
	 */
	memID = _ownerid;

	/*
	 * Allocate the memory.  For v0 messages, We need:
	 *	sizeof(SyslogDataBuffer_t) for version, prio, and numstrings
	 *	sizeof(int) * 1		for the offset value
	 *	sizeof(int) * 1		for the "sizeof struct" value
	 *	sizeof(int) * 1		for the length word of the first str
	 *	len			for the characters in buf
	 *	1			for a terminating NULL byte at the end
	 *				of buf -- expected by Phil's syslogd
	 */
	datahand = NewHandle(sizeof(SyslogDataBuffer_t) +  (3 * sizeof(int))
			     + len +1, memID, 0x4000, NULL);
	if (_toolErr) {
		saved_toolerr = _toolErr;
		DeleteID(memID);
		_toolErr = saved_toolerr;
		errno = _mapErr(_toolErr);
		return -1;
	}

	/*
	 * Initialize the data structure and copy the data.  The handle
	 * is already locked. 
	 */
	header = (SyslogDataBuffer_t *) *datahand;
	header->sdb0_version = 0;
	header->sdb0_prio = pri;
	header->sdb0_numstrings = 1;

	offptr = (int *) (((char *) header) + sizeof(SyslogDataBuffer_t));
	magicptr = (int *) (((char *) offptr) + sizeof(int));
	lenptr = (int *) (((char *) magicptr) + sizeof(int));
	strptr = (((char *) lenptr) + sizeof(int));
	*offptr = 0;
	*magicptr = sizeof(SyslogDataBuffer_t) + 2 * sizeof(int);
	*lenptr = len;
	memcpy(strptr, buf, len);
	strptr[len] = '\0';	/* Phil's syslogd expects a NULL byte */

	HUnlock(datahand);

	if (psend(port, (long) datahand) == -1) {
		/* We failed.  Dump the region to avoid a mem leak. */
		saved_errno = errno;
		saved_toolerr = _toolErr;
		DisposeHandle(datahand);
		_toolErr = saved_toolerr;
		errno = saved_errno;
		return -1;
	}

	/* 
	 * Busy wait until we no longer own the memory block.  Use the
	 * COP 0x7F instruction to force a context switch and thus minimize
	 * wasted clock cycles.  We don't want to do a sleep(2) here because
	 * we don't futz with the user's world of signals.  This is cleaner
	 * anyway.
	 *
	 * We reuse (overload) 'magicptr' here so we don't have to allocate
	 * more stack space just for this test.
	 */
	magicptr = (int *) ((char *) datahand + 6); /* pointer to owner */
	while (*magicptr == memID) {
		asm {
			cop 127
		}
	}

	return 0;
}

static void
closePort(int port) {
	return;
}

#endif	/* USE_PORTS */

#ifdef __ORCAC__
#pragma optimize 78
#pragma debug 0
#endif

void
#if __STDC__
syslog(int pri, const char *fmt, ...)
#else
syslog(pri, fmt, va_alist)
	int pri;
	char *fmt;
	va_dcl
#endif
{
	va_list ap;

#if __STDC__
	va_start(ap, fmt);
#else
	va_start(ap);
#endif
	vsyslog(pri, fmt, ap);
	va_end(ap);
}

#ifdef __GNO__
void
syslogmt(int pri, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vsyslogmt(pri, fmt, ap);
	va_end(ap);
}
#endif	/* __GNO__ */

