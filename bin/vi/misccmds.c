/*
 * STEVIE - Simply Try this Editor for VI Enthusiasts
 *
 * Code Contributions By : Tim Thompson           twitch!tjt
 *                         Tony Andrews           onecom!wldrdg!tony 
 *                         G. R. (Fred) Walter    watmath!grwalter 
 */

#ifdef __ORCAC__
segment "seg2";
#endif

#include "stevie.h"

extern int      did_ai;

/*
 * OpenForward 
 *
 * Add a blank line below the current line. 
 */

bool_t
OpenForward(int can_ai)
{
    LINE           *l;
    LPtr           *next;
    char           *s;		/* string to be moved to new line, if any */
    int             newindex = 0;	/* index of the cursor on the new
					 * line */

    /*
     * If we're in insert mode, we need to move the remainder of the current
     * line onto the new line. Otherwise the new line is left blank. 
     */
    if (State == INSERT)
	s = &Curschar->linep->s[Curschar->index];
    else
	s = "";

    if ((next = nextline(Curschar)) == NULL)	/* open on last line */
	next = Fileend;

    /*
     * By asking for as much space as the prior line had we make sure that
     * we'll have enough space for any auto-indenting. 
     */
    l = newline(strlen(Curschar->linep->s) + SLOP);
    if (l == NULL) {
	emsg("out of memory");
	beep();
	sleep(2);
	return (FALSE);
    }
    if (can_ai && P(P_AI)) {
	char           *p;

	/*
	 * Copy prior line, and truncate after white space 
	 */
	strcpy(l->s, Curschar->linep->s);

	for (p = l->s; *p == ' ' || *p == TAB; p++);
	*p = NUL;
	newindex = p - l->s;
	AppendToInsbuff(l->s);
	if (*s != NUL)
	    strcat(l->s, s);

	/*
	 * If we just did an auto-indent, then we didn't type anything on the
	 * prior line, and it should be truncated. 
	 */
	if (did_ai)
	    Curschar->linep->s[0] = NUL;

	did_ai = TRUE;
    } else if (*s != NUL) {
	strcpy(l->s, s);	/* copy string to new line */
    }
    if (State == INSERT)	/* truncate current line at cursor */
	*s = NUL;

    Curschar->linep->next = l;	/* link neighbors to new line */
    next->linep->prev = l;

    l->prev = Curschar->linep;	/* link new line to neighbors */
    l->next = next->linep;

    if (next == Fileend) {	/* new line at end */
	l->num = Curschar->linep->num + LINEINC;
    } else if ((l->prev->num) + 1 == l->next->num) {	/* no gap, renumber */
	renum();
    } else {			/* stick it in the middle */
	long            lnum;

	lnum = (l->prev->num + l->next->num) / 2;
	l->num = lnum;
    }

    *Curschar = *nextline(Curschar);	/* cursor moves down */
    Curschar->index = newindex;

    S_NOT_VALID;
    CHANGED;

    return (TRUE);
}

/*
 * OpenBackward 
 *
 * Add a blank line above the current line. 
 */

bool_t
OpenBackward(int can_ai)
{
    LINE           *l;
    LINE           *prev;
    int             newindex = 0;	/* index of the cursor on the new
					 * line */

    prev = Curschar->linep->prev;

    l = newline(strlen(Curschar->linep->s) + SLOP);
    if (l == NULL) {
	emsg("out of memory");
	beep();
	sleep(2);
	return (FALSE);
    }
    Curschar->linep->prev = l;	/* link neighbors to new line */
    prev->next = l;

    l->next = Curschar->linep;	/* link new line to neighbors */
    l->prev = prev;

    if (can_ai && P(P_AI)) {
	char           *p;

	/*
	 * Copy current line, and truncate after white space 
	 */
	strcpy(l->s, Curschar->linep->s);

	for (p = l->s; *p == ' ' || *p == TAB; p++);
	*p = NUL;
	newindex = p - l->s;
	AppendToInsbuff(l->s);

	did_ai = TRUE;
    }
    Curschar->linep = Curschar->linep->prev;
    Curschar->index = newindex;

    if (prev == Filetop->linep) {	/* new start of file */
	Filemem->linep = l;
	renum();
    } else if ((l->prev->num) + 1 == l->next->num) {	/* no gap, renumber */
	renum();
    } else {			/* stick it in the middle */
	long            lnum;

	lnum = (l->prev->num + l->next->num) / 2;
	l->num = lnum;
    }

    S_NOT_VALID;
    CHANGED;

    return (TRUE);
}

int
cntllines(LPtr *pbegin, LPtr *pend)
{
    register LINE  *lp;
    register int    lnum = 1;

    for (lp = pbegin->linep; lp != pend->linep; lp = lp->next)
	lnum++;

    return (lnum);
}

/*
 * plines(s) - return the number of physical screen lines taken by the
 *             line pointed to by 's'
 */

int
plines(char *s)
{
    register int    col = 0;

    if (*s == NUL)		/* empty line */
	return 1;

    for (; *s != NUL; s++) {
	if (*s == TAB && !P(P_LS))
	    col += P(P_TS) - (col % P(P_TS));
	else
	    col += chars[*s].ch_size;
    }

    /*
     * If list mode is on, then the '$' at the end of the line takes up one
     * extra column. 
     */
    if (P(P_LS))
	col += 1;

    /*
     * If 'number' mode is on, add another 8. 
     */
    if (P(P_NU))
	col += 8;

    return ((col + (Columns - 1)) / Columns);
}

void
fileinfo(void)
{
    long            l1, l2;
    char            buf[MAX_COLUMNS + 1];

    l1 = cntllines(Filemem, Curschar);
    l2 = cntllines(Filemem, Fileend) - 1;
    sprintf(buf, "\"%s\"%s line %ld of %ld -- %ld %% --",
	    (Filename != NULL) ? Filename : "No File",
	    Changed ? " [Modified]" : "",
	    l1, l2, (l1 * 100) / l2);
    msg(buf);
}

/*
 * gotoline(n) - return a pointer to line 'n' 
 *
 * Returns a pointer to the last line of the file if n is zero, or beyond the
 * end of the file. 
 */
LPtr           *
gotoline(int n)
{
    static LPtr     l;

    l.index = 0;

    if (n == 0)
	l = *prevline(Fileend);
    else {
	LPtr           *p;

	for (l = *Filemem; --n > 0; l = *p)
	    if ((p = nextline(&l)) == NULL)
		break;
    }
    return &l;
}

void
inschar(char c)
{
    register char  *p;
    register char  *pend;

    if (last_command == 'R' && (gchar(Curschar) != NUL)) {
	pchar(Curschar, c);
    } else {
	/* make room for the new char. */
	if (!canincrease(1))
	    return;

	p = &Curschar->linep->s[strlen(Curschar->linep->s) + 1];
	pend = &Curschar->linep->s[Curschar->index];

	for (; p > pend; p--)
	    *p = *(p - 1);

	*p = c;
    }

    if (RedrawingDisabled) {
	Curschar->index++;
	return;
    }
    /*
     * If we're in insert mode and showmatch mode is set, then check for
     * right parens and braces. If there isn't a match, then beep. If there
     * is a match AND it's on the screen, then flash to it briefly. If it
     * isn't on the screen, don't do anything. 
     */
    if (P(P_SM) && State == INSERT && (c == ')' || c == '}' || c == ']')) {
	LPtr           *lpos, csave;

	if ((lpos = showmatch()) == NULL)	/* no match, so beep */
	    beep();
	else if (LINEOF(lpos) >= LINEOF(Topchar)) {
	    /* show the new char first */
	    s_refresh(VALID_TO_CURSCHAR);
	    csave = *Curschar;
	    *Curschar = *lpos;	/* move to matching char */
	    cursupdate(UPDATE_CURSOR);
	    windgoto(Cursrow, Curscol);
	    delay();		/* brief pause */
	    *Curschar = csave;	/* restore cursor position */
	    cursupdate(UPDATE_ALL);
	}
    }
    inc(Curschar);

    CHANGED;
}

void
insstr(char *s)
{
    register char  *p;
    register char  *pend;
    register int    n = strlen(s);

    /* Move everything in the file over to make */
    /* room for the new string. */
    if (!canincrease(n))
	return;

    p = &Curschar->linep->s[strlen(Curschar->linep->s) + n];
    pend = &Curschar->linep->s[Curschar->index];

    for (; p > pend; p--)
	*p = *(p - n);

    for (; n > 0; n--) {
	*p++ = *s++;
	Curschar->index++;
    }
    CHANGED;
}

bool_t
delchar(bool_t fixpos, bool_t undo)
/*    bool_t          fixpos;	/* if TRUE fix the cursor position when done */
/*    bool_t          undo;	/* if TRUE put char deleted into Undo buffer */
{
    int             i;

    /* Check for degenerate case; there's nothing in the file. */
    if (bufempty())
	return FALSE;

    if (lineempty(Curschar))	/* can't do anything */
	return FALSE;

    if (undo)
	AppendToUndobuff(mkstr(gchar(Curschar)));

    /* Delete the char. at Curschar by shifting everything in the line down. */
    for (i = Curschar->index + 1; i < Curschar->linep->size; i++)
	Curschar->linep->s[i - 1] = Curschar->linep->s[i];

    /*
     * If we just took off the last character of a non-blank line, we don't
     * want to end up positioned at the newline. 
     */
    if (fixpos) {
	if (gchar(Curschar) == NUL && Curschar->index > 0 && State != INSERT)
	    Curschar->index--;
    }
    CHANGED;
    return TRUE;
}

void
delline(int nlines)
{
    register LINE  *p;
    register LINE  *q;

    while (nlines-- > 0) {

	if (bufempty())		/* nothing to delete */
	    break;

	if (buf1line()) {	/* just clear the line */
	    Curschar->linep->s[0] = NUL;
	    Curschar->index = 0;
	    break;
	}
	p = Curschar->linep->prev;
	q = Curschar->linep->next;

	if (p == Filetop->linep) {	/* first line of file so... */
	    Filemem->linep = q;	/* adjust start of file */
	    Topchar->linep = q;	/* and screen */
	}
	p->next = q;
	q->prev = p;

	clrmark(Curschar->linep);	/* clear marks for the line */

	/*
	 * If deleting the top line on the screen, adjust Topchar 
	 */
	if (Topchar->linep == Curschar->linep)
	    Topchar->linep = q;

	free(Curschar->linep->s);
	free((char *) (Curschar->linep));

	Curschar->linep = q;
	Curschar->index = 0;	/* is this right? */

	S_NOT_VALID;
	CHANGED;

	/* If we delete the last line in the file, back up */
	if (Curschar->linep == Fileend->linep) {
	    Curschar->linep = Curschar->linep->prev;
	    /* and don't try to delete any more lines */
	    break;
	}
    }
}
