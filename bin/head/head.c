/*
 * Copyright (c) 1980, 1987, 1992, 1993
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
 * Modified for GNO (Apple IIGS) by Dave Tribby, July 1997
 *
 * Constructs unacceptable to compiler are replaced using #ifndef __ORCAC__
 *
 * Changes not related to compiler are replaced using #ifndef __GNO__
 *
 * Added prototyped headers, surrounded by #ifndef __STDC__
 *
 * $Id: head.c,v 1.3 1997/10/03 03:58:18 gdr Exp $
 */

#ifndef __GNO__
#ifndef lint
static const char copyright[] =
"@(#) Copyright (c) 1980, 1987, 1992, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
#if 0
static char sccsid[] = "@(#)head.c	8.2 (Berkeley) 5/4/95";
#endif
static const char rcsid[] =
	"$Id: head.c,v 1.3 1997/10/03 03:58:18 gdr Exp $";
#endif /* not lint */
#endif

#include <sys/types.h>

#include <ctype.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * head - give the first few lines of a stream or of each of a set of files
 *
 * Bill Joy UCB August 24, 1977
 */

void head __P((FILE *, int));
void head_bytes __P((FILE *, int));
void obsolete __P((char *[]));
void usage __P((void));

int eval;

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
	char *argv[];
#else
main(int argc, char *argv[])
#endif
{
	register int ch;
	FILE *fp;
	int first, linecnt = -1, bytecnt = -1;
	char *ep;

#if defined(__GNO__)  &&  defined(__STACK_CHECK__)
	_beginStackCheck();
	atexit(report_stack);
#endif
	obsolete(argv);
	while ((ch = getopt(argc, argv, "n:c:")) != -1)
		switch(ch) {
		case 'c':
			bytecnt = strtol(optarg, &ep, 10);
			if (*ep || bytecnt <= 0)
				errx(1, "illegal byte count -- %s", optarg);
			break;
		case 'n':
			linecnt = strtol(optarg, &ep, 10);
			if (*ep || linecnt <= 0)
				errx(1, "illegal line count -- %s", optarg);
			break;
		case '?':
		default:
			usage();
		}
	argc -= optind;
#ifndef __ORCAC__
	argv += optind;
#else
	/* ORCA/C 2.1 compiler cannot handle += on pointer  */
        argv = argv + optind;
#endif


	if (linecnt != -1 && bytecnt != -1)
		errx(1, "can't combine line and byte counts");
	if (linecnt == -1 )
		linecnt = 10;
	if (*argv) {
		for (first = 1; *argv; ++argv) {
			if ((fp = fopen(*argv, "r")) == NULL) {
				warn("%s", *argv);
				eval = 1;
				continue;
			}
			if (argc > 1) {
				(void)printf("%s==> %s <==\n",
				    first ? "" : "\n", *argv);
				first = 0;
			}
			if (bytecnt == -1)
				head(fp, linecnt);
			else
				head_bytes(fp, bytecnt);
			(void)fclose(fp);
		}
	}
	else if (bytecnt == -1)
		head(stdin, linecnt);
	else
		head_bytes(stdin, bytecnt);

	exit(eval);
}

void
#ifndef __STDC__
head(fp, cnt)
	FILE *fp;
	register int cnt;
#else
head(FILE *fp, register int cnt)
#endif
{
	register int ch;

	while (cnt && (ch = getc(fp)) != EOF) {
			if (putchar(ch) == EOF)
				err(1, "stdout");
			if (ch == '\n')
				cnt--;
		}
}

void
#ifndef __STDC__
head_bytes(fp, cnt)
	 FILE *fp;
	 register int cnt;
#else
head_bytes(FILE *fp, register int cnt)
#endif
{
#ifndef __GNO__
	char buf[4096];
#else
	/* Cannot handle 4096 bytes on stack. Since head_bytes is */
        /* non-recursive, make it a static array.             DMT */
	static char buf[4096];
#endif        
	register int readlen;

	while (cnt) {
		if (cnt < sizeof(buf))
			readlen = cnt;
		else
			readlen = sizeof(buf);
		readlen = fread(buf, sizeof(char), readlen, fp);
		if (readlen == EOF)
			break;
		if (fwrite(buf, sizeof(char), readlen, stdout) != readlen)
			err(1, "stdout");
		cnt -= readlen;
	}
}

void
#ifndef __STDC__
obsolete(argv)
	char *argv[];
#else
obsolete(char *argv[])
#endif
{
	char *ap;

	while ((ap = *++argv)) {
		/* Return if "--" or not "-[0-9]*". */
		if (ap[0] != '-' || ap[1] == '-' || !isdigit(ap[1]))
			return;
		if ((ap = malloc(strlen(*argv) + 2)) == NULL)
			err(1, NULL);
		ap[0] = '-';
		ap[1] = 'n';
		(void)strcpy(ap + 2, *argv + 1);
		*argv = ap;
	}
}

void
#ifndef __STDC__
usage()
#else
usage(void)
#endif
{
#ifndef __GNO__
	(void)fprintf(stderr, "usage: head [-n lines] [-c bytes] [file ...]\n");
#else
	/* Show -n and -c as exclusive, and add obsolescent format (DMT) */
	(void)fprintf(stderr, "usage: head [-n lines | -c bytes] [file ...]\n");
	(void)fprintf(stderr, "       head [-count] [file ...]\n");
#endif
	exit(1);
}
