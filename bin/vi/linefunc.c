/*
 * STEVIE - Simply Try this Editor for VI Enthusiasts
 *
 * Code Contributions By : Tim Thompson           twitch!tjt
 *                         Tony Andrews           onecom!wldrdg!tony 
 *                         G. R. (Fred) Walter    watmath!grwalter 
 */

#include "stevie.h"

/*
 * nextline(curr) 
 *
 * Return a pointer to the beginning of the next line after the one referenced
 * by 'curr'. Return NULL if there is no next line (at EOF). 
 */

LPtr           *
nextline(LPtr *curr)
{
    static LPtr     next;

    if (curr != NULL) {
	if (curr->linep->next != Fileend->linep) {
	    next.index = 0;
	    next.linep = curr->linep->next;
	    return &next;
	}
    }
    return (LPtr *) NULL;
}

/*
 * prevline(curr) 
 *
 * Return a pointer to the beginning of the line before the one referenced by
 * 'curr'. Return NULL if there is no prior line. 
 */

LPtr           *
prevline(LPtr *curr)
{
    static LPtr     prev;

    if (curr != NULL) {
	if (curr->linep->prev != Filetop->linep) {
	    prev.index = 0;
	    prev.linep = curr->linep->prev;
	    return &prev;
	}
    }
    return (LPtr *) NULL;
}

/*
 * coladvance(p,col) 
 *
 * Try to advance to the specified column, starting at p. 
 */

void
coladvance(LPtr *p, int want_col)
{
    register char   c;
    register int    col;
    register int    incr;

    if (gchar(p) != NUL) {	/* already at the end of line */
	for (col = 0; want_col > 0;) {
	    c = gchar(p);
	    if (c == TAB && !P(P_LS))
		incr = (P(P_TS) - col % P(P_TS));
	    else
		incr = chars[c].ch_size;
	    want_col -= incr;
	    col += incr;

	    /* Don't go past the end of the file or the line. */
	    if (inc(p)) {
		dec(p);
		break;
	    }
	}
    }
}
