/*
 *	low.c - misc low-level functions for nroff word processor
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
 * $Id: low.c,v 1.2 1997/03/20 06:40:50 gdr Exp $
 */

#ifdef __ORCAC__
segment "low_______";
#pragma noroot
#pragma optimize 79
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#ifdef __GNO__
#include <err.h>
#else
#include "unix/err.h"
#endif

#ifdef sparc
#include "unix/sunos.h"
#endif

#include "nroff.h"


/* convert ascii character to decimal */
#define ATOD(c) (((c) < '0') || ((c) > '9')) ? -1 : ((c) - '0')

#if 0 /* not currently used */
static void	inptobu (char *);	/* convert input units to b.u. */
static void	butochar (char *);	/* convert b.u. to char spaces */
#endif

/*
 * ctod
 *
 *	convert string to decimal. processes only positive values.
 *	this takes a constant like "1", "1.0i", etc. 
 *
 *      Returns converted value (including zero) on success, zero on failure.
 */
int
ctod (char  *p) {
    long  val;
    int   d;
    char *pp = p;
    char *ptmp;
    int   rside = 0;
    int	  lside = 0;
    int	  has_rside = 0;
    int	  has_lside = 0;
    
    if (*p == EOS) {
	return 0;
    }
    
    ptmp = skipwd (pp);
    pp = --ptmp;
    
    switch (*pp) {
    case 'i':
    case 'c':
	val = 0L;
	while (*p != EOS && isdigit (*p)) {
	    has_lside++;
	    lside = ATOD(*p);
	    p++;
	    if (lside == -1) {
		break;
	    }
	    val = 10L * val + (long) lside;
	}
	lside = (int) val;
	if (*p == '.') {
	    p++;
	    val = 0L;
	    while (*p != EOS && isdigit (*p)) {
		has_rside++;
		rside = ATOD(*p);
		p++;
		if (rside == -1) {
		    break;
		}
		val = 10L * val + (long) rside;
		if (has_rside > 2) {	/* more than enough */
		    break;
		}
	    }
	    rside = (int) val;
	}

	/*
	 *   now put it together. 1.0i -> 240, 1.50i -> 360, etc.
	 */
	val = 0L;
	if (has_lside) {
	    val = (long) lside * BU_INCH;
	}
	switch (has_rside) {
	case 1:
	    val = val + ((long) rside * BU_INCH / 10L);
	    break;
	case 2:
	    val = val + ((long) rside * BU_INCH / 100L);
	    break;
	case 3:
	    val = val + ((long) rside * BU_INCH / 1000L);
	    break;
	default:
	    break;
	}

	if (*pp == 'c') {
	    val = (val * BU_CM) / BU_INCH;
	}
	
	/*
	 *   for now we convert to basic char size, 1 em...
	 */
	val = val / BU_EM;
	break;
	
    case 'P':
    case 'm':
    case 'n':
    case 'p':
    case 'u':
    case 'v':
	val = 0L;
	while (*p != EOS) {
	    d = ATOD(*p);
	    p++;
	    if (d == -1)
	      break;
	    val = 10L * val + (long) d;
	}
	switch (*pp) {
	case 'P':
	    val = val * BU_PICA;
	    break;
	case 'p':
	    val = val * BU_POINT;
	    break;
	case 'u':
	    val = val * BU_BU;
	    break;
	case 'm':
	    val = val * BU_EM;
	    break;
	case 'n':
	    val = val * BU_EN;
	    break;
	case 'v':
	    val = val * BU_EM;
	    break;
	}

	/*
	 *   for now we convert to basic char size, 1 em...
	 */
	val = val / BU_EM;
	break;

    default:
	/*
	 *   this is the default behavior. it SHOULD make things
	 *   compatible with the old way...
	 */
	val = 0L;
	while (*p != EOS) {
	    d = ATOD(*p);
	    p++;
	    if (d == -1) {
		break;
	    }
	    val = 10L * val + (long) d;
	}
	break;
    }
    
    return (int) val;
}

#if 0  /* not currently used */

/*
 * inptobu
 *
 *	convert input units to b.u.
 */
static void
inptobu (char *ps)
{
    return;
}

/*
 * butochar
 *
 *	convert b.u. to char spaces
 */
static void
butochar (char *ps)
{
    return;
}

#endif /* 0 */

/*
 * skipbl
 *
 *	skip blanks and tabs in character buffer. return ptr to first
 *	non-space or non-tab char. this could mean EOS or \r or \n.
 */
char *
skipbl (register char *p)
{
    while ((*p != EOS) && (*p == ' ' || *p == '\t')) {
	p++;
    }
    return (p);
}


/*
 * skipwd
 *
 *	skip over word and punctuation. anything but space,\t,\r,\n, and EOS
 *	is skipped. return ptr to the first of these found.
 */
char *
skipwd (register char *p) {
    while (*p != EOS && *p != ' ' && *p != '\t' && *p != '\r' && *p != '\n') {
	p++;
    }
    return (p);
}


/*
 * nroffSpace
 *
 *	space vertically n lines. this does header and footer also.
 */
void
nroffSpace (int n) {
    robrk ();
    if (pg.lineno > pg.bottom) {
	return;
    }
    if (pg.lineno == 0) {
	phead ();
    }
    skip (MIN(n, pg.bottom + 1 - pg.lineno));
    pg.lineno += n;
    set_ireg ("ln", pg.lineno, 0);
    if (pg.lineno > pg.bottom) {
	pfoot ();
    }
}


/*
 * getfield
 *	get field from title
 */
char   *
getfield (register char *p, register char *q, char delim) {
    while (*p != delim && *p != '\r' && *p != '\n' && *p != EOS) {
	*q++ = *p++;
    }
    *q = EOS;
    if (*p == delim) {
	++p;
    }
    return p;
}


/*
 * getwrd
 *
 *	get non-blank word from p0 into p1.
 *	return number of characters processed.
 */
int
getwrd (register char *src, register char *dest) {
#if 0   /* gdr changes */
    char *orgsrc, *orgdest;
    
    orgsrc = src;
    orgdest = dest;

    /*
     *   skip leading whitespace
     */
    while (*src == ' ' || *src == '\t') {
	src++;
    }

    /* find end of word */
    while (*src != '\0' && !isspace(*src)) {
	*dest++ = *src++;
    }

    /* I don't understand what this is supposed to achieve */
#ifdef WHATTHEFUCK
    assert (dest - orgdest >= 1);  /* gdr */
    c = *(dest - 1);
    if (c == '"') {
	asssert(dest - orgdest >= 2);  /* gdr */
	c = *(dest - 2);
    }
    if (c == '?' || c == '!') {
	*dest++ = ' ';
    }
    if (c == '.' && (*src == '\n' || *src == '\r' || islower (*p))) {
	*dest++ = ' ';
    }
#endif /* WHATTHEFUCK */
    *dest = EOS;

    return src - orgsrc;
#elif 1
    char		c;
    register int	i;
    register char  *p;
    char *orgsrc, *orgdest;

    orgsrc = src;
    orgdest = dest;
    /*
     *   init counter...
     */
    i = 0;

    /*
     *   skip leading whitespace
     */
    while (*src == ' ' || *src == '\t') {
	++i;
	++src;
    }
    
    /*
     *   set ptr and start to look for end of word
     */
    p = src;
    while (*src != ' ' && *src != EOS && *src != '\t') {
	if (*src == '\n' || *src == '\r') {
	    break;
	}
	*dest = *src++;
	++dest;
	++i;
    }

    if (dest > orgdest) {
	c = *(dest - 1);
	if (c == '"' && (dest > orgdest + 1)) {
	    c = *(dest - 2);
	}
	if (c == '?' || c == '!') {
	    *dest++ = ' ';
	    ++i;
	}
	if (c == '.' && (*src == '\n' || *src == '\r' || islower (*p))) {
	    *dest++ = ' ';
	    ++i;
	}
    }
    *dest = EOS;

    return (i);
#else
    char		c;
    register int	i;
    register char  *p;
    char *orgsrc, *orgdest;

    orgsrc = src;
    orgdest = dest;
    /*
     *   init counter...
     */
    i = 0;

    /*
     *   skip leading whitespace
     */
    while (*src == ' ' || *src == '\t') {
	++i;
	++src;
    }
    
    /*
     *   set ptr and start to look for end of word
     */
    p = src;
    while (*src != ' ' && *src != EOS && *src != '\t') {
	if (*src == '\n' || *src == '\r') {
	    break;
	}
	*dest = *src++;
	++dest;
	++i;
    }

    if (dest > orgdest) {
	c = *(dest - 1);
	if (c == '"') {
	    ASSERT(dest > orgdest + 1,
		   ("%s:%d: array indexing error", __FILE__, __LINE__));
	    c = *(dest - 2);
	}
	if (c == '?' || c == '!') {
	    *dest++ = ' ';
	    ++i;
	}
	if (c == '.' && (*src == '\n' || *src == '\r' || islower (*p))) {
	    *dest++ = ' ';
	    ++i;
	}
    }
    *dest = EOS;

    return (i);
#endif
}


/*
 * countesc
 *	count atari escape sequence characters in given null-terminated
 *	string
 */

#define ESC			27

int
countesc (register char *p) {

	#ifdef SHORT_STANDOUT
	return 0;
	#else

    register char  *pp;
    register int	num;
    
    pp  = p;
    num = 0;
    
    while (*pp != EOS) {
	if (*pp == ESC) {
	    /*
	     *   count escape char (atari-specific, vt52)
	     *   generally only p,q,b,and c will show up...
	     */
	    switch (*(pp+1)) {
	    case 'A':			/* ESC-a */
	    case 'B':
	    case 'C':
	    case 'D':
	    case 'E':
	    case 'H':
	    case 'I':
	    case 'J':
	    case 'K':
	    case 'L':
	    case 'M':
	    case 'd':
	    case 'e':
	    case 'f':
	    case 'j':
	    case 'k':
	    case 'l':
	    case 'o':
	    case 'p':
	    case 'q':
	    case 'v':
	    case 'w':
		num += 2;
		break;
	    case 'b':			/* ESC-a-b */
	    case 'c':
		num += 3;
		break;
	    case 'Y':			/* ESC-a-b-c */
	    case '[':			/* Esc [ 7 m */
		num += 4;
		break;
	    default:
		num += 1;
		break;
	    }
	}
	pp++;
    }
    return num;
    #endif
}


/*
 * itoda
 *	convert integer to decimal ascii string
 *
 *	Pre:	<value> is the number to convert
 *		<p>	is the output buffer
 *		<size>	is the number of bytes in the output buffer
 *
 *	Post:	<p>	contains an ascii representation of <value>
 *
 *	Returns:	
 */
int
itoda (int value, register char *p, register int size) {
#if 0
    int len;
    /*
     * buffer must be big enough for representation of largest integer,
     * plus 1 byte for sign, plus 1 byte for terminator.  On a 32-bit
     * machine this is 10+1+1 == 12 bytes.
     */
#define MAX_INT_LENGTH 15
    char buffer[MAX_INT_LENGTH];

    len = sprintf(buffer, "%d", value);
    if (len == EOF) {
	errx(1, "itoda failed with EOF");
    }
    if (len >= size) {
	errx(1, "%s:%d:  buffer overflow", __FILE__, __LINE__);
    }
    strncpy(p, buffer, len);
    return len;
#else
    
    int i, j, aval;
    char c[20];
    
    aval = abs (value);
    c[0] = EOS;
    i = 1;
    do {
	c[i++] = (aval % 10) + '0';
	aval /= 10;
    } while (aval > 0 && i <= size);
    
    if (value < 0 && i <= size) {
	c[i++] = '-';
    }
    for (j = 0; j < i; ++j) {
	*p++ = c[i - j - 1];
    }

    return i;
#endif
}


/*
 * itoROMAN
 *	convert integer to upper roman. must be positive
 */
int
itoROMAN (int value, register char *p, register int size) {
    register int	i;
    register int	j;
    register int	aval;
    char		c[100];
    int		rem;
    
    aval = abs (value);
    c[0] = EOS;
    i = 1;
    
    /*
     *   trivial case:
     */
    if (aval == 0) {
	c[i++] = '0';
	goto done_100;
    }
    
    /*
     *   temporarily mod 100...
     */
    aval = aval % 100;
    
    if (aval > 0) {
	/*
	 *   build backward
	 *
	 *   | I|		1
	 *   | II|		2
	 *   | III|		3
	 *   | VI|		4
	 *   | V|		5
	 *   | IV|		6
	 *   | IIV|		7
	 *   | IIIV|		8
	 *   | XI|		9
	 *   | X|		0
	 *   | IX|		11
	 *   | IIX|		12
	 */
	if ((aval % 5 == 0) && (aval % 10 != 0)) { /* 5 */ 
	    c[i++] = 'V';
	} else {
	    rem = aval % 10;
	    if (rem == 9) {			/* 9 */
		c[i++] = 'X';
		c[i++] = 'I';
	    }else if (rem == 8) {		/* 8 */
		c[i++] = 'I';
		c[i++] = 'I';
		c[i++] = 'I';
		c[i++] = 'V';
	    } else if (rem == 7) {		/* 7 */
		c[i++] = 'I';
		c[i++] = 'I';
		c[i++] = 'V';
	    } else if (rem == 6) {		/* 6 */
		c[i++] = 'I';
		c[i++] = 'V';
	    } else if (rem == 4) {		/* 4 */
		c[i++] = 'V';
		c[i++] = 'I';
	    } else {				/* 3,2,1 */
		for (j = 0; j < rem; j++) {
		    c[i++] = 'I';
		}
	    }
	}

	aval /= 10;
	if (aval == 0) {
	    goto done_100;
	}

	rem = aval % 10;
	if (rem == 4) {
	    c[i++] = 'L';
	    c[i++] = 'X';
	} else if (rem == 5) {
	    c[i++] = 'L';
	} else if (rem < 4) {
	    for (j = 0; j < rem; j++) {
		c[i++] = 'X';
	    }
	} else {
	    for (j = 0; j < rem - 5; j++) {
		c[i++] = 'X';
	    }
	    c[i++] = 'L';
	}
    }

    
 done_100:
    /*
     *   divide by 100 (they are done) and temp mod by another 10
     */
    aval  = abs (value);
    aval /= 100;
    
    if (aval > 0) {
	rem  = aval % 10;
	if (rem == 4) {
	    c[i++] = 'D';
	    c[i++] = 'C';
	}
	if (rem == 5) {
	    c[i++] = 'D';
	} else if (rem < 4) {
	    for (j = 0; j < rem; j++) {
		c[i++] = 'C';
	    }
	} else if (rem == 9) {
	    c[i++] = 'M';
	    c[i++] = 'C';
	} else if (rem < 9) {
	    for (j = 0; j < rem - 5; j++) {
		c[i++] = 'C';
	    }
	    c[i++] = 'D';
	}
    }

    aval /= 10;
    
    if (aval > 0) {
	rem  = aval % 10;
	if (rem < 4) {
	    for (j = 0; j < rem; j++) {
		c[i++] = 'M';
	    }
	}
    }

    if (value < 0) {
	c[i++] = '-';
    }
    
    for (j = 0; j < i; ++j) {
	*p++ = c[i - j - 1];
    }
    
    return i;
}


/*
 * itoroman
 *	convert integer to lower roman
 */
int
itoroman (int value, char *p, int size)
{
    register int	i;
    register int	len;
    char		c[100];
    
    c[0] = EOS;
    len = itoROMAN (value, c, size);
    
    for (i = 0; i < len; i++) {
	p[i] = c[i];
	if (isalpha (p[i])) {
	    p[i] = tolower (c[i]);
	}
    }
    return len;
}

/*
 * itoLETTER
 *	convert integer to upper letter value: 0,A,B,C,...,AA,AB,AC,...
 */
int
itoLETTER (int value, register char *p, register int size) {
    register int	i;
    register int	j;
    register int	aval;
    char		c[20];
    
    aval = abs (value);
    c[0] = EOS;
    i = 1;
    
    /*
     *   1 based:
     *
     *   0	0
     *   1	A
     *   25	Z
     *   26	AA
     *   51 AZ
     *   52 AAA
     *   ...
     */
    if (aval == 0) {
	c[i++] = '0';
    } else if (aval < 27) {
	c[i++] = aval - 1 + 'A';
    } else {
	do {
	    c[i++] = ((aval - 1) % 26) + 'A';
	    aval = (aval - 1)  / 26;
	} while (aval > 0 && i <= size);
    }

    if (value < 0 && i <= size) {
	c[i++] = '-';
    }

    for (j = 0; j < i; ++j) {
	*p++ = c[i - j - 1];
    }

    return i;
}


/*
 * itoletter
 *	convert integer to upper letter value: 0,a,b,c,...,aa,ab,ac,...
 */
int
itoletter (int value, register char *p, register int size) {
    register int	i;
    register int	j;
    register int	aval;
    char		c[20];
    
    aval = abs (value);
    c[0] = EOS;
    i = 1;
    
    /*
     *   1 based:
     *
     *   0	0
     *   1	A
     *   25	Z
     *   26	AA
     *   51 AZ
     *   52 AAA
     *   ...
     */
    if (aval == 0) {
	c[i++] = '0';
    } else if (aval < 27) {
	c[i++] = aval - 1 + 'a';
    } else {
	do {
	    c[i++] = ((aval - 1) % 26) + 'a';
	    aval = (aval - 1)  / 26;
	} while (aval > 0 && i <= size);
    }

    if (value < 0 && i <= size) {
	c[i++] = '-';
    }

    for (j = 0; j < i; ++j) {
	*p++ = c[i - j - 1];
    }

    return i;
}


#if 0	/* min, max not needed */
/*
 * min
 *	find minimum of two integer ONLY
 */
#ifdef min
#undef min
#endif

int
min (register int v1, register int v2) {
    return ((v1 < v2) ? v1 : v2);
}


/*
 * max
 *	find maximum of two integers ONLY
 */

#ifdef max
#undef max
#endif

int
max (register int v1, register int v2) {
    return ((v1 > v2) ? v1 : v2);
}
#endif /* 0 */

/*
 * err_exit
 *	exit cleanly on fatal error (close files, etc). also handles normal
 *	exit.
 */
void 
err_exit (int code) {
    if (err_stream != stderr && err_stream != (FILE *) 0) {
	/*
	 *   not going to stderr (-o file)
	 */
	fflush (err_stream);
	fclose (err_stream);
    }
    if (debugging && dbg_stream != stderr && dbg_stream != (FILE *) 0) {
	fflush (dbg_stream);
	fclose (dbg_stream);
    }
    if (out_stream != stdout && out_stream != (FILE *) 0) {
	/*
	 *   not going to stdout (-l)
	 */
	fflush (out_stream);
	fclose (out_stream);
    }
    
#ifdef GEMDOS
    if (hold_screen) {
	wait_for_char();
    }
#endif
    exit(code);
}



#ifdef GEMDOS
#include <osbind.h>

/*
 * Wait for a character
 */
void 
wait_for_char (void) {
    printf ("enter any key..."); fflush (stdout);
    Cconin ();
}
#endif
