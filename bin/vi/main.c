/*
 * STEVIE - Simply Try this Editor for VI Enthusiasts
 *
 * Code Contributions By : Tim Thompson           twitch!tjt
 *                         Tony Andrews           onecom!wldrdg!tony 
 *                         G. R. (Fred) Walter    watmath!grwalter 
 */

#ifdef __ORCAC__
#pragma stacksize 2048
#define MAINSEG
#include <stdio.h>
#include <gno/gno.h>
#include <gno/signal.h>
#endif

#ifdef AMIGA
# include <proto/exec.h>
#endif

#include "stevie.h"

#ifdef AMIGA
int             Aux_Device = FALSE;
#endif

int             Rows;		/* Number of Rows and Columns */
int             Columns;	/* in the current window. */

int             CheckTopcharAndBotchar = FALSE;
int             MustUpdateBotchar = FALSE;
int             ValidToCurschar = FALSE;
int             LineNotValid = FALSE;

int             NumLineSizes = -1;	/* # of active LineSizes */
LINE          **LinePointers = NULL;	/* Pointer to the line for LineSizes */
char           *LineSizes = NULL;	/* Size of a line (pline output) */

char           *Filename = NULL;/* Current file name */

LPtr           *Filemem;	/* The contents of the file, as a single
				 * array. */
LPtr           *Filetop;	/* Line 'above' the start of the file */

LPtr           *Fileend;	/* Pointer to the end of the file in Filemem.
				 * (It points to the byte AFTER the last
				 * byte.) */

LPtr           *Topchar;	/* Pointer to the byte in Filemem which is in
				 * the upper left corner of the screen. */

LPtr           *Botchar;	/* Pointer to the byte in Filemem which is
				 * just off the bottom of the screen. */

LPtr           *Curschar;	/* Pointer to byte in Filemem at which the
				 * cursor is currently placed. */

int             Curscol;	/* Current position of cursor (column) */
int             Cursrow;	/* Current position of cursor (row) */

int             Cursvcol;	/* Current virtual column, the column number
				 * of the file's actual line, as opposed to
				 * the column number we're at on the screen.
				 * This makes a difference on lines that span
				 * more than one screen line. */

int             Curswant = 0;	/* The column we'd like to be at. This is
				 * used try to stay in the same column
				 * through up/down cursor motions. */

bool_t          set_want_col;	/* If set, then update Curswant the next time
				 * through cursupdate() to the current
				 * virtual column. */

int             State = NORMAL;	/* This is the current state of the command
				 * interpreter. */

int             Prenum = 0;	/* The (optional) number before a command. */

LPtr           *Insstart;	/* This is where the latest insert/append
				 * mode started. */

bool_t          Changed = FALSE;/* Set to TRUE if something in the file has
				 * been changed and not written out. */

char           *IObuff;		/* file reads are done, one line at a time,
				 * into this buffer; as well as sprintf's */

char           *Insbuffptr = NULL;
char           *Insbuff;	/* Each insertion gets stuffed into this
				 * buffer. */

char           *Readbuffptr = NULL;
char           *Readbuff;	/* Having this buffer allows STEVIE to easily
				 * make itself do commands */

char           *Redobuffptr = NULL;
char           *Redobuff;	/* Each command should stuff characters into
				 * this buffer that will re-execute itself. */

bool_t          UndoInProgress = FALSE;	/* Set to TRUE if undo'ing */
char           *Undobuffptr = NULL;
char           *Undobuff;	/* Each command should stuff characters into
				 * this buffer that will undo its effects. */

char           *UndoUndobuffptr = NULL;
char           *UndoUndobuff;	/* Each command should stuff characters into
				 * this buffer that will undo its undo. */

char           *Yankbuffptr = NULL;
char           *Yankbuff;	/* Yank buffer */

char            last_command = NUL;	/* last command */
char            last_command_char = NUL;	/* character needed to undo
						 * last command */

bool_t          RedrawingDisabled = FALSE;	/* Set to TRUE if undo'ing or
						 * put'ing */

char          **files = NULL;	/* list of input files */
int             numfiles = 0;	/* number of input files */
int             curfile;	/* number of the current file */

static void
usage(void)
{
    fprintf(stderr, "usage: stevie [file ...]\n");
    fprintf(stderr, "       stevie -t tag\n");
    fprintf(stderr, "       stevie +[num] file\n");
    fprintf(stderr, "       stevie +/pat  file\n");
    exit(1);
}

#ifdef AMIGA
void
#else
int
#endif
main(int argc, char **argv)
{
    char           *initstr;	/* init string from the environment */
    char           *tag = NULL;	/* tag from command line */
    char           *pat = NULL;	/* pattern from command line */
    int             line = -1;	/* line number from command line */

    int             atoi(char *);
    char           *getenv(char *);

#ifdef __ORCAC__
{
extern void stopHandler(int,int);
    signal(SIGINT, SIG_IGN);      /* vi should ignore ^C requests */
    signal(SIGTSTP, stopHandler); /* handle ^Z with a message */
}
#endif

#ifdef AMIGA
    {
	struct Library *DosBase;/* Used for checking version */

	DosBase = OpenLibrary("dos.library", 33);
	if (!DosBase) {
	    fprintf(stderr,
		 "\nSTEVIE requires Version 33 or later of dos.library.\n");
	    exit(2);
	} else {
	    CloseLibrary(DosBase);
	}

/*
 * I don't think STEVIE should be exited with a break.
 */
	(void) signal(SIGINT, SIG_IGN);
    }
#endif

    /*
     * Process the command line arguments. 
     */
    if (argc > 1) {
	switch (argv[1][0]) {

	  case '-':		/* -t tag */
	    if (argv[1][1] != 't')
		usage();

	    if (argv[2] == NULL)
		usage();

	    tag = argv[2];
	    numfiles = 1;
	    break;

	  case '+':		/* +n or +/pat */
	    if (argv[1][1] == '/') {
		if (argv[2] == NULL)
		    usage();
		Filename = strsave(argv[2]);
		pat = &(argv[1][1]);
		numfiles = 1;

	    } else if (isdigit(argv[1][1]) || argv[1][1] == NUL) {
		if (argv[2] == NULL)
		    usage();
		Filename = strsave(argv[2]);
		numfiles = 1;

		line = (isdigit(argv[1][1])) ?
		    atoi(&(argv[1][1])) : 0;
	    } else
		usage();

	    break;

	  default:		/* must be a file name */
#ifdef WILD_CARDS
	    ExpandWildCards(argc - 1, &(argv[1]), &numfiles, &files);
	    if (numfiles == 0)
		numfiles = 1;
	    else
		Filename = strsave(files[0]);
#else
	    Filename = strsave(argv[1]);
	    files = &(argv[1]);
	    numfiles = argc - 1;
#endif
	    if (numfiles > 1)
		printf("%d files to edit\n", numfiles);
	    break;
	}
    } else {
	numfiles = 1;
    }
    curfile = 0;

    windinit();

    /*
     * Allocate LPtr structures for all the various position pointers and for
     * the many buffers. 
     */
    Filetop = (LPtr *) alloc((unsigned) sizeof(LPtr));
    Filemem = (LPtr *) alloc((unsigned) sizeof(LPtr));
    Fileend = (LPtr *) alloc((unsigned) sizeof(LPtr));
    Topchar = (LPtr *) alloc((unsigned) sizeof(LPtr));
    Curschar = (LPtr *) alloc((unsigned) sizeof(LPtr));
    Botchar = (LPtr *) alloc((unsigned) sizeof(LPtr));
    Insstart = (LPtr *) alloc((unsigned) sizeof(LPtr));
    IObuff = alloc(IOSIZE);
    Insbuff = alloc(INSERT_SIZE);
    Readbuff = alloc(READSIZE);
    Redobuff = alloc(REDO_UNDO_SIZE);
    Undobuff = alloc(REDO_UNDO_SIZE);
    UndoUndobuff = alloc(REDO_UNDO_SIZE);
    Yankbuff = alloc(YANKSIZE);
    if (Filetop == NULL ||
	Filemem == NULL ||
	Fileend == NULL ||
	Topchar == NULL ||
	Curschar == NULL ||
	Botchar == NULL ||
	Insstart == NULL ||
	IObuff == NULL ||
	Insbuff == NULL ||
	Readbuff == NULL ||
	Redobuff == NULL ||
	Undobuff == NULL ||
	UndoUndobuff == NULL ||
	Yankbuff == NULL) {
	fprintf(stderr, "Can't allocate data structures\n");
	windexit(0);
    }
    screenalloc();
    filealloc();		/* Initialize Filemem, Filetop & Fileend */

    s_clear();

    initstr = getenv("EXINIT");
    if (initstr != NULL) {
	char           *lp, buf[128];

	lp = getenv("LINES");
	if (lp != NULL) {
	    sprintf(buf, "%s lines=%s", initstr, lp);
	    readcmdline(':', buf);
	} else
	    readcmdline(':', initstr);
    }
    if (Filename != NULL) {
	if (readfile(Filename, Filemem, FALSE))
	    filemess("[New File]");
    } else {
	s_refresh(NOT_VALID);
	msg("Empty Buffer");
    }

    setpcmark();

    if (tag) {
	stuffReadbuff(":ta ");
	stuffReadbuff(tag);
	stuffReadbuff("\n");
    } else if (pat) {
	stuffReadbuff(pat);
	stuffReadbuff("\n");
    } else if (line >= 0) {
	if (line > 0)
	    stuffnumReadbuff(line);
	stuffReadbuff("G");
    }
    edit();
    /* NOTREACHED */
    /* windexit(0); */
}

void
stuffReadbuff(char *s)
{
    if (Readbuffptr == NULL) {
	if ((strlen(s) + 1) < READSIZE) {
	    strcpy(Readbuff, s);
	    Readbuffptr = Readbuff;
	    return;
	}
    } else if ((strlen(Readbuff) + (strlen(s) + 1)) < READSIZE) {
	strcat(Readbuff, s);
	return;
    }
    emsg("Couldn't stuffReadbuff() - clearing Readbuff\n");
    *Readbuff = NUL;
    Readbuffptr = NULL;
}

void
stuffnumReadbuff(int n)
{
    char            buf[32];

    sprintf(buf, "%d", n);
    stuffReadbuff(buf);
}

/* OPTRESULT */
char
vgetc(void)
{
    int             c;

    /*
     * inchar() may map special keys by using stuffReadbuff(). If it does so,
     * it returns -1 so we know to loop here to get a real char. 
     */
    do {
	if (Readbuffptr != NULL) {
	    char            nextc = *Readbuffptr++;

	    if (*Readbuffptr == NUL) {
		*Readbuff = NUL;
		Readbuffptr = NULL;
	    }
	    return (nextc);
	}
	c = inchar();
    } while (c == -1);

    return (char) c;
}

char
vpeekc(void)
{
    if (Readbuffptr != NULL)
	return (*Readbuffptr);
    return (NUL);
}
