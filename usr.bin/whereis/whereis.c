/*-
 * Copyright (c) 1980 The Regents of the University of California.
 * All rights reserved.
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
 * $Id: whereis.c,v 1.3 1997/09/30 04:22:43 gdr Exp $
 */

#if !defined(lint) && !defined(__GNO__)
char copyright[] =
"@(#) Copyright (c) 1980 The Regents of the University of California.\n\
 All rights reserved.\n";

static char sccsid[] = "@(#)whereis.c	5.5 (Berkeley) 4/18/91";
#endif /* not lint */

#include <sys/param.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <ctype.h>
#include <err.h>
#include <assert.h>
#include <gno/contrib.h>

#define LINEBUF_SIZE	128			/* max size of conf line */
#define	CONF_PATH	"/etc/whereis.conf"	/* default path */
#define CONF_ENV	"WHEREIS_CONF"		/* name of environ variable */

#define PROWORD_BIN	"bindirs"	/* keyword in whereis.conf */
#define PROWORD_MAN	"mandirs"	/* keyword in whereis.conf */
#define PROWORD_SRC	"srcdirs"	/* keyword in whereis.conf */

/*
 * These string arrays will hold the paths that we want to check, as
 * defined in the whereis.conf file.
 */

LC_StringArray_t Sbindirs = NULL;
LC_StringArray_t Smandirs = NULL;
LC_StringArray_t Ssrcdirs = NULL;

/* pointers to the vectors in the above string arrays; legacy code */
char **bindirs;
char **mandirs;
char **srcdirs;

int	cflag = 1;
char	sflag = 1;
char	bflag = 1;
char	mflag = 1;
char	**Sflag;
int	Scnt;
char	**Bflag;
int	Bcnt;
char	**Mflag;
int	Mcnt;
char	uflag;

char  *verstring = "whereis version 1.3";

void	getlist (int *argcp, char ***argvp, char ***flagp, int *cntp);
void	zerof(void);
void	lookup (register char *cp);
void	findin (char *dir, char *cp);
int	itsit (register char *cp, register char *dp);
void	getDirectories (void);
void	addIdent (int ident, char *path);

/*
 * If INLINE_FUNCS is defined, we use macro versions of some very
 * short routines.  Unfortunately, ORCA/C v2.1.x doesn't have the
 * ability to do this itself.
 */
#define INLINE_FUNCS

#ifdef INLINE_FUNCS
#define find(dirs, cp) \
{ \
	char **ldirs = dirs; \
	while (*ldirs) { \
		findin(*ldirs++, cp); \
	} \
}

#define findv(dirv, dirc, cp) \
{ \
	char **ldirv = dirv; \
	int ldirc = dirc; \
	while (ldirc > 0) { \
		findin(*ldirv++, cp); \
		dirc--; \
	} \
}

#define looksrc(cp) \
{ \
	if (Sflag == 0) { \
		find(srcdirs, cp); \
	} else { \
		findv(Sflag, Scnt, cp); \
	} \
}

#define lookbin(cp) \
{ \
	if (Bflag == 0) { \
		find(bindirs, cp); \
	} else { \
		findv(Bflag, Bcnt, cp); \
	} \
}

#define lookman(cp) \
{ \
	if (Mflag == 0) { \
		find(mandirs, cp); \
	} else { \
		findv(Mflag, Mcnt, cp); \
	} \
}

#else
void	looksrc (char *cp);
void	lookbin (char *cp);
void	lookman (char *cp);
void	find (char **dirs, char *cp);
void	findv (char **dirv, int dirc, char *cp);
#endif

#if defined(__GNO__) && defined(__STACK_CHECK__)
#include <gno/gno.h>
static void
stackResults(void) {
	fprintf(stderr, "stack usage:\t ===> %d bytes <===\n",
		_endStackCheck());
}
#endif

/*
 * whereis name
 * look for source, documentation and binaries
 */
int
main(int argc, char **argv) {

#ifdef __STACK_CHECK__
	_beginStackCheck();
	atexit(stackResults);
#endif

	/* quick usage check */
	argc--, argv++;
	if (argc == 0) {
usage:
		fprintf(stderr,
			"whereis [ -sbmucV ] [ -SBM dir ... -f ] name...\n");
		exit(1);
	}

	/* initialize string arrays */
	Sbindirs = LC_StringArrayNew();
	Smandirs = LC_StringArrayNew();
	Ssrcdirs = LC_StringArrayNew();

	/* get the list of directories where we will look */
        getDirectories();

	bindirs = Sbindirs->lc_vec;
	mandirs = Smandirs->lc_vec;
	srcdirs = Ssrcdirs->lc_vec;

	do {
		if (argv[0][0] == '-') {
			register char *cp = argv[0] + 1;
			while (*cp) switch (*cp++) {

			case 'f':
				break;

			case 'S':
				getlist(&argc, &argv, &Sflag, &Scnt);
				break;

			case 'B':
				getlist(&argc, &argv, &Bflag, &Bcnt);
				break;

			case 'M':
				getlist(&argc, &argv, &Mflag, &Mcnt);
				break;

			case 's':
				zerof();
				sflag++;
				continue;

			case 'u':
				uflag++;
				continue;

			case 'b':
				zerof();
				bflag++;
				continue;

			case 'm':
				zerof();
				mflag++;
				continue;

			case 'c':
				cflag = 0;
				continue;

			case 'V':
				printf("%s\n",verstring);
				continue;

			default:
				goto usage;
			}
			argv++;
		} else {
			lookup(*argv++);
		}
	} while (--argc > 0);

	exit(0);
}

void getlist (int *argcp, char ***argvp, char ***flagp, int *cntp) {
	(*argvp)++;
	*flagp = *argvp;
	*cntp = 0;
	for ((*argcp)--; *argcp > 0 && (*argvp)[0][0] != '-'; (*argcp)--)
		(*cntp)++, (*argvp)++;
	(*argcp)++;
	(*argvp)--;
}


void zerof (void) {

	if (sflag && bflag && mflag)
		sflag = bflag = mflag = 0;
}

int	count;
int	print;


void lookup (register char *cp) {
	register char *dp;

	for (dp = cp; *dp; dp++)
		continue;
	for (; dp > cp; dp--) {
		if (*dp == '.') {
			*dp = 0;
			break;
		}
	}
	for (dp = cp; *dp; dp++)
		if (*dp == '/')
			cp = dp + 1;
	if (uflag) {
		print = 0;
		count = 0;
	} else
		print = 1;
again:
	if (print)
		printf("%s:", cp);
	if (sflag) {
		looksrc(cp);
		if (uflag && print == 0 && count != 1) {
			print = 1;
			goto again;
		}
	}
	count = 0;
	if (bflag) {
		lookbin(cp);
		if (uflag && print == 0 && count != 1) {
			print = 1;
			goto again;
		}
	}
	count = 0;
	if (mflag) {
		lookman(cp);
		if (uflag && print == 0 && count != 1) {
			print = 1;
			goto again;
		}
	}
	if (print)
		printf("\n");
}

#ifndef INLINE_FUNCS

void looksrc (char *cp) {
	if (Sflag == 0) {
		find(srcdirs, cp);
	} else
		findv(Sflag, Scnt, cp);
}

void lookbin (char *cp) {
	if (Bflag == 0)
		find(bindirs, cp);
	else
		findv(Bflag, Bcnt, cp);
}

void lookman (char *cp) {     
	if (Mflag == 0) {
		find(mandirs, cp);
	} else
		findv(Mflag, Mcnt, cp);
}

void findv (char **dirv, int dirc, char *cp) {
	while (dirc > 0) {
		findin(*dirv++, cp);
		dirc--;
	}
}

void find (char **dirs, char *cp) {
	while (*dirs)
		findin(*dirs++, cp);
}

#endif	/* ! INLINE_FUNCS */

void findin (char *dir, char *cp) {
	DIR *dirp;
	struct dirent *dp;

	dirp = opendir(dir);
	if (dirp == NULL)
		return;
	while ((dp = readdir(dirp)) != NULL) {
		if (itsit(cp, dp->d_name)) {
			count++;
			if (print)
				printf(" %s/%s", dir, dp->d_name);
		}
	}
	closedir(dirp);
}

int itsit (register char *cp, register char *dp) {
	register int i = strlen(dp);

	if (cflag) {
		if ( (dp[0] == 's' || dp[0] == 'S')
		    && dp[1] == '.' && itsit(cp, dp+2))
			return (1);
		while (*cp && *dp && (tolower(*cp) == tolower(*dp)))
			cp++, dp++, i--;
		if (*cp == 0 && *dp == 0)
			return (1);
		while (isdigit(*dp))
			dp++;
		if (*cp == 0 && *dp++ == '.') {
/* removed for GNO, because we want to look up compressed files also. */
#ifndef __GNO__
			--i;
			while (i > 0 && *dp)
				if (--i, *dp++ == '.')
					return (*dp++ == 'C' && *dp++ == 0);
#endif
			return (1);
		}
   } else {
		if (dp[0] == 's' && dp[1] == '.' && itsit(cp, dp+2))
			return (1);
		while (*cp && *dp && *cp == *dp)
			cp++, dp++, i--;
		if (*cp == 0 && *dp == 0)
			return (1);
		while (isdigit(*dp))
			dp++;
		if (*cp == 0 && *dp++ == '.') {
/* removed for GNO, because we want to look up compressed files also. */
#ifndef __GNO__
			--i;
			while (i > 0 && *dp)
				if (--i, *dp++ == '.')
					return (*dp++ == 'C' && *dp++ == 0);
#endif
			return (1);
		}
   }
	return (0);
}

/*
 * addDirectories
 *	Parse the configuration file and determine in what directories
 *	we're supposed to be looking.
 */

char linebuf[LINEBUF_SIZE];

#define STATE_AT_FILE	1
#define STATE_SAW_ID	2
#define STATE_SAW_COLON	3
#define STATE_SAW_PATHS	4

#define IDENT_BIN	1
#define	IDENT_MAN	2
#define	IDENT_SRC	3

void
getDirectories (void) {
	char *confpath, *p;
        FILE *fp;
        int state, line, ident;

	if (((confpath = getenv(CONF_ENV)) == NULL) ||
	    (*confpath == '\0')) {
		confpath = CONF_PATH;
	}
	if ((fp = fopen(confpath, "r")) == NULL) {
		err(1, "couldn't open config file %s", confpath);
	}
	state = STATE_AT_FILE;
	line = 0;
	while (fgets (linebuf, LINEBUF_SIZE, fp) != NULL) {

		/* increment the line count */
		line++;

		/* delete any comments */
		if ((p = strchr(linebuf, '#')) != NULL) {
			*p = '\0';
		}

		/* parse each word */
		if ((p = strtok(linebuf, " \t\r\n\v")) == NULL) {
			continue;
		}
		do {
			switch(state) {
			case STATE_AT_FILE:
				if (!strcmp(p, PROWORD_BIN)) {
					state = STATE_SAW_ID;
					ident = IDENT_BIN;
				} else if (!strcmp(p, PROWORD_MAN)) {
					state = STATE_SAW_ID;
					ident = IDENT_MAN;
				} else if (!strcmp(p, PROWORD_SRC)) {
					state = STATE_SAW_ID;
					ident = IDENT_SRC;
				} else {                  
					errx(1,
					     "%s: line %d: identifier expected",
					     confpath, line);
				}
				break;
			case STATE_SAW_ID:
				if ((*p != ':') || (*(p+1) != '\0')) {
					errx(1, "%s: line %d: expected ':'",
					     confpath, line);
				}
				state = STATE_SAW_COLON;
				break;
			case STATE_SAW_COLON:
				if ((*p == ';') && (*(p+1) == '\0')) {
					state = STATE_AT_FILE;
				} else {
					addIdent(ident, p);
				}
				break;
			case STATE_SAW_PATHS:
				break;
			default:	assert(0);
			}             
		} while ((p = strtok(NULL, NULL)) != NULL);
	}
	fclose(fp);
}

/*
 * addIdent
 *	Add an entry to the bindirs, mandirs, or srcdirs arrays, as
 *	appropriate.
 */
void addIdent (int ident, char *path) {
	char ***dirptr;

	switch(ident) {
	case IDENT_BIN:
		LC_StringArrayAdd(Sbindirs, path);
		break;
	case IDENT_MAN:
		LC_StringArrayAdd(Smandirs, path);
		break;
	case IDENT_SRC:
		LC_StringArrayAdd(Ssrcdirs, path);
		break;
	default:
		assert(0);
	}
}
