/*-
 * Copyright (c) 1993
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

/*
 * Modified for GNO (Apple IIGS) by Steve Reeves, January 1998
 *
 * Added conditional compilation based on these macros:
 *
 *   __ORCAC__ for constructs unacceptable to the compiler,
 *   __GNO__ for changes not related to the compiler,
 *   __STDC__ or __P() from <sys/cdefs.h> for ISO C prototyping,
 *   __STACK_CHECK__ for reporting stack space usage,
 *   NOID for excluding copyright and version control strings
 *
 * $Id: uname.c,v 1.1 1998/02/17 02:50:29 gdr-ftp Exp $
 *
 */

#ifndef NOID
#ifndef lint
static char copyright[] =
"@(#) Copyright (c) 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)uname.c	8.2 (Berkeley) 5/4/95";
#endif /* not lint */
#endif

#ifndef __GNO__
#include <sys/param.h>
#include <sys/sysctl.h>
#else
#include <sys/utsname.h>
#endif

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void usage __P((void));

#if defined(__GNO__)  &&  defined(__STACK_CHECK__)
/* Interface to check on how much stack space a C program uses. */
#ifndef _GNO_GNO_H_
#include <gno/gno.h>
#endif
static void report_stack(void)
{
	fprintf(stderr,"\n ==> %d stack bytes used <== \n", _endStackCheck());
}
#endif

int
#ifndef __STDC__
main(argc, argv)
	int argc;
	char *argv[];
#else
main(int argc, char *argv[])
#endif
{
#define	MFLAG	0x01
#define	NFLAG	0x02
#define	RFLAG	0x04
#define	SFLAG	0x08
#define	VFLAG	0x10
	u_int flags;
#ifndef __GNO__
	int ch, mib[2];
	size_t len, tlen;
	char *p, *prefix, buf[1024];
#else
	int ch;
	char *prefix;
	static struct utsname name;
#endif

#if defined(__GNO__)  &&  defined(__STACK_CHECK__)
	_beginStackCheck();
	atexit(report_stack);
#endif

	flags = 0;
	while ((ch = getopt(argc, argv, "amnrsvV")) != -1)
		switch(ch) {
		case 'a':
			flags |= (MFLAG | NFLAG | RFLAG | SFLAG | VFLAG);
			break;
		case 'm':
			flags |= MFLAG;
			break;
		case 'n':
			flags |= NFLAG;
			break;
		case 'r':
			flags |= RFLAG;
			break;
		case 's':
			flags |= SFLAG;
			break;
		case 'v':
			flags |= VFLAG;
			break;
		case 'V':
			printf("uname v3.0\n");
			exit(0);
		case '?':
		default:
			usage();
		}

	argc -= optind;
#ifndef __ORCAC__
	argv += optind;
#else
	argv = argv + optind;
#endif

	if (argc)
		usage();

	if (!flags)
		flags |= SFLAG;

	prefix = "";

#ifndef __GNO__
	if (flags & SFLAG) {
		mib[0] = CTL_KERN;
		mib[1] = KERN_OSTYPE;
		len = sizeof(buf);
		if (sysctl(mib, 2, &buf, &len, NULL, 0) == -1)
			err(1, "sysctl");
		(void)printf("%s%.*s", prefix, len, buf);
		prefix = " ";
	}
	if (flags & NFLAG) {
		mib[0] = CTL_KERN;
		mib[1] = KERN_HOSTNAME;
		len = sizeof(buf);
		if (sysctl(mib, 2, &buf, &len, NULL, 0) == -1)
			err(1, "sysctl");
		(void)printf("%s%.*s", prefix, len, buf);
		prefix = " ";
	}
	if (flags & RFLAG) {
		mib[0] = CTL_KERN;
		mib[1] = KERN_OSRELEASE;
		len = sizeof(buf);
		if (sysctl(mib, 2, &buf, &len, NULL, 0) == -1)
			err(1, "sysctl");
		(void)printf("%s%.*s", prefix, len, buf);
		prefix = " ";
	}
	if (flags & VFLAG) {
		mib[0] = CTL_KERN;
		mib[1] = KERN_VERSION;
		len = sizeof(buf);
		if (sysctl(mib, 2, &buf, &len, NULL, 0) == -1)
			err(1, "sysctl");
		for (p = buf, tlen = len; tlen--; ++p)
			if (*p == '\n' || *p == '\t')
				*p = ' ';
		(void)printf("%s%.*s", prefix, len, buf);
		prefix = " ";
	}
	if (flags & MFLAG) {
		mib[0] = CTL_HW;
		mib[1] = HW_MACHINE;
		len = sizeof(buf);
		if (sysctl(mib, 2, &buf, &len, NULL, 0) == -1)
			err(1, "sysctl");
		(void)printf("%s%.*s", prefix, len, buf);
		prefix = " ";
	}

#else	/* __GNO__ */

	if (uname(&name) == -1)
		err(1, "uname");

	if (flags & SFLAG) {
		printf("%s%s", prefix, name.sysname);
		prefix = " ";
	}
	if (flags & NFLAG) {
		printf("%s%s", prefix, name.nodename);
		prefix = " ";
	}
	if (flags & RFLAG) {
		printf("%s%s", prefix, name.release);
		prefix = " ";
	}
	if (flags & VFLAG) {
		printf("%s%s", prefix, name.version);
		prefix = " ";
	}
	if (flags & MFLAG) {
		printf("%s%s", prefix, name.machine);
		prefix = " ";
	}
#endif

	(void)printf("\n");
	exit (0);
}

void
usage __P((void))
{
	(void)fprintf(stderr, "usage: uname [-amnrsvV]\n");
	exit(1);
}
