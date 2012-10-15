/*
 * Implementation by Devin Reade
 *
 * $Id: case.c,v 1.2 1997/09/21 06:23:07 gdr Exp $
 *
 * This file is formatted with tab stops every 8 columns.
 */

#include <string.h>
#include <ctype.h>

#undef TOLOWER
#define TOLOWER(c) isupper(c) ? _tolower(c) : c

int
strcasecmp (const char *s1, const char *s2) {
	unsigned int c1, c2;
	
	for (;;) {
		c1 = TOLOWER(*s1);
		c2 = TOLOWER(*s2);
		if (c1 == '\0' && c2 == '\0') {
			return 0;
		} else if (c1 == c2) {
			s1++; s2++;
		} else {
			/* don't do subtraction -- see man page */
			return (c1 > c2) ? 1 : -1;
		}
	}
}

int
strncasecmp (const char *s1, const char *s2, size_t n) {
	unsigned int c1, c2;
	size_t i;

	for (i=0; i<n; i++) {
		c1 = TOLOWER(*s1);
		c2 = TOLOWER(*s2);
		if (c1 == '\0' && c2 == '\0') {
			return 0;
		} else if (c1 == c2) {
			s1++; s2++;
		} else {
			/* don't do subtraction -- see man page */
			return (c1 > c2) ? 1 : -1;
		}
	}
	return 0;
}                                                     

short
stricmp (const char *s1, const char *s2) {
	return strcasecmp(s1, s2);
}

short
strincmp (const char *s1, const char *s2, unsigned n) {
	return strncasecmp(s1, s2, n);
}
