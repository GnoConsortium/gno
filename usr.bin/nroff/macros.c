/*
 *	macros.c - macro input/output processing for nroff word processor
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
 * $Id: macros.c,v 1.3 1997/10/30 04:04:34 gdr Exp $
 */

#ifdef __ORCAC__
segment "macros____";
#pragma noroot
#pragma optimize 78
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __GNO__
#include <err.h>
#else
#include "unix/err.h"
#endif

#ifdef sparc
#include "unix/sunos.h"
#endif

#include "nroff.h"
#include "macros.h"
#include "io.h"

static int colmac (char *p, char *d, int i);
static int putmac (char *name, char *p);

void 
initMacros (void) {
    memset(mac.mnames, 0, MXMDEF * sizeof(char*));
    memset(mac.mb, 0, MACBUF);
    memset(mac.pbb, 0, MAXLINE);
#if 0
    for (i = 0; i < MXMDEF; ++i) {
	mac.mnames[i] = NULL_CPTR;
    }
    for (i = 0; i < MACBUF; ++i) {
	mac.mb[i] = EOS;
    }
    for (i = 0; i < MAXPBB; ++i) {
	mac.pbb[i] = EOS;
    }
#endif
    mac.lastp = 0;
    mac.emb   = &mac.mb[0];
    mac.ppb   = NULL_CPTR;

}

/*
 * defmac
 *
 *	Define a macro. top level, read from stream.
 *
 *	we should read macro without interpretation EXCEPT:
 *
 *	1) number registers are interpolated
 *	2) strings indicated by \* are interpolated
 *	3) arguments indicated by \$ are interpolated
 *	4) concealed newlines indicated by \(newline) are eliminated
 *	5) comments indicated by \" are eliminated
 *	6) \t and \a are interpreted as ASCII h tab and SOH.
 *	7) \\ is interpreted as backslash and \. is interpreted as a period.
 *
 *	currently, we do only 3. a good place to do it would be here before
 *	putmac, after colmac...
 */

void
defmac (char *line, FILE *infp)
{
    register char  *q;
    register int	i;
    char    	name[MNLEN];
    char    	defn[MXMLEN];
    char		newend[10];

#undef BORK
#ifdef BORK
    fprintf(stderr, "DEBUG defmac (%s:%d): \"%s\"\n", __FILE__, __LINE__, line);
#endif

    /*
     * terminate defn; if it's still this way when we're ready to putmac(),
     * then the macro definition was empty and we're going to ignore it.
     */
    defn[0] = '\0';

    /*
     *   skip the .de and get to the name...
     */
    q = skipwd (line);
    q = skipbl (q);

    /*
     *   q now points to the name.  Copy it into the "name" buffer.
     *   Make sure it is valid (i.e. first char is alpha...). The
     *   getwrd function returns the length of the word.
     */
    i = getwrd (q, name);
    if (!isprint (*name)) {
	errx(-1, "missing or illegal macro definition name");
    }
    
    /*
     *   truncate to 2 char max name.
     */
    if (i > 2) {
	name[2] = EOS;
    }

    /*
     *   skip the name and see if we have a new end defined...
     */
    q = skipwd (line);
    q = skipbl (q);
    for (i = 0; i < 10; i++) {
	newend[i] = EOS;
    }
    
    for (i = 0; (i < 10) && ( isalpha (q[i]) || isdigit (q[i]) ); i++) {
	newend[i] = q[i];
    }

    /*
     *   read a line from input stream until we get the end of macro
     *   command (.en or ..). actually. we should have read the next
     *   field just above here to get the .de NA . or .de NA en string
     *   to be new end of macro.
     */
    i = 0;
    while (getlin (line, infp) != EOF) {
#ifdef BORK
	fprintf(stderr,"DEBUG: %s:%d line is \"%s\"\n", __FILE__, __LINE__,
		line);
#endif
	if (line[0] == dc.cmdchr && line[1] == '\\' && line[2] == '\"') {
	    /*
	     *   comment, ignore it
	     */
	    continue;
	}
	if (line[0] == dc.cmdchr && newend[0] != EOS
	    &&  line[1] == newend[0] && line[2] == newend[1]) {
	    /*
	     *   replacement end found
	     */
	    break;
	}
	if (line[0] == dc.cmdchr && line[1] == 'e' && line[2] == 'n') {
	    /*
	     *   .en found
	     */
	    break;
	}
	if (line[0] == dc.cmdchr && line[1] == dc.cmdchr) {
	    /*
	     *   .. found
	     */
#ifdef BORK
	    fprintf(stderr,"DEBUG: %s:%d: dot-dot found\n",__FILE__,__LINE__);
#endif
	    break;
	}
	
	/*
	 *   collect macro from the line we just read. all this does
	 *   is put it in the string defn.
	 */
#ifdef BORK
	fprintf(stderr,"DEBUG: %s:%d: collecting macro\n",__FILE__,__LINE__);
#endif
	if ((i = colmac (line, defn, i)) == ERR) {
	    errx(1, "macro definition too long");
	}
    }

    /*
     *   store the macro
     */
    if (!ignoring && defn[0] != '\0') {
	if (putmac (name, defn) == ERR) {
	    errx(-1, "macro definition table full");
	}
    }
}


/*
 * colmac
 *
 *	Collect macro definition from input stream
 */
static int 
colmac (char *p, char *d, int i) {

    char *pstart;
    int	istart;

    pstart = p;
    istart = i;
    while (*p != EOS) {
	/*
	 *   are we over the length limit for a single macro?
	 */
	if (i >= MXMLEN - 1) {
	    d[i - 1] = EOS;
	    return (ERR);
	}

	/*
	 *   "i break for comments..."
	 */
	if (*p == '\\' && *(p+1) == '\"') {
	    /*
	     *   first back over any whitespace between comment
	     *   start and last character in line. remember to
	     *   decrement counter i, too...
	     */
	    p--;
	    while (isspace (*p) && p > pstart && i > istart) {
		p--;
		i--;
	    }

	    /*
	     *   now skip over the comment until we reach the
	     *   trailing newline
	     */
	    while (*p != EOS) {
		if (*p == '\n' || *p == '\r') {
		    break;
		}
		p++;
	    }
	}
	
	/*
	 *   skip quoted things
	 */
	if (*p == '\\' && *(p+1) == '\\') {
	    p++;
	}

	/*
	 *   copy it
	 */
	d[i++] = *p++;
    }
    d[i] = EOS;
    return (i);
}


/*
 * putmac
 *
 *	Put macro definition into table
 *
 *	NOTE: any expansions of things like number registers SHOULD
 *	have been done already.
 */

static int
putmac (char *name, char *p) {
    
    /*
     *   any room left? (did we exceed max number of possible macros)
     */
    if (mac.lastp >= MXMDEF) {
	return (ERR);
    }

    /*
     *   will new one fit in big buffer?
     */
#ifdef DEBUG
    strlen(name);
    strlen(p);
#endif
    if (mac.emb + strlen (name) + strlen (p) + 1 > &mac.mb[MACBUF]) {
	return (ERR);
    }

    /*
     *   add it...
     *
     *   bump counter, set ptr to name, copy name, copy def.
     *   finally increment end of macro buffer ptr (emb).
     *
     *   macro looks like this in mb:
     *
     *	mac.mb[MACBUF]		size of total buf
     *	lastp < MXMDEF		number of macros possible
     *	*mnames[MXMDEF]		-> names, each max length
     *	..._____________________________...____________________...
     *	    / / /|X|X|0|macro definition      |0| / / / / / / /
     *	.../_/_/_|_|_|_|________________...___|_|/_/_/_/_/_/_/_...
     *		  ^
     *		  |
     *		  \----- mac.mnames[mac.lastp] points here
     *
     *   both the 2 char name (XX) and the descripton are null term and
     *   follow one after the other.
     */
    ++mac.lastp;
    mac.mnames[mac.lastp] = mac.emb;
    strcpy (mac.emb, name);
    strcpy (mac.emb + strlen (name) + 1, p);
    mac.emb += strlen (name) + strlen (p) + 2;
    
    return (OK);
}

/*
 * getmac
 *
 *	Get (lookup) macro definition from namespace
 */
char *
getmac (char *name) {
    int i;

    /*
     *   loop for all macros, starting with last one
     */
    for (i = mac.lastp; i >= 0; --i) {
	/*
	 *   is this REALLY a macro?
	 */
	if (mac.mnames[i]) {
	    /*
	     *   if it compares, return a ptr to it
	     */
	    if (!strcmp (name, mac.mnames[i])) {
#if 0  /* !!!debug */
		puts (mac.mnames[i]);
#endif
		
		if (mac.mnames[i][1] == EOS) {
		    return (mac.mnames[i] + 2);
		} else {
		    return (mac.mnames[i] + 3);
		}
		/*NOTREACHED*/
	    }
	}
    }
    
    /*
     *   none found, return null
     */
    return (NULL_CPTR);
}


/*
 * maceval
 *
 *	Evaluate macro expansion from p into m
 */
void
maceval (char *p, char *m) {
    register int	i;
    char	       *argp[15];
    char		c;
    int		xc;
    
    /*
     *   replace command char with EOS
     */
    *p++ = EOS;

    /* 
     *   initialize argp array to substitute command
     *   string for any undefined argument
     *
     *	NO!!! this is fixed...
     */
#if 0
    for (i = 0; i < 10; ++i) {
	argp[i] = p;
    }
#endif

    /*
     *   skip the command name
     */
    p = skipwd (p);
    *p++ = EOS;

    /*
     *   loop for all $n variables...
     */
    for (i = 0; i < 10; ++i) {
	/*
	 *   get to substituted param and if no more, reset remaining
	 *   args to NULL and stop. using "i" here IS ok...
	 */
	p = skipbl (p);
	if (*p == '\r' || *p == '\n' || *p == EOS) {
	    DEBUGGING(("maceval: set_ireg(.$, %d, 0)", i));

	    set_ireg (".$", i, 0);
	    for ( ; i < 10; i++) {
		argp[i] = NULL_CPTR;
	    }
	    break;
	}

	/*
	 *   ...otherwise, see if this param is quoted. if it is,
	 *   it is all one parameter, even with blanks (but not
	 *   newlines...). look for another "c" (which is the quote).
	 *
	 *   if no quote, just read the arg as a single word and null
	 *   terminate it.
	 */
	if (*p == '\'' || *p == '"') {
	    c = *p++;
	    argp[i] = p;
	    while (*p != c && *p != '\r' && *p != '\n' && *p != EOS) {
		++p;
	    }
	    *p++ = EOS;
	} else {
	    argp[i] = p;
	    p = skipwd (p);
	    *p++ = EOS;
	}
    }
    
    /*
     *   m contains text of the macro. p contained the input line.
     *   here we start at the end of the macro def and see if there
     *   are any $n thingies. go backwards.
     */
    for (i = strlen (m) - 1; i >= 0; --i) {
	/*
	 *   found a $.
	 */
	if (i > 0 && m[i - 1] == '$') {
	    if (!isdigit (m[i])) {
		/*
		 *   it wasn't a numeric replacement arg so
		 *   push this char back onto input stream
		 */
		PUTBAK(m[i]);
	    } else {
		/*
		 *   it WAS a numeric replacement arg. so we
		 *   want to push back the appropriate macro
		 *   invocation arg. m[i]-'0' is the numerical
		 *   value of the $1 thru $9. if the arg is
		 *   not there, argp[n] will be (char *) 0
		 *   and pbstr will do nothing.
		 */
		xc = m[i] - '1';
		if (argp[xc]) {
		    pbstr (argp[xc]);
		}
		--i;
	    }
	} else {
	    /*
	     *   no $ so push back the char...
	     */
	    PUTBAK(m[i]);
	}
    }
    
    /*
     *   at this point, the iobuf will hold the new macro command, full
     *   expanded for $n things. the return gets us right back to the
     *   main loop in main() and we parse the (new) command just as if
     *   it were read from a file.
     */
}


/*
 * printmac
 *	print macro data:
 *		opt		print
 *		===		=====
 *		 0		name and size
 *		 1		total size
 *		 2		full information
 */

void
printmac (int opt) {
    register int	i;		/* was long, minix needs int */
    register long	localSpace;
    register long	totalspace;
    register char  *pname;
    register char  *pdef;
    
    
    localSpace = 0L;
    totalspace = 0L;
    
    fflush (out_stream);
    fflush (err_stream);
    
    for (i = mac.lastp; i >= 0; --i) {
	/*
	 *   is this REALLY a macro?
	 */
	if (mac.mnames[i]) {
	    pname = (char *) (mac.mnames[i]);
	    pdef  = pname + 3;
	    if (*(pname + 1) == '\0') {
		pdef = pname + 2;
	    }
	    
	    localSpace  = (long) strlen (pdef);
	    totalspace += localSpace;
	    
	    switch (opt) {
	    case 0:
		fprintf (err_stream, "%s %ld\n", pname, localSpace);
		break;
	    case 2:
		fprintf (err_stream, "%s %ld\n", pname, localSpace);
		fprintf (err_stream, "%s\n", pdef);
		break;
	    case 1:
	    default:
		break;
	    }
	}
    }
    fprintf (err_stream, "Total space: %ld\n", totalspace);
}




/*
 * putstr
 *
 *	Put string definition into (macro) table
 *
 *	NOTE: any expansions of things like number registers SHOULD
 *	have been done already. strings and macros share mb buffer
 */

int
putstr (const char *name, const char *p) {

    /*
     *   any room left? (did we exceed max number of possible macros)
     */
    if (mac.lastp >= MXMDEF) {
	return (ERR);
    }

    /*
     *   will new one fit in big buffer?
     */
    if (mac.emb + strlen (name) + strlen (p) + 1 > &mac.mb[MACBUF]) {
	return (ERR);
    }
    
    
    /*
     *   add it...
     *
     *   bump counter, set ptr to name, copy name, copy def.
     *   finally increment end of macro buffer ptr (emb).
     *
     *   string looks like this in mb:
     *
     *	mac.mb[MACBUF]		size of total buf
     *	lastp < MXMDEF		number of macros/strings possible
     *	*mnames[MXMDEF]		-> names, each max length
     *	...______________________________...____________________...
     *	    / / /|X|X|0|string definition      |0| / / / / / / /
     *	.../_/_/_|_|_|_|_________________...___|_|/_/_/_/_/_/_/_...
     *		    ^
     *		    |
     *		    \----- mac.mnames[mac.lastp] points here
     *
     *   both the 2 char name (XX) and the descripton are null term and
     *   follow one after the other.
     */
    ++mac.lastp;
    mac.mnames[mac.lastp] = mac.emb;
    strcpy (mac.emb, name);
    strcpy (mac.emb + strlen (name) + 1, p);
    mac.emb += strlen (name) + strlen (p) + 2;
    return (OK);
}


/*
 * getstr
 *	Get (lookup) string definition from namespace
 */
char *
getstr (char *name) {
    int i;
    
    /*
     *   loop for all macros, starting with last one
     */
    for (i = mac.lastp; i >= 0; --i) {
	/*
	 *   is this REALLY a macro?
	 */
	if (mac.mnames[i]) {
	    /*
	     *   if it compares, return a ptr to it
	     */
	    if (!strcmp (name, mac.mnames[i])) {
#if 0		/* !!!debug */
		puts (mac.mnames[i]);
#endif
		if (mac.mnames[i][1] == EOS) {
		    return (mac.mnames[i] + 2);
		} else {
		    return (mac.mnames[i] + 3);
		}
	    }
	}
    }
    
    /*
     *   none found, return null
     */
    return NULL;
}


#ifdef NOT_USED

/*
 * putbak
 *
 *	Push character back into input stream. we use the push-back buffer
 *	stored with macros.
 */
void
putbak (char c)
{
    if (mac.ppb == NULL) {  /* first time executing this code */
	mac.ppb = mac.pbb;
	*mac.ppb = c;
    } else if (mac.ppb < mac.pbb + MAXPBB) {
	mac.ppb++;
	*(mac.ppb) = c;
    } else {
	errx(-1, "push back buffer overflow (%d chars)", MAXPBB);
    }
}



/*
 * ngetc
 *	get a character from the putback buffer or from <infp>
 */
int ngetc (FILE *infp)
{
    register int	c;

    if (mac.ppb >= mac.pbb) {
	c = *mac.ppb--;
    } else {
	c = getc (infp);
    }
    return (c);
}
#endif /* NOT_USED */
