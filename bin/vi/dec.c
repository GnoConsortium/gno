/*
 * STEVIE - Simply Try this Editor for VI Enthusiasts
 *
 * Code Contributions By : Tim Thompson           twitch!tjt
 *                         Tony Andrews           onecom!wldrdg!tony 
 *                         G. R. (Fred) Walter    watmath!grwalter 
 */

#include "stevie.h"

/*
 * dec(p) 
 *
 * Decrement the line pointer 'p' crossing line boundaries as necessary. Return
 * 1 when crossing a line, -1 when at start of file, 0 otherwise. 
 */
int
dec(LPtr *lp)
{
    if (lp->index > 0) {	/* still within line */
	lp->index--;
	return 0;
    }
    if (lp->linep->prev != Filetop->linep) {	/* there is a prior line */
	lp->linep = lp->linep->prev;
	lp->index = strlen(lp->linep->s);
	return 1;
    }
    lp->index = 0;		/* stick at first char */
    return -1;			/* at start of file */
}
