/*
 * Apple IIgs specific string handling routines.
 *
 * The routines GIinit, GOinit, GIchange, GOchange, the macros
 * GIfree and GOfree (in <gsos.h>), and typdefs for the GSString
 * and ResultBuf pointers (in <types.h>) come from Soenke Behrens'
 * GSString library.
 *
 * __C2GSMALLOC, __GS2CMALLOC, and __GS2C were rewritten from spec
 * from the GNO v2.0.4 libc(3) man page (with a change of argument and
 * result types) by Devin Reade.  __C2GS was written by Devin Reade.
 *
 * $Id: gsstring.c,v 1.1 1997/02/28 05:12:47 gdr Exp $
 */

#ifdef __ORCAC__
segment "libc_gno__";
#endif

#pragma optimize 0	/* 79 seems to work */
#pragma debug 0
#pragma memorymodel 0

#include <machine/limits.h>
#include <types.h>
#include <gno/gno.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

/*
 * Initialize a GSString structure by allocating memory for a string
 * of length 'length'. The caller has to check for errors, if any
 * occured, the returned ptr will be NULL.
 *
 * gdr:
 * - Add space for an extra NULL byte in the text field.
 * - Allow the user to specify a C string initializer.
 */

GSStringPtr
GIinit (word length, const char *str)
{
	return GIchange(NULL, length, str);
}

/*
 * Change the length of a GSString. Again, the caller has to check for errors,
 * the returned ptr is NULL if any occured.
 *
 * gdr:
 * - Add space for an extra NULL byte in the text field.
 * - Allow the user to specify a C string initializer.
 */

GSStringPtr
GIchange (GSStringPtr original, word length, const char *str)
{
	/* this depends on a well-behaved realloc when given the NULL ptr */
	original = realloc (original, sizeof(word) + length + 1);
	if (original == NULL) {
		return NULL;
	}
	original->length = length;
	if (str != NULL) {
		/* can't walk off end of str, so don't use memcpy */
		strncpy(original->text, str, length);
	} else {
		original->text[0] = '\0';
	}
	original->text[length] = '\0';
	return original;
}

/*
 * Initialize a ResultBuf structure by allocating memory for a string
 * of length 'length'. The caller has to check for errors, if any
 * occured, the returned ptr will be NULL.
 */

ResultBufPtr
GOinit (unsigned int length, const char *str)
{
	return GOchange(NULL, length, str);
}

/*
 * Change the length of the ResultBuf. Again, the caller has to check for
 * errors, the function returns NULL if any occured.
 *
 * gdr:
 * - Add space for an extra NULL byte in the text field.
 * - Allow the user to specify a C string initializer.
 */

ResultBufPtr
GOchange (ResultBufPtr original, unsigned int length, const char *str)
{
	size_t len, orglen;
	ResultBufPtr result;
	
	/* this depends on a well-behaved realloc when given the NULL ptr */
	len = 2 * sizeof(word) + length;
	orglen = (original) ? original->bufString.length : 0;
	result = realloc (original, len + 1);
	if (result == NULL) {
		return NULL;
	}
	result->bufSize = len;
	if (str != NULL) {
		/* can't walk off end of str, so don't use memcpy */
		len = strlen(str);
		len = MIN(len, length);
		result->bufString.length = len;
		strncpy(result->bufString.text, str, len);
		result->bufString.text[len] = '\0';
	} else if (original == NULL) { /* && str == NULL */
		result->bufString.length = 0;
		result->bufString.text[0] = '\0';
	} else if (length < orglen) {
		result->bufString.length = length;
	} /* else leave original GSStringPtr untouched */
	
	result->bufString.text[length] = '\0';
	return result;
}

/*
 * reimplementations for the next three ...
 */

GSStringPtr
__C2GSMALLOC (const char *s) {
	size_t len;

	len = strlen(s);
	if (len > USHRT_MAX - 1) {
		errno = EINVAL;
		return NULL;
	}
	return GIchange(NULL, len, s);
}

char *
__GS2CMALLOC (const GSStringPtr g) {
	char *p;

	if ((p = malloc(g->length + 1)) != NULL) {
		strncpy(p, g->text, g->length);
		p[g->length] = '\0';
	}
	return p;
}

char *
__GS2C (char *s, const GSStringPtr g) {
	strncpy(s, g->text, g->length);
	s[g->length] = '\0';
	return s;
}

/*
 * __C2GS
 *
 * Converts a null-terminated C string into a class 1 GS/OS string.
 * Space for the GS/OS string must already be allocated, and the
 * length of s must be less than USHRT_MAX chars.
 *
 * If the s is too long, __C2GS will return NULL, otherwise it will
 * return the GS/OS string g.
 *
 * As a side effect, g->text will also be NULL-terminated.
 */

GSStringPtr
__C2GS(const char *s, GSStringPtr g)
{
	size_t len;

	len = strlen(s);
	if (len > USHRT_MAX - 1) {
		errno = EINVAL;
		return NULL;  /* the string won't fit */
	}
	g->length = len;
	strncpy(g->text, s, len);
	g->text[len] = '\0';
	return g;
}
 
