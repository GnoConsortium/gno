/*
 * s_io() - routines that do screen I/O or effect what we think is
 *          on the screen.
 *
 * By G. R. (Fred) Walter    watmath!grwalter 
 */

#include "stevie.h"

segment "s_io";

/*
 * s_cursor_off() - turn off the cursor (if it is appropriate)
 */

void
s_cursor_off(void)
{
#ifdef AMIGA
    if (!Aux_Device)
	outstr(T_CI);
#else
    toutstr(T_CI);
#endif
}

/*
 * s_cursor_on() - turn on the cursor (if it is appropriate)
 */

void
s_cursor_on(void)
{
#ifdef AMIGA
    if (!Aux_Device)
	outstr(T_CV);
#else
    toutstr(T_CV);
#endif
}

/*
 * screen_ins(row, nlines, total_rows) - insert 'nlines' lines at 'row' 
 *
 * NOTE: this routine assumes it is called with valid arguments.
 */

static void
screen_ins(int row, int nlines, int total_rows)
{
    if (nlines < 1 || (row + nlines) > total_rows)
	return;

#ifndef T_IL_B
    {
	int             i;

	for (i = 0; i < nlines; i++) {
	    windgoto(row, 0);
	    if (T_IL != NULL)
	    	toutstr(T_IL);
	    else InsertLine();
	}
    }
#else
    windgoto(row, 0);
    toutstr(T_IL);

    if (nlines >= 10)
	outchar((char) (nlines / 10 + '0'));
    outchar((char) (nlines % 10 + '0'));
    toutstr(T_IL_B);
#endif

    /* delete any garbage that may have been shifted to the status line */
    windgoto(total_rows - 1, 0);
    toutstr(T_EL);
}

/*
 * screen_del(row, nlines, total_rows) - delete 'nlines' lines at 'row' 
 *
 * NOTE: this routine assumes it is called with valid arguments.
 */

static void
screen_del(int row, int nlines, int total_rows)
{
    if (nlines < 1 || (row + nlines) > total_rows)
	return;

    /* delete any garbage that may have been on the status line */
    windgoto(total_rows - 1, 0);
    toutstr(T_EL);

#ifndef T_DL_B
    {
	int             i;

	for (i = 0; i < nlines; i++) {
	    windgoto(row, 0);
	    if (T_DL != NULL)
	        toutstr(T_DL);	/* delete a line */
	    else DeleteLine();
	}
    }
#else
    windgoto(row, 0);
    toutstr(T_DL);
    if (nlines >= 10)
	outchar((char) (nlines / 10 + '0'));
    outchar((char) (nlines % 10 + '0'));
    toutstr(T_DL_B);
#endif
}

/*
 * screen_refresh()
 *
 * Based on the current value of Topchar, refresh the contents of the screen
 * and update Botchar.
 */

static void
screen_refresh(int type)
{
    char           *ptr;
    int             total_rows;
    int             row;
    int             col;
    LINE           *memp;
    LPtr            start;
    bool_t          off_top;
    bool_t          done;	/* if TRUE, we hit the end of the file */
    bool_t          didline;	/* if TRUE, we finished the last line */
    int             lno;	/* number of the line we're doing */
    int             idx;
    int             i;
    int             j;

    if (NumLineSizes <= 0)
	type = NOT_VALID;

    if (!RedrawingDisabled)
	s_cursor_off();

    off_top = FALSE;
    idx = 0;
    row = 0;
    total_rows = Rows - 1;
    memp = Topchar->linep;

    if ((type == VALID) || (type == VALID_TO_CURSCHAR)) {
	j = -1;
	for (i = 0; i < NumLineSizes; i++) {
	    if (LinePointers[i] == memp) {
		j = i;
		break;
	    }
	    row += LineSizes[i];
	}
	if (j == -1) {
	    /* Are we off the top of the screen by one line ? */
	    if (memp->next == LinePointers[0]) {
		i = plines(Topchar->linep->s);
		if (i < (total_rows)) {
		    off_top = TRUE;
		    for (idx = NumLineSizes; idx > 0; idx--) {
			LinePointers[idx] = LinePointers[idx - 1];
			LineSizes[idx] = LineSizes[idx - 1];
		    }
		    LineSizes[idx] = (char) i;
		    if (!RedrawingDisabled)
			screen_ins(0, i, Rows);
		}
	    }
	    row = 0;
	} else if (j == 0 && type == VALID) {
	    if (!RedrawingDisabled)
		s_cursor_on();
	    return;
	} else {
	    if (!RedrawingDisabled)
		screen_del(0, row, Rows);
	    row = 0;
	    for (;;) {
		LineSizes[idx] = LineSizes[j];
		LinePointers[idx] = LinePointers[j];

		if (type == VALID_TO_CURSCHAR) {
		    if (LinePointers[idx] == Curschar->linep) {
			memp = LinePointers[idx];
			break;
		    }
		}
		j++;
		if (j >= NumLineSizes) {
		    memp = LinePointers[idx];
		    if (memp->next != Fileend->linep) {
			row += LineSizes[idx];
			idx++;
			memp = memp->next;
		    }
		    break;
		}
		row += LineSizes[idx];
		idx++;
	    }
	}
    }
    if (P(P_NU)) {
	start.linep = memp;
	lno = cntllines(Filemem, &start);
    }
    didline = TRUE;
    done = FALSE;

    for (;;) {
	ptr = format_line(memp->s, &col);
	i = 1 + ((col - 1) / Columns);
	if ((row + i) <= total_rows) {
	    LinePointers[idx] = memp;
	    LineSizes[idx++] = (char) i;
	    if (!RedrawingDisabled) {
		windgoto(row, 0);

		if (P(P_NU))
		    outstr(mkline(lno++));
		outstr(ptr);

		j = col;
		col %= Columns;
		if ((col != 0) || (j == 0)) {
#ifdef T_END_L
		    windgoto(row + i - 1, col);
		    toutstr(T_END_L);
#else
		    for (; col < Columns; col++)
			outchar(' ');
#endif
		}
	    }
	    row += i;
	    if (memp->next != Fileend->linep) {
		memp = memp->next;
	    } else {
		done = TRUE;
		break;
	    }
	    if (off_top)
		break;
	} else {
	    didline = FALSE;
	    break;
	}
    }

    /* Do we have to do 'off the top of the screen' processing ? */
    if (off_top && !done) {
	row = 0;
	for (idx = 0; idx <= NumLineSizes && row < total_rows; idx++) {
	    row += LineSizes[idx];
	}

	idx--;

	if (row < total_rows) {
	    if (LinePointers[idx]->next == Fileend->linep)
		done = TRUE;
	    idx++;
	} else if (row > total_rows) {
	    row -= LineSizes[idx];
	    didline = FALSE;
	    memp = LinePointers[idx];
	} else {
	    didline = TRUE;
	    memp = LinePointers[idx]->next;
	    idx++;
	}
    }
    NumLineSizes = idx;

    if (done && didline) {
	ptr = "~\n";
    } else {
	ptr = "@\n";
    }

    if (!RedrawingDisabled) {
	if (row < total_rows) {
	    /* Clear the rest of the screen. */
#ifdef T_END_D
	    windgoto(row, 0);
	    toutstr(T_END_D);
#else
	    screen_del(row, total_rows - row, Rows);
	    windgoto(row, 0);
#endif
	}
	/* put '@'s or '~'s on the remaining rows */
	for (; row < total_rows; row++)
	    outstr(ptr);

	s_cursor_on();
    }
    if (done)
	*Botchar = *Fileend;	/* we hit the end of the file */
    else
	Botchar->linep = memp;
}

/*
 * s_refresh()
 *
 * Based on the current value of Curschar, (if necessary) update Topchar and
 * Botchar and refresh the screen contensts.
 */

void
s_refresh(int type)
{
    LPtr           *p;
    LPtr           *pp;
    int             i;
    int             nlines;
    int             refreshed;

    refreshed = FALSE;

    if (bufempty()) {		/* special case - file is empty */
	*Topchar = *Filemem;
	*Curschar = *Filemem;
	screen_refresh(NOT_VALID);
	return;
    }
    if (NumLineSizes < 0) {
	type = NOT_VALID;
    }
    if (type != VALID) {
	screen_refresh(type);
	refreshed = TRUE;
	type = VALID;
    }
    if (LINEOF(Curschar) < LINEOF(Topchar)) {
	nlines = cntllines(Curschar, Topchar);
	/*
	 * if the cursor is above the top of the screen, put it at the top of
	 * the screen.. 
	 */
	*Topchar = *Curschar;
	Topchar->index = 0;
	/*
	 * ... and, if we weren't very close to begin with, we scroll so that
	 * the line is close to the middle. 
	 */
	if (nlines > Rows / 3) {
	    p = Topchar;
	    for (i = 0; i < Rows / 3; i += plines(p->linep->s)) {
		pp = prevline(p);
		if (pp == NULL)
		    break;
		p = pp;
	    }
	    *Topchar = *p;
	}
	screen_refresh(VALID);
    } else if (LINEOF(Curschar) >= LINEOF(Botchar)) {
	nlines = cntllines(Botchar, Curschar);
	/*
	 * If the cursor is off the bottom of the screen, put it at the top
	 * of the screen.. ... and back up 
	 */
	if (nlines > Rows / 3) {
	    p = Curschar;
	    for (i = 0; i < (2 * Rows) / 3; i += plines(p->linep->s)) {
		pp = prevline(p);
		if (pp == NULL)
		    break;
		p = pp;
	    }
	    *Topchar = *p;
	} else {
	    scrollup(nlines);
	}
	screen_refresh(VALID);
    } else if (refreshed == FALSE) {
	screen_refresh(type);
    }
    /* Check if we are below Botchar (this can occur). */
    if (LINEOF(Curschar) == LINEOF(Botchar)) {
	pp = nextline(Topchar);
	if (pp != NULL) {
	    Topchar->linep = pp->linep;
	    screen_refresh(VALID);
	}
    } else if (LINEOF(Curschar) >= LINEOF(Botchar)) {
	nlines = cntllines(Botchar, Curschar);
	/*
	 * If the cursor is off the bottom of the screen, put it at the top
	 * of the screen.. ... and back up 
	 */
	if (nlines > Rows / 3) {
	    p = Curschar;
	    for (i = 0; i < (2 * Rows) / 3; i += plines(p->linep->s)) {
		pp = prevline(p);
		if (pp == NULL)
		    break;
		p = pp;
	    }
	    *Topchar = *p;
	} else {
	    scrollup(nlines);
	}
	screen_refresh(VALID);
    }
}

/*
 * s_clear() - clear the screen and mark the stored information as invalid.
 */
void
s_clear(void)
{
    toutstr(T_ED);		/* clear the display */
    S_NOT_VALID;
}

/*
 * Update_Botchar()
 *
 * Based on the current value of Topchar update Botchar.
 */

void
Update_Botchar(void)
{
    int             row;
    LINE           *memp;
    int             total_rows;
    int             i;

    row = 0;
    total_rows = Rows - 1;
    memp = Topchar->linep;

    for (;;) {
	i = plines(memp->s);
	if ((row + i) <= total_rows) {
	    row += i;
	    memp = memp->next;
	    if (memp == Fileend->linep)
		break;
	} else {
	    break;
	}
    }
    Botchar->linep = memp;

    MustUpdateBotchar = FALSE;
}

#ifdef DONTINCLUDEANYMORE
/*
 * NotValidFromCurschar()
 *
 * Mark the lines in NumLinePointers and NumLineSizes from Curschar on as
 * not valid.
 */

void
NotValidFromCurschar(void)
{
    register int    idx;
    register unsigned long num;

    S_VALID_TO_CURSCHAR;

    num = LINEOF(Curschar);
    for (idx = 0; idx < NumLineSizes; idx++) {
	if (LinePointers[idx]->num >= num)
	    break;
    }
    NumLineSizes = idx;
}
#endif
