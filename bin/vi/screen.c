/*
 * STEVIE - Simply Try this Editor for VI Enthusiasts
 *
 * Code Contributions By : Tim Thompson           twitch!tjt
 *                         Tony Andrews           onecom!wldrdg!tony 
 *                         G. R. (Fred) Walter    watmath!grwalter 
 */

#include "stevie.h"

/*
 * The following variable is set (in cursupdate) to the number of physical
 * lines taken by the line the cursor is on. We use this to avoid extra calls
 * to plines(). The optimized routine updateline() makes sure that the size of
 * the cursor line hasn't changed. If so, lines below the cursor will move up
 * or down and we need to call the routine s_refresh() to examine the
 * entire screen. 
 */
static int      Cline_size;	/* size (in rows) of the cursor line */
static int      Cline_row;	/* starting row of the cursor line */

/*
 * updateline() - like s_refresh() but only for cursor line 
 *
 * This determines whether or not we need to call s_refresh() to examine
 * the entire screen for changes. This occurs if the size of the cursor line
 * (in rows) has changed.
 */
static void
updateline(void)
{
    char           *ptr;
    int             col;
    int             size;
    int             j;

    if (RedrawingDisabled)	/* Is this the correct action ? */
	return;

    ptr = format_line(Curschar->linep->s, &col);

    size = 1 + ((col - 1) / Columns);
    if (Cline_size == size) {
	s_cursor_off();
	windgoto(Cline_row, 0);
	if (P(P_NU)) {
	    /*
	     * This should be done more efficiently. 
	     */
	    outstr(mkline(cntllines(Filemem, Curschar)));
	}
	outstr(ptr);

	j = col;
	col %= Columns;
	if ((col != 0) || (j == 0)) {
#ifdef T_END_L
	    windgoto(Cline_row + size - 1, col);
	    toutstr(T_END_L);
#else
	    for (; col < Columns; col++)
		outchar(' ');
#endif
	}
	s_cursor_on();
    } else {
	s_refresh(VALID_TO_CURSCHAR);
    }
}

void
cursupdate(int type)
{
    register char   c;
    register int    incr;
    register int    i;
    register int    didincr;

    if (MustUpdateBotchar == TRUE)
	Update_Botchar();

    if (NumLineSizes < 0) {
	s_refresh(NOT_VALID);
    } else {
	if (LineNotValid == TRUE)
	    updateline();

	if (type != UPDATE_CURSOR) {
	    s_refresh(type);
	} else if (ValidToCurschar == TRUE) {
	    s_refresh(VALID_TO_CURSCHAR);
	} else if (CheckTopcharAndBotchar == TRUE) {
	    s_refresh(VALID);
	}
    }

    CheckTopcharAndBotchar = FALSE;
    MustUpdateBotchar = FALSE;
    ValidToCurschar = FALSE;
    LineNotValid = FALSE;

    Cursrow = Curscol = Cursvcol = i = 0;
    for (i = 0; i < NumLineSizes; i++) {
	if (LinePointers[i] == Curschar->linep)
	    break;
	Cursrow += LineSizes[i];
    }

    if (P(P_NU))
	Curscol = 8;

    Cline_row = Cursrow;
    Cline_size = LineSizes[i];

    for (i = 0; i <= Curschar->index; i++) {
	c = Curschar->linep->s[i];
	/* A tab gets expanded, depending on the current column */
	if (c == TAB && !P(P_LS))
	    incr = P(P_TS) - (Cursvcol % P(P_TS));
	else
	    incr = chars[c].ch_size;
	Curscol += incr;
	Cursvcol += incr;
	if (Curscol >= Columns) {
	    Curscol -= Columns;
	    Cursrow++;
	    didincr = TRUE;
	} else
	    didincr = FALSE;
    }
    if (didincr)
	Cursrow--;

    if (c == TAB && State == NORMAL && !P(P_LS)) {
	Curscol--;
	Cursvcol--;
    } else {
	Curscol -= incr;
	Cursvcol -= incr;
    }
    if (Curscol < 0)
	Curscol += Columns;

    if (set_want_col) {
	Curswant = Cursvcol;
	set_want_col = FALSE;
    }
}
