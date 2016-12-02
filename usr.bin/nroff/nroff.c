/*
 *	main.c - main for nroff word processor
 *
 *	similar to Unix(tm) nroff or RSX-11M RNO. adaptation of text processor
 *	given in "Software Tools", Kernighan and Plauger.
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
 * $Id: nroff.c,v 1.3 1997/10/30 04:04:35 gdr Exp $
 */

#ifdef __ORCAC__
segment "main______";
#pragma optimize 79
#endif

#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#ifdef __GNO__
#include <err.h>
#include <termcap.h>
#include <gno/gno.h>
#else
#include "unix/err.h"
#include "unix/termcap.h"
#endif

#ifdef sparc
#include <memory.h>
#include "unix/sunos.h"
#endif

#include <libgen.h>

#include "nroff.h"
#include "macros.h"
#include "io.h"

static void	init (void);
static void	processFile (void);
static int	pswitch (char *p, int *q);
static void	usage (void);

/*************************************************************************
 *
 * Global variables block.  Keep them in main.c so that the IIgs'
 * debuggers can find them.
 */

struct docctl		dc;
struct page		pg;
struct macros_t		mac;
struct regs		rg[MAXREGS];
FILE		       *out_stream;
FILE		       *err_stream;
FILE		       *dbg_stream;
FILE		       *sofile[Nfiles+1];
int			ignoring;		/* .ig vs .de */
int			hold_screen;
int			debugging;
int			stepping;		/* paging */
int			mc_ing = 0;		/* turned off */
int			mc_space = 2;
char			mc_char = '|';
char			tmpdir[256];
char			s_standout[20];
char			e_standout[20];
char			s_bold[20];
char			e_bold[20];
char			s_italic[20];
char			e_italic[20];
char		       *dbgfile = "nroff.dbg";
#ifdef GEMDOS
char		       *printer = "prn:";	/* this WON'T work!!! */
#else
char		       *printer = ".ttyb";	/* this probably won't */
#endif

static char		termcap[1024];		/* _must_ be 1024 */
static char  *progname;
static char    *version = "(GNO) v1.2.2, 20 Aug 2016 kws";

/*
 * End of global variable definitions.
 *
 *************************************************************************/

#ifdef __STACK_CHECK__
static void
printStack (void) {
    fprintf(stderr, "stack usage: %d bytes\n", _endStackCheck());
}
#endif

int
main (int argc, char *argv[]) {
    register int	i;
    int		swflg;
    int		ifp = 0;
    char	       *ptmp;
    char	       *pterm;
    static char		capability[100];
    char	       *pcap;
    char	       *ps;

#ifdef __GNO__
#ifdef __STACK_CHECK__
    _beginStackCheck();
    atexit(printStack);
#endif
    if (argc > 0) {
	progname = __prognameGS();
    } else {
	exit(1);		/* not running from a shell? */
    }
#else
    if (argc > 0) {
	progname = basename(argv[0]);
    } else {
	progname = "nroff";
    }
#endif


    /*
     *   set up initial flags and file descriptors
     */
    swflg       = FALSE;
    ignoring    = FALSE;
    hold_screen = FALSE;
    debugging   = FALSE;
    stepping    = FALSE;
    mc_ing      = FALSE;
    out_stream  = stdout;
    err_stream  = stderr;
    dbg_stream  = stderr;
    
  
    /*
     *   this is for tmp files, if ever needed. it SHOULD start
     *   out without the trailing slash. if not in env, use default
     */
    if ((ptmp = getenv ("TMPDIR")) != NULL) {
	strcpy (tmpdir, ptmp);
    } else {
	strcpy (tmpdir, ".");
    }
  
    /*
     *   handle terminal for \fB, \fI
     */
    s_standout[0] = '\0';
    e_standout[0] = '\0';
    s_bold[0]     = '\0';
    e_bold[0]     = '\0';
    s_italic[0]   = '\0';
    e_italic[0]   = '\0';

    /*
     * get termcap information
     */
    if ((pterm = getenv("TERM")) == NULL) {
	errx(1, "TERM environment variable not set");
    }
    switch (tgetent(termcap, pterm)) {
    case -1:
	errx(1, "couldn't open termcap database");
	/*NOTREACHED*/
    case 0:
	errx(1, "terminal type %s not found in termcap database", pterm);
	/*NOTREACHED*/
    }
	
    /*
     *   we currently use standout mode for all weirdness
     *   like BOLD, italic, etc.
     */

    #undef PC
    pcap = capability;
    if (tgetstr("pc", &pcap)) {
        PC = capability[0];
    } 
    else PC = 0;

    pcap = s_italic;
    tgetstr("us", &pcap);

    pcap = e_italic;
    tgetstr("ue", &pcap);
    if (s_italic[0] && !e_italic[0]) s_italic[0] = '\0';

    pcap = capability;
    if ((ps = tgetstr ("so", &pcap)) != NULL) {
	/*
	 *   sun has padding in here. this is NOT portable.
	 *   better to use tputs() to strip it...
	 */
	/*	while (*ps && *ps != 0x1b)	ps++;  */
    /* tputs uses leading (and embedded) digits as the delay. */
	strcpy (s_standout, ps);
	strcpy (s_bold, ps);
	if (!s_italic[0]) strcpy (s_italic, ps);
    } else { 
	err(1, "couldn't get standout mode");
	/*NOTREACHED*/
    }


    pcap = capability;
    if ((ps = tgetstr ("se", &pcap)) != NULL) {
	/*	while (*ps && *ps != 0x1b)	ps++; */
	strcpy (e_standout, ps);
	strcpy (e_bold, ps);
	if (!e_italic[0]) strcpy (e_italic, ps);
    } else { 
        err(1, "couldn't get end standout mode");
        /*NOTREACHED*/
    }
    
    /*
     *   initialize structures (defaults)
     */
    init ();

    /*
     *   parse cmdline flags
     */
    for (i = 1; i < argc; ++i) {   
	if (*argv[i] == '-' || *argv[i] == '+') {
	    if (pswitch (argv[i], &swflg) == ERR) {
		err_exit (-1);
	    }
	}
    }

    /*
     *   loop on files
     */
    for (i = 1; i < argc; ++i) {
	if (*argv[i] != '-' && *argv[i] != '+') {
	    /*
	     *   open this file...
	     */
	    if ((sofile[0] = fopen (argv[i], "r")) == NULL_FPTR) {
		err(-1, "unable to open file %s", argv[i]);
	    } else {
		/*
		 *   do it for this file...
		 */
		ifp = 1;
		processFile ();
		fclose (sofile[0]);
	    }
	} else if (*argv[i] == '-' && *(argv[i]+1) == 0) {
	    /*
	     *   - means read stdin (anywhere in file list)
	     */
	    sofile[0] = stdin;
	    ifp = 1;
	    sleep(1);
	    processFile ();
	}
    }
    
    
    /*
     *   if no files, usage (should really use stdin...)
     */
    if ((ifp == 0 && swflg == FALSE) || argc <= 1) {
	usage ();
	err_exit (-1);
    }
    
    /*
     *   normal exit. this will fflush/fclose streams...
     */
    err_exit (0);
#if defined(__GNUC__) || defined(__INSIGHT__)
    return 0;
#endif
}


/*------------------------------*/
/*	usage			*/
/*------------------------------*/
static void
usage (void) {
    /*
     *   note: -l may not work correctly
     */
    
    fprintf(stderr, "Usage:   %s [options] file [...]\n", progname);
    fputs("\t-a\tno font changes\n", stderr);
    fputs("\t-b\t\tbackspace\n", stderr);
    fputs("\t-d\t\tdebug mode (file: nroff.dbg)\n", stderr);
#ifdef GEMDOS
    fputs("\t-h\t\thold screen before desktop\n", stderr);
#endif
#if 0
    fputs("\t-l\t\toutput to printer\n", stderr);
#endif
    fputs("\t-m<name>\tmacro file (e.g. -man)\n", stderr);
    fputs("\t-o<file>\terror log file (stderr is default)\n", stderr);
    fputs("\t-po<n>\t\tpage offset\n", stderr);
    fputs("\t-pn<n>\t\tinitial page number\n", stderr);
    fputs("\t-pl<n>\t\tpage length\n", stderr);
    fputs("\t-s\t\tstep through pages\n", stderr);
    fputs("\t-v\t\tprint version only\n", stderr);
    fputs("\t+<n>\t\tfirst page to do\n", stderr);
    fputs("\t-<n>\t\tlast page to do\n", stderr);
    fputs("\t-\t\tuse stdin (in file list)\n", stderr);
}


/*
 * init
 *	initialize parameters for nro word processor
 */
static void
init (void) {
    
    
    register int	i;
    time_t		tval;
    struct tm *tm;
    
    /*
     *   misc global flags, etc...
     */
    mc_space   = 2;
    mc_char    = '|';
    tval       = time (0L);
    tm         = localtime(&tval);

    /*
     *   basic document controls...
     */
    dc.fill    = YES;
    dc.dofnt   = YES;
    dc.lsval   = 1;
    dc.inval   = 0;
    dc.rmval   = PAGEWIDTH - 1;
    dc.llval   = PAGEWIDTH - 1;
    dc.ltval   = PAGEWIDTH - 1;
    dc.tival   = 0;
    dc.ceval   = 0;
    dc.ulval   = 0;
    dc.cuval   = 0;
    dc.juval   = YES;
    dc.adjval  = ADJ_BOTH;
    dc.boval   = 0;
    dc.bsflg   = FALSE;
    dc.prflg   = TRUE;
    dc.sprdir  = 0;
    dc.flevel  = 0;
    dc.lastfnt = 1;
    dc.thisfnt = 1;
    dc.escon   = YES;
    dc.pgchr   = '%';
    dc.cmdchr  = '.';
    dc.escchr  = '\\';
    dc.nobrchr  = '\'';
    for (i = 0; i < 26; ++i) {
	dc.nr[i] = 0;
    }
    for (i = 0; i < 26; ++i) {
	dc.nrauto[i] = 1;
    }
    for (i = 0; i < 26; ++i) {
	dc.nrfmt[i] = '1';
    }
    
    /*
     *   initialize internal regs. first zero out...
     */
    for (i = 0; i < MAXREGS; i++) {
	rg[i].rname[0] = EOS;
	rg[i].rname[1] = EOS;
	rg[i].rname[2] = EOS;
	rg[i].rname[3] = EOS;
	rg[i].rauto = 1;
	rg[i].rval  = 0;
	rg[i].rflag = RF_READ;
	rg[i].rfmt  = '1';
    }
  
    /*
     *   predefined regs. these are read/write:
     */
    i = 0;
    
    strcpy (rg[i].rname, "%");		/* current page */
    rg[i].rauto = 1;
    rg[i].rval  = 0;
    rg[i].rflag = RF_READ | RF_WRITE;
    rg[i].rfmt  = '1';
    i++;
    
    strcpy (rg[i].rname, "ct");		/* character type */
    rg[i].rauto = 1;
    rg[i].rval  = 0;
    rg[i].rflag = RF_READ | RF_WRITE;
    rg[i].rfmt  = '1';
    i++;
    
    strcpy (rg[i].rname, "dl");		/* width of last complete di */
    rg[i].rauto = 1;
    rg[i].rval  = 0;
    rg[i].rflag = RF_READ | RF_WRITE;
    rg[i].rfmt  = '1';
    i++;
    
    strcpy (rg[i].rname, "dn");		/* height of last complete di */
    rg[i].rauto = 1;
    rg[i].rval  = 0;
    rg[i].rflag = RF_READ | RF_WRITE;
    rg[i].rfmt  = '1';
    i++;
  
    strcpy (rg[i].rname, "dw");		/* day of week (1-7) */
    rg[i].rval = tm->tm_wday + 1;

    rg[i].rauto = 1;
    rg[i].rflag = RF_READ | RF_WRITE;
    rg[i].rfmt  = '1';
    i++;
    
    strcpy (rg[i].rname, "dy");		/* day of month (1-31) */
    rg[i].rauto = 1;
    rg[i].rval  = tm->tm_mday;

    rg[i].rflag = RF_READ | RF_WRITE;
    rg[i].rfmt  = '1';
    i++;
    
    strcpy (rg[i].rname, "hp");		/* current h pos on input */
    rg[i].rauto = 1;
    rg[i].rval  = 0;
    rg[i].rflag = RF_READ | RF_WRITE;
    rg[i].rfmt  = '1';
    i++;
  
    strcpy (rg[i].rname, "ln");		/* output line num */
    rg[i].rauto = 1;
    rg[i].rval  = 0;
    rg[i].rflag = RF_READ | RF_WRITE;
    rg[i].rfmt  = '1';
    i++;
    
    strcpy (rg[i].rname, "mo");		/* current month (1-12) */
    rg[i].rval  = tm->tm_mon+1;
    rg[i].rauto = 1;
    rg[i].rflag = RF_READ | RF_WRITE;
    rg[i].rfmt  = '1';
    i++;
    
    strcpy (rg[i].rname, "nl");		/* v pos of last base-line */
    rg[i].rauto = 1;
    rg[i].rval  = 0;
    rg[i].rflag = RF_READ | RF_WRITE;
    rg[i].rfmt  = '1';
    i++;
    
    strcpy (rg[i].rname, "sb");		/* depth of str below base */
    rg[i].rauto = 1;
    rg[i].rval  = 0;
    rg[i].rflag = RF_READ | RF_WRITE;
    rg[i].rfmt  = '1';
    i++;
    
    strcpy (rg[i].rname, "st");		/* height of str above base */
    rg[i].rauto = 1;
    rg[i].rval  = 0;
    rg[i].rflag = RF_READ | RF_WRITE;
    rg[i].rfmt  = '1';
    i++;
  
    strcpy (rg[i].rname, "yr");		/* last 2 dig of current year*/
    rg[i].rauto = 1;
    rg[i].rval  = tm->tm_year % 100;
    rg[i].rflag = RF_READ | RF_WRITE;
    rg[i].rfmt  = '1';
    i++;
    
    strcpy (rg[i].rname, "hh");		/* current hour (0-23) */
    rg[i].rauto = 1;
    rg[i].rval  = tm->tm_hour;
    rg[i].rflag = RF_READ | RF_WRITE;
    rg[i].rfmt  = 2 | 0x80;
    i++;
    
    strcpy (rg[i].rname, "mm");		/* current minute (0-59) */
    rg[i].rauto = 1;
    rg[i].rval  = tm->tm_min;
    rg[i].rflag = RF_READ | RF_WRITE;
    rg[i].rfmt  = 2 | 0x80;
    i++;
    
    strcpy (rg[i].rname, "ss");		/* current second (0-59) */
    rg[i].rauto = 1;
    rg[i].rval  = tm->tm_sec;
    rg[i].rflag = RF_READ | RF_WRITE;
    rg[i].rfmt  = 2 | 0x80;
    i++;
    
  
    /*
     *   these are read only (by user):
     */
    strcpy (rg[i].rname, ".$");		/* num args at current macro*/
    rg[i].rauto = 1;
    rg[i].rval  = 0;
    rg[i].rflag = RF_READ;
    rg[i].rfmt  = '1';
    i++;
    
    strcpy (rg[i].rname, ".A");		/* 1 for nroff */
    rg[i].rauto = 1;
    rg[i].rval  = 1;
    rg[i].rflag = RF_READ;
    rg[i].rfmt  = '1';
    i++;
    
    strcpy (rg[i].rname, ".H");		/* hor resolution */
    rg[i].rauto = 1;
    rg[i].rval  = 1;
    rg[i].rflag = RF_READ;
    rg[i].rfmt  = '1';
    i++;
    
    strcpy (rg[i].rname, ".T");		/* 1 for troff */
    rg[i].rauto = 0;
    rg[i].rval  = 0;
    rg[i].rflag = RF_READ;
    rg[i].rfmt  = '1';
    i++;
  
    strcpy (rg[i].rname, ".V");		/* vert resolution */
    rg[i].rauto = 1;
    rg[i].rval  = 1;
    rg[i].rflag = RF_READ;
    rg[i].rfmt  = '1';
    i++;
    
    strcpy (rg[i].rname, ".a");
    rg[i].rauto = 1;
    rg[i].rval  = 0;
    rg[i].rflag = RF_READ;
    rg[i].rfmt  = '1';
    i++;
    
    strcpy (rg[i].rname, ".c");
    rg[i].rauto = 1;
    rg[i].rval  = 0;
    rg[i].rflag = RF_READ;
    rg[i].rfmt  = '1';
    i++;
  
    strcpy (rg[i].rname, ".d");
    rg[i].rauto = 1;
    rg[i].rval  = 0;
    rg[i].rflag = RF_READ;
    rg[i].rfmt  = '1';
    i++;
	
    strcpy (rg[i].rname, ".f");		/* current font (1-4) */
    rg[i].rauto = 1;
    rg[i].rval  = 1;
    rg[i].rflag = RF_READ;
    rg[i].rfmt  = '1';
    i++;
  
    strcpy (rg[i].rname, ".h");
    rg[i].rauto = 1;
    rg[i].rval  = 0;
    rg[i].rflag = RF_READ;
    rg[i].rfmt  = '1';
    i++;
  
    strcpy (rg[i].rname, ".i");		/* current indent */
    rg[i].rauto = 1;
    rg[i].rval  = 0;
    rg[i].rflag = RF_READ;
    rg[i].rfmt  = '1';
    i++;
  
    strcpy (rg[i].rname, ".l");		/* current line length */
    rg[i].rauto = 1;
    rg[i].rval  = PAGEWIDTH - 1;
    rg[i].rflag = RF_READ;
    rg[i].rfmt  = '1';
    i++;
  
    strcpy (rg[i].rname, ".n");
    rg[i].rauto = 1;
    rg[i].rval  = 0;
    rg[i].rflag = RF_READ;
    rg[i].rfmt  = '1';
    i++;
    
    strcpy (rg[i].rname, ".o");		/* current offset */
    rg[i].rauto = 1;
    rg[i].rval  = 0;
    rg[i].rflag = RF_READ;
    rg[i].rfmt  = '1';
    i++;
    
    strcpy (rg[i].rname, ".p");		/* current page len */
    rg[i].rauto = 1;
    rg[i].rval  = PAGELEN;
    rg[i].rflag = RF_READ;
    rg[i].rfmt  = '1';
    i++;
    
    strcpy (rg[i].rname, ".s");		/* current point size */
    rg[i].rauto = 1;
    rg[i].rval  = 1;
    rg[i].rflag = RF_READ;
    rg[i].rfmt  = '1';
    i++;
  
    strcpy (rg[i].rname, ".t");
    rg[i].rauto = 1;
    rg[i].rval  = 0;
    rg[i].rflag = RF_READ;
    rg[i].rfmt  = '1';
    i++;
  
    strcpy (rg[i].rname, ".u");
    rg[i].rauto = 1;
    rg[i].rval  = 0;
    rg[i].rflag = RF_READ;
    rg[i].rfmt  = '1';
    i++;
    
    strcpy (rg[i].rname, ".v");		/* current v line spacing */
    rg[i].rauto = 1;
    rg[i].rval  = 1;
    rg[i].rflag = RF_READ;
    rg[i].rfmt  = '1';
    i++;
  
    strcpy (rg[i].rname, ".w");		/* width of prev char */
    rg[i].rauto = 1;
    rg[i].rval  = 1;
    rg[i].rflag = RF_READ;
    rg[i].rfmt  = '1';
    i++;
  
    strcpy (rg[i].rname, ".x");
    rg[i].rauto = 1;
    rg[i].rval  = 0;
    rg[i].rflag = RF_READ;
    rg[i].rfmt  = '1';
    i++;
  
    strcpy (rg[i].rname, ".y");
    rg[i].rauto = 1;
    rg[i].rval  = 0;
    rg[i].rflag = RF_READ;
    rg[i].rfmt  = '1';
    i++;
  
    strcpy (rg[i].rname, ".z");
    rg[i].rauto = 1;
    rg[i].rval  = 0;
    rg[i].rflag = RF_READ;
    rg[i].rfmt  = '1';

    /*
     *   page controls...
     */
    pg.curpag   = 0;
    pg.newpag   = 1;
    pg.lineno   = 0;
    pg.plval    = PAGELEN;
    pg.m1val    = 2;
    pg.m2val    = 2;
    pg.m3val    = 2;
    pg.m4val    = 2;
    pg.bottom   = pg.plval - pg.m4val - pg.m3val;
    pg.offset   = 0;
    pg.frstpg   = 0;
    pg.lastpg   = 30000;
    pg.ehead[0] = pg.ohead[0] = '\n';
    pg.efoot[0] = pg.ofoot[0] = '\n';
    memset(pg.ehead + 1, EOS, MAXLINE -1);
    memset(pg.ohead + 1, EOS, MAXLINE -1);
    memset(pg.efoot + 1, EOS, MAXLINE -1);
    memset(pg.ofoot + 1, EOS, MAXLINE -1);

#if 0  
    for (i = 1; i < MAXLINE; ++i) {
	pg.ehead[i] = pg.ohead[i] = EOS;
	pg.efoot[i] = pg.ofoot[i] = EOS;
    }
#endif
    pg.ehlim[LEFT]  = pg.ohlim[LEFT]  = dc.inval;
    pg.eflim[LEFT]  = pg.oflim[LEFT]  = dc.inval;
    pg.ehlim[RIGHT] = pg.ohlim[RIGHT] = dc.rmval;
    pg.eflim[RIGHT] = pg.oflim[RIGHT] = dc.rmval;
    
    /*
     *   output buffer controls...
     */
    initOutbuf();
  
    /*
     *   macros...
     */
    initMacros();
  
    /*
     *   file descriptors (for sourced files)
     */
    for (i = 0; i < Nfiles+1; ++i) {
	sofile[i] = NULL_FPTR;
    }
}


/*
 * pswitch
 *
 *	process switch values from command line
 */
static int 
pswitch (char *p, int *q) {
    int     swgood;
    char    mfile[256];
    char   *ptmac;
    int	indx;
    int	val;
    
    swgood = TRUE;
    if (*p == '-') {
	/*
	 *   since is STILL use the goofy atari/dri xmain code, i
	 *   look for both upper and lower case. if you use dLibs
	 *   (and if its startup code does not ucase the cmd line),
	 *   you can probably look for just lower case. gulam and
	 *   other shells typically don't change case of cmd line.
	 */
	switch (*++p) {
	case 0:					/* stdin */
	    break;
	    
	case 'a': 				/* font changes */
	case 'A': 
	    dc.dofnt = NO;
	    break;
	    
	case 'b': 				/* backspace */
	case 'B': 
	    dc.bsflg = TRUE;
	    break;
	    
	case 'd': 				/* debug mode */
	case 'D': 
	    if ((dbg_stream = fopen (dbgfile, "w")) == NULL) {
		warn ("unable to open debug file %s, using stderr", dbgfile);
		dbg_stream  = stderr;
	    }
	    debugging  = TRUE;
	    break;
	    
	case 'h': 				/* hold screen */
	case 'H': 
	    hold_screen = TRUE;
	    break;
	    
	case 'l': 				/* to lpr (was P) */
	case 'L': 
#ifdef GEMDOS
	    out_stream = (FILE *) 0;
#else
	    out_stream = fopen (printer, "w");
#endif
	    setPrinting(TRUE);
	    break;
	    
	case 'm': 				/* macro file */
	case 'M': 
	    /*
	     *   build macro file name. start with lib
	     *
	     *   put c:\lib\tmac in environment so we can
	     *   read it here. else use default. if you want
	     *   file from cwd, "setenv TMACDIR ." from shell.
	     *
	     *   we want file names like "tmac.an" (for -man)
	     */
	    if ((ptmac = getenv ("TMACDIR")) != NULL) {
		/*
		 *   this is the lib path (e.g. "c:\lib\tmac")
		 */
		strcpy (mfile, ptmac);
		
		/*
		 *   this is the prefix (i.e. "\tmac.")
		 */
		strcat (mfile, TMACPRE);
	    } else {
		/*
		 *   use default lib/prefix (i.e.
		 *   "c:\lib\tmac\tmac.")
		 */
		strcpy (mfile, TMACFULL);
	    }

	    /*
	     *   finally, add extension (e.g. "an")
	     */
	    strcat (mfile, ++p);
	    
	    /*
	     *   open file and read it
	     */
	    if ((sofile[0] = fopen (mfile, "r")) == NULL_FPTR) {
		err(-1, "unable to open macro file %s", mfile);
		/*NOTREACHED*/
	    }
	    processFile ();
	    fclose (sofile[0]);
	    break;
	    
	case 'o': 				/* output error log */
	case 'O': 
	    if (!*(p+1)) {
		err(-1, "no error file specified");
		/*NOTREACHED*/
	    }
	    if ((err_stream = fopen (p+1, "w")) == NULL) {
		err(-1, "unable to open error file %s", p+1);
		/*NOTREACHED*/
	    }
	    err_set_file(err_stream);
	    break;
	    
	case 'p': 				/* .po, .pn */
	case 'P':
	    if (*(p+1) == 'o' || *(p+1) == 'O') {	/* -po___ */
		p += 2;
		set (&pg.offset, ctod (p), '1', 0, 0, HUGE);
		set_ireg (".o", pg.offset, 0);
	    } else if (*(p+1) == 'n' || *(p+1) == 'N') { /* -pn___ */
		p += 2;
		set (&pg.curpag, ctod (p) - 1, '1', 0, -HUGE, HUGE);
		pg.newpag = pg.curpag + 1;
		set_ireg ("%", pg.newpag, 0);
	    } else if (*(p+1) == 'l' || *(p+1) == 'L') { /* -pl___ */
		p += 2;
		set (&pg.plval, ctod (p) - 1, '1', 0,
		     pg.m1val + pg.m2val + pg.m3val + pg.m4val + 1,
		     HUGE);
		set_ireg (".p", pg.plval, 0);
		pg.bottom = pg.plval - pg.m3val - pg.m4val;
	    } else {					/* -p___ */
		p++;
		set (&pg.offset, ctod (p), '1', 0, 0, HUGE);
		set_ireg (".o", pg.offset, 0);
	    }
	    break;
	    
	case 'r':				/* set number reg */
	case 'R':
	    if (!isalpha (*(p+1))) {
		warnx("invalid number register name (%c)", *(p+1));
	    } else {
		/*
		 *   indx is the user num register and val
		 *   is the final value.
		 */
		indx = tolower (*(p+1)) - 'a';
		val  = atoi (p+2);
		set (&dc.nr[indx], val, '1', 0, -INFINITE, INFINITE);
	    }
	    break;
	    
	case 's': 				/* page step mode */
	case 'S': 
	    stepping = TRUE;
	    break;
	    
	case 'v': 				/* version */
	case 'V': 
	    printf ("%s %s\n", progname, version);
	    *q = TRUE;
	    break;
	    
	case '0': 				/* last page */
	case '1': 
	case '2': 
	case '3': 
	case '4': 
	case '5': 
	case '6': 
	case '7': 
	case '8': 
	case '9': 
	    pg.lastpg = ctod (p);
	    break;
	    
	default: 				/* illegal */
	    swgood = FALSE;
	    break;
	}
    } else if (*p == '+') {				/* first page */
	pg.frstpg = ctod (++p);
    } else {						/* illegal */
	swgood = FALSE;
    }
    
  
    if (swgood == FALSE) {
	warnx("illegal option: %s", p);
	return (ERR);
    }
    
    return (OK);
}


/*
 * processFile
 *
 *	process input files from command line
 */
static void 
processFile (void) {
    
    static char ibuf[MAXLINE];
    int i;
    
    /*
     *   handle nesting of includes (.so). note that .so causes dc.flevel
     *   to be increased...
     */
    for (dc.flevel = 0; dc.flevel >= 0; dc.flevel -= 1) {
	while ((i = getlin (ibuf, sofile[dc.flevel])) != EOF) {
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
	}
	
	/*
	 *   close included file
	 */
	if (dc.flevel > 0) {
	    fclose (sofile[dc.flevel]);
	}
    }
    if (pg.lineno > 0) {
	nroffSpace (HUGE);
    }
}

#pragma optimize 78
#pragma debug 0

void
debugMessage (const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    fprintf(err_stream, "%s: ", progname);
    vfprintf(err_stream, fmt, ap);
    va_end(ap);
    return;
}
