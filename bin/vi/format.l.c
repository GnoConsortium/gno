/*
 * format_line() 
 *
 * Return a pointer to a string buffer containing a formated screen line.
 *
 * By G. R. (Fred) Walter    watmath!grwalter 
 */

#include "stevie.h"

char           *tab_expand = "                ";

char           *
format_line(char *ptr, int *len)
{
    register char  *dest;
    register char   c;
    register int    col;
    char           *p_extra;
    int             n_extra;
    int             coff;	/* column offset */

    dest = IObuff;
    col = 0;

    coff = P(P_NU) ? 8 : 0;

    n_extra = 0;
    p_extra = NULL;

    for (;;) {
	if (n_extra > 0) {
	    c = *p_extra++;
	    n_extra--;
	} else {
	    c = *ptr++;
	    while (c >= 32 && c < 127) {
		*dest++ = c;
		col++;
		if (col >= IOSIZE)
		    goto DONE_FORMAT_LINE;
		c = *ptr++;
	    }
	    if (!P(P_LS)) {
		if (c == TAB) {
		    /* tab amount depends on current column */
		    p_extra = tab_expand;
		    n_extra = (P(P_TS) - 1) - (col - coff) % P(P_TS);
		    c = ' ';
		    goto I_HATE_GOTOS;
		} else if (c == NUL) {
		    break;
		}
	    } else if (c == NUL) {
		*dest++ = '$';
		col++;
		break;
	    }
	    if ((n_extra = chars[c].ch_size - 1) > 0) {
		p_extra = chars[c].ch_str;
		c = *p_extra++;
	    }
	}
I_HATE_GOTOS:
	*dest++ = c;
	col++;
	if (col >= IOSIZE)
	    break;
    }
DONE_FORMAT_LINE:
    if (col >= IOSIZE) {
	dest--;
	col--;
    }
    *dest = NUL;

    if (len != NULL)
	*len = col + coff;

    return (IObuff);
}
