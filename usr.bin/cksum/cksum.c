/*-
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * James W. Williams of NASA Goddard Space Flight Center.
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
 * Modified for GNO (Apple IIGS) by Dave Tribby, December 1997
 *
 * Constructs unacceptable to compiler are replaced using #ifndef __ORCAC__
 *
 * Changes not related to compiler are replaced using #ifndef __GNO__
 *
 * Added prototyped headers, surrounded by #ifndef __STDC__
 */


#ifndef __GNO__
#ifndef lint
static const char copyright[] =
"@(#) Copyright (c) 1991, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
#if 0
static char sccsid[] = "@(#)cksum.c	8.2 (Berkeley) 4/28/95";
#endif
static const char rcsid[] =
	"$Id: cksum.c,v 1.1 1998/01/16 04:16:11 gdr Exp $";
#endif /* not lint */
#endif /* not GNO */

#include <sys/cdefs.h>
#include <sys/types.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "extern.h"

static void usage __P((void));


/* Interface to check on how much stack space a C program uses. */
#if defined(__GNO__)  &&  defined(__STACK_CHECK__)
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
	char **argv;
#else
main(int argc,
	char *argv[])
#endif
{
	register int ch, fd, rval;
	u_long len, val;
	char *fn, *p;
	int (*cfncn) __P((int, unsigned long *, unsigned long *));
	void (*pfncn) __P((char *, unsigned long, unsigned long));

#if defined(__GNO__)  &&  defined(__STACK_CHECK__)
	_beginStackCheck();
	atexit(report_stack);
#endif
	if ((p = rindex(argv[0], '/')) == NULL)
		p = argv[0];
	else
		++p;
	if (!strcmp(p, "sum")) {
		cfncn = csum1;
		pfncn = psum1;
	} else {
		cfncn = crc;
		pfncn = pcrc;
	}

	while ((ch = getopt(argc, argv, "o:")) != -1)
		switch (ch) {
		case 'o':
			if (!strcmp(optarg, "1")) {
				cfncn = csum1;
				pfncn = psum1;
			} else if (!strcmp(optarg, "2")) {
				cfncn = csum2;
				pfncn = psum2;
			} else if (*optarg == '3') {
				cfncn = crc32;
				pfncn = pcrc;
			} else {
				warnx("illegal argument to -o option");
				usage();
			}
			break;
		case '?':
		default:
			usage();
		}
#ifndef __ORCAC__
	argv += optind;
#else
	argv = argv + optind;
#endif

	fd = STDIN_FILENO;
	fn = NULL;
	rval = 0;
	do {
		if (*argv) {
			fn = *argv++;
#ifndef __GNO__
			if ((fd = open(fn, O_RDONLY, 0)) < 0) {
#else			/* Under GNO, 3rd param only valid when creating */
			if ((fd = open(fn, O_RDONLY)) < 0) {
#endif
				warn("%s", fn);
				rval = 1;
				continue;
			}
		}
		if (cfncn(fd, &val, &len)) {
			warn("%s", fn ? fn : "stdin");
			rval = 1;
		} else
			pfncn(fn, val, len);
		(void)close(fd);
	} while (*argv);
	exit(rval);
}

static void
#ifndef __STDC__
usage()
#else
usage(void)
#endif
{
	(void)fprintf(stderr, "usage: cksum [-o 1 | 2] [file ...]\n");
	(void)fprintf(stderr, "       sum [file ...]\n");
	exit(1);
}
