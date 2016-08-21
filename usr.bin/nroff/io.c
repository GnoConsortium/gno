/*
 *	io.c - low level I/O processing portion of nroff word processor
 *
 *	adapted for atariST/TOS by Bill Rosenkranz 11/89
 *	net:	rosenkra@hall.cray.com
 *	CIS:	71460,17
 *	GENIE:	W.ROSENKRANZ
 *
 *	original author:
 *
 *	Stephen L. Browning
 *	5723 North Parker Avenue
 *	Indianapolis, Indiana 46220
 *
 *	history:
 *
 *	- Originally written in BDS C;
 *	- Adapted for standard C by W. N. Paul
 *	- Heavily hacked up to conform to "real" nroff by Bill Rosenkranz
 *      - Heavily modified by Devin Reade to avoid memory trashing bugs.
 *
 * $Id: io.c,v 1.2 1997/03/20 06:40:50 gdr Exp $
 */

#ifdef __ORCAC__
segment "io________";
#pragma noroot
#pragma optimize 79
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#ifdef __GNO__
#include <err.h>
#include <termcap.h>
#else
#include "unix/err.h"
#include "unix/termcap.h"
#endif

#ifdef sparc
#include "unix/sunos.h"
#endif

#include "nroff.h"
#include "macros.h"
#include "io.h"
#include "escape.h"

char  __i;
char *__s;
FILE *fpGLOB;

int
tprchar(char c)
{
    return putc( c, fpGLOB );
}

/*
 * getlin
 *
 *	retrieve one line of input text from the push-back buffer or
 *	in_buf, place it in out_buf
 */


int
getlin (char *out_buf, FILE *in_buf) {
#if 0
    /* 
     * gdr: these optimizations should work, but don't as yet ...
     */
    register char  *q;
    register int	i;
    int		c;
    int		nreg;
    const char *dotc = ".c";
    

    i = 0;

    /* get characters from the push-back buffer */
    q = out_buf;
    while ((i < MAXLINE - 1) && (mac.ppb >= mac.pbb)) {
#if 1 /* doesn't happen */
	c = *mac.ppb--;
	if (c == EOF) {
	    *q = EOS;
	    return (i == 0) ? EOF : i;
	}
	*q++ = c;
#else
	*q++ = c = *mac.ppb--;
#endif
	if (c == '\n') {
	    break;
	}
	i++;
    }
    *q = EOS;

    /* get characters from stream */
    c = MAXLINE - i;
    if (c > 0) {
	fgets(q, c, in_buf);
    }
    
    FINDREG(dotc, nreg, q);
    if (nreg > 0) {
	set_ireg (dotc, rg[nreg].rval + 1, 0);
    }
    
    i = strlen(out_buf);
    assert(i || feof(in_buf));
    return (i == 0) ? EOF : i;
#else  /* gdr: old way */
    register char  *q;
    register int	i;
    int		c;
    int		nreg;
    const char *dotc = ".c";
    
    q = out_buf;
    for (i = 0; i < MAXLINE - 1; ++i) {
	c = NGETC (in_buf);
	if (c == EOF) {
	    *q = EOS;
	    c  = strlen (out_buf);
	    return (c == 0 ? EOF : c);
	}
	if (c == '\r') c = '\n';

	*q++ = c;
	if (c == '\n') {
	    break;
	}
    }
    *q = EOS;
    
    FINDREG(dotc, nreg, q);
    if (nreg > 0) {
	set_ireg (dotc, rg[nreg].rval + 1, 0);
    }
    
    return (strlen (out_buf));
#endif
}


/*
 * pbstr
 *	Push back string into input stream
 */
void
pbstr (char *str) {
    char *p;

    if (str == NULL) {
	return;
    }
    p = str + strlen(str) - 1;
    while (p >= str) {
	PUTBAK(*p);
	--p;
    }
    return;
}


/*
 * put
 *	put out line with proper spacing and indenting
 */
void
put (char *p) {
    register int	j;
    static char		os[MAXLINE];
    
    if (pg.lineno == 0 || pg.lineno > pg.bottom) {
	phead ();
    }
    if (dc.prflg == TRUE) {
	if (!dc.bsflg) {
	    if (strkovr (p, os) == TRUE) {
		for (j = 0; j < pg.offset; ++j) {
		    PRCHAR2(' ', out_stream);
		}
		for (j = 0; j < dc.tival; ++j) {
		    PRCHAR2(' ', out_stream);
		}
		putlin (os, out_stream);
	    }
	}
	for (j = 0; j < pg.offset; ++j) {
	    PRCHAR2(' ', out_stream);
	}
	for (j = 0; j < dc.tival; ++j) {
	    PRCHAR2(' ', out_stream);
	}
	putlin (p, out_stream);
    }
    dc.tival = dc.inval;
    skip (MIN(dc.lsval - 1, pg.bottom - pg.lineno));
    pg.lineno = pg.lineno + dc.lsval;
    set_ireg ("ln", pg.lineno, 0);
    if (pg.lineno > pg.bottom) {
	pfoot ();
#ifdef GEMDOS
	if (stepping) {
	    wait_for_char();
	}
#endif
    }
}

/*
 * putlin
 *	output a null terminated string to the file
 *	specified by pbuf.
 */
void
putlin (char *p, FILE *pbuf)
{
    while (*p != EOS) {
	PRCHAR(*p, pbuf);
	p++;
    }
}



#ifdef NOT_USED

/*
 * prchar
 *	print character with test for printer
 */
void
prchar (char __c, FILE *fp)
{
#if 0
/* this really slows things down. it should be fixed. for now, ignore
 * line printer...
 */
    if (fp == stdout) {
	putc (c, fp);
    } else {
	putc_lpr (c, fp);
    }
#endif

#if 0
    switch(c) {
    case S_STANDOUT:
	fpGLOB = fp;
	tputs(s_standout,1,tprchar);
	break;
    case E_STANDOUT:
	fpGLOB = fp;
	tputs(e_standout,1,tprchar);
	break;
    case S_BOLD:
	fpGLOB = fp;
	tputs(s_bold,1,tprchar);
	break;
    case E_BOLD:
	fpGLOB = fp;
	tputs(e_bold,1,tprchar);
	break;
    case S_ITALIC:
	fpGLOB = fp;
	tputs(s_italic,1,tprchar);
	break;
    case E_ITALIC:
	fpGLOB = fp;
	tputs(e_italic,1,tprchar);
	break;
    case 13:
	break;
    default:
	putc(c, fp);
    }
#else
    /*
     * Don't use a case statement here; the macros are out of range
     * for some compilers.
     */
    if (__c == S_STANDOUT) {
	fpGLOB = fp;
	tputs(s_standout,1,tprchar);
    } else if (__c == E_STANDOUT) {
	fpGLOB = fp;
	tputs(e_standout,1,tprchar);
    } else if (__c == S_BOLD) {
	fpGLOB = fp;
	tputs(s_bold,1,tprchar);
    } else if (__c == E_BOLD) {
	fpGLOB = fp;
	tputs(e_bold,1,tprchar);
    } else if (__c == S_ITALIC) {
	fpGLOB = fp;
	tputs(s_italic,1,tprchar);
    } else if (__c == E_ITALIC) {
	fpGLOB = fp;
	tputs(e_italic,1,tprchar);
    } else if (__c == 13) {
	;
    } else {
	putc(c, fp);
    }
#endif

}

/*
 * putc_lpr
 *	write char to printer
 */
void
putc_lpr (char c, FILE *fp)
{
    putc (c, fp);
}

#endif /* NOT_USED */
