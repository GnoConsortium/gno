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

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)syslog.c	8.4 (Berkeley) 3/18/94";
#endif /* LIBC_SCCS and not lint */

#define USE_PORTS		/* use ports mech rather than TCP/IP */
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
#include <gno/gno.h>	/* __prognameGS() */
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
static int	sendPort(int port, const void *buf, int len);
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

#ifdef __ORCAC__
#pragma optimize 78
#pragma debug 0
#endif

/*
 * syslog, vsyslog --
 *	print message on log file; output is intended for syslogd(8).
 */
void
#if __STDC__
syslog(long pri, const char *fmt, ...)
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

#ifdef __ORCAC__
#pragma optimize 0
#pragma debug 25
#endif

void
vsyslog(long pri, const char *fmt, va_list ap)
{
	register int cnt;
	register char ch, *p, *t;
	time_t now;
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
	(void)time(&now);
	(void)fprintf(fp, "<%ld>", pri);
	(void)fprintf(fp, "%.15s ", ctime(&now) + 4);
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
	if (sendPort(LogFile, tbuf, cnt) >= 0)
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

static struct sockaddr SyslogAddr;	/* AF_UNIX address of local logger */

void
openlog(const char *ident, int logstat, long logfac)
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
long
setlogmask(long pmask)
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
sendPort(int port, const void *buf, int len) {
	static SyslogDataBuffer_t data;
	static pid_t pid = 0;
	long answer;
	
	if (port == -1) {
		return 0;
	}
	if (pid == 0) {
		data.magic  = SYSLOG_MAGIC;
		data.sender = pid = getpid();
	}
	if (len > MSG_BUF_LEN) {
		len = MSG_BUF_LEN;
	}
	memcpy(data.msg_buffer, buf, len);
	data.len = len;

	/* send the data */
	psend (port, (long) &data);

	/* wait for a reply */
	while((answer = procreceive()) != SYSLOG_MAGIC) {
		/* try to write a message to the console */
		int fd;
		char *s;
#define BAD_MAGIC ": bad magic from syslogd\r"
		if ((fd = open(_PATH_CONSOLE, O_WRONLY)) >= 0) {
			s = __prognameGS();
			write(fd, s, strlen(s));
			write(fd, BAD_MAGIC, sizeof(BAD_MAGIC)-1);
			close(fd);
		}
#undef BAD_MAGIC
	}
	return len;
}

static void
closePort(int port) {
	return;
}

#endif
