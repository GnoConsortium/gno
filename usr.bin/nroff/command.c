/*
 *	command.c - command input parser/processor for nroff text processor
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
 * $Id: command.c,v 1.1 1997/03/14 06:22:26 gdr Exp $
 */


#ifdef __ORCAC__
segment "command___";
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef __GNO__
#include <err.h>
#else
#include "err.h"
#endif

#ifdef sparc
#include "sunos.h"
#endif

#include "nroff.h"
#include "escape.h"
#include "macros.h"
#include "io.h"

#define iscond(x)	((x)=='>'||(x)=='<'||(x)=='=')
#define isoper(x)	((x)=='+'||(x)=='-'||(x)=='*'||(x)=='/'||(x)=='%')

static int comtyp (char *p, char *m);
static void read_if (void);
static void gettl (char *p, char *q, int *limit);
static int getval (char *p, char *p_argtyp);
static int getnumeric (char *p);
static int do_oper (int first, int oper, int second);

/*
 * comand
 *	main command processor
 */
void
comand (char *p) {
    register int	i;
    register int	ct;
    register int	val;
    register int   	indx;
    int     	spval;
    int		not_cond;
    char    	argtyp;
    char    	name[MAXLINE];
    char    	macexp[MXMLEN];
    int		tmp;
    char	       *pfs;
    char		fs[20];
    char		c;
    char	       *ps1;
    char	       *ps2;
    
    DEBUGGING(("comand: enter, p=|%s|",p));

    /*
     *   get command code
     */
    ct = comtyp (p, macexp);
    
    /*
     *   error?
     */
    if (ct == UNKNOWN) {
	warnx("unrecognized command %s", p);
	return; 	/* gdr: added in this return */
    }
    
    /*
     *   ignore comments
     */
    if (ct == COMMENT) {
	return;
    }
    
    /*
     *   do escape expansion on command line args from p into name
     */
    expesc (p, name, MAXLINE);
  
  
    /*
     *   get value of command
     */
#if 0
    val = getval (p, &argtyp);
#endif

  
    /*
     *   do the command
     */
    switch (ct) {
	/* set (&param, val, type, defval, minval, maxval) */
    case FC:
	/*
	 *   field delim/pad chars
	 *
	 *   .fc [delim] [pad]
	 */
	warnx(".fc not available");
	break;
	
    case TR:
	/*
	 *   translate
	 *
	 *   .tr ab...
	 */
	warnx(".tr not available");
	break;
    
    case AD:
	/*
	 *   adjust
	 *
	 *   .ad [mode]
	 */
	val = getval (p, &argtyp);
	p = skipwd (p);
	p = skipbl (p);
	
	switch (*p) {
	case 'l':
	    dc.adjval = ADJ_LEFT;
	    dc.juval  = YES;
	    break;
	case 'r':
	    dc.adjval = ADJ_RIGHT;
	    dc.juval  = YES;
	    break;
	case 'c':
	    dc.adjval = ADJ_CENTER;
	    dc.juval  = YES;
	    break;
	case 'b':
	case 'n':
	    dc.adjval = ADJ_BOTH;
	    dc.juval  = YES;
	    break;
	default:
	    break;
	}
	break;

    case AF:
	/*
	 *   assign format to number reg
	 *
	 *   .af R {1,a,A,i,I,0...1}
	 */
	val = getval (p, &argtyp);
	p = skipwd (p);
	p = skipbl (p);
	if (!isalpha (*p)) {
	    warn("invalid or missing number register name");
	} else {
	    /*
	     *   number register format is 1,a,A,i,I,0...1
	     *   default is 1. for 0001 format, store num dig
	     *   or'ed with 0x80, up to 8 digits.
	     */
	    indx = tolower (*p) - 'a';
	    p = skipwd (p);
	    p = skipbl (p);
	    if (*p == '1')	dc.nrfmt[indx] = '1';
	    else if (*p == 'a')	dc.nrfmt[indx] = 'a';
	    else if (*p == 'A')	dc.nrfmt[indx] = 'A';
	    else if (*p == 'i')	dc.nrfmt[indx] = 'i';
	    else if (*p == 'I')	dc.nrfmt[indx] = 'I';
	    else if (*p == '0') {
		for (i = 0; isdigit (p[i]); i++)
		  ;
		dc.nrfmt[indx] = (char) (i);
		if (dc.nrfmt[indx] <= 0)
		  dc.nrfmt[indx] = '1';
		else if (dc.nrfmt[indx] > 8)
		  {
		      dc.nrfmt[indx]  = 8;
		      dc.nrfmt[indx] |= 0x80;
		  }
		else
		  dc.nrfmt[indx] |= 0x80;
		
	    }
	    else
	      dc.nrfmt[indx] = '1';
	}
	break;
    case BD:
	/*
	 *   embolden font (IGNORED)
	 *
	 *   .bd [S] F N
	 */
	break;
    case BO:
	/*
	 *   bold face
	 *
	 *   .bo [N]
	 */
	val = getval (p, &argtyp);
	set (&dc.boval, val, argtyp, 1, 0, HUGE);
	dc.cuval = dc.ulval = 0;
	break;
    case BP:
	/*
	 *   begin page
	 *
	 *   .bp [+/-N]
	 */
	val = getval (p, &argtyp);
	if (pg.lineno > 0)
	  nroffSpace (HUGE);
	set (&pg.curpag, val, argtyp, pg.curpag + 1, -HUGE, HUGE);
	pg.newpag = pg.curpag;
	set_ireg ("%", pg.newpag, 0);
	break;
    case BR:
	/*
	 *   break (page)
	 *
	 *   .br
	 */
	robrk ();
	break;
    case BS:
	/*
	 *   backspc in output
	 *
	 *   .bs [N]
	 */
	val = getval (p, &argtyp);
	set (&dc.bsflg, val, argtyp, 1, 0, 1);
	break;
    case C2:
	/*
	 *   nobreak char
	 *
	 *   .c2 [c=']
	 */
	val = getval (p, &argtyp);
	if (argtyp == '\r' || argtyp == '\n') {
	    dc.nobrchr = '\'';
	} else {
	    dc.nobrchr = argtyp;
	}
	break;
    case CC:
	/*
	 *   command character
	 *
	 *   .cc [c=.]
	 */
	val = getval (p, &argtyp);
	if (argtyp == '\r' || argtyp == '\n') {
	    dc.cmdchr = '.';
	} else {
	    dc.cmdchr = argtyp;
	}
	break;
    case CE:
	/*
	 *   center
	 *
	 *   .ce [N]
	 */
	val = getval (p, &argtyp);
	robrk ();
	set (&dc.ceval, val, argtyp, 1, 0, HUGE);
	break;
    case CS:
	/*
	 *   constant space char (IGNORED)
	 *
	 *   .cs F N M
	 */
	break;
    case CU:
	/*
	 *   continuous underline
	 *
	 *   .cu [N]
	 */
	val = getval (p, &argtyp);
	set (&dc.cuval, val, argtyp, 1, 0, HUGE);
	dc.ulval = dc.boval = 0;
	break;
    case DE:
	/*
	 *   define macro
	 *
	 *   .de name [end]
	 */
	val = getval (p, &argtyp);
	ignoring = FALSE;
	defmac (p, sofile[dc.flevel]);
	break;
    case DS:
	/*
	 *   define string
	 *
	 *   .ds name string
	 */
	val = getval (p, &argtyp);
	defstr (p);
	break;
    case EC:
	/*
	 *   escape char
	 *
	 *   .ec [c=\]
	 */
	val = getval (p, &argtyp);
	if (argtyp == '\r' || argtyp == '\n') {
	    dc.escchr = '\\';
	} else {
	    dc.escchr = argtyp;
	}
	dc.escon = YES;
	break;
    case EF:
	/*
	 *   even footer
	 *
	 *   .ef "a" "b" "c"
	 */
	val = getval (p, &argtyp);
	gettl (p, pg.efoot, &pg.eflim[0]);
	break;
    case EH:
	/*
	 *   even header
	 *
	 *   .eh "a" "b" "c"
	 */
	val = getval (p, &argtyp);
	gettl (p, pg.ehead, &pg.ehlim[0]);
	break;
    case EN:
	/*
	 *   end macro def (should not get one here...)
	 *
	 *   .en or ..
	 */
	warnx("missing .de command");
	break;
    case EO:
	/*
	 *   escape off
	 *
	 *   .eo
	 */
	dc.escon = NO;
	break;
    case EX:
	/*
	 *   exit
	 *
	 *   .ex
	 */
	if (sofile[0] != stdin) {
	    fclose (sofile[0]);
	}
	for (i = 1; i <= Nfiles; i++) {
	    if (sofile[i] != NULL_FPTR) {
		fclose (sofile[i]);
	    }
	}
	err_exit(0);
	break;
    case FI:
	/*
	 *   fill
	 *
	 *   .fi
	 */
	robrk ();
	dc.fill = YES;
	break;
    case FL:
	/*
	 *   flush NOW
	 *
	 *   .fl
	 */
	fflush (out_stream);
	break;
    case FO:
	/*
	 *   footer
	 *
	 *   .fo "a" "b" "c"
	 */
	val = getval (p, &argtyp);
	gettl (p, pg.efoot, &pg.eflim[0]);
	gettl (p, pg.ofoot, &pg.oflim[0]);
	break;
    case FT:
	/*
	 *   font change
	 *
	 *   .ft {R,I,B,S,P}
	 *
	 *   the way it's implemented here, it causes a break
	 *   rather than be environmental...
	 */
	val = getval (p, &argtyp);
	p = skipwd (p);
	p = skipbl (p);
	if (!isalpha (*p)) {
	    warnx("invalid or missing font name");
	} else {
	    pfs = &fs[0];
	    
	    fontchange (*p, pfs);
	    
	    robrk ();
	    fflush (out_stream);
	    fprintf (out_stream, "%s", pfs);
	    fflush (out_stream);
	}
	break;
    case TL:
    case HE:
	/*
	 *   header (both are currently identical. .he is -me)
	 *
	 *   .tl "a" "b" "c"
	 *   .he "a" "b" "c"
	 */
	val = getval (p, &argtyp);
	gettl (p, pg.ehead, &pg.ehlim[0]);
		gettl (p, pg.ohead, &pg.ohlim[0]);
	break;
    case IE:
	/*
	 *   if of if/else conditional
	 *
	 *   .ie condition anything
	 *   .el anything
	 *
	 *   .ie condition \{\
	 *   ...
	 *   ... \}
	 *   .el \{\
	 *   ...
	 *   ... \}
	 */
	warnx(".ie not available");
	break;
    case EL:
	/*
	 *   else of if/else conditional
	 *
	 *   .ie condition anything
	 *   .el anything
	 *
	 *   .ie condition \{\
	 *   ...
	 *   ... \}
	 *   .el \{\
	 *   ...
	 *   ... \}
	 */
	warnx(".el not available");
	break;
    case IF:
	/*
	 *   conditional
	 *
	 *   .if c command		[c=n(roff),t(roff),e(ven),o(dd)]
	 *   .if !c command
	 *   .if 's1's2' command	[s1 == s2]
	 *   .if !'s1's2' command	[s1 != s2]
	 *   .if N command		[N > 0]
	 *   .if !N command		[N <= 0]
	 *
	 *   .if cond \{\
	 *   command
	 *   ... \}
	 */
	p = skipwd (p);
	p = skipbl (p);
	not_cond = 0;
	if (*p == '!') {
	    p++;
	    not_cond = 1;
	}
	if (islower (*p) && isspace (*(p+1))) {
	    /*
	     *   single char: n=nroff,t=troff,e=evenpage,o=oddpage
	     */
	    c = *p;
	    switch (c) {
	    case 'n':		/* if nroff... (always T) */
		p = skipwd (p);
		p = skipbl (p);
		
		DEBUGGING(("comand: p=|%s|", p));
		if (*p != EOS && not_cond == 0) {
		    if (*p == '\\' && *(p+1) == '{') {
			read_if ();
		    } else {
			if (*p == dc.cmdchr) {
			    comand (p);
			} else {
			    if (*p == '\"') {
				p++;
			    }
			    if (*p == ' ') {
				robrk ();
			    }
			    text (p);
			}
		    }
		}
		break;
	    case 't':		/* if troff... (always F) */
		p = skipwd (p);
		p = skipbl (p);
		
		DEBUGGING(("comand: p=|%s|", p));
		
		if (*p != EOS && not_cond != 0) {
		    if (*p == '\\' && *(p+1) == '{') {
			read_if ();
		    } else {
			if (*p == dc.cmdchr) {
			    comand (p);
			} else {
			    if (*p == '\"') {
				p++;
			    }
			    if (*p == ' ') {
				robrk ();
			    }
			    text (p);
			}
		    }
		}
		break;
	    case 'e':		/* if even page... */
		p = skipwd (p);
		p = skipbl (p);
		
		DEBUGGING(("comand: p=|%s|", p));
		
		if (((pg.curpag % 2) == 0 && not_cond == 0)
		    ||  ((pg.curpag % 2) != 0 && not_cond != 0)) {
		    /* could be newpag, too */
		    if (*p == '\\' && *(p+1) == '{') {
			read_if ();
		    } else {
			if (*p == dc.cmdchr) {
			    comand (p);
			} else {
			    if (*p == '\"') {
				p++;
			    }
			    if (*p == ' ') {
				robrk ();
			    }
			    text (p);
			}
		    }
		}
		break;
	    case 'o':		/* if odd page... */
		p = skipwd (p);
		p = skipbl (p);
		
		DEBUGGING(("comand: p=|%s|", p));
		
		if (((pg.curpag % 2) == 1 && not_cond == 0)
		    ||  ((pg.curpag % 2) != 1 && not_cond != 0)) {
		    if (*p == '\\' && *(p+1) == '{') {
			read_if ();
		    } else {
			if (*p == dc.cmdchr) {
			    comand (p);
			} else {
			    if (*p == '\"') {
				p++;
			    }
			    if (*p == ' ') {
				robrk ();
			    }
			    text (p);
			}
		    }
		}
		break;
	    }
	} else if (*p == '\'' || *p == '/' || *p == '\"') {
	    /*
	     *   compare strings. we need to interpolate here
	     */
	    c = *p;
	    ps1 = ++p;
	    while (*p != EOS && *p != c) {
		p++;
	    }
	    *p = EOS;
	    ps2 = ++p;
	    while (*p != EOS && *p != c) {
		p++;
	    }
	    *p = EOS;
	    
	    DEBUGGING(("comand: strcmp (ps1=|%s|,ps2=|%s|)", ps1, ps2));

	    if ((!strcmp (ps1, ps2) && not_cond == 0)
		||  ( strcmp (ps1, ps2) && not_cond != 0)) {
		p++;
		p = skipbl (p);
		
		if (*p == '\\' && *(p+1) == '{') {
		    read_if ();
		} else {
		    if (*p == dc.cmdchr) {
			comand (p);
		    } else {
			if (*p == '\"') {
			    p++;
			}
			if (*p == ' ') {
			    robrk ();
			}
			text (p);
		    }
		}
	    }
	} else {
	    /*
	     *   number
	     */
	    
	    DEBUGGING(("comand: p=|%s|", p));
	    val = getnumeric (p);
	    if ((val >  0 && not_cond == 0)
		||  (val <= 0 && not_cond != 0)) {
		p = skipwd (p);
		p = skipbl (p);
		
		if (*p == '\\' && *(p+1) == '{') {
		    read_if ();
		} else {
		    if (*p == dc.cmdchr) {
			comand (p);
		    } else {
			if (*p == '\"') {
			    p++;
			}
			if (*p == ' ') {
			    robrk ();
			}
			text (p);
		    }
		}
	    }
	}
	break;
    case IG:
	/*
	 *   ignore input lines
	 *
	 *   .ig name
	 */
	val = getval (p, &argtyp);
	ignoring = TRUE;
	defmac (p, sofile[dc.flevel]);
	break;
    case IN:
	/*
	 *   indenting
	 *
	 *   .in [+/-N]
	 */
	val = getval (p, &argtyp);
	set (&dc.inval, val, argtyp, 0, 0, dc.rmval - 1);
	set_ireg (".i", dc.inval, 0);
	dc.tival = dc.inval;
	break;
    case JU:
	/*
	 *   justify
	 *
	 *   .ju
	 */
	dc.juval = YES;
	break;
    case LG:
	/*
	 *   ligature (IGNORED)
	 *
	 *   .lg [N]
	 */
	break;
    case LL:
	/*
	 *   line length
	 *
	 *   .ll [+/-N]
	 *   .rm [+/-N]
	 */
	val = getval (p, &argtyp);
	set (&dc.rmval, val, argtyp, PAGEWIDTH, dc.tival + 1, HUGE);
	set (&dc.llval, val, argtyp, PAGEWIDTH, dc.tival + 1, HUGE);
	set_ireg (".l", dc.llval, 0);
	break;
    case LS:
	/*
	 *   line spacing
	 *
	 *   .ls [+/-N=+1]
	 */
	val = getval (p, &argtyp);
	set (&dc.lsval, val, argtyp, 1, 1, HUGE);
	set_ireg (".v", dc.lsval, 0);
	break;
    case LT:
	/*
	 *   title length
	 *
	 *   .lt N
	 */
	val = getval (p, &argtyp);
	set (&dc.ltval, val, argtyp, PAGEWIDTH, 0, HUGE);
	pg.ehlim[RIGHT] = dc.ltval;
	pg.ohlim[RIGHT] = dc.ltval;
		break;
    case M1:
	/*
	 *   topmost margin
	 *
	 *   .m1 N
	 */
	val = getval (p, &argtyp);
	set (&pg.m1val, val, argtyp, 2, 0, HUGE);
	break;
    case M2:
	/*
	 *   second top margin
	 *
	 *   .m2 N
	 */
	val = getval (p, &argtyp);
	set (&pg.m2val, val, argtyp, 2, 0, HUGE);
	break;
    case M3:
	/*
	 *   1st bottom margin
	 *
	 *   .m3 N
	 */
	val = getval (p, &argtyp);
	set (&pg.m3val, val, argtyp, 2, 0, HUGE);
	pg.bottom = pg.plval - pg.m4val - pg.m3val;
	break;
    case M4:
	/*
	 *   bottom-most marg
	 *
	 *   .m4 N
	 */
	val = getval (p, &argtyp);
	set (&pg.m4val, val, argtyp, 2, 0, HUGE);
	pg.bottom = pg.plval - pg.m4val - pg.m3val;
	break;
    case MACRO:
	/*
	 *   macro expansion
	 *
	 *   (internal)
	 */
	maceval (p, macexp);
	break;
    case MC:
	/*
	 *   margin character (change bars)
	 *
	 *   .mc [c [N]]
	 *
	 *   right margin only, default 0.2i
	 */
	val = getval (p, &argtyp);
	if (argtyp == '\r' || argtyp == '\n') {
	    mc_ing = FALSE;		/* turn off... */
	} else {
	    mc_ing   = TRUE;	/* turn on... */
	    mc_space = 2;		/* force these for now... */
	    mc_char  = argtyp;	/* single char only!!! */
	    
	    p = skipwd (p);
	    p = skipbl (p);
	    
	    val = getval (p, &argtyp);
	    set (&mc_space, val, argtyp, 2, 0, dc.llval);
	}
	break;
    case NA:
	/*
	 *   no adjust
	 *
	 *   .na
	 */
	dc.adjval = ADJ_OFF;
	dc.juval  = NO;
	break;
    case NE:
	/*
	 *   need n lines
	 *
	 *   .ne N
	 */
	val = getval (p, &argtyp);
	robrk ();
	if ((pg.bottom - pg.lineno + 1) < (val * dc.lsval)) {
	    nroffSpace (HUGE);
	}
	break;
    case NF:
	/*
	 *   no fill
	 *
	 *   .nf
	 */
	robrk ();
	dc.fill = NO;
	break;
    case NJ:
	/*
	 *   no justify
	 *
	 *   .nj
	 */
	dc.juval = NO;
	break;
    case NR:
	/*
	 *   set number reg
	 *
	 *   .nr R +/-N M
	 */
	val = getval (p, &argtyp);
	p = skipwd (p);
	p = skipbl (p);
	if (!isalpha (*p)) {
	    warn("invalid or missing number register name");
	} else {
	    /*
	     *   indx is the register, R, and val is the final
	     *   value (default = 0). getval does skipwd,skipbl
	     */
	    indx = tolower (*p) - 'a';
	    val = getval (p, &argtyp);
	    set (&dc.nr[indx], val, argtyp, 0, -INFINITE, INFINITE);
	    
	    /*
	     *   now get autoincrement M, if any (default = 1).
	     *   getval does skipwd,skipbl
	     */
	    p = skipwd (p);
	    p = skipbl (p);
	    val = getval (p, &argtyp);
	    set (&dc.nrauto[indx], val, '1', 1, -INFINITE, INFINITE);
	}
	break;
    case OF:
	/*
	 *   odd footer
	 *
	 *   .of "a" "b" "c"
	 */
	val = getval (p, &argtyp);
	gettl (p, pg.ofoot, &pg.oflim[0]);
	break;
    case OH:
	/*
	 *   odd header
	 *
	 *   .oh "a" "b" "c"
		 */
	val = getval (p, &argtyp);
	gettl (p, pg.ohead, &pg.ohlim[0]);
	break;
    case PC:
	/*
	 *   page number char
	 *
	 *   .pc [c=NULL]
	 */
	val = getval (p, &argtyp);
	if (argtyp == '\r' || argtyp == '\n') {
	    dc.pgchr = EOS;
	} else {
	    dc.pgchr = argtyp;
	}
	break;
    case PL:
	/*
	 *   page length
	 *
	 *   .pl N
	 */
	val = getval (p, &argtyp);
	set (&pg.plval,
	     val,
	     argtyp,
	     PAGELEN,
	     pg.m1val + pg.m2val + pg.m3val + pg.m4val + 1,
	     HUGE);
	set_ireg (".p", pg.plval, 0);
	pg.bottom = pg.plval - pg.m3val - pg.m4val;
	break;
    case PM:
	/*
	 *   print macro names and sizes
	 *
	 *   .pm [t]
	 */
	val = getval (p, &argtyp);
	if (argtyp == '\r' || argtyp == '\n') {
	    printmac (0);
	} else if (argtyp == 't') {
	    printmac (1);
	} else if (argtyp == 'T') {
	    printmac (2);
	} else {
	    printmac (0);
	}
	break;
    case PN:
	/*
	 *   page number
	 *
	 *   .pn N
	 */
	val = getval (p, &argtyp);
	tmp = pg.curpag;
	set (&pg.curpag, val - 1, argtyp, tmp, -HUGE, HUGE);
	pg.newpag = pg.curpag + 1;
	set_ireg ("%", pg.newpag, 0);
	break;
    case PO:
	/*
	 *   page offset
	 *
	 *   .po N
	 */
	val = getval (p, &argtyp);
	set (&pg.offset, val, argtyp, 0, 0, HUGE);
	set_ireg (".o", pg.offset, 0);
	break;
    case PS:
	/*
	 *   point size (IGNORED)
	 *
	 *   .ps +/-N
	 */
	break;
    case RR:
	/*
	 *   unset number reg
	 *
	 *   .rr R
	 */
	val = getval (p, &argtyp);
	p = skipwd (p);
	p = skipbl (p);
	if (!isalpha (*p)) {
	    warnx("invalid or missing number register name");
	} else {
	    indx = tolower (*p) - 'a';
	    val = 0;
	    set (&dc.nr[indx], val, argtyp, 0, -HUGE, HUGE);
	}
	break;
    case SO:
	/*
	 *   source file
	 *
	 *   .so name
	 */
	val = getval (p, &argtyp);
	p = skipwd (p);
	p = skipbl (p);
	if (getwrd (p, name) == 0) {
	    break;
	}
	if (dc.flevel + 1 >= Nfiles) {
	    errx(-1, ".so commands nested too deeply");
	}
	if ((sofile[dc.flevel + 1] = fopen (name, "r")) == NULL_FPTR) {
	    errx(-1, "unable to open %s\n", name);
	}
	dc.flevel += 1;
	break;
    case SP:
	/*
	 *   space
	 *
	 *   .sp [N=1]
	 */
	val = getval (p, &argtyp);
	set (&spval, val, argtyp, 1, 0, HUGE);
	nroffSpace (spval);
	break;
    case SS:
	/*
	 *   space char size (IGNORED)
	 *
	 *   .ss N
	 */
	break;
    case TI:
	/*
	 *   temporary indent
	 *
	 *   .ti [+/-N]
	 */
	val = getval (p, &argtyp);
	robrk ();
	set (&dc.tival, val, argtyp, 0, 0, dc.rmval);
	break;
    case UL:
	/*
	 *   underline
	 *
	 *   .ul [N]
	 */
	val = getval (p, &argtyp);
	set (&dc.ulval, val, argtyp, 0, 1, HUGE);
	dc.cuval = dc.boval = 0;
	break;
    }
}


/*
 * comtyp
 *	decodes nro command and returns its associated value.
 *	ptr "p" is incremented (and returned)
 */
static int 
comtyp (char *p, char *m) {
    register char	c1;
    register char	c2;
    char	       *s;
    char    	macnam[MNLEN];
    int result;
    
    /*
     *   quick check: if null, ignore
     */
    if (*p == EOS) {
	return (COMMENT);
    }
    
    /*
     *   skip past dot and any whitespace
     */
    p++;
    while (*p && (*p == ' ' || *p == '\t')) {
	p++;
    }
    if (*p == '\0') {
	return (COMMENT);
    }

    /* 
     *   First check to see if the command is a macro. If it is,
     *   truncate to two characters and return expansion in m
     *   (i.e. the text of the macro). Note that upper and lower
     *   case characters are handled differently.
     */
    getwrd (p, macnam);
    macnam[2] = EOS;
    if ((s = getmac (macnam)) != NULL_CPTR) {
	strcpy (m, s);
	return (MACRO);
    }
    c1 = *p++;
    c2 = *p;
    result = UNKNOWN;

    switch (c1) {
    case '\\':
	switch (c2) {
	case '"':  result = COMMENT; break;
	}
	break;
    case 'a':
	switch (c2) {
	case 'd':  result = AD;  break;
	case 'f':  result = AF;  break;
	}
	break;
    case 'b':
	switch (c2) {
	case 'd':  result = BD;  break;
	case 'o':  result = BO;  break;
	case 'p':  result = BP;  break;
	case 'r':  result = BR;  break;
	case 's':  result = BS;  break;
	}
	break;
    case 'c':
	switch (c2) {
	case '2':  result = C2;  break;
	case 'c':  result = CC;  break;
	case 'e':  result = CE;  break;
	case 's':  result = CS;  break;
	case 'u':  result = CU;  break;
	}
	break;
    case 'd':
	switch (c2) {
	case 'e':  result = DE;  break;
	case 's':  result = DS;  break;
	}
	break;
    case 'e':
	switch (c2) {
	case 'c':  result = EC;  break;
	case 'f':  result = EF;  break;
	case 'h':  result = EH;  break;
#if 0
	case 'l':  result = EL;  break;
#endif
	case 'n':  result = EN;  break;
	case 'o':  result = EO;  break;
	case 'x':  result = EX;  break;
	}
	break;
    case 'f':
	switch (c2) {
	case 'c':  result = FC;  break;
	case 'i':  result = FI;  break;
	case 'l':  result = FL;  break;
	case 'o':  result = FO;  break;
	case 't':  result = FT;  break;
	}
	break;
    case 'h':
	switch (c2) {
	case 'e':  result = HE;  break;
	}
	break;
    case 'i':
	switch (c2) {
	case 'f':  result = IF;  break;
#if 0
	case 'e':  result = IE;  break;
#endif
	case 'g':  result = IG;  break;
	case 'n':  result = IN;  break;
	}
	break;
    case 'j':
	switch (c2) {
	case 'u':  result = JU;  break;
	}
	break;
    case 'l':
	switch (c2) {
	case 'g':  result = LG;  break;
	case 'l':  result = LL;  break;
	case 's':  result = LS;  break;
	case 't':  result = LT;  break;
	}
	break;
    case 'm':
	switch (c2) {
	case '1':  result = M1;  break;
	case '2':  result = M2;  break;
	case '3':  result = M3;  break;
	case '4':  result = M4;  break;
	case 'c':  result = MC;  break;
	}
	break;
    case 'n':
	switch (c2) {
	case 'a':  result = NA;  break;
	case 'e':  result = NE;  break;
	case 'f':  result = NF;  break;
	case 'j':  result = NJ;  break;
	case 'r':  result = NR;  break;
	}
	break;
    case 'o':
	switch (c2) {
	case 'f':  result = OF;  break;
	case 'h':  result = OH;  break;
	}
	break;
    case 'p':
	switch (c2) {
	case 'c':  result = PC;  break;
	case 'l':  result = PL;  break;
	case 'm':  result = PM;  break;
	case 'n':  result = PN;  break;
	case 'o':  result = PO;  break;
	case 's':  result = PS;  break;
	}
	break;
    case 'r':
	switch (c2) {
	case 'm':  result = RM;  break;
	case 'r':  result = RR;  break;
	}
	break;
    case 's':
	switch (c2) {
	case 'o':  result = SO;  break;
	case 'p':  result = SP;  break;
	case 's':  result = SS;  break;
	}
	break;
    case 't':
	switch (c2) {
	case 'i':  result = TI;  break;
	case 'l':  result = TL;  break;
	case 'r':  result = TR;  break;
	}
	break;
    case 'u':
	switch (c2) {
	case 'l':  result = UL;  break;
	}
	break;
    case '.':
	result = EN;
	break;
    }
    return result;
}



/*
 * gettl
 *	get header or footer title
 */
static void 
gettl (char *p, char *q, int *limit) {
    /*
     *   skip forward a word...
     */
    p = skipwd (p);
    p = skipbl (p);
    
    /*
     *   copy and set limits
     */
    strcpy (q, p);
    limit[LEFT]  = dc.inval;
    limit[RIGHT] = dc.rmval;
}

/*
 * getval
 *	retrieves optional argument following command.
 *	returns positive integer value with sign (if any)
 *	saved in character addressed by p_argt.
 */
static int 
getval (char *p, char *p_argtyp) {
    p = skipwd (p);
    p = skipbl (p);
    *p_argtyp = *p;
    if ((*p == '+') || (*p == '-')) {
	++p;
    }
    return (ctod (p));
}

#define N_ADD			0
#define N_SUB			1
#define N_MUL			2
#define N_DIV			3
#define N_MOD			4
#define N_LT			5
#define N_GT			6
#define N_LE			7
#define N_GE			8
#define N_EQ			9
#define N_AND			10
#define N_OR			11


/*
 * getnumeric
 *
 *	retrieves numeric argument. will parse for number registers,
 *	constants, operations, and logical comparisons. no imbeded spaces!
 *	start at p (don't skip)
 */
static int
getnumeric (char *p) {
    char		name[10];
    int		val;
    int		thisval;
    int		autoinc;
    char		buf[256];
    char	       *pbuf;
    int		next_op;
    int		nreg;
    const char *percent = "%";
    char *scratch;
    
    val     = 0;
    next_op = N_ADD;
    while (*p != EOS && !isspace (*p)) {
	if (!strncmp (p, "\\n", 2)) {
	    DEBUGGING(("getnumeric: found number reg..."));
	    /*
	     *   number register
	     */
	    autoinc = 0;
	    p += 2;
	    if (*p == '+') {
		autoinc = 1;
		p++;
	    } else if (*p == '-') {
		autoinc = -1;
		p++;
	    }
	    if (isalpha (*p)) {
		/*
		 *   \nx form. find reg (a-z)
		 */
		nreg = tolower (*p) - 'a';
		p++;
		
		/*
		 *   was this \n+x or \n-x? if so, do the
		 *   auto incr
		 */
		if (autoinc > 0) {
		    dc.nr[nreg] += dc.nrauto[nreg];
		} else if (autoinc < 0) {
		    dc.nr[nreg] -= dc.nrauto[nreg];
		}
				
		val = do_oper (val, next_op, dc.nr[nreg]);
	    } else if (*p == '%') {
		/*
		 *   \n% form. find index into reg struct
		 */
		FINDREG(percent, nreg, scratch);
		p++;
		if (nreg < 0) {
		    errx(-1, "no register match");
		}
		
		/*
		 *   was this \n+% or \n-%? if so, do the
		 *   auto incr
		 */
		if (autoinc > 0) {
		    rg[nreg].rval += rg[nreg].rauto;
		} else if (autoinc < 0) {
		    rg[nreg].rval -= rg[nreg].rauto;
		}

		
		val = do_oper (val, next_op, rg[nreg].rval);
	    } else if (*p == '(') {
		/*
		 *   \n(xx form. find index into reg struct
		 */
		p++;
		name[0] = *p++;
		name[1] = *p++;
		if (name[1] == ' '  || name[1] == '\t'
		    ||  name[1] == '\n' || name[1] == '\r') {
		    name[1] = '\0';
		}
		name[2] = '\0';
		FINDREG(name, nreg, scratch);
		if (nreg < 0) {
		    errx(-1, "no register match");
		}
		
		/*
		 *   was this \n+(xx or \n-(xx? if so, do the
		 *   auto incr
		 */
		if (rg[nreg].rflag & RF_WRITE) {
		    if (autoinc > 0) {
			rg[nreg].rval += rg[nreg].rauto;
		    } else if (autoinc < 0) {
			rg[nreg].rval -= rg[nreg].rauto;
		    }
		}
		
		val = do_oper (val, next_op, rg[nreg].rval);
	    }
	} else if (isdigit (*p)) {
	    pbuf = buf;
	    while (1) {
		if ((*p == EOS || isspace (*p)) ||
		    (*p == '\\') ||
		    (iscond (*p)) ||
		    (isoper (*p))) {
		    break;
		}
		*pbuf++ = *p++;
	    }
	    *pbuf = EOS;
	    
	    DEBUGGING(("getnumeric: buf:|%s| next_op:%d val:%d",
		       buf, next_op, val));
		
	    thisval = ctod (buf);
	    val     = do_oper (val, next_op, thisval);
	    DEBUGGING(("getnumeric: thisval:%d val:%d", thisval, val));
	}
	
	/*
	 *   p should now be at the next thing, either a
	 *   space, a null, or an operator
	 */
	if (*p == EOS || isspace (*p)) {
	    break;
	}
	switch (*p) {
	case '+':
	    next_op = N_ADD;
	    p++;
	    break;
	case '-':
	    next_op = N_SUB;
	    p++;
	    break;
	case '*':
	    next_op = N_MUL;
	    p++;
	    break;
	case '/':
	    next_op = N_DIV;
	    p++;
	    break;
	case '%':
	    next_op = N_MOD;
	    p++;
	    break;
	case '&':
	    next_op = N_AND;
	    p++;
	    break;
	case ':':
	    next_op = N_OR;
	    p++;
			break;
	case '<':
	    p++;
	    if (*p == '=') {
		p++;
		next_op = N_LE;
	    } else {
		next_op = N_LT;
	    }
	    break;
	case '>':
	    p++;
	    if (*p == '=') {
		p++;
		next_op = N_GE;
	    } else {
		next_op = N_GT;
	    }
	    break;
	case '=':
	    p++;
	    if (*p == '=') {
		p++;
	    }
	    next_op = N_EQ;
	    break;
	}
    }
    return (val);
}


/*
 * do_oper
 */

static int 
do_oper (int first, int oper, int second) {
    int	val;
    
    DEBUGGING(("do_oper: first:%d op:%d second:%d", first, oper, second));
    switch (oper) {
    case N_ADD:
	val = first + second;
	break;
    case N_SUB:
	val = first - second;
	break;
    case N_MUL:
	val = first * second;
	break;
    case N_DIV:
	val = first / second;
	break;
    case N_MOD:
	val = first % second;
	break;
    case N_LT:
	val = ((first < second) ? 1 : 0);
	break;
    case N_GT:
	val = ((first > second) ? 1 : 0);
	break;
    case N_LE:
	val = ((first <= second) ? 1 : 0);
	break;
    case N_GE:
	val = ((first >= second) ? 1 : 0);
	break;
    case N_EQ:
	val = ((first == second) ? 1 : 0);
	break;
    case N_AND:
	val = ((first && second) ? 1 : 0);
	break;
    case N_OR:
	val = ((first || second) ? 1 : 0);
	break;
    }
    return (val);
}


/*
 * set
 *	set parameter and check range. this is for basically all commands
 *	which take interger args
 *
 *	no param (i.e. \r or \n) means reset default
 *	+ means param += val (increment)
 *	- means param -= val (decrement)
 *	anything else makes an assignment within the defined numerical limits
 *
 *	examples:
 *
 *	.nr a 14	set register 'a' to 14
 *	.nr a +1	increment register 'a' by 1
 *	.nr a		reset register 'a' to default value (0)
 */
void 
set (int *param, int val, char type, int defval, int minval, int maxval) {
    switch (type) {
    case '\r': 
    case '\n': 
	*param = defval;
	break;
    case '+': 
	*param += val;
	break;
    case '-': 
	*param -= val;
	break;
    default: 
	*param = val;
	break;
    }
    *param = MIN(*param, maxval);
    *param = MAX(*param, minval);
}

/*
 * set_ireg
 *	set internal register "name" to val. ret 0 if ok, else -1 if reg not
 *	found or 1 if read only
 *
 *      0=internal, 1=user set
 */
int 
set_ireg (const char *name, int val, int opt) {
    register int	nreg;
    char *p;
    
    FINDREG(name, nreg, p);
    if (nreg < 0) {
	return (-1);
    }

    if ((rg[nreg].rflag & RF_WRITE) || (opt == 0)) {
	rg[nreg].rval = val;
	
	return (0);
    }
    
    return (1);
}


/*
 * read_if
 *	read input while in if statement. stop when a line starts with \}
 */
static void 
read_if (void) {
    char	ibuf[MAXLINE];
    char   *pp;
    
    while (getlin (ibuf, sofile[dc.flevel]) != EOF) {
	DEBUGGING(("read_if: ibuf=|%s|", ibuf));

	pp = skipbl (ibuf);
	if (*pp == '\\' && *(pp+1) == '}') {
	    return;
	}
	
	/*
	 *   if line is a command or text
	 */
	if (ibuf[0] == dc.cmdchr) {
	    comand (ibuf);
	} else {
	    /*
	     *   this is a text line. first see if
	     *   first char is space. if it is, break
	     *   line.
	     */
	    if (ibuf[0] == ' ') {
		robrk ();
	    }
	    text (ibuf);
	}
	
	pp = ibuf;
	while (*pp != EOS) {
	    if (*pp == '\\' && *(pp+1) == '}') {
		return;
	    }
	}
    }
}
