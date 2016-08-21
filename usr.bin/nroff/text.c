#undef OLD_WAY
/*
 *	text.c - text output processing portion of nroff word processor
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
 * $Id: text.c,v 1.2 1997/03/20 06:40:51 gdr Exp $
 */

#ifdef __ORCAC__
segment "text______";
#pragma noroot
#pragma optimize 78
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef __GNO__
#include <termcap.h>
#else
#include "unix/termcap.h"
#endif

#include "nroff.h"
#include "io.h"
#include "escape.h"

/*
 *	output buffer control parameters
 */
static struct {
    int	outp;		/* next avail char pos in outbuf, init = 0 */
    int	outw;		/* width of text currently in buffer */
    int	outwds;		/* number of words in buffer, init = 0 */
    int	lpr;		/* output to printer, init = FALSE */
    int	outesc;		/* number of escape char on this line */
    char	outbuf[MAXLINE];/* output of filled text */
} co;


static void 	bold (char *p0, char *p1, int size);
static void 	center (char *p);
static void 	expand (char *p0, char c, char *s);
static void 	justcntr (char *p, char *q, int *limit);
static void 	justleft (char *p, char *q, int limit);
static void 	justrite (char *p, char *q, int limit);
static void 	leadbl (char *p);
static void 	puttl (char *p, int *lim, int pgno);
static void 	putwrd (char *wrdbuf);
static void 	spread (char *p, int outp, int nextra, int outwds,int escapes);
static void 	underl (char *p0, char *p1, int size);
static int	width (char *s);
static void do_mc (char *p);


/*
 * text
 *	main text processing
 *
 *	Pre:	<line>	contains the line we wish to process (and print)
 *			it is MAXLINE characters long, including the
 *			terminating NULL character.
 *
 *	Post:
 */
void
text (char *line) {
    register int	i;
    char		wrdbuf[MAXLINE];

    /*
     *   skip over leading blanks if in fill mode. we indent later.
     *   since leadbl does a robrk, do it if in .nf mode
     */
    if (dc.fill == YES) {
	if (*line == ' ' || *line == '\n' || *line == '\r') {
	    /* note that leadbl shifts <line> left as necessary */
	    leadbl (line);
	}
    } else {
	robrk ();
    }

    /*
     *   expand escape sequences from <line> into <wrdbuf>
     */
    expesc (line, wrdbuf, MAXLINE);
    
    /*
     *   test for how to output
     */
    if (dc.ulval > 0) {
	/*
	 *   underline (.ul)
	 *
	 *   Because of the way underlining is handled,
	 *   MAXLINE should be declared to be three times
	 *   larger than the longest expected input line
	 *   for underlining.  Since many of the character
	 *   buffers use this parameter, a lot of memory
	 *   can be allocated when it may not really be
	 *   needed.  A MAXLINE of 180 would allow about
	 *   60 characters in the output line to be
	 *   underlined (remember that only alphanumerics
	 *   get underlined - no spaces or punctuation).
	 */
	underl (line, wrdbuf, MAXLINE);
	--dc.ulval;
    }
    if (dc.cuval > 0) {
	/*
	 *   continuous underline (.cu)
	 */
	underl (line, wrdbuf, MAXLINE);
	--dc.cuval;
    }
    if (dc.boval > 0) {
	/*
	 *   bold (.bo)
	 */
	bold (line, wrdbuf, MAXLINE);
	--dc.boval;
    }
    if (dc.ceval > 0) {
	/*
	 *   centered (.ce)
	 */
	center (line);
	do_mc (line);
	put (line);
	--dc.ceval;
    } else if ((*line == '\r' || *line == '\n') && dc.fill == NO) {
	/*
	 *   all blank line
	 */
	do_mc (line);
	put (line);
    } else if (dc.fill == NO) {
	/*
	 *   unfilled (.nf)
	 */
	do_mc (line);
	put (line);
    } else {
	/*
	 *   anything else...
	 *
	 *   init escape char counter for this line...
	 */
	/*		co.outesc = 0;*/
	
	
	/*
	 *   get a word and put it out. increment ptr to the next
	 *   word.
	 */
	while ((i = getwrd (line, wrdbuf)) > 0) {
	    /*			co.outesc += countesc (wrdbuf);*/
	    
	    putwrd (wrdbuf);
	    line += i;
	}
    }
}


/*
 * bold
 *	insert bold face text (by overstriking)
 */
static void
bold (register char *p0, register char *p1, int size)
{

	register int	i;
	register int	j;

	j = 0;
	for (i = 0; (p0[i] != '\n') && (j < size - 1); ++i)
	{
		if (isalpha (p0[i]) || isdigit (p0[i]))
		{
			p1[j++] = p0[i];
			p1[j++] = '\b';
		}
		p1[j++] = p0[i];
	}
	p1[j++] = '\n';
	p1[j] = EOS;
	while (*p1 != EOS)
		*p0++ = *p1++;
	*p0 = EOS;
}

/*
 * center
 *	center a line by setting tival
 */
static void
center (register char *p) {
    int t;
    
    t = (dc.rmval + dc.tival - width (p)) >> 1;
    dc.tival = MAX(t, 0);
}


/*
 * expand
 *	expand title buffer to include character string
 */
static void
expand (register char *p0, char c, register char *s) {
    register char  *p;
    register char  *q;
    register char  *r;
    char    	tmp[MAXLINE];
    
    p = p0;
    q = tmp;
    while (*p != EOS) {
	if (*p == c) {
	    r = s;
	    while (*r != EOS) {
		*q++ = *r++;
	    }
	} else {
	    *q++ = *p;
	}
	++p;
    }
    *q = EOS;
    strcpy (p0, tmp);		/* copy it back */
}

/*
 * justcntr
 *	center title text into print buffer
 */
static void
justcntr (register char *p, char *q, int *limit) {
    register int	len;
    
    len = width (p);
    q   = &q[(limit[RIGHT] + limit[LEFT] - len) >> 1];
    while (*p != EOS) {
	*q++ = *p++;
    }
}

/*
 * justleft
 *	left justify title text into print buffer
 */
static void
justleft (register char *p, char *q, int limit) {
    q = &q[limit];
    while (*p != EOS) {
	*q++ = *p++;
    }
}

/*
 * justrite
 *	right justify title text into print buffer
 */
static void
justrite (register char *p, char *q, int limit) {
    register int	len;
    
    len = width (p);
    q = &q[limit - len];
    while (*p != EOS) {
	*q++ = *p++;
    }
}

/*
 * leadbl
 *	delete leading blanks, set tival
 *
 * REVIEWED
 */
static void
leadbl (register char *p) {
    register char *q;
    register int i;
    
    /*
     *   end current line and reset co struct
     */
    robrk ();
    
    /*
     *   skip spaces
     */
    for (i = 0; p[i] == ' ' || p[i] == '\t'; ++i);
    
    /*
     *   if not end of line, reset current temp indent
     */
    if (p[i] != '\n' && p[i] != '\r') {
	dc.tival = i;
    }

    /*
     *   shift string
     */
    q = &p[i];
#ifdef DEBUG
    i = 0;
#endif
    while (*q) {
#ifdef DEBUG
	i++;
	ASSERT(i<MAXLINE, ("leadbl buffer overflow\n"));
#endif
	*p++ = *q++;
    }
    *p = '\0';
}


/*
 * pfoot
 *	put out page footer
 */
void
pfoot (void) {
    if (dc.prflg == TRUE) {
	skip (pg.m3val);
	if (pg.m4val > 0) {
	    if ((pg.curpag % 2) == 0) {
		puttl (pg.efoot, pg.eflim, pg.curpag);
	    } else {
		puttl (pg.ofoot, pg.oflim, pg.curpag);
	    }
	    skip (pg.m4val - 1);
	}
    }
}


/*
 * phead
 *	put out page header
 */
void
phead (void) {
    pg.curpag = pg.newpag;
    if (pg.curpag >= pg.frstpg && pg.curpag <= pg.lastpg) {
	dc.prflg = TRUE;
    } else {
	dc.prflg = FALSE;
    }
    ++pg.newpag;
    set_ireg ("%", pg.newpag, 0);
    if (dc.prflg == TRUE) {
	if (pg.m1val > 0) {
	    skip (pg.m1val - 1);
	    if ((pg.curpag % 2) == 0) {
		puttl (pg.ehead, pg.ehlim, pg.curpag);
	    } else {
		puttl (pg.ohead, pg.ohlim, pg.curpag);
	    }
	}
	skip (pg.m2val);
    }
    /* 
     *	initialize lineno for the next page
     */
    pg.lineno = pg.m1val + pg.m2val + 1;
    set_ireg ("ln", pg.lineno, 0);
}


/*
 * puttl
 *	put out title or footer
 */
static void
puttl (register char *p, int *lim, int pgno) {
    register int	i;
    char		pn[8];
    static char		t[MAXLINE];
    static char		h[MAXLINE];
    char		delim;
    
    itoda (pgno, pn, 6);
    for (i = 0; i < MAXLINE; ++i) {
	h[i] = ' ';
    }
    delim = *p++;
    p = getfield (p, t, delim);
    expand (t, dc.pgchr, pn);
    justleft (t, h, lim[LEFT]);
    p = getfield (p, t, delim);
    expand (t, dc.pgchr, pn);
    justcntr (t, h, lim);
    p = getfield (p, t, delim);
    expand (t, dc.pgchr, pn);
    justrite (t, h, lim[RIGHT]);
    for (i = MAXLINE - 4; h[i] == ' '; --i) {
	h[i] = EOS;
    }
    h[++i] = '\n';
    h[++i] = '\r';
    h[++i] = EOS;
    if (strlen (h) > 2) {
	for (i = 0; i < pg.offset; ++i) {
	    PRCHAR2(' ', out_stream);
	}
    }
    putlin (h, out_stream);
}

/*
 * putwrd
 *	put word in output buffer
 */

static void
putwrd (register char *wrdbuf) {
    register char  *p0;
    register char  *p1;
    int     	w;
    int     	last;
    int     	llval;
    int         nextra;
    int     	esc;
    
    /*
     *   check if this word puts us over the limit
     */
    w     = width (wrdbuf);
    last  = strlen (wrdbuf) + co.outp;
    llval = dc.rmval - dc.tival;
    /* if (((co.outp > 0) && ((co.outw + w) > llval))*/
    esc = countesc (wrdbuf);
    co.outesc += esc;
    if (((co.outp > 0) && ((co.outw + w - co.outesc) > llval))
	||(last > MAXLINE)) {
	/*
	 *   last word exceeds limit so prepare to break line, print
	 *   it, and reset outbuf.
	 */
	last -= co.outp;
	if (dc.juval == YES) {
	    nextra = llval - co.outw + 1;
	    
	    /*
	     *      Do not take in the escape char of the
	     *      word that didn't fit on this line anymore
	     */
	    co.outesc -= esc;
	    
	    /* 
	     *	Check whether last word was end of
	     *	sentence and modify counts so that
	     *	it is right justified.
	     */
	    if (co.outbuf[co.outp - 2] == ' ') {
		--co.outp;
		++nextra;
	    }
#ifdef OLD_WAY
	    spread (co.outbuf, co.outp - 1, nextra, co.outwds, co.outesc);
	    if ((nextra > 0) && (co.outwds > 1)) {
		co.outp += (nextra - 1);
	    }
#if 0
	    if (co.outesc > 0) {
		co.outp += co.outesc;
	    }
#endif /* 0 */
#else  /* OLD_WAY */
	    spread (co.outbuf, co.outp - 1, nextra, co.outwds, co.outesc);
	    if ((nextra + co.outesc > 0) && (co.outwds > 1)) {
		co.outp += (nextra + co.outesc - 1);
	    }
#endif
	}

	/*
	 *   break line, output it, and reset all co members. reset
	 *   esc count.
	 */
	robrk ();
	co.outesc = esc;
    }
    
    /*
     *   copy the current word to the out buffer which may have been
     *   reset
     */
    p0 = wrdbuf;
    p1 = co.outbuf + co.outp;
    while (*p0 != EOS) {
	*p1++ = *p0++;
    }
    
    co.outp              = last;
    co.outbuf[co.outp++] = ' ';
    co.outw             += w + 1;
    co.outwds           += 1;
}

/*
 * skip
 *	skips the number of lines specified by n.
 */
void 
skip (register int n) {
    register int	i;
    register int	j;
    
    if (dc.prflg == TRUE && n > 0) {
	for (i = 0; i < n; ++i) {
	    /*
	     *   handle blank line with changebar
	     */
	    if (mc_ing == TRUE) {
		for (j = 0; j < pg.offset; ++j) {
		    PRCHAR2(' ', out_stream);
		}
		for (j = 0; j < dc.rmval; ++j) {
		    PRCHAR2(' ', out_stream);
		}
		for (j = 0; j < mc_space; j++) {
		    PRCHAR2(' ', out_stream);
		}
		PRCHAR(mc_char, out_stream);
	    }
	    PRCHAR2('\n', out_stream);
#if 0
	    /* gdr: not required */
	    prchar ('\r', out_stream);
#endif
	}
    }
}


/*
 * spread
 *	spread words to justify right margin
 */
static void
spread (register char *p, int outp, int nextra, int outwds, int escapes) {
    register int	i;
    register int	j;
    register int	nb;
    register int	ne;
    register int	nholes;


    /*
     *   quick sanity check...
     */
#ifdef OLDWAY
    if ((nextra <= 0) || (outwds <= 1)) {
	return;
    }
#else
    if ((nextra + escapes < 1) || (outwds < 2)) {
	return;
    }
#endif
    
    /*
     *   set up for the spread and do it...
     */
    dc.sprdir = ~dc.sprdir;
#ifdef OLD_WAY
    ne        = nextra;
#else
    ne        = nextra + escapes;
#endif
    nholes    = outwds - 1;			/* holes between words */
    i         = outp - 1;			/* last non-blank character */
    j         = MIN(MAXLINE - 3, i + ne);	/* leave room for CR,LF,EOS */
#if 0
    j        += escapes;
    if (p[i-1] == 27) {
	j += 2;
    }
    j = MIN(j, MAXLINE - 3);
#endif
    while (i < j) {
	p[j] = p[i];
	if (p[i] == ' ') {
	    if (dc.sprdir == 0) {
		nb = (ne - 1) / nholes + 1;
	    } else {
		nb = ne / nholes;
	    }
	    ne -= nb;
	    --nholes;
	    for (; nb > 0; --nb) {
		--j;
		p[j] = ' ';
	    }
	}
	--i;
	--j;
    }
}


/*
 * strkovr
 *	split overstrikes (backspaces) into seperate buffer
 */
int 
strkovr (char *p, char *q) {
    register char  *pp;
    int		bsflg;
    
    bsflg = FALSE;
    pp = p;
    while (*p != EOS) {
	*q = ' ';
	*pp = *p;
	++p;
	if (*p == '\b') {
	    if (*pp >= ' ' && *pp <= '~') {
		bsflg = TRUE;
		*q = *pp;
		++p;
		*pp = *p;
		++p;
	    }
	}
	++q;
	++pp;
    }
    *q++ = NEWLINE;
    *q = *pp = EOS;
    
    return bsflg;
}


/*
 * underl
 *	underline a line
 */
static void
underl (register char *p0, register char *p1, int size) {
    register int	i;
    register int	j;
    
    j = 0;
    for (i = 0; (p0[i] != '\n') && (j < size - 1); ++i) {
	if (p0[i] >= ' ' && 
	    p0[i] <= '~' &&
	    (isalpha (p0[i]) || isdigit (p0[i]) || dc.cuval > 0)) 
	  {
	      p1[j++] = '_';
	      p1[j++] = '\b';
	  }
	p1[j++] = p0[i];
    }
    p1[j++] = '\n';
    p1[j] = EOS;
    while (*p1 != EOS) {
	*p0++ = *p1++;
    }
    *p0 = EOS;
}

/*
 * width
 *	compute width of character string
 */
static int 
width (char *s) {
    register int	w;
    
    w = 0;
    for (; *s != '\0'; s++) {
        char c = *s;
	ASSERT((c != '\n' && c != '\r'), ("\n"));
	if ((c >= 32) && (c < 127)) {
	    ++w;
	} 
    else if (c == '\b') {
	    --w;
	}
	/* ignore high-bit chars and other control chars */
    }
    return w;
}


/*
 * do_mc
 *	add margin char (change bar) for .nf and .ce lines.
 *
 *	filled lines handled in robrk(). blank lines (.sp) handled in skip().
 *	note: robrk() calls this routine, too.
 */
static void 
do_mc (char *p) {
    register char  *ps;
    register int	nspaces;
    register int	i;
    register int	has_cr;
    register int	has_lf;
    int		len;
    int		nesc;
    
    if (mc_ing == FALSE) {
	return;
    }

    len = strlen (p);

    /*
     *   get to the end...
     */
    ps = p;
    while (*ps) {
	ps++;
    }

    /*
     *   check for cr and lf
     */
    ps--;
    has_lf = 0;
    has_cr = 0;
    while (ps >= p && (*ps == '\r' || *ps == '\n')) {
	if (*ps == '\n') {
	    has_lf++;
	} else {
	    has_cr++;
	}
	len--;
	ps--;
    }
    if (has_lf < has_cr) {
	has_lf = has_cr;
    } else if (has_cr < has_lf) {
	has_cr = has_lf;
    }

    /*
     *   remove any trailing blanks here
     */
    while (ps >= p && *ps == ' ') {
	ps--;
	len--;
    }
    *++ps = EOS;


    /*
     *   add trailing spaces for short lines. count escapes, subtract
     *   from len. use rmval for rigth margin (minus tival which is
     *   added later in put).
     */
    nesc    = countesc (p);
    len    -= nesc;
    nspaces = dc.rmval - dc.tival - len;
    for (i = 0; i < nspaces; i++, ps++) {
	*ps = ' ';
    }

    /*
     *   add the bar...
     */
    for (i = 0; i < mc_space; i++, ps++) {
	*ps = ' ';
    }
    *ps++ = mc_char;
    
    /*
     *   replace cr, lf, and EOS
     */
    while (has_lf--) {
	*ps++ = '\r';
	*ps++ = '\n';
    }
    *ps = EOS;
    
    return;
}

void
initOutbuf (void) {
    co.outp   = 0;
    co.outw   = 0;
    co.outwds = 0;
    co.lpr    = FALSE;
    co.outesc = 0;
    memset(co.outbuf, EOS, MAXLINE);
#if 0
    for (i = 0; i < MAXLINE; ++i) {
	co.outbuf[i] = EOS;
    }
#endif
}


/*
 * robrk
 *
 *	End current filled line.  References and modifies globals:
 *          co
 */
void
robrk (void) {
    if (co.outp > 0) {
	/*
	 *   handle margin char (change bar) here for all filled lines
	 */
	ASSERT(MAXLINE - co.outp > 2, ("output buffer overrun: >%d:%d:%d<\n", 
				       MAXLINE, co.outp, MAXLINE- co.outp));
	co.outbuf[co.outp]   = '\r';
	co.outbuf[co.outp+1] = '\n';
	co.outbuf[co.outp+2] = EOS;

	do_mc (co.outbuf);
	
	put (co.outbuf);
    }
    co.outp   = 0;
    co.outw   = 0;
    co.outwds = 0;
    co.outesc = 0;
}

void
setPrinting(int val) {
    co.lpr = val;
}
