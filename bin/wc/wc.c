/*
 * Copyright (c) 1980, 1987, 1991, 1993
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
 * Modified for GNO (Apple IIGS) by Dave Tribby, August 1997
 *
 * Constructs unacceptable to compiler are replaced using #ifndef __ORCAC__
 *
 * Changes not related to compiler are replaced using #ifndef __GNO__
 *
 * Added prototyped headers, surrounded by #ifndef __STDC__
 *
 * $Id: wc.c,v 1.2 1997/09/26 06:27:59 gdr Exp $
 */


#ifndef __GNO__			/* GNO doesn't use what strings */
#ifndef lint
static const char copyright[] =
"@(#) Copyright (c) 1980, 1987, 1991, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
#if 0
static const char sccsid[] = "@(#)wc.c	8.1 (Berkeley) 6/6/93";
#else
static const char rcsid[] =
	"$Id: wc.c,v 1.2 1997/09/26 06:27:59 gdr Exp $";
#endif
#endif /* not lint */
#endif


#include <sys/param.h>
#include <sys/stat.h>

#include <ctype.h>
#include <err.h>
#include <fcntl.h>
#ifndef __GNO__
#include <locale.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

u_long tlinect, twordct, tcharct;
int doline, doword, dochar;

int cnt __P((char *));
void usage __P((void));


/* Interface to check on how much stack space a C program uses. */
#if defined(__GNO__)  &&  defined(__STACK_CHECK__)
#ifndef _STDLIB_H_
#include <stdlib.h>
#endif
#include <gno/gno.h>
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
main(int argc,
	char *argv[])
#endif
{
	register int ch;
	int errors, total;

#if defined(__GNO__)  &&  defined(__STACK_CHECK__)
	_beginStackCheck();
	atexit(report_stack);
#endif
#ifndef __GNO__
	(void) setlocale(LC_CTYPE, "");
#endif

	while ((ch = getopt(argc, argv, "lwc")) != -1)
		switch((char)ch) {
		case 'l':
			doline = 1;
			break;
		case 'w':
			doword = 1;
			break;
		case 'c':
			dochar = 1;
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
	argc -= optind;

	/* Wc's flags are on by default. */
	if (doline + doword + dochar == 0)
		doline = doword = dochar = 1;

	errors = 0;
	total = 0;
	if (!*argv) {
		if (cnt((char *)NULL) != 0)
			++errors;
		else
			(void)printf("\n");
	}
	else do {
		if (cnt(*argv) != 0)
			++errors;
		else
			(void)printf(" %s\n", *argv);
		++total;
	} while(*++argv);

	if (total > 1) {
		if (doline)
			(void)printf(" %7ld", tlinect);
		if (doword)
			(void)printf(" %7ld", twordct);
		if (dochar)
			(void)printf(" %7ld", tcharct);
		(void)printf(" total\n");
	}
	exit(errors == 0 ? 0 : 1);
}

int
#ifndef __STDC__
cnt(file)
	char *file;
#else
cnt(char *file)
#endif
{     
	register u_char *p, ch;
	register short gotsp;
	register int len;
	register u_long linect, wordct, charct;
	struct stat sb;
	int fd;
#ifndef __GNO__
	u_char buf[MAXBSIZE];
#else
	static u_char buf[MAXBSIZE];
#endif

	linect = wordct = charct = 0;
	if (file == NULL) {
		file = "stdin";
		fd = STDIN_FILENO;
	} else {
#ifndef __GNO__
		if ((fd = open(file, O_RDONLY, 0)) < 0) {
#else		/* GNO: 3rd parameter legal only when creating */
		if ((fd = open(file, O_RDONLY)) < 0) {
#endif
			warn("%s: open", file);    
			return (1);
		}
		if (doword)
			goto word;
		/*
		 * Line counting is split out because it's a lot faster to get
		 * lines than to get words, since the word count requires some
		 * logic.
		 */
		if (doline) {
			while (len = read(fd, buf, MAXBSIZE)) {
				if (len == -1) {
					warn("%s: read", file);
					(void)close(fd);
					return (1);
				}
				charct += len;
				for (p = buf; len--; ++p)
#ifndef __GNO__		/* GNO delimits lines with \r, not \n */
					if (*p == '\n')
#else
					if (*p == '\r')
#endif
						++linect;
			}
			tlinect += linect;
			(void)printf(" %7lu", linect);
			if (dochar) {
				tcharct += charct;
				(void)printf(" %7lu", charct);
			}
			(void)close(fd);
			return (0);
		}
		/*
		 * If all we need is the number of characters and it's a
		 * regular or linked file, just stat the puppy.
		 */
		if (dochar) {
			if (fstat(fd, &sb)) {
				warn("%s: fstat", file);
				(void)close(fd);
				return (1);
			}
			if (S_ISREG(sb.st_mode) || S_ISLNK(sb.st_mode)) {
#ifndef __GNO__
				(void)printf(" %7qu", sb.st_size);
#else
				(void)printf(" %7lu", sb.st_size);
#endif
				tcharct += sb.st_size;
				(void)close(fd);
				return (0);
			}
		}
	}

	/* Do it the hard way... */
word:	for (gotsp = 1; len = read(fd, buf, MAXBSIZE);) {
		if (len == -1) {
			warn("%s: read", file);
			(void)close(fd);
			return (1);
		}
		/*
		 * This loses in the presence of multi-byte characters.
		 * To do it right would require a function to return a
		 * character while knowing how many bytes it consumed.
		 */
		charct += len;
		for (p = buf; len--;) {
			ch = *p++;
#ifndef __GNO__		/* GNO delimits lines with \r, not \n */
			if (ch == '\n')
#else
			if (ch == '\r')
#endif
				++linect;
			if (isspace(ch))
				gotsp = 1;
			else if (gotsp) {
				gotsp = 0;
				++wordct;
			}
		}
	}
	if (doline) {
		tlinect += linect;
		(void)printf(" %7lu", linect);
	}
	if (doword) {
		twordct += wordct;
		(void)printf(" %7lu", wordct);
	}
	if (dochar) {
		tcharct += charct;
		(void)printf(" %7lu", charct);
	}
	(void)close(fd);
	return (0);
}

void
#ifndef __STDC__
usage()
#else
usage(void)
#endif
{
	(void)fprintf(stderr, "usage: wc [-clw] [files]\n");
	exit(1);
}
