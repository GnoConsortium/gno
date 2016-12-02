#ifndef NRO_H
#define NRO_H

#include "config.h"			/* os/compiler options */

#define SHORT_STANDOUT


#ifndef NEWLINE
#define NEWLINE	'\n'
#endif

/*
 *	nroff.h - stuff for nroff
 *
 *	adapted for atariST/TOS by Bill Rosenkranz 10/89
 *	net:	rosenkra@hall.cray.com
 *	CIS:	71460,17
 *	GENIE:	W.ROSENKRANZ
 *
 *	things to look for here:
 *	1) TMAC definition for default macro package lib
 *	2) configuration sizes (see _STKSIZ below if alcyon/dri)
 *	3) libc should have getenv(), time(), and ctime()
 *	4) look in version.h for *printer file name (included below)
 *
 *	all data is currently allocated in static arrays. the biggest
 *	chunks are the parameters which control number registers and the
 *	macro name space area. these are defined below: MAXREGS, MACBUF.
 *	MACBUF is the larger. it holds all macros, strings, etc. if you
 *	find yourself running out of macro space, increase MACBUF.
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
 */

#include <ctype.h>

/*
 *	default prefix of macro files. files will be of the form "tmac.an"
 *	(for -man), "tmac.s" (for -ms), "tmac.e" (for -me), etc. first
 *	checks environment for TMACDIR which would be path (e.g. "c:\lib\tmac"
 *	or ".", no trailing slash char!).
 */
#ifdef tmacfull
# define TMACFULL	tmacfull
#endif
#ifdef tmacpre
# define TMACPRE	tmacpre
#endif

#ifdef GEMDOS
# ifndef TMACFULL
#  define TMACFULL	"c:\\lib\\tmac\\tmac."
# endif
# ifndef TMACPRE
#  define TMACPRE	"\\tmac."
# endif
#endif

#ifdef MINIX
# ifndef TMACFULL
#  define TMACFULL	"/usr/lib/tmac/tmac."
# endif
# ifndef TMACPRE
#  define TMACPRE	"/tmac."
# endif
#endif

#ifdef UNIX
# ifndef TMACFULL
#  define TMACFULL	"/usr/lib/tmac/tmac."
# endif
# ifndef TMACPRE
#  define TMACPRE	"/tmac."
# endif
#endif

/*
 *	command codes. indented defines are commands not yet implemented
 */
#undef PI
#define MACRO		0	/* macro definition */
#define BP	 	1	/* begin page */
#define BR	 	2	/* break */
#define CE	 	3	/* center */
#define FI	 	4	/* fill	*/
#define FO	 	5	/* footer */
#define HE	 	6	/* header */
#define IN	 	7	/* indent */
#define LS	 	8	/* line spacing	*/
#define NF	 	9	/* no fill */
#define PL		10	/* page length */
#define RM		11	/* remove macro */
#define SP		12	/* line space */
#define TI		13	/* temp indent */
#define UL		14	/* underline */
#define JU		15	/* justify */
#define NJ		16	/* no justify */
#define M1		17	/* top margin */
#define M2		18	/* second top margin */
#define M3		19	/* first bottom margin */
#define M4		20	/* bottom-most margin */
#define BS		21	/* allow/disallow '\b' in output */
#define NE		22	/* need n lines */
#define PC		23	/* page number character (%) */
#define CC		24	/* control character (.) */
#define PO		25	/* page offset */
#define BO		26	/* bold face */
#define EH		27	/* header for even numbered pages */
#define OH		28	/* header for odd numbered pages */
#define EF		29	/* footer for even numbered pages */
#define OF		30	/* footer for odd numbered pages */
#define SO		31	/* source file */
#define CU		32	/* continuous underline	*/
#define DE		33	/* define macro	*/
#define EN		34	/* end macro definition	*/
#define NR		35	/* set number register */
#define EC		36	/* escape character (\) */
#define FT		37	/* font change (R,B,I,S,P) */
#define EO		38	/* turn escape parsing off */
#define LL		39	/* line length (same as RM) */
#define FL		40	/* flush output NOW */
#define PN		41	/* page number for next page */
#define RR		42	/* remove register */
#define C2		43	/* nobreak char */
#  define TR		44	/* translate character */
#define LT		45	/* length of title */
#  define FC		46	/* field delimeter */
#define TL		47	/* like HE */
#define AF		48	/* assign format to nr */
#define AD		49	/* adjust line */
#define NA		50	/* no adjust */
#define DS		51	/* define string */
#define PM		52	/* print macro names */
#define IF		53	/* if */
#  define IE		54	/* if/else */
#  define EL		55	/* else */
#define PS		56	/* point size (IGNORED in nroff) */
#define SS		57	/* space char size (IGNORED in nroff) */
#define CS		58	/* constant char space (IGNORED in nroff) */
#define BD		59	/* embolden font (IGNORED in nroff) */
#  define FP		60	/* font position */
#  define MK		61	/* mark vertical place */
#  define RT		62	/* return to marked vert place */
#  define VS		63	/* vertical baseline spacing */
#  define SV		64	/* save vertical distance */
#  define OS		65	/* output saved vertical distance */
#  define NS		66	/* no-space mode */
#  define RS		67	/* restore spacing mode */
#  define AM		68	/* append to macro */
#  define AS		69	/* append to string */
#  define RN		70	/* rename */
#  define DI		71	/* divert to macro */
#  define DA		72	/* divert/append to macro */
#  define WH		73	/* set location trap */
#  define CH		74	/* change trap location */
#  define DT		75	/* set diversion trap */
#  define IT		76	/* set input line trap */
#  define EM		77	/* end macro */
#  define TA		78	/* tab settings */
#  define TC		79	/* tab repetition char */
#  define LC		80	/* leader repetition char */
#  define LG		81	/* ligature mode */
#  define UF		82	/* underline font */
#  define NH		83	/* no hyphenation */
#  define HY		84	/* hyphenate */
#  define HC		85	/* hyphenation indication char */
#  define HW		86	/* hyphenation exception words */
#  define NM		87	/* number mode */
#  define NN		88	/* no number next lines */
#  define EV		89	/* environment switch */
#  define RD		90	/* read insertion */
#  define EX		91	/* exit */
#  define NX		92	/* next file */
#  define PI		93	/* pipe to program */
#  define MC		94	/* set margin char */
#  define TM		95	/* print to terminal */
#define IG		96	/* ignore */

#define COMMENT		1000	/* comment (.\") */
#define UNKNOWN		-1


/*
 *	MAXLINE is set to a value slightly larger than twice the longest
 *	expected input line. Because of the way underlining is handled, the
 *	input line which is to be underlined, can almost triple in length.
 *	Unlike normal underlining and boldfacing, continuous underlining
 *	affects all characters in the buffer, and represents the worst case
 *	condition.  If the distance between the left margin and the right
 *	margin is greater than about 65 characters, and continuous underlining
 *	is in effect, there is a high probability of buffer overflow.
 */
/* FIXME */
#define MAXLINE		500 /*500*/		/* 200 */
#define PAGELEN	 	66
#define PAGEWIDTH 	80
#define HUGE		256
#define INFINITE	32760
#define LEFT		0	/* indecies into hdr margin lim arrays */
#define RIGHT		1
#define Nfiles		4	/* nesting depth for input files */

/*
 *	The following parameters may be defined elsewhere so undef/def
 */
#undef min
#undef max
#undef YES
#define YES		1
#undef NO
#define NO		0
#undef ERR
#define ERR		-1
#define EOS 		'\0'
#undef FALSE
#define FALSE 		0
#undef TRUE
#define TRUE 		!FALSE
#undef OK
#define OK 		!ERR

/*
 *	a rational way of dealing with the NULL thing...
 */
#define NULL_CPTR	(char *) 0
#define NULL_FPTR	(FILE *) 0
#define NULL_IPTR	(int *) 0
#define NULL_LPTR	(long *) 0
#define NULL_SPTR	(short *) 0
#define NULL_PTR	(char *) 0
#define NULLP(type)	(type *) 0


/*
 *	for justification during line fill
 */
#define ADJ_OFF		0
#define ADJ_LEFT	1
#define ADJ_RIGHT	2
#define ADJ_CENTER	3
#define ADJ_BOTH	4

/*
 *	Markers for various terminal/printer modes
 */
#define	S_STANDOUT 128
#define	E_STANDOUT 129
#define	S_BOLD 130
#define	E_BOLD 131
#define	S_ITALIC 132
#define	E_ITALIC 133

/*
 *	basic unit (b.u.) conversions. in nroff, all output is fixed spaced,
 *	at least in THIS nroff. so unit conversion to b.u. amount to 1 Em per
 *	character or 24 b.u. per character. thus 0.5i = 120 b.u. = 5 chars.
 *	everything is rounded up to the nearest Em. it is highly recommended
 *	to use inches for everything...
 *
 *	to convert (say inches) to char spaces, do this:
 *
 *		char_spaces = (int)(inches * (float) BU_INCH) / BU_EM;
 */
#define BU_INCH		240		/* 1.0i = 240 b.u. */
#define BU_CM		945/10		/* 1.0c = 240*50/127 b.u. */
#define BU_PICA		40		/* 1P   = 240/6 b.u. */
#define BU_EM		24		/* 1m   = 240/10 b.u. (10 char/inch) */
#define BU_EN		24		/* 1n   = 240/10 b.u. */
#define BU_POINT	240/72		/* 1p   = 240/72 b.u. */
#define BU_BU		1		/* 1    = 1 b.u. */


/*
 *	The parameter values selected for macro definitions are somewhat
 *	arbitrary.  MACBUF is the storage area for both macro names and
 *	definitions.  Since macro processing is handled by pushing back
 *	the expansion into the input buffer, the longest possible expansion
 *	would be MAXLINE characters.  Allowing for argument expansion,
 *	MXMLEN was chosen slightly less than MAXLINE. It is assumed that
 *	most macro definitions will not exceed 20 characters, hence MXMDEF
 *	of 150.
 */
#define MXMDEF		150	/* max no. of macro definitions */
#define MACBUF		32000 /*100000*//* macro definition buffer size (32000) */
#define MXMLEN		1000	/* max length of each macro definition (250) */
#define MNLEN		10	/* max length of macro name */
#define MAXREGS		100	/* max number of registers (2-char) */
#define MAXPBB		5000	/* size of push back buffer */


/*
 *	number registers
 */
#define RF_READ		0x0001	/* register flags */
#define RF_WRITE	0x0002

struct regs
{
	char	rname[4];	/* 2-char register name */
	int	rauto;		/* autoincrement value */
	int	rval;		/* current value of the register */
	int	rflag;		/* register flags */
	char	rfmt;		/* register format (1,a,A,i,I,...) */
};



/*
 *	control parameters for nroff
 */
struct docctl
{
	int	fill;		/* fill if YES, init = YES */
	int	dofnt;		/* handle font change, init = YES */
	int	lsval;		/* current line spacing, init = 1 */
	int	inval;		/* current indent, >= 0, init = 0 */
	int	rmval;		/* current right margin, init = 60 */
	int	llval;		/* current line length, init = 60 */
	int	ltval;		/* current title length, init = 60 */
	int	tival;		/* current temp indent, init = 0 */
	int	ceval;		/* number of lines to center, init = 0 */
	int	ulval;		/* number of lines to underline, init = 0 */
	int	cuval;		/* no lines to continuously uline, init = 0 */
	int	juval;		/* justify if YES, init = YES */
	int	adjval;		/* adjust type, init = ADJ_BOTH */
	int	boval;		/* number of lines to bold face, init = 0 */
	int	bsflg;		/* can output contain '\b', init = FALSE */
	int	prflg;		/* print on or off, init = TRUE */
	int	sprdir;		/* direction for spread(), init = 0 */
	int	flevel;		/* nesting depth for source cmd, init = 0 */
	int	lastfnt;	/* previous used font */
	int	thisfnt;	/* current font, init = 1 (1=R,2=I,3=B,4=S) */
	int fontbits;   /* currently active fonts. */
	int	escon;		/* whether esc parsing is on, init = YES */
	int	nr[26];		/* number registers */
	int	nrauto[26];	/* number registers auto increment */
	char	nrfmt[26];	/* number registers formats, init = '1' */
				/* input code how printed */
				/* 1     '1'  1,2,3,... */
				/* a     'a'  a,b,c,...,aa,bb,cc,... */
				/* A     'A'  A,B,C,...,AA,BB,CC,... */
				/* i     'i'  i,ii,iii,iv,v... */
				/* I     'I'  I,II,III,IV,V... */
				/* 01     2   01,02,03,... */
				/* 001    3   001,002,003,... */
				/* 0..1   8   00000001,00000002,... */
	char	pgchr;		/* page number character, init = '%' */
	char	cmdchr;		/* command character, init = '.' */
	char	escchr;		/* escape char, init = '\' */
	char	nobrchr;	/* nobreak char, init = '\'' */
};


/*
 *	page control parameters
 */
struct page
{
	int	curpag;		/* current output page number, init =0 */
	int	newpag;		/* next output page number, init = 1 */
	int	lineno;		/* next line to be printed, init = 0 */
	int	plval;		/* page length in lines, init = 66 */
	int	m1val;		/* margin before and including header */
	int	m2val;		/* margin after header */
	int	m3val;		/* margin after last text line */
	int	m4val;		/* bottom margin, including footer */
	int	bottom;		/* last live line on page
					= plval - m3val - m4val	*/
	int	offset;		/* page offset from left, init = 0 */
	int	frstpg;		/* first page to print, init = 0 */
	int	lastpg;		/* last page to print, init = 30000 */
	int	ehlim[2];	/* left/right margins for headers/footers */
	int	ohlim[2];	/* init = 0 and PAGEWIDTH */
	int	eflim[2];
	int	oflim[2];
	char	ehead[MAXLINE];	/* top of page title, init = '\n' */
	char	ohead[MAXLINE];
	char	efoot[MAXLINE];	/* bottom of page title, init = '\n' */
	char	ofoot[MAXLINE];
};




/*
 *	forward refs from nroff
 */

/* main.c */
void	debugMessage (const char *fmt, ...);

/* command.c */
void comand (char *p);
int set_ireg (const char *name, int val, int opt);
void set (int *param, int val, char type, int defval, int minval, int maxval);

/* low.c */
int ctod (char  *p);
char *skipbl (char *p);
char *skipwd (char *p);
void nroffSpace (int n);
char *getfield (char *p, char *q, char delim);
int getwrd (char *src, char *dest);
int countesc (char *p);
int itoda (int value, char *p, int size);
int itoROMAN (int value, char *p, int size);
int itoroman (int value, char *p, int size);
int itoLETTER (int value, char *p, int size);
int itoletter (int value, char *p, int size);
void err_exit (int code);

/* text.c */
void	initOutbuf (void);
void	pfoot (void);
void	phead (void);
void	robrk (void);
void	setPrinting (int val);
void	skip (int n);
int 	strkovr (char *p, char *q);
void 	text (char *p);


/* strings.c */
void defstr (char *line);

/*
 *	Globals.
 */
extern struct docctl	dc;
extern struct page	pg;
extern struct regs	rg[MAXREGS];
extern FILE	       *out_stream;
extern FILE	       *err_stream;
extern FILE	       *dbg_stream;
extern FILE	       *sofile[Nfiles+1];
extern int		ignoring;
extern int		hold_screen;
extern int		debugging;
extern int		stepping;
extern int		mc_ing;
extern int		mc_space;
extern char		mc_char;
extern char		tmpdir[];
extern char		s_standout[];
extern char		e_standout[];
extern char		s_italic[];
extern char		e_italic[];
extern char		s_bold[];
extern char		e_bold[];
extern char	       *dbgfile;
extern char	       *printer;

#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))

#define DEBUGGING(args) \
{ \
    if (debugging) { \
	debugMessage args; \
    } \
}

#ifdef DEBUG
#define ASSERT(condition, args) \
{ \
    if (!(condition)) { \
	printf("%s:%d: assertion failed (%s): ",__FILE__,__LINE__,#condition);\
	printf args; \
	abort(); \
    } \
}
#else
#define ASSERT(condition, args)
#endif

#endif /*NRO_H*/
