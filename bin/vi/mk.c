/*
 * STEVIE - Simply Try this Editor for VI Enthusiasts
 *
 * Code Contributions By : Tim Thompson           twitch!tjt
 *                         Tony Andrews           onecom!wldrdg!tony 
 *                         G. R. (Fred) Walter    watmath!grwalter 
 */

#include "stevie.h"

char           *
mkline(int n)
{
    static char     lbuf[9];
    int             i = 6;

    strcpy(lbuf, "        ");

    lbuf[i--] = (char) ((n % 10) + '0');
    n /= 10;
    if (n != 0) {
	lbuf[i--] = (char) ((n % 10) + '0');
	n /= 10;
    }
    if (n != 0) {
	lbuf[i--] = (char) ((n % 10) + '0');
	n /= 10;
    }
    if (n != 0) {
	lbuf[i--] = (char) ((n % 10) + '0');
	n /= 10;
    }
    if (n != 0)
	lbuf[i] = (char) ((n % 10) + '0');

    return lbuf;
}

char           *
mkstr(char c)
{
    static char     s[2];

    s[0] = c;
    s[1] = NUL;

    return s;
}
