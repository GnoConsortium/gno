/*
 * $Id: escape.h,v 1.1 1997/03/14 06:22:27 gdr Exp $
 *
 * FINDREG -- macro version of findreg().
 *		<name> is the register name to find
 *		<result> is the returned index (-1 iff <name> was not found)
 *		<scratch_i> is a scratch integer variable
 *		<scratch_p> is a scratch char * variable
 */

#define FINDREG(name,result,scratch_p) \
{ \
    for (result = 0; result < MAXREGS; result++) { \
	scratch_p = rg[result].rname; \
	if (*scratch_p == *name && *(scratch_p + 1) == *(name + 1)) { \
	    break; \
	} \
    } \
    if (result >= MAXREGS) { \
	result = -1; \
    } \
}

void expesc (char *src, char *dest, size_t len);
void fontchange (char fnt, char *s);
