/*
 *	strings.c - String input/output processing for nroff word processor
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
 * $Id: strings.c,v 1.2 1997/03/20 06:40:51 gdr Exp $
 */

#ifdef __ORCAC__
segment "strings___";
#pragma noroot
#pragma optimize 79
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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

static int colstr (const char *src, char *dest, size_t len);

/*
 * defstr
 *
 *	Define a string. top level, read from command line.
 *
 *	we should read string without interpretation EXCEPT:
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
 *	putstr, after colstr...
 */
void 
defstr (char *line) {
    register char  *q;
    register int	i;
    static char		name[MNLEN];
    static char		defn[MXMLEN+1];
    
    name[0] = '\0';
    defn[0] = '\0';

#ifdef BORK    
    fprintf(stderr, "DEBUG: %s:%d name = 0x%x\tdefn = 0x%x line=%s\n",
	    __FILE__, __LINE__, name, defn, line);
#endif
    /*
     *   skip the .ds and get to the name...
     */
    q = skipwd (line);
    q = skipbl (q);
    
    /*
     *   ok, name now holds the name. make sure it is valid (i.e. first
     *   char is alpha...). getwrd returns the length of the word.
     */
    i = getwrd (q, name);
    if (!name[0]) {
	errx(-1, "missing or illegal string definition name");
    }
    
    /*
     *   truncate to 2 char max name.
     */
    if (i > 2) {
	name[2] = EOS;
    }

    /*
     *   skip the name to get to the string. it CAN start with a " to
     *   have leading blanks...
     */
    q = skipwd (q);
    q = skipbl (q);
    
    /*
     *   read rest of line from input stream and collect string into
     *   temp buffer defn
     */
    if ((i = colstr (q, defn, MXMLEN)) == ERR) {
	errx(-1, "string definition too long");
    }
    
    /*
     *   store the string
     */
    if (putstr (name, defn) == ERR) {
	errx(-1, "string definition table full");
    }
}




/*
 * colstr
 *	Collect string definition from input stream
 */
static int
colstr (const char *src, char *dest, size_t len) {
    const char *orgsrc;
    char *orgdest, *destlimit;

    orgsrc = src;
    orgdest = dest;
    destlimit = dest + len;  /* don't let dest go past this pointer */

#ifdef BORK
    fprintf(stderr,"DEBUG: %s:%d src=\"%s\"\n", __FILE__, __LINE__, src);
#endif
    /*
     *   if there is a " here, we have leading blanks (skipbl in caller
     *   found it). just get past it...
     */
    if (*src == '\"') {
	src++;
    }

    while ((*src != '\n') && (*src != '\r') && (*src != '\0')
	   && (dest < destlimit - 1)) {

	/*
	 * If it's a comment, it ends terminates string collection
	 */
	if (*src == '\\' && *(src+1) == '\"') {
	    /*
	     * first back over any whitespace between the start of the
	     * comment and the last non-whitespace character preceeding
	     * the comment
	     */
	    while (dest > orgdest && isspace(*dest)) {
		--dest;
	    }
	    
	    /* forward src until we hit newline or end of string */
	    while (*src != '\n' && *src != '\r' && *src != '\0') {
		src++;
	    }
	    break;
	}

	/*
	 *   stop at the newline...
	 */
	if (*src == '\n' || *src == '\r') {
	    break;
	}
	
	/*
	 *   copy it
	 */
	*dest++ = *src++;
    }
    *dest = '\0';

    /* did we attempt to go over the allowed length for dest? */
    if (*src == '\0' || *src == '\n' || *src == '\r') {
	return dest - orgdest;
    } else {
	return ERR;
    }
}
