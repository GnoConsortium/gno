/*
 * STEVIE - Simply Try this Editor for VI Enthusiasts
 *
 * Code Contributions By : Tim Thompson           twitch!tjt
 *                         Tony Andrews           onecom!wldrdg!tony 
 *                         G. R. (Fred) Walter    watmath!grwalter 
 */

/*
 * This file contains the main routine for processing characters in command
 * mode as well as routines for handling the operators. 
 */

#ifdef __ORCAC__
segment "normal";
#endif

#include "stevie.h"

static void     doshift(int);
static void     dodelete(bool_t, bool_t, bool_t);
static void     doput(int);
static void     dochange(void);
static void     startinsert(int);
static bool_t   dojoin(bool_t,bool_t);
static bool_t   doyank(void);

/*
 * Macro evaluates true if char 'c' is a valid identifier character 
 */
#define IDCHAR(c)       (isalpha(c) || isdigit(c) || (c) == '_')

/*
 * Operators 
 */
#define NOP     0		/* no pending operation */
#define DELETE  1
#define YANK    2
#define CHANGE  3
#define LSHIFT  4
#define RSHIFT  5

#define CLEAROP (operator = NOP)/* clear any pending operator */

static int      operator = NOP;	/* current pending operator */

/*
 * When a cursor motion command is made, it is marked as being a character or
 * line oriented motion. Then, if an operator is in effect, the operation
 * becomes character or line oriented accordingly. 
 *
 * Character motions are marked as being inclusive or not. Most char. motions
 * are inclusive, but some (e.g. 'w') are not. 
 *
 * Generally speaking, every command in normal() should either clear any pending
 * operator (with CLEAROP), or set the motion type variable. 
 */

/*
 * Motion types 
 */
#define MBAD    (-1)		/* 'bad' motion type marks unusable yank buf */
#define MCHAR   0
#define MLINE   1

static int      mtype;		/* type of the current cursor motion */
static bool_t   mincl;		/* true if char motion is inclusive */
static int      ybtype = MBAD;
static int      ybcrossline = FALSE;

static LPtr     startop;	/* cursor pos. at start of operator */

/*
 * Operators can have counts either before the operator, or between the
 * operator and the following cursor motion as in: 
 *
 * d3w or 3dw 
 *
 * If a count is given before the operator, it is saved in opnum. If normal() is
 * called with a pending operator, the count in opnum (if present) overrides
 * any count that came later. 
 */
static int      opnum = 0;

#define DEFAULT1(x)     (((x) == 0) ? 1 : (x))

/*
 * normal 
 *
 * Execute a command in normal mode. 
 */

void
normal(char c)
{
    char           *p;
    int             n;
    int             nn;
    bool_t          flag = FALSE;
    int             type = 0;	/* used in some operations to modify type */
    int             dir = FORWARD;	/* search direction */
    char            nchar = NUL;
    bool_t          finish_op;
    LPtr            temp_Curschar;

    last_command = NUL;
    /*
     * If there is an operator pending, then the command we take this time
     * will terminate it. Finish_op tells us to finish the operation before
     * returning this time (unless the operation was cancelled). 
     */
    finish_op = (operator != NOP);

    /*
     * If we're in the middle of an operator AND we had a count before the
     * operator, then that count overrides the current value of Prenum. What
     * this means effectively, is that commands like "3dw" get turned into
     * "d3w" which makes things fall into place pretty neatly. 
     */
    if (finish_op) {
	if (opnum != 0)
	    Prenum = opnum;
    } else
	opnum = 0;

    switch (c) {

      case K_HELP:
	CLEAROP;
	if (help())
	    s_clear();
	break;

      case CTRL('L'):
	CLEAROP;
	s_clear();
	break;

      case CTRL('D'):
	CLEAROP;
	if (Prenum)
	    P(P_SS) = (Prenum > Rows - 1) ? Rows - 1 : Prenum;
	scrollup((P(P_SS) < Rows) ? P(P_SS) : Rows - 1);
	onedown((P(P_SS) < Rows) ? P(P_SS) : Rows - 1);
	break;

      case CTRL('U'):
	CLEAROP;
	if (Prenum)
	    P(P_SS) = (Prenum > Rows - 1) ? Rows - 1 : Prenum;
	scrolldown((P(P_SS) < Rows) ? P(P_SS) : Rows - 1);
	oneup((P(P_SS) < Rows) ? P(P_SS) : Rows - 1);
	break;

      case CTRL('F'):
	CLEAROP;
	if (nextline(Topchar) == NULL) {
	    beep();
	    break;
	}
	Prenum = DEFAULT1(Prenum);
	while (Prenum > 0) {
	    *Curschar = *prevline(Botchar);
	    *Topchar = *Curschar;
	    Topchar->index = 0;
	    Update_Botchar();
	    Prenum--;
	}
	beginline(TRUE);
	s_clear();
	break;

      case CTRL('B'):
	CLEAROP;
	if (prevline(Topchar) == NULL) {
	    beep();
	    break;
	}
	Prenum = DEFAULT1(Prenum);
	while (Prenum > 0) {
	    *Curschar = *Topchar;
	    n = Rows - 1;
	    {
		LPtr           *lp = Curschar;
		int             l = 0;

		while ((l < n) && (lp != NULL)) {
		    l += plines(lp->linep->s);
		    *Topchar = *lp;
		    lp = prevline(lp);
		}
	    }
	    Topchar->index = 0;
	    Prenum--;
	}
	beginline(TRUE);
	s_clear();
	break;

      case CTRL('E'):
	CLEAROP;
	scrollup(DEFAULT1(Prenum));
	if (LINEOF(Curschar) < LINEOF(Topchar))
	    Curschar->linep = Topchar->linep;
	break;

      case CTRL('Y'):
	CLEAROP;
	scrolldown(DEFAULT1(Prenum));
	Update_Botchar();
	if (LINEOF(Curschar) >= LINEOF(Botchar)) {
	    LPtr           *lp;

	    lp = prevline(Botchar);
	    if (lp == NULL)
		lp = Topchar;
	    Curschar->linep = lp->linep;
	}
	break;

      case 'z':
	CLEAROP;
	S_CHECK_TOPCHAR_AND_BOTCHAR;
	switch (vgetc()) {
	  case NL:		/* put Curschar at top of screen */
	  case CR:
	    *Topchar = *Curschar;
	    Topchar->index = 0;
	    break;

	  case '.':		/* put Curschar in middle of screen */
	    n = Rows / 2;
	    goto dozcmd;

	  case '-':		/* put Curschar at bottom of screen */
	    n = Rows - 1;
	    /* FALLTHROUGH */

    dozcmd:
	    {
		register LPtr  *lp = Curschar;
		register int    l = 0;

		while ((l < n) && (lp != NULL)) {
		    l += plines(lp->linep->s);
		    *Topchar = *lp;
		    lp = prevline(lp);
		}
	    }
	    Topchar->index = 0;
	    break;

	  default:
	    beep();
	}
	break;

      case CTRL('G'):
	CLEAROP;
	fileinfo();
	break;

      case 'G':
	mtype = MLINE;
	*Curschar = *gotoline(Prenum);
	if (!UndoInProgress) {
	    beginline(TRUE);
	    S_CHECK_TOPCHAR_AND_BOTCHAR;
	}
	break;

      case 'H':
	mtype = MLINE;
	*Curschar = *Topchar;
	for (n = Prenum; n && onedown(1); n--);
	beginline(TRUE);
	break;

      case 'M':
	mtype = MLINE;
	*Curschar = *Topchar;
	for (n = 0; n < Rows / 2 && onedown(1); n++);
	beginline(TRUE);
	break;

      case 'L':
	mtype = MLINE;
	*Curschar = *prevline(Botchar);
	for (n = Prenum; n && oneup(1); n--);
	beginline(TRUE);
	break;

      case 'l':
      case K_RARROW:
      case ' ':
	mtype = MCHAR;
	mincl = FALSE;
	n = DEFAULT1(Prenum);
	while (n--) {
	    if (!oneright()) {
		if (operator != DELETE && operator != CHANGE) {
		    beep();
		} else {
		    if (lineempty(Curschar)) {
			CLEAROP;
			beep();
		    } else {
			mincl = TRUE;
		    }
		}
		break;
	    }
	}
	set_want_col = TRUE;
	break;

      case 'h':
      case K_LARROW:
      case CTRL('H'):
	mtype = MCHAR;
	mincl = FALSE;
	Prenum = DEFAULT1(Prenum);
	n = Prenum;
	while (n--) {
	    if (!oneleft()) {
		if (operator != DELETE && operator != CHANGE) {
		    beep();
		} else if (Prenum == 1) {
		    CLEAROP;
		    beep();
		}
		break;
	    }
	}
	set_want_col = TRUE;
	break;

      case '-':
	flag = TRUE;
	/* FALLTHROUGH */

      case 'k':
      case K_UARROW:
      case CTRL('P'):
	mtype = MLINE;
	if (!oneup(DEFAULT1(Prenum))) {
	    CLEAROP;
	    beep();
	} else if (flag)
	    beginline(TRUE);
	break;

      case '+':
      case CR:
      case NL:
	flag = TRUE;
	/* FALLTHROUGH */

      case 'j':
      case K_DARROW:
      case CTRL('N'):
	mtype = MLINE;
	if (!onedown(DEFAULT1(Prenum))) {
	    CLEAROP;
	    beep();
	} else if (flag)
	    beginline(TRUE);
	break;

	/*
	 * This is a strange motion command that helps make operators more
	 * logical. It is actually implemented, but not documented in the
	 * real 'vi'. This motion command actually refers to "the current
	 * line". Commands like "dd" and "yy" are really an alternate form of
	 * "d_" and "y_". It does accept a count, so "d3_" works to delete 3
	 * lines. 
	 */
      case '_':
lineop:
	mtype = MLINE;
	if (!onedown(DEFAULT1(Prenum) - 1)) {
	    CLEAROP;
	    beep();
	} else
	    beginline(TRUE);
	break;

      case '|':
	mtype = MCHAR;
	mincl = TRUE;
	beginline(FALSE);
	if (Prenum > 0)
	    coladvance(Curschar, Prenum - 1);
	Curswant = Prenum - 1;
	break;

      case CTRL(']'):		/* :ta to current identifier */
	CLEAROP;
	{
	    char            ch;
	    LPtr            save;

	    save = *Curschar;
	    /*
	     * First back up to start of identifier. This doesn't match the
	     * real vi but I like it a little better and it shouldn't bother
	     * anyone. 
	     */
	    ch = gchar(Curschar);
	    while (IDCHAR(ch)) {
		if (!oneleft())
		    break;
		ch = gchar(Curschar);
	    }
	    if (!IDCHAR(ch))
		oneright();

	    stuffReadbuff(":ta ");
	    /*
	     * Now grab the chars in the identifier 
	     */
	    ch = gchar(Curschar);
	    while (IDCHAR(ch)) {
		stuffReadbuff(mkstr(ch));
		if (!oneright())
		    break;
		ch = gchar(Curschar);
	    }
	    stuffReadbuff("\n");

	    *Curschar = save;	/* restore, in case of error */
	}
	break;

      case '%':
	S_CHECK_TOPCHAR_AND_BOTCHAR;
	mtype = MCHAR;
	mincl = TRUE;
	{
	    LPtr           *pos;

	    if ((pos = showmatch()) == NULL) {
		CLEAROP;
		beep();
	    } else {
		setpcmark();
		*Curschar = *pos;
		set_want_col = TRUE;
	    }
	}
	break;

	/*
	 * Word Motions 
	 */

      case 'B':
	type = 1;
	/* FALLTHROUGH */

      case 'b':
	mtype = MCHAR;
	mincl = FALSE;
	set_want_col = TRUE;
	for (n = DEFAULT1(Prenum); n > 0; n--) {
	    LPtr           *pos;

	    if ((Curschar->linep->prev == Filetop->linep)
		&& (Curschar->index == 0)) {
		CLEAROP;
		beep();
		break;
	    }
	    pos = bck_word(Curschar, type);
	    if (pos == NULL) {
		CLEAROP;
		beep();
		*Curschar = *gotoline(1);	/* goto top of file */
	    } else
		*Curschar = *pos;
	}
	break;

      case 'W':
	type = 1;
	/* FALLTHROUGH */

      case 'w':
	/*
	 * This is a little strange. To match what the real vi does, we
	 * effectively map 'cw' to 'ce', and 'cW' to 'cE'. This seems
	 * impolite at first, but it's really more what we mean when we say
	 * 'cw'. 
	 */
	if (operator == CHANGE)
	    goto do_e_cmd;

	mtype = MCHAR;
	mincl = FALSE;
	set_want_col = TRUE;
	for (n = DEFAULT1(Prenum); n > 0; n--) {
	    LPtr           *pos;

	    if ((pos = fwd_word(Curschar, type)) == NULL) {
		CLEAROP;
		beep();
		break;
	    } else
		*Curschar = *pos;
	}
	if (operator == DELETE && DEFAULT1(Prenum) == 1) {
	    if (LINEOF(&startop) != LINEOF(Curschar)) {
		*Curschar = startop;
		while (oneright());
		mincl = TRUE;
	    }
	}
	break;

      case 'E':
	type = 1;
	/* FALLTHROUGH */

      case 'e':
do_e_cmd:
	mtype = MCHAR;
	mincl = TRUE;
	set_want_col = TRUE;
	if (c == 'e' || c == 'E') {
	    if (inc(Curschar) == -1) {
		CLEAROP;
		beep();
		break;
	    }
	}
	for (n = DEFAULT1(Prenum); n > 0; n--) {
	    LPtr           *pos;

	    if ((pos = end_word(Curschar, type)) == NULL) {
		CLEAROP;
		beep();
		break;
	    } else
		*Curschar = *pos;
	}
	break;

      case '$':
	mtype = MCHAR;
	mincl = TRUE;
	while (oneright());
	Curswant = 999;		/* so we stay at the end */
	break;

      case '^':
	flag = TRUE;
	/* FALLTHROUGH */

      case '0':
	mtype = MCHAR;
	mincl = TRUE;
	beginline(flag);
	break;

      case 'R':
	ResetBuffers();
	AppendToRedobuff("R");
	CLEAROP;
	n = RowNumber(Curschar);
	AppendPositionToUndobuff(Curschar->index, n);
	AppendPositionToUndoUndobuff(Curschar->index, n);
	AppendToUndoUndobuff("R");

	last_command = 'R';
	startinsert(FALSE);
	break;

      case 'A':
	set_want_col = TRUE;
	while (oneright());
	ResetBuffers();
	AppendToRedobuff("A");
	goto doAPPENDcmd;

      case 'a':
	ResetBuffers();
	AppendToRedobuff("a");

doAPPENDcmd:
	CLEAROP;
	/* Works just like an 'i'nsert on the next character. */
	n = RowNumber(Curschar);
	AppendPositionToUndoUndobuff(Curschar->index, n);
	AppendToUndoUndobuff("a");

	if (!lineempty(Curschar))
	    inc(Curschar);

	n = RowNumber(Curschar);
	AppendPositionToUndobuff(Curschar->index, n);

	startinsert(FALSE);
	break;

      case 'I':
	beginline(TRUE);
	ResetBuffers();
	AppendToRedobuff("I");
	goto doINSERTcmd;
	/* FALLTHROUGH */

      case 'i':
      case K_INSERT:
	ResetBuffers();
	AppendToRedobuff("i");

doINSERTcmd:
	CLEAROP;

	n = RowNumber(Curschar);
	AppendPositionToUndobuff(Curschar->index, n);
	AppendPositionToUndoUndobuff(Curschar->index, n);
	AppendToUndoUndobuff("i");

	startinsert(FALSE);
	break;

      case 'o':
	CLEAROP;
	ResetBuffers();

	n = RowNumber(Curschar);
	AppendToRedobuff("o");
	AppendPositionToUndobuff(Curschar->index, n);
	AppendPositionToUndoUndobuff(Curschar->index, n);
	AppendToUndoUndobuff("o");

	if (OpenForward(!RedrawingDisabled))
	    startinsert(TRUE);

	last_command = 'o';
	break;

      case 'O':
	CLEAROP;
	ResetBuffers();

	n = RowNumber(Curschar);
	AppendToRedobuff("O");
	AppendPositionToUndobuff(Curschar->index, n);
	AppendPositionToUndoUndobuff(Curschar->index, n);
	AppendToUndoUndobuff("O");

	if (OpenBackward(!RedrawingDisabled))
	    startinsert(TRUE);

	last_command = 'O';
	break;

      case 'd':
	if (operator == DELETE)	/* handle 'dd' */
	    goto lineop;
	if (Prenum != 0)
	    opnum = Prenum;
	startop = *Curschar;
	operator = DELETE;
	break;

	/*
	 * Some convenient abbreviations... 
	 */

      case 'x':
	if (Prenum)
	    stuffnumReadbuff(Prenum);
	stuffReadbuff("dl");
	break;

      case 'X':
	if (Prenum)
	    stuffnumReadbuff(Prenum);
	stuffReadbuff("dh");
	break;

      case 'D':
	stuffReadbuff("d$");
	break;

      case 'Y':
	if (Prenum)
	    stuffnumReadbuff(Prenum);
	stuffReadbuff("yy");
	break;

      case 'C':
	stuffReadbuff("c$");
	break;

      case 'c':
	if (operator == CHANGE) {	/* handle 'cc' */
	    CLEAROP;
	    stuffReadbuff("0c$");
	    break;
	}
	if (Prenum != 0)
	    opnum = Prenum;
	startop = *Curschar;
	operator = CHANGE;
	break;

      case 'y':
	if (operator == YANK)	/* handle 'yy' */
	    goto lineop;
	if (Prenum != 0)
	    opnum = Prenum;
	startop = *Curschar;
	operator = YANK;
	break;

      case ENABLE_REDRAWING:
	RedrawingDisabled = FALSE;
	S_NOT_VALID;
	break;

      case 'p':
	if (Yankbuffptr != NULL) {
	    doput(FORWARD);

	    stuffReadbuff(ENABLE_REDRAWING_STR);
	    RedrawingDisabled = TRUE;
	} else
	    beep();
	break;

      case 'P':
	if (Yankbuffptr != NULL) {
	    doput(BACKWARD);

	    stuffReadbuff(ENABLE_REDRAWING_STR);
	    RedrawingDisabled = TRUE;
	} else
	    beep();
	break;

      case '>':
	if (operator == RSHIFT)	/* handle >> */
	    goto lineop;
	if (operator == LSHIFT) {
	    CLEAROP;
	    beep();
	    break;
	}
	if (Prenum != 0)
	    opnum = Prenum;
	startop = *Curschar;	/* save current position */
	operator = RSHIFT;
	break;

      case '<':
	if (operator == LSHIFT)	/* handle << */
	    goto lineop;
	if (operator == RSHIFT) {
	    CLEAROP;
	    beep();
	    break;
	}
	if (Prenum != 0)
	    opnum = Prenum;
	startop = *Curschar;	/* save current position */
	operator = LSHIFT;
	break;

      case 's':		/* substitute characters */
	if (Prenum)
	    stuffnumReadbuff(Prenum);
	stuffReadbuff("cl");
	break;

      case '?':
      case '/':
      case ':':
	CLEAROP;
	readcmdline(c, (char *) NULL);
	break;

      case 'n':
	mtype = MCHAR;
	mincl = FALSE;
	set_want_col = TRUE;
	if (!repsearch(0)) {
	    CLEAROP;
	    beep();
	}
	break;

      case 'N':
	mtype = MCHAR;
	mincl = FALSE;
	set_want_col = TRUE;
	if (!repsearch(1)) {
	    CLEAROP;
	    beep();
	}
	break;

	/*
	 * Character searches 
	 */
      case 'T':
	dir = BACKWARD;
	/* FALLTHROUGH */

      case 't':
	type = 1;
	goto docsearch;

      case 'F':
	dir = BACKWARD;
	/* FALLTHROUGH */

      case 'f':
docsearch:
	mtype = MCHAR;
	mincl = TRUE;
	set_want_col = TRUE;
	if ((nchar = vgetc()) == ESC)	/* search char */
	    break;
	if (!searchc(nchar, dir, type)) {
	    CLEAROP;
	    beep();
	}
	break;

      case ',':
	flag = 1;
	/* FALLTHROUGH */

      case ';':
	mtype = MCHAR;
	mincl = TRUE;
	set_want_col = TRUE;
	if (!crepsearch(flag)) {
	    CLEAROP;
	    beep();
	}
	break;

	/*
	 * Function searches 
	 */

      case '[':
	dir = BACKWARD;
	/* FALLTHROUGH */

      case ']':
	mtype = MLINE;
	set_want_col = TRUE;
	if (vgetc() != c) {
	    CLEAROP;
	    beep();
	    break;
	}
	if (!findfunc(dir)) {
	    CLEAROP;
	    beep();
	}
	break;

	/*
	 * Marks 
	 */

      case 'm':
	CLEAROP;
	if (!setmark(vgetc()))
	    beep();
	break;

      case '\'':
	flag = TRUE;
	/* FALLTHROUGH */

      case '`':
	S_CHECK_TOPCHAR_AND_BOTCHAR;
	{
	    LPtr            mtmp;
	    LPtr           *mark = getmark(vgetc());

	    if (mark == NULL) {
		CLEAROP;
		beep();
	    } else {
		mtmp = *mark;
		setpcmark();
		*Curschar = mtmp;
		if (flag)
		    beginline(TRUE);
	    }
	    mtype = flag ? MLINE : MCHAR;
	    mincl = TRUE;	/* ignored if not MCHAR */
	    set_want_col = TRUE;
	}
	break;

      case 'r':
	CLEAROP;
	if (lineempty(Curschar)) {	/* Nothing to replace */
	    beep();
	    break;
	}
	nosuspend();
        nchar = vgetc();
	dosuspend();
	if (nchar == ESC) break;

	Prenum = DEFAULT1(Prenum);
	n = strlen(Curschar->linep->s) - Curschar->index;
	if (n < Prenum) {
	    beep();
	    break;
	}
	ResetBuffers();

	nn = RowNumber(Curschar);
	AppendPositionToUndobuff(Curschar->index, nn);
	AppendPositionToUndoUndobuff(Curschar->index, nn);

	while (Prenum > 0) {
	    AppendToRedobuff("r");
	    AppendToRedobuff(mkstr(nchar));

	    AppendToUndobuff("r");
	    AppendToUndobuff(mkstr(gchar(Curschar)));

	    AppendToUndoUndobuff("r");
	    AppendToUndoUndobuff(mkstr(nchar));

	    pchar(Curschar, nchar);	/* Change current character. */

	    if (Prenum > 1) {
		oneright();
		AppendToRedobuff("l");
		AppendToUndobuff("l");
		AppendToUndoUndobuff("l");
	    }
	    Prenum--;
	}

	CHANGED;
	S_LINE_NOT_VALID;
	break;

      case '~':		/* swap case */
	CLEAROP;
	if (lineempty(Curschar)) {
	    beep();
	    break;
	}
	ResetBuffers();

	n = RowNumber(Curschar);
	AppendPositionToUndobuff(Curschar->index, n);
	AppendPositionToUndoUndobuff(Curschar->index, n);

	Prenum = DEFAULT1(Prenum);
	if (Prenum > 0) {
	    AppendNumberToRedobuff(Prenum);
	    AppendNumberToUndobuff(Prenum);
	    AppendNumberToUndoUndobuff(Prenum);
	}
	AppendToRedobuff("~");
	AppendToUndobuff("~");
	AppendToUndoUndobuff("~");

	while (Prenum > 0) {
	    c = gchar(Curschar);
	    if (isalpha(c)) {
		if (islower(c))
		    pchar(Curschar, toupper(c));
		else
		    pchar(Curschar, tolower(c));
	    }
	    if (!oneright())
		break;
	    Prenum--;
	}

	CHANGED;
	S_LINE_NOT_VALID;
	break;

      case UNDO_SHIFTJ:
	CLEAROP;
	if (UndoInProgress) {
	    (void) dojoin(FALSE, FALSE);
	    break;
	}
	goto doSHIFTJcommand;

      case 'J':
	CLEAROP;
doSHIFTJcommand:
	if (nextline(Curschar) == NULL) {	/* on last line */
	    beep();
	    break;
	}
	ResetBuffers();

	temp_Curschar = *Curschar;
	nn = strlen(Curschar->linep->s);
	if (nn < 0)
	    nn = 0;
	n = RowNumber(&temp_Curschar);

	AppendToRedobuff("J");

	AppendPositionToUndobuff(nn, n);

	AppendPositionToUndoUndobuff(0, n);
	AppendToUndoUndobuff("J");

	if (linewhite(nextline(Curschar))) {
	    AppendToUndobuff("a\n");
	    if (!dojoin(FALSE, TRUE)) {
		beep();
		break;
	    }
	} else if (lineempty(Curschar)) {
	    AppendToUndobuff("i\n");
	    if (!dojoin(FALSE, TRUE)) {
		beep();
		break;
	    }
	} else {
	    AppendToUndobuff("dli\n");
	    if (!dojoin(TRUE, TRUE)) {
		beep();
		break;
	    }
	}

	AppendToUndobuff(ESC_STR);
	AppendPositionToUndobuff(nn, n);
	break;

      case K_CGRAVE:		/* shorthand command */
	CLEAROP;
	stuffReadbuff(":e #\n");
	break;

      case 'Z':		/* write, if changed, and exit */
	if (vgetc() != 'Z') {
	    beep();
	    break;
	}
	if (Changed) {
	    if (Filename != NULL) {
		if (!writeit(Filename, (LPtr *) NULL, (LPtr *) NULL))
		    return;
	    } else {
		emsg("No output file");
		return;
	    }
	}
	getout(0);
	break;

      case '.':
	CLEAROP;
	if (Redobuffptr != NULL) {
	    stuffReadbuff(Redobuff);

	    stuffReadbuff(ENABLE_REDRAWING_STR);
	    RedrawingDisabled = TRUE;
	} else
	    beep();
	break;

      case 'u':
      case K_UNDO:
	CLEAROP;
	if (UndoInProgress) {
	    p = UndoUndobuff;
	    UndoUndobuff = Undobuff;
	    Undobuff = p;
	    p = UndoUndobuffptr;
	    UndoUndobuffptr = Undobuffptr;
	    Undobuffptr = p;

	    UndoInProgress = FALSE;
	    RedrawingDisabled = FALSE;
	    S_NOT_VALID;
	} else if (Undobuffptr != NULL) {
	    stuffReadbuff(Undobuff);
	    stuffReadbuff("u");
	    UndoInProgress = TRUE;
	    RedrawingDisabled = TRUE;
	} else {
	    beep();
	}
	break;

      default:
	CLEAROP;
	beep();
	break;
    }

    /*
     * If an operation is pending, handle it... 
     */
    if (finish_op) {		/* we just finished an operator */
	if (operator == NOP)	/* ... but it was cancelled */
	    return;

	switch (operator) {

	  case LSHIFT:
	  case RSHIFT:
	    ResetBuffers();

	    n = RowNumber(&startop);
	    AppendPositionToUndobuff(startop.index, n);
	    AppendPositionToUndoUndobuff(startop.index, n);
	    if (Prenum != 0) {
		AppendNumberToRedobuff(Prenum);
		AppendNumberToUndobuff(Prenum);
		AppendNumberToUndoUndobuff(Prenum);
	    }
	    AppendToRedobuff((operator == LSHIFT) ? "<" : ">");
	    AppendToUndobuff((operator == LSHIFT) ? ">" : "<");
	    AppendToUndoUndobuff((operator == LSHIFT) ? "<" : ">");
	    AppendToRedobuff(mkstr(c));
	    if (c == '>')
		AppendToUndobuff("<");
	    else if (c == '<')
		AppendToUndobuff(">");
	    else
		AppendToUndobuff(mkstr(c));
	    AppendToUndoUndobuff(mkstr(c));

	    doshift(operator);
	    break;

	  case DELETE:
	    ResetBuffers();

	    n = RowNumber(&startop);
	    AppendPositionToUndoUndobuff(startop.index, n);

	    if (lt(&startop, Curschar))
		temp_Curschar = startop;
	    else
	        temp_Curschar = *Curschar;
	    n = RowNumber(&temp_Curschar);
	    if (Prenum != 0) {
		AppendNumberToRedobuff(Prenum);
		AppendNumberToUndoUndobuff(Prenum);
	    }
	    AppendToRedobuff("d");
	    AppendToUndoUndobuff("d");
	    AppendToRedobuff(mkstr(c));
	    AppendToUndoUndobuff(mkstr(c));
	    if (nchar != NUL) {
		AppendToRedobuff(mkstr(nchar));
		AppendToUndoUndobuff(mkstr(nchar));
	    }
	    AppendPositionToUndobuff(temp_Curschar.index, n);

	    dodelete(!UndoInProgress, !UndoInProgress, !UndoInProgress);

	    AppendPositionToUndobuff(temp_Curschar.index, n);
	    break;

	  case YANK:
	    ResetBuffers();	/* no redo/undo/(undo of undo) on yank... */
	    if (!doyank())
		msg("yank buffer exceeded");
	    if (!ybcrossline)
		*Curschar = startop;
	    else if (lt(&startop, Curschar))
		*Curschar = startop;
	    break;

	  case CHANGE:
	    ResetBuffers();

	    n = RowNumber(&startop);
	    AppendPositionToUndoUndobuff(startop.index, n);

	    if (lt(&startop, Curschar))
		temp_Curschar = startop;
	    else
	        temp_Curschar = *Curschar;
	    n = RowNumber(&temp_Curschar);
	    if (mtype == MLINE)
		AppendPositionToUndobuff(0, n);
	    else
		AppendPositionToUndobuff(temp_Curschar.index, n);

	    if (Prenum != 0) {
		AppendNumberToRedobuff(Prenum);
		AppendNumberToUndoUndobuff(Prenum);
	    }
	    AppendToRedobuff("c");
	    AppendToUndoUndobuff("c");
	    AppendToRedobuff(mkstr(c));
	    AppendToUndoUndobuff(mkstr(c));
	    if (nchar != NUL) {
		AppendToRedobuff(mkstr(nchar));
		AppendToUndoUndobuff(mkstr(nchar));
	    }
	    dochange();

	    last_command = 'c';
	    break;

	  default:
	    beep();
	}
	operator = NOP;
    }
}

/*
 * tabinout(shift_type, num) 
 *
 * If shift_type == RSHIFT, add a tab to the begining of the next num lines;
 * otherwise delete a tab from the beginning of the next num lines. 
 */
static void
tabinout(int shift_type, int num)
{
    LPtr           *p;

    beginline(FALSE);
    while (num-- > 0) {
	beginline(FALSE);
	if (shift_type == RSHIFT)
	    inschar(TAB);
	else {
	    if (gchar(Curschar) == TAB)
		delchar(TRUE, FALSE);
	}
	if (num > 0) {
	    if ((p = nextline(Curschar)) != NULL)
		*Curschar = *p;
	    else
		break;
	}
    }
}

/*
 * doshift - handle a shift operation 
 */
static void
doshift(int op)
{
    LPtr            top, bot;
    int             nlines;

    top = startop;
    bot = *Curschar;

    if (lt(&bot, &top))
    pswap(top, bot);

    nlines = cntllines(&top, &bot);
    *Curschar = top;
    tabinout(op, nlines);

    /*
     * The cursor position afterward is the prior of the two positions. 
     */
    *Curschar = top;

    /*
     * If we were on the last char of a line that got shifted left, then move
     * left one so we aren't beyond the end of the line 
     */
    if (gchar(Curschar) == NUL && Curschar->index > 0)
	Curschar->index--;

    if (op == RSHIFT)
	oneright();
    else
	oneleft();

    S_NOT_VALID;

    if (nlines > P(P_RP))
	smsg("%d lines %ced", nlines, (op == RSHIFT) ? '>' : '<');
}

/*
 * dodelete - handle a delete operation 
 */
static void
dodelete(bool_t redraw, bool_t setup_for_undo, bool_t try_to_yank)
{
    LPtr            top, bot;
    int             nlines;
    int             n;

    /*
     * Do a yank of whatever we're about to delete. If there's too much stuff
     * to fit in the yank buffer, then get a confirmation before doing the
     * delete. This is crude, but simple. And it avoids doing a delete of
     * something we can't put back if we want. 
     */
    if (try_to_yank) {
	if (!doyank()) {
	    msg("yank buffer exceeded: press <y> to confirm");
	    if (vgetc() != 'y') {
		emsg("delete aborted");
		*Curschar = startop;
		return;
	    }
	}
    }
    top = startop;
    bot = *Curschar;

    if (lt(&bot, &top))
	pswap(top, bot);

    *Curschar = top;
    nlines = cntllines(&top, &bot);

    if (mtype == MLINE) {
	if (operator == CHANGE) {
	    last_command_char = 'a';
	    delline(nlines - 1);
	    Curschar->index = 0;
	    while (delchar(TRUE, FALSE));
	} else {
	    if ((Filetop->linep->next == top.linep) &&
		(bot.linep->next == Fileend->linep))
		last_command_char = 'a';
	    else if (bot.linep->next == Fileend->linep)
		last_command_char = 'o';
	    else
		last_command_char = 'O';
	    if (setup_for_undo)
		AppendToUndobuff(mkstr(last_command_char));
	    delline(nlines);
	}
    } else if (top.linep == bot.linep) {	/* del. within line */
	if (!mincl)
	    dec(&bot);

	if (endofline(&bot))
	    last_command_char = 'a';
	else
	    last_command_char = 'i';
	if (setup_for_undo)
	    AppendToUndobuff(mkstr(last_command_char));
	n = bot.index - top.index + 1;
	while (n--)
	    if (!delchar(TRUE, FALSE))
		break;
    } else {			/* del. between lines */
	if (endofline(&top)) {
	    if (nextline(&top)) {
		if (lineempty(nextline(&top)))
		    last_command_char = 'a';
		else
		    last_command_char = 'i';
	    } else {
		last_command_char = 'a';
	    }
	} else {
	    last_command_char = 'i';
	}
	if (setup_for_undo)
	    AppendToUndobuff(mkstr(last_command_char));

	n = Curschar->index;
	while (Curschar->index >= n)
	    if (!delchar(TRUE, FALSE))
		break;

	top = *Curschar;
	*Curschar = *nextline(Curschar);
	delline(nlines - 2);
	Curschar->index = 0;
	n = bot.index;
	if (!mincl)
	    n--;

	while (n-- >= 0)
	    if (!delchar(TRUE, FALSE))
		break;
	*Curschar = top;
	dojoin(FALSE, FALSE);
    }

    if (mtype == MCHAR && nlines == 1 && redraw && P(P_NU) == FALSE) {
	S_LINE_NOT_VALID;
    } else {
	S_NOT_VALID;
    }

    if (nlines > P(P_RP))
	smsg("%d fewer lines", nlines);

    if (setup_for_undo) {
	AppendToUndobuff(Yankbuff);
	AppendToUndobuff(ESC_STR);
    }
}

/*
 * dochange - handle a change operation 
 */
static void
dochange(void)
{
    LPtr            l;

    if (lt(Curschar, &startop))
	l = *Curschar;
    else
	l = startop;

    dodelete(FALSE, FALSE, !UndoInProgress);

    if ((l.index > Curschar->index) && !lineempty(Curschar))
	inc(Curschar);

    startinsert(FALSE);
}

static          bool_t
doyank(void)
{
    LPtr            top, bot;
    char           *ybend = &Yankbuff[YANKSIZE - 1];
    int             nlines;

    Yankbuffptr = Yankbuff;

    top = startop;
    bot = *Curschar;

    if (lt(&bot, &top))
	pswap(top, bot);

    nlines = cntllines(&top, &bot);

    ybtype = mtype;		/* set the yank buffer type */
    ybcrossline = FALSE;
    if (LINEOF(&top) != LINEOF(&bot))
	ybcrossline = TRUE;

    if (mtype == MLINE) {
	ybcrossline = TRUE;
	top.index = 0;
	bot.index = strlen(bot.linep->s);
	/*
	 * The following statement checks for the special case of yanking a
	 * blank line at the beginning of the file. If not handled right, we
	 * yank an extra char (a newline). 
	 */
	if (dec(&bot) == -1) {
	    *Yankbuff = NUL;
	    Yankbuffptr = NULL;
	    return TRUE;
	}
    } else {
	if (!mincl)
	    if (!equal(&top, &bot))
		dec(&bot);
    }

    for (; ltoreq(&top, &bot); inc(&top)) {
	*Yankbuffptr = (gchar(&top) != NUL) ? gchar(&top) : NL;
	Yankbuffptr++;
	if (Yankbuffptr >= ybend) {
	    *Yankbuffptr = NUL;
	    msg("yank too big for buffer");
	    ybtype = MBAD;
	    return FALSE;
	}
    }

    *Yankbuffptr = NUL;

    if (operator == YANK)
	if (nlines > P(P_RP))
	    smsg("%d lines yanked", nlines);

    return TRUE;
}

static void
doput(int dir)
{
    bool_t          type;

    if (ybtype == MBAD) {
	beep();
	return;
    }
    type = (ybtype == MCHAR);
    if (dir == FORWARD)
	stuffReadbuff(type ? "a" : "o");
    else
	stuffReadbuff(type ? "i" : "O");

    stuffReadbuff(Yankbuff);
    stuffReadbuff(ESC_STR);

    if (ybtype != MCHAR)
	stuffReadbuff("^");
}

static void
startinsert(int startln)
/*    int             startln;	/* if set, insert at start of line */
{
extern void nosuspend(void);
extern void dosuspend(void);

    nosuspend(); /* turn off the suspend character in insert mode */
    *Insstart = *Curschar;
    if (startln) {
	Insstart->index = 0;
    }
    *Insbuff = NUL;
    Insbuffptr = NULL;

    State = INSERT;
    if (P(P_MO)) {
	if (last_command == 'R')
	    msg("Replace Mode");
	else
	    msg("Insert Mode");
    }
}

void
ResetBuffers(void)
{
    if (UndoInProgress)
	return;

    *Redobuff = NUL;
    Redobuffptr = NULL;

    *Undobuff = NUL;
    Undobuffptr = NULL;

    *UndoUndobuff = NUL;
    UndoUndobuffptr = NULL;
}

void
AppendToInsbuff(char *s)
{
    if (UndoInProgress)
	return;

    if (Insbuffptr == NULL) {
	if ((strlen(s) + 1) < INSERT_SIZE) {
	    strcpy(Insbuff, s);
	    Insbuffptr = Insbuff;
	    return;
	}
    } else if ((strlen(Insbuff) + strlen(s) + 1) < INSERT_SIZE) {
	strcat(Insbuff, s);
	return;
    }
    emsg("Couldn't AppendToInsbuff() - clearing Insbuff\n");
    *Insbuff = NUL;
    Insbuffptr = NULL;
}

void
AppendToRedobuff(char *s)
{
    if (UndoInProgress)
	return;

    if (Redobuffptr == (char *) (-2)) {
	return;
    }
    if (Redobuffptr == (char *) (-1)) {
	Redobuffptr = (char *) (-2);
	emsg("Couldn't AppendToRedobuff() - Redobuff corrupt");
	return;
    }
    if (Redobuffptr == NULL) {
	if ((strlen(s) + 1) < REDO_UNDO_SIZE) {
	    strcpy(Redobuff, s);
	    Redobuffptr = Redobuff;
	    return;
	}
    } else if ((strlen(Redobuff) + strlen(s) + 1) < REDO_UNDO_SIZE) {
	strcat(Redobuff, s);
	return;
    }
    emsg("Couldn't AppendToRedobuff() - clearing Redobuff");
    *Redobuff = NUL;
    Redobuffptr = (char *) (-1);
}

void
AppendNumberToRedobuff(int n)
{
    char            buf[32];

    if (UndoInProgress)
	return;

    sprintf(buf, "%d", n);
    AppendToRedobuff(buf);
}

void
AppendToUndobuff(char *s)
{
    if (UndoInProgress)
	return;

    if (Undobuffptr == (char *) (-2)) {
	return;
    }
    if (Undobuffptr == (char *) (-1)) {
	Undobuffptr = (char *) (-2);
	emsg("Couldn't AppendToUndobuff() - Undobuff corrupt");
	return;
    }
    if (Undobuffptr == NULL) {
	if ((strlen(s) + 1) < REDO_UNDO_SIZE) {
	    strcpy(Undobuff, s);
	    Undobuffptr = Undobuff;
	    return;
	}
    } else if ((strlen(Undobuff) + strlen(s) + 1) < REDO_UNDO_SIZE) {
	strcat(Undobuff, s);
	return;
    }
    emsg("Couldn't AppendToUndobuff() - clearing Undobuff");
    *Undobuff = NUL;
    Undobuffptr = (char *) (-1);
}

void
AppendNumberToUndobuff(int n)
{
    char            buf[32];

    if (UndoInProgress)
	return;

    sprintf(buf, "%d", n);
    AppendToUndobuff(buf);
}

void
AppendPositionToUndobuff(int column, int row)
{
    if (UndoInProgress)
	return;

    AppendNumberToUndobuff(row);
    AppendToUndobuff("G");
    AppendNumberToUndobuff(column);
    if (column)
	AppendToUndobuff("l");
}

void
AppendToUndoUndobuff(char *s)
{
    if (UndoInProgress)
	return;

    if (UndoUndobuffptr == (char *) (-2)) {
	return;
    }
    if (UndoUndobuffptr == (char *) (-1)) {
	UndoUndobuffptr = (char *) (-2);
	emsg("Couldn't AppendToUndoUndobuff() - UndoUndobuff corrupt");
	return;
    }
    if (UndoUndobuffptr == NULL) {
	if ((strlen(s) + 1) < REDO_UNDO_SIZE) {
	    strcpy(UndoUndobuff, s);
	    UndoUndobuffptr = Undobuff;
	    return;
	}
    } else if ((strlen(UndoUndobuff) + strlen(s) + 1) < REDO_UNDO_SIZE) {
	strcat(UndoUndobuff, s);
	return;
    }
    emsg("Couldn't AppendToUndoUndobuff() - clearing UndoUndobuff");
    *UndoUndobuff = NUL;
    UndoUndobuffptr = (char *) (-1);
}

void
AppendNumberToUndoUndobuff(int n)
{
    char            buf[32];

    if (UndoInProgress)
	return;

    sprintf(buf, "%d", n);
    AppendToUndoUndobuff(buf);
}

void
AppendPositionToUndoUndobuff(int column, int row)
{
    if (UndoInProgress)
	return;

    AppendNumberToUndoUndobuff(row);
    AppendToUndoUndobuff("G");
    AppendNumberToUndoUndobuff(column);
    if (column)
	AppendToUndoUndobuff("l");
}

static          bool_t
dojoin(bool_t leading_space, bool_t strip_leading_spaces)
{
    int             scol;	/* save cursor column */
    int             currsize;	/* size of the current line */
    int             nextsize;	/* size of the next line */

    if (nextline(Curschar) == NULL)	/* on last line */
	return FALSE;

    nextsize = strlen(Curschar->linep->next->s);
    if (!canincrease(nextsize))
	return FALSE;

    currsize = strlen(Curschar->linep->s);

    while (oneright());		/* to end of line */

    strcat(Curschar->linep->s, Curschar->linep->next->s);

    /*
     * Delete the following line. To do this we move the cursor there
     * briefly, and then move it back. Don't back up if the delete made us
     * the last line. 
     */
    Curschar->linep = Curschar->linep->next;
    scol = Curschar->index;

    if (nextline(Curschar) != NULL) {
	delline(1);
	Curschar->linep = Curschar->linep->prev;
    } else
	delline(1);

    Curschar->index = scol;

    if (currsize)
	oneright();		/* go to first char. of joined line */

    if (nextsize != 0 && strip_leading_spaces) {
	/*
	 * Delete leading white space on the joined line and insert a single
	 * space. 
	 */
	while (gchar(Curschar) == ' ' || gchar(Curschar) == TAB) {
	    delchar(TRUE, TRUE);
	}
	if (leading_space)
	    inschar(' ');
    }
    CHANGED;

    return TRUE;
}

/*
 * linewhite() - returns TRUE if the line consists only of white space
 */

bool_t
linewhite(LPtr *p)
{
    register int    i;
    register char   c;

    i = 1;
    c = p->linep->s[0];
    while (c != NUL) {
	if (c != ' ' && c != '\t')
	    return (FALSE);
	c = p->linep->s[i++];
    }

    return (TRUE);
}
