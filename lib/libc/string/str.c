/*
 * str.c
 *
 * Various string routines not available in OrcaLib.  For an explanation
 * of these functions, see the appropriate man page.
 *
 * $Id: str.c,v 1.2 1997/09/21 06:23:08 gdr Exp $
 *
 * This file is formatted with tabs in every 8 columns.
 */

#include <stdlib.h>
#include <string.h>

/*
 * index
 */

char *
index(const char *a, int b)
{
	return strchr(a,b);
}

/*
 * rindex
 */

char *
rindex(const char *a, int b)
{
	return strrchr(a,b);
}

/*
 * strsep
 *
 * Get next token from string *stringp, where tokens are nonempty
 * strings separated by characters from delim.  
 *
 * Writes NULs into the string at *stringp to end tokens.
 * delim need not remain constant from call to call.
 * On return, *stringp points past the last NUL written (if there might
 * be further tokens), or is NULL (if there are definitely no more tokens).
 *
 * If *stringp is NULL, strtoken returns NULL.
 */

char *
strsep(register char **stringp, register const char *delim)
{
	register char *s;
	register const char *spanp;
	register int c, sc;
	char *tok;

	if ((s = *stringp) == NULL) {
		return (NULL);
	}
	for (tok = s;;) {
		c = *s++;
		spanp = delim;
		do {
			if ((sc = *spanp++) == c) {
				if (c == 0) {
					s = NULL;
				} else {
					s[-1] = 0;
				}
				*stringp = s;
				return (tok);
			}
		} while (sc != 0);
	}
	/* NOTREACHED */
}

/*
 * strdup
 */

char *
strdup(const char *str)
{
	size_t len;
	char *buf;

	len = strlen(str) + 1;
	if ((buf = malloc(len)) == NULL) {
		return NULL;
	}
	strcpy(buf, str);
	return buf;
}
