/*
 * STEVIE - Simply Try this Editor for VI Enthusiasts
 *
 * Code Contributions By : Tim Thompson           twitch!tjt
 *                         Tony Andrews           onecom!wldrdg!tony 
 *                         G. R. (Fred) Walter    watmath!grwalter 
 */

#ifdef __ORCAC__
#ifndef MAINSEG
#pragma noroot
#endif
#endif

#include "env.h"

#include <stdio.h>
#ifndef ATARI
# ifndef UNIX
#   include <stdlib.h>
# endif
#endif
#include <ctype.h>
#ifndef MWC
# include <string.h>
#endif

#include "ascii.h"
#include "keymap.h"
#include "param.h"
/*#include "term.h"*/
#include "macros.h"

#ifdef AMIGA
/*
 * This is used to disable break signal handling.
 */
#include <signal.h>
#endif

extern char    *strchr();

#define NORMAL			 0
#define CMDLINE			 1
#define INSERT			 2
#define REPLACE			 3
#define APPEND			 4
#define UNDO			 5
#define REDO			 6
#define PUT			 7
#define FORWARD			 8
#define BACKWARD		 9
#define VALID			10
#define NOT_VALID		11
#define VALID_TO_CURSCHAR	12
#define UPDATE_CURSOR           13
#define UPDATE_ALL              14

/*
 * Boolean type definition and constants 
 */
typedef int     bool_t;

#ifndef	TRUE
# define	FALSE	(0)
# define	TRUE	(1)
#endif
#define	SORTOF	(2)
#define YES      TRUE
#define NO       FALSE
#define MAYBE    SORTOF

/*
 * Maximum screen dimensions
 */
#define MAX_COLUMNS 140L

/*
 * Buffer sizes
 */
#define CMDBUFFSIZE MAX_COLUMNS	/* size of the command processing buffer */

#define LSIZE        512	/* max. size of a line in the tags file */

#define IOSIZE     (1024+1)	/* file i/o and sprintf buffer size */

#define YANKSIZE    5200	/* yank buffer size */
#define INSERT_SIZE 5300	/* insert, redo and undo buffer size must be
				 * bigger than YANKSIZE */
#define REDO_UNDO_SIZE 5400	/* redo, undo and (undo an undo) buffer size
				 * must be bigger than INSERT_SIZE */
#define READSIZE    5500	/* read buffer size must be bigger than
				 * YANKSIZE and REDO_UNDO_SIZE */

/*
 * SLOP is the amount of extra space we get for text on a line during editing
 * operations that need more space. This keeps us from calling alloc every
 * time we get a character during insert mode. No extra space is allocated
 * when the file is initially read. 
 */
#define	SLOP	10

/*
 * LINEINC is the gap we leave between the artificial line numbers. This
 * helps to avoid renumbering all the lines every time a new line is
 * inserted. 
 *
 * Since line numbers are stored in longs (32 bits), a LINEINC of 10000
 * lets us have > 200,000 lines and we won't have to renumber very often.
 */
#define	LINEINC	10000

#define CHANGED    Changed = TRUE
#define UNCHANGED  Changed = FALSE

#define S_NOT_VALID NumLineSizes = -1

#define S_LINE_NOT_VALID LineNotValid = TRUE

#define S_CHECK_TOPCHAR_AND_BOTCHAR CheckTopcharAndBotchar = TRUE

#define S_MUST_UPDATE_BOTCHAR MustUpdateBotchar = TRUE

#define S_VALID_TO_CURSCHAR ValidToCurschar = TRUE

struct line {
    struct line    *next;	/* next line */
    struct line    *prev;	/* previous line */
    char           *s;		/* text for this line */
    int             size;	/* actual size of space at 's' */
    unsigned long   num;	/* line "number" */
};

#define	LINEOF(x)	((x)->linep->num)

struct lptr {
    struct line    *linep;	/* line we're referencing */
    int             index;	/* position within that line */
};

typedef struct line LINE;
typedef struct lptr LPtr;

struct charinfo {
    char            ch_size;
    char           *ch_str;
};

extern struct charinfo chars[];

#ifdef AMIGA
extern int      Aux_Device;
#endif
extern int      State;
extern int      Rows;
extern int      Columns;
extern int      CheckTopcharAndBotchar;
extern int      MustUpdateBotchar;
extern int      ValidToCurschar;
extern int      LineNotValid;
extern int      NumLineSizes;
extern LINE   **LinePointers;
extern char    *LineSizes;
extern char    *Filename;
extern LPtr    *Filemem;
extern LPtr    *Filetop;
extern LPtr    *Fileend;
extern LPtr    *Topchar;
extern LPtr    *Botchar;
extern LPtr    *Curschar;
extern LPtr    *Insstart;
extern int      Cursrow;
extern int      Curscol;
extern int      Cursvcol;
extern int      Curswant;
extern bool_t   set_want_col;
extern int      Prenum;
extern bool_t   Changed;
extern bool_t   RedrawingDisabled;
extern bool_t   UndoInProgress;
extern char    *IObuff;
extern char    *Insbuffptr;
extern char    *Insbuff;
extern char    *Readbuffptr;
extern char    *Readbuff;
extern char    *Redobuffptr;
extern char    *Redobuff;
extern char    *Undobuffptr;
extern char    *Undobuff;
extern char    *UndoUndobuffptr;
extern char    *UndoUndobuff;
extern char    *Yankbuffptr;
extern char    *Yankbuff;
extern char     last_command;
extern char     last_command_char;

extern char    *strcpy();

/* alloc.c */
char  *alloc();
char  *strsave();
void   screenalloc();
void   filealloc();
void   freeall();
LINE  *newline();
bool_t canincrease();

/* cmdline.c */
void   readcmdline();
void   dotag();
void   msg();
void   emsg();
void   smsg();
void   gotocmdline();
void   wait_return();

/* dec.c */
int    dec();

/* edit.c */
void   edit();
void   insertchar();
void   getout();
void   scrollup();
void   scrolldown();
void   beginline();
bool_t oneright();
bool_t oneleft();
bool_t oneup();
bool_t onedown();

/* fileio.c */
void   filemess();
void   renum();
bool_t readfile();
bool_t writeit();

/* s_io.c */
void   s_cursor_off();
void   s_cursor_on();
void   s_clear();
void   s_refresh();
void   NotValidFromCurschar();
void   Update_Botchar();

/* help.c */
bool_t help();

/* inc.c */
int    inc();

/* linefunc.c */
LPtr  *nextline();
LPtr  *prevline();
void   coladvance();

/* main.c */
void   stuffReadbuff();
void   stuffnumReadbuff();
char   vgetc();
char   vpeekc();

/* mark.c */
void   setpcmark();
void   clrall();
void   clrmark();
bool_t setmark();
LPtr  *getmark();

/* misccmds.c */
bool_t OpenForward();
bool_t OpenBackward();
void   fileinfo();
void   inschar();
void   insstr();
void   delline();
bool_t delchar();
int    cntllines();
int    plines();
LPtr  *gotoline();

/* normal.c */
void   normal();
void   ResetBuffers();
void   AppendToInsbuff();
void   AppendToRedobuff();
void   AppendNumberToRedobuff();
void   AppendToUndobuff();
void   AppendNumberToUndobuff();
void   AppendPositionToUndobuff();
void   AppendToUndoUndobuff();
void   AppendNumberToUndoUndobuff();
void   AppendPositionToUndoUndobuff();
bool_t linewhite();

/* mk.c */
char  *mkstr();
char  *mkline();

/* param.c */
void   doset();

/* screen.c */
void   cursupdate();

/* search.c */
void   doglob();
void   dosub();
void   searchagain();
bool_t dosearch();
bool_t repsearch();
bool_t searchc();
bool_t crepsearch();
bool_t findfunc();
LPtr  *showmatch();
LPtr  *fwd_word();
LPtr  *bck_word();
LPtr  *end_word();

/* format_l.c */
char *format_line();

/*
 * Machine-dependent routines. 
 */
#ifdef AMIGA
# include "amiga.h"
#endif
#ifdef BSD
# include "bsd.h"
#endif
#ifdef UNIX
# include "unix.h"
#endif
#ifdef TOS
# include "tos.h"
#endif
#ifdef OS2
# include "os2.h"
#endif
#ifdef DOS
# include "dos.h"
#endif
#ifdef GSOS
# include "gsos.h"
#endif
