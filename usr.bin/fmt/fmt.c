/*
 * Copyright (c) 1980, 1993
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

#ifndef __GNO__

#ifndef lint
static char copyright[] =
"@(#) Copyright (c) 1980, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)fmt.c	8.1 (Berkeley) 7/20/93";
#endif /* not lint */

#endif	/* ! __GNO__ */

#include <stdio.h>
#include <ctype.h>
#ifndef NO_LOCALE
#include <locale.h>
#endif
#include <stdlib.h>
#include <string.h>

/* some architectures have a broken (and non-standard) realloc(3) */
#ifdef SUNOS4
#define REALLOC my_realloc
static void *	my_realloc __P((void *buf, size_t size));
#else
#define REALLOC realloc
#endif

/* reduce stack usage */
#ifdef	__ORCAC__
#define	STATIC	static
#else
#define	STATIC
#endif

static void fmt __P((FILE *fi));
static void prefix __P((char *line));
static void split __P((char *line));
static void setout __P((void));
static void pack __P((char *textword, int wl));
static void oflush __P((void));
static void tabulate __P((char *line));
static void leadin __P((void));
static int  ispref __P((char *s1, char *s2));

extern int  ishead __P((char *));

#if defined(SOLARIS) || defined(HALOS) || defined(__ORCAC__)
#define INDEX(s,c) strchr((s),(c))
#else
#define INDEX index
#endif

/*
 * fmt -- format the concatenation of input files or standard input
 * onto standard output.  Designed for use with Mail ~|
 *
 * Syntax : fmt [ goal [ max ] ] [ name ... ]
 * Authors: Kurt Shoens (UCB) 12/7/78;
 *          Liz Allen (UMCP) 2/24/83 [Addition of goal length concept].
 */

/* LIZ@UOM 6/18/85 -- Don't need LENGTH any more.
 * #define	LENGTH	72		Max line length in output
 */
#define	NOSTR	((char *) 0)	/* Null string pointer for lint */

/* LIZ@UOM 6/18/85 --New variables goal_length and max_length */
#define GOAL_LENGTH 65
#define MAX_LENGTH 75
int	goal_length;		/* Target or goal line length in output */
int	max_length;		/* Max line length in output */
int	pfx;			/* Current leading blank count */
int	lineno;			/* Current input line */
int	mark;			/* Last place we saw a head line */

char	*headnames[] = {"To", "Subject", "Cc", 0};

#ifdef __STACK_CHECK__
#include <gno/gno.h>
static void
printStack (void) {
	printf("stack usage: %d bytes\n", _endStackCheck());
}
#endif

/*
 * Drive the whole formatter by managing input files.  Also,
 * cause initialization of the output stuff and flush it out
 * at the end.
 */

int
main(int argc, char **argv)
{
	register FILE *fi;
	register int errs = 0;
	int number;		/* LIZ@UOM 6/18/85 */

#ifdef __STACK_CHECK__
	_beginStackCheck();
	atexit(printStack);
#endif

#ifndef NO_LOCALE
	(void) setlocale(LC_CTYPE, "");
#endif

	goal_length = GOAL_LENGTH;
	max_length = MAX_LENGTH;
	setout();
	lineno = 1;
	mark = -10;
	/*
	 * LIZ@UOM 6/18/85 -- Check for goal and max length arguments
	 */
	if (argc > 1 && (1 == (sscanf(argv[1], "%d", &number)))) {
		argv++;
		argc--;
		goal_length = number;
		if (argc > 1 && (1 == (sscanf(argv[1], "%d", &number)))) {
			argv++;
			argc--;
			max_length = number;
		}
	}
	if (max_length <= goal_length) {
		fprintf(stderr, "Max length must be greater than %s\n",
			"goal length");
		exit(1);
	}
	if (argc < 2) {
		fmt(stdin);
		oflush();
		exit(0);
	}
	while (--argc) {
		if ((fi = fopen(*++argv, "r")) == NULL) {
			perror(*argv);
			errs++;
			continue;
		}
		fmt(fi);
		fclose(fi);
	}
	oflush();
	exit(errs);
}

/*
 * Read up characters from the passed input file, forming lines,
 * doing ^H processing, expanding tabs, stripping trailing blanks,
 * and sending each line down for analysis.
 */
static void
fmt(FILE *fi)
{
	static char *linebuf = 0, *canonb = 0;
	register char *cp, *cp2, cc;
	register int c, col;
#define CHUNKSIZE 1024
	static int lbufsize = 0, cbufsize = 0;

	c = getc(fi);
	while (c != EOF) {
		/*
		 * Collect a line, doing ^H processing.
		 * Leave tabs for now.
		 */
		cp = linebuf;
		while (c != '\n' && c != EOF) {
			if (cp - linebuf >= lbufsize) {
				int offset = cp - linebuf;
				lbufsize += CHUNKSIZE;
				linebuf = REALLOC(linebuf, lbufsize);
				if(linebuf == NULL) {
				    perror("linebuf allocation failed");
				    abort();
				}
				cp = linebuf + offset;
			}
			if (c == '\b') {
				if (cp > linebuf)
					cp--;
				c = getc(fi);
				continue;
			}
			if (!isprint(c) && c != '\t') {
				c = getc(fi);
				continue;
			}
			*cp++ = c;
			c = getc(fi);
		}

		/*
		 * Toss anything remaining on the input line.
		 */
		while (c != '\n' && c != EOF)
			c = getc(fi);

		if (cp != NULL) {
			*cp = '\0';
		} else {
			putchar('\n');
			c = getc(fi);
			continue;
		}

		/*
		 * Expand tabs on the way to canonb.
		 */
		col = 0;
		cp = linebuf;
		cp2 = canonb;
		while ((cc = *cp++)) {
			if (cc != '\t') {
				col++;
				if (cp2 - canonb >= cbufsize) {
					int offset = cp2 - canonb;
					cbufsize += CHUNKSIZE;
					canonb = REALLOC(canonb, cbufsize);
					if(canonb == 0) {
					    perror("canob (1) allocation failed");
					    abort();
					}
					cp2 = canonb + offset;
				}
				*cp2++ = cc;
				continue;
			}
			do {
				if (cp2 - canonb >= cbufsize) {
					int offset = cp2 - canonb;
					cbufsize += CHUNKSIZE;
					canonb = REALLOC(canonb, cbufsize);
					if(canonb == 0) {
					    perror("canob (2) allocation failed");
					    abort();
					}
					cp2 = canonb + offset;
				}
				*cp2++ = ' ';
				col++;
			} while ((col & 07) != 0);
		}

		/*
		 * Swipe trailing blanks from the line.
		 */
		for (cp2--; cp2 >= canonb && *cp2 == ' '; cp2--)
			;
		*++cp2 = '\0';
		prefix(canonb);
		if (c != EOF)
			c = getc(fi);
	}
}

/*
 * Take a line devoid of tabs and other garbage and determine its
 * blank prefix.  If the indent changes, call for a linebreak.
 * If the input line is blank, echo the blank line on the output.
 * Finally, if the line minus the prefix is a mail header, try to keep
 * it on a line by itself.
 */
static void
prefix(char *line)
{
	register char *cp, **hp;
	register int np, h;

	if (!*line) {
		oflush();
		putchar('\n');
		return;
	}
	for (cp = line; *cp == ' '; cp++)
		;
	np = cp - line;

	/*
	 * The following horrible expression attempts to avoid linebreaks
	 * when the indent changes due to a paragraph.
	 */
	if (np != pfx && (np > pfx || abs(pfx-np) > 8))
		oflush();
	if ((h = ishead(cp)))
		oflush(), mark = lineno;
	if (lineno - mark < 3 && lineno - mark > 0)
		for (hp = &headnames[0]; *hp != (char *) 0; hp++)
			if (ispref(*hp, cp)) {
				h = 1;
				oflush();
				break;
			}
	if (!h && (h = (*cp == '.')))
		oflush();
	pfx = np;
	if (h)
		pack(cp, strlen(cp));
	else	split(cp);
	if (h)
		oflush();
	lineno++;
}

/*
 * Split up the passed line into output "words" which are
 * maximal strings of non-blanks with the blank separation
 * attached at the end.  Pass these words along to the output
 * line packer.
 */
static void
split(char *line)
{
	register char *cp, *cp2;
	STATIC char textword[BUFSIZ];
	int wordl;		/* LIZ@UOM 6/18/85 */

	cp = line;
	while (*cp) {
		cp2 = textword;
		wordl = 0;	/* LIZ@UOM 6/18/85 */

		/*
		 * Collect a 'textword,' allowing it to contain escaped white
		 * space.
		 */
		while (*cp && *cp != ' ') {
			if (*cp == '\\' && isspace(cp[1]))
				*cp2++ = *cp++;
			*cp2++ = *cp++;
			wordl++;/* LIZ@UOM 6/18/85 */
		}

		/*
		 * Guarantee a space at end of line. Two spaces after end of
		 * sentence punctuation.
		 */
		if (*cp == '\0') {
			*cp2++ = ' ';
			if (INDEX(".:!", cp[-1]))
				*cp2++ = ' ';
		}
		while (*cp == ' ')
			*cp2++ = *cp++;
		*cp2 = '\0';
		/*
		 * LIZ@UOM 6/18/85 pack(textword);
		 */
		pack(textword, wordl);
	}
}

/*
 * Output section.
 * Build up line images from the words passed in.  Prefix
 * each line with correct number of blanks.  The buffer "outbuf"
 * contains the current partial line image, including prefixed blanks.
 * "outp" points to the next available space therein.  When outp is NOSTR,
 * there ain't nothing in there yet.  At the bottom of this whole mess,
 * leading tabs are reinserted.
 */
char	outbuf[BUFSIZ];			/* Sandbagged output line image */
char	*outp;				/* Pointer in above */

/*
 * Initialize the output section.
 */
static void
setout(void)
{
	outp = NOSTR;
}

/*
 * Pack a textword onto the output line.  If this is the beginning of
 * the line, push on the appropriately-sized string of blanks first.
 * If the textword won't fit on the current line, flush and begin a new
 * line.  If the textword is too long to fit all by itself on a line,
 * just give it its own and hope for the best.
 *
 * LIZ@UOM 6/18/85 -- If the new textword will fit in at less than the
 *	goal length, take it.  If not, then check to see if the line
 *	will be over the max length; if so put the textword on the next
 *	line.  If not, check to see if the line will be closer to the
 *	goal length with or without the textword and take it or put it on
 *	the next line accordingly.
 */

/*
 * LIZ@UOM 6/18/85 -- pass in the length of the textword as well
 * pack(textword)
 *	char textword[];
 */
static void
pack(char *textword, int wl)
{
	register char *cp;
	register int s, t;

	if (outp == NOSTR)
		leadin();
	/*
	 * LIZ@UOM 6/18/85 -- change condition to check goal_length; s is the
	 * length of the line before the textword is added; t is now the length
	 * of the line after the textword is added
	 *	t = strlen(textword);
	 *	if (t+s <= LENGTH)
	 */
	s = outp - outbuf;
	t = wl + s;
	if ((t <= goal_length) ||
	    ((t <= max_length) && (t - goal_length <= goal_length - s))) {
		/*
		 * In like flint!
		 */
		for (cp = textword; *cp; *outp++ = *cp++);
		return;
	}
	if (s > pfx) {
		oflush();
		leadin();
	}
	for (cp = textword; *cp; *outp++ = *cp++);
}

/*
 * If there is anything on the current output line, send it on
 * its way.  Set outp to NOSTR to indicate the absence of the current
 * line prefix.
 */
static void
oflush(void)
{
	if (outp == NOSTR)
		return;
	*outp = '\0';
	tabulate(outbuf);
	outp = NOSTR;
}

/*
 * Take the passed line buffer, insert leading tabs where possible, and
 * output on standard output (finally).
 */
static void
tabulate(char *line)
{
	register char *cp;
	register int b, t;

	/*
	 * Toss trailing blanks in the output line.
	 */
	cp = line + strlen(line) - 1;
	while (cp >= line && *cp == ' ')
		cp--;
	*++cp = '\0';

	/*
	 * Count the leading blank space and tabulate.
	 */
	for (cp = line; *cp == ' '; cp++)
		;
	b = cp-line;
	t = b >> 3;
	b &= 07;
	if (t > 0)
		do
			putc('\t', stdout);
		while (--t);
	if (b > 0)
		do
			putc(' ', stdout);
		while (--b);
	while (*cp)
		putc(*cp++, stdout);
	putc('\n', stdout);
}

/*
 * Initialize the output line with the appropriate number of
 * leading blanks.
 */
static void
leadin(void)
{
	register int b;
	register char *cp;

	for (b = 0, cp = outbuf; b < pfx; b++)
		*cp++ = ' ';
	outp = cp;
}

#if 0
/*
 * Save a string in dynamic space.
 * This little goodie is needed for
 * a headline detector in head.c
 */
static char *savestr __P((char *str));

static char *
savestr(char *str)
{
	register char *top;

	top = malloc(strlen(str) + 1);
	if (top == NOSTR) {
		fprintf(stderr, "fmt:  Ran out of memory\n");
		exit(1);
	}
	strcpy(top, str);
	return (top);
}
#endif

/*
 * Is s1 a prefix of s2??
 */
static int
ispref(register char *s1, register char *s2)
{

	while (*s1++ == *s2)
		;
	return (*s1 == '\0');
}


#ifndef __ORCAC__
/*
 * some architectures' reallocs are too damned stupid to accept NULL
 * buf arguments ...
 */

static void *
my_realloc (void *buf, size_t size)
{
    if (buf == NULL) {
	return (void *) malloc(size);
    } else {
	return (void *) realloc(buf, size);
    }
}
#endif
