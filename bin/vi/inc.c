/*
 * STEVIE - Simply Try this Editor for VI Enthusiasts
 *
 * Code Contributions By : Tim Thompson           twitch!tjt
 *                         Tony Andrews           onecom!wldrdg!tony 
 *                         G. R. (Fred) Walter    watmath!grwalter 
 */

#include "stevie.h"

/*
 * inc(p) 
 *
 * Increment the line pointer 'p' crossing line boundaries as necessary. Return
 * 1 when crossing a line, -1 when at end of file, 0 otherwise. 
 */
int
inc(LPtr *lp)
{
    register char  *p = &(lp->linep->s[lp->index]);

    if (*p != NUL) {		/* still within line */
	lp->index++;
	return ((p[1] != NUL) ? 0 : 1);
    }
    if (lp->linep->next != Fileend->linep) {	/* there is a next line */
	lp->index = 0;
	lp->linep = lp->linep->next;
	return 1;
    }
    return -1;
}
