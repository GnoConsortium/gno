/*
 * Copyright (c) 1987, 1990, 1993, 1994
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
 * cmp for GNO/ME v2.0.6
 *
 * $Id: cmp.c,v 1.1 1997/10/03 05:13:22 gdr Exp $
 */

#ifndef __GNO__
#ifndef lint
static char copyright[] =
"@(#) Copyright (c) 1987, 1990, 1993, 1994\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */
#endif

#ifndef __GNO__
#ifndef lint
static char sccsid[] = "@(#)cmp.c	8.3 (Berkeley) 4/2/94";
#endif /* not lint */
#endif

#include <sys/types.h>
#include <sys/stat.h>

#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "extern.h"

int	lflag, sflag;
#ifdef __GNO__
int	rflag;
#endif

static void usage __P((void));

#ifdef __STACK_CHECK__
#include <gno/gno.h>

static void cleanup(void)
{
    (void) fprintf(stderr, "%d\n", _endStackCheck());
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
	struct stat sb1, sb2;
	off_t skip1, skip2;
	int ch, fd1, fd2, special;
	char *file1, *file2;

#ifdef __STACK_CHECK__
	atexit(cleanup);
	_beginStackCheck();
#endif

#ifdef __GNO__
	while ((ch = getopt(argc, argv, "-lsr")) != -1)
#else
	while ((ch = getopt(argc, argv, "-ls")) != -1)
#endif
		switch (ch) {
		case 'l':		/* print all differences */
			lflag = 1;
			break;
		case 's':		/* silent run */
			sflag = 1;
			break;
#ifdef __GNO__
		case 'r':
			rflag = 1;	/* ignore resource fork */
			break;
#endif
		case '-':		/* stdin (must be after options) */
			--optind;
			goto endargs;
		case '?':
		default:
			usage();
		}
endargs:
#ifndef __ORCAC__
	argv += optind;
#else
	argv = argv + optind;
#endif
	argc -= optind;

	if (lflag && sflag)
		errx(ERR_EXIT, "only one of -l and -s may be specified");

	if (argc < 2 || argc > 4)
		usage();

	/* Backward compatibility -- handle "-" meaning stdin. */
	special = 0;
	if (strcmp(file1 = argv[0], "-") == 0) {
		special = 1;
		fd1 = 0;
		file1 = "stdin";
	}
#ifndef __GNO__
	else if ((fd1 = open(file1, O_RDONLY, 0)) < 0) {
#else
	/* open() for GNO/ME requires 3rd arg iff creating file */
	else if ((fd1 = open(file1, O_RDONLY)) < 0) {
#endif
		if (!sflag)
			err(ERR_EXIT, "%s", file1);
		else
			exit(1);
	}
	if (strcmp(file2 = argv[1], "-") == 0) {
		if (special)
			errx(ERR_EXIT,
				"standard input may only be specified once");
		special = 1;
		fd2 = 0;
		file2 = "stdin";
	}
#ifndef __GNO__
	else if ((fd2 = open(file2, O_RDONLY, 0)) < 0) {
#else
	else if ((fd2 = open(file2, O_RDONLY)) < 0) {
#endif
		if (!sflag)
			err(ERR_EXIT, "%s", file2);
		else
			exit(1);
	}

	skip1 = argc > 2 ? strtol(argv[2], NULL, 10) : 0;
	skip2 = argc == 4 ? strtol(argv[3], NULL, 10) : 0;

#ifndef __GNO__
	if (!special) {
		if (fstat(fd1, &sb1)) {
			if (!sflag)
				err(ERR_EXIT, "%s", file1);
			else
				exit(1);
		}
		if (!S_ISREG(sb1.st_mode))
			special = 1;
		else {
			if (fstat(fd2, &sb2)) {
				if (!sflag)
					err(ERR_EXIT, "%s", file2);
				else
					exit(1);
			}
			if (!S_ISREG(sb2.st_mode))
				special = 1;
		}
	}
#else
	special = 1; 	/* GNO doesn't have mmap.h, so treat every file as
			   a special file and process byte by byte */
#endif
	if (special)
#ifdef __GNO__
	{
#endif
		c_special(fd1, file1, skip1, fd2, file2, skip2);
#ifdef __GNO__
		close(fd1);
		close(fd2);
	}

	if (rflag != 1) {
		rflag = 2;
		rcmp(&fd1, &fd2, file1, file2);
		close(fd1);
		close(fd2);
	}
#else
	else
		c_regular(fd1, file1, skip1, sb1.st_size,
		    fd2, file2, skip2, sb2.st_size);
#endif
/*	exit(0); */
}

static void
#ifndef __STDC__
usage()
#else
usage(void)
#endif
{

	(void)fprintf(stderr,
#ifndef __GNO__
	    "usage: cmp [-l | -s] file1 file2 [skip1 [skip2]]\n");
#else
	    "usage: cmp [-l | -s] [-r] file1 file2 [skip1 [skip2]]\n");
#endif
	exit(ERR_EXIT);
}
