/*
 *  Apple //gs GSOS system-dependent code
 */

#include <sys/types.h>
#include <texttool.h>
#include <misctool.h>
#include <shell.h>
#include <gsos.h>
/* 2/orcacdefs/gsos.h, not OUR include file */
#include "stevie.h"
#include <gno/gno.h>
#include <gno/signal.h>
#include <unistd.h>
#include "gsos.h"
#include <sys/ioctl.h>
#include <fcntl.h>

#ifdef __ORCAC__
#pragma optimize 8
#endif

char *tc_ED;
char *tc_EL;
char *tc_IL;
char *tc_DL;
char *tc_CI;
char *tc_CV;
char *tc_TP;
char *tc_TI;
char *tc_END_D;
char *tc_END_L;

typedef struct {
    int flag;
    char *comm;
    } execPB;
typedef struct {
    char *var_name;
    char *value;
    } readPB;

/* 
 *	These routines only work by the grace of God (and because I
 *	track the old IIe cursor locations)
 *	GNO 1.1 has insert-line and delete-line chars in it's console
 *	driver.  If present in the termcap file, stevie uses these.
 */
void InsertLine(void)
{
byte *WindTop = (byte *) 0x0022,
     *cy = (byte *) 0x0025;

    *WindTop = *cy;
    WriteChar(22);
    *WindTop = 0;
}

void DeleteLine(void)
{
byte *WindTop = (byte *) 0x0022,
     *cy = (byte *) 0x0025;

    *WindTop = *cy;
    WriteChar(23);
    *WindTop = 0;
}

char outbuf[1024];
int indbuf = 0;


void
flushbuf(void)
{
    write(STDOUT_FILENO, outbuf, indbuf);
    indbuf = 0;
}

int
outchar(char c)
{
    if (c == '\n') c = '\r';
    outbuf[indbuf++] = c;
    if (indbuf == 1024) flushbuf(); /* GOD DAMN, JAWAID!!! */
}

int
toutchar(char c)
{
    outbuf[indbuf++] = c;
    if (indbuf == 1024) flushbuf(); /* GOD DAMN, JAWAID!!! */
}

void
outstr(char *s)
{
char c;

    while (*s) {
	if ((c = *s++) == '\n') c = '\r';
	outbuf[indbuf++] = c;
        if (indbuf == 1024) flushbuf(); /* GOD DAMN, JAWAID!!! */
    }
}

void
toutstr(char *s)
{
    /*  this pretty much ignores how the delay characters need to work,
	but I don't think delays really matter all that much */
       tputs(s,1,toutchar);
}

int inchar(void)
{
int c;

    flushbuf();
    c = ReadChar(0);
    if (!(c & 0x0200)) 	/* make sure control is not down */
	switch (c & 0xFF) {
       	    case 0x0a:
           	return K_DARROW;
            case 0x0b:
           	return K_UARROW;
            case 0x08:
           	return K_LARROW;
            case 0x15:
           	return K_RARROW;
	}
    c &= 0x7F;
    if (c == 0x7f) return 0x8;
    return c;
}

void beep(void)
{
    outchar(7);
    flushbuf();
}

char *CM;
char *mp;
char tcb[100];
struct sgttyb ss;
int oldFlags;
struct ltchars ltch;

char oldsuspchar;
void nosuspend(void)
{
    ioctl(STDOUT_FILENO, TIOCGLTC, &ltch);
    ltch.t_suspc = -1;
    ioctl(STDOUT_FILENO, TIOCSLTC, &ltch);
}

void dosuspend(void)
{
    ioctl(STDOUT_FILENO, TIOCGLTC, &ltch);
    ltch.t_suspc = oldsuspchar;
    /*smsg("Old Suspend Char: %02X\n",oldsuspchar);*/
    ioctl(STDOUT_FILENO, TIOCSLTC, &ltch);
}

/* startup termcap, and initialize the characters */
void
windinit(void)
{
char *term,*tcp;

    Columns = 80;
    P(P_LI) = Rows = 24;
    SetInGlobals(0xFFFF,0x00);
/*    TextStartUp();    */
    ioctl(STDOUT_FILENO,TIOCGETP,&ss);
    oldFlags = ss.sg_flags;
    ss.sg_flags &= ~ECHO;
    ss.sg_flags |= CBREAK; /* for benefit of terminals */
    ioctl(STDOUT_FILENO,TIOCSETP,&ss); /* make sure echo is off for terms */
    ioctl(STDOUT_FILENO,TIOCGLTC,&ltch);
    oldsuspchar = ltch.t_suspc;

    if (!(term = getenv("TERM"))) {
	fprintf(stderr, "vi: TERM: parameter not set\n");
	exit(1);
    }
    if (!(mp = malloc((size_t)1024))) {
	fprintf(stderr, "vi: out of termcap space.\n");
	exit(1);
    }
    if (tgetent(mp, term) <= 0) {
	fprintf(stderr, "vi: %s: unknown terminal type\n", term);
	exit(1);
    }
    tcp = tcb;
    if (!(CM = tgetstr("cm", &tcp))) {
    	fprintf(stderr, "vi: terminal not capable of cursor motion\n");
	exit(1);
    }
    tc_END_D	= tgetstr("cd", &tcp);
    tc_END_L	= tgetstr("ce", &tcp);
    tc_ED	= tgetstr("cl", &tcp);
    tc_TP	= tgetstr("se", &tcp);
    tc_TI       = tgetstr("so", &tcp);
    tc_CI	= tgetstr("vi", &tcp);
    tc_CV	= tgetstr("vs", &tcp);
    tc_EL	= tgetstr("ce", &tcp);
    tc_IL	= tgetstr("al", &tcp);
    tc_DL 	= tgetstr("dl", &tcp);
}

void
windexit(int r)
{
    flushbuf();
    ss.sg_flags = oldFlags;
    ioctl(STDOUT_FILENO,TIOCSETP,&ss);
    exit(r);
}

void
windgoto(int r, int c)
{
    tputs(tgoto(CM,c,r),1,toutchar);

    /*WriteChar(0x1e);
    WriteChar(32+c);
    WriteChar(32+r); */
}

    /*
     * Should do something reasonable here. 
     */
/*void
sleep(n)
    int             n;
{
}*/

void
delay(void)
{
    long            l;

    flushbuf();
    /*
     * Should do something better here... 
     */

    sleep(1);
}

FILE           *
fopenb(fname, mode)
    char           *fname;
    char           *mode;
{
    char            modestr[10];

    sprintf(modestr, "b%s", mode);

    return fopen(fname, modestr);
}

#pragma optimize 8
/* we _must_ have this in here or we won't be in the right databank! */
#pragma databank 1
void stopHandler(int sig, int code)
{
void *oldHndl;
int redrawChar = 12;

    windgoto(23,0);
    flushbuf();
    printf("\nType 'fg' to restart vi...\n");
    oldHndl = signal(SIGTSTP,SIG_DFL);
    ss.sg_flags = oldFlags;
    ioctl(STDOUT_FILENO,TIOCSETP,&ss);
    kill(getpid(),SIGSTOP);
    signal(SIGTSTP,oldHndl);
    /* push a control-L into the input buffer- kinda goofy, but it works */
    ioctl(STDOUT_FILENO,TIOCSTI,&redrawChar);
    ss.sg_flags &= ~ECHO;
    ss.sg_flags |= CBREAK;
    ioctl(STDOUT_FILENO,TIOCSETP,&ss);
}
