/*
 * Temporary file and filename routines.
 *
 * $Id: tempnam.c,v 1.1 1997/02/28 05:12:49 gdr Exp $
 *
 * This file is formatted with tab stops every 8 characters.
 */

#ifdef __ORCAC__
segment "libc_stdio";
#endif

#pragma optimize 0
#pragma debug 0
#pragma memorymodel 0

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define max(A,B) (((A)<(B))?(B):(A))

/*
 * tempnam
 *
 * Generate a pathname for a temporary file.
 *
 * tempnam will select a directory for the temporary file by using the
 * following criteria:
 *
 *   If dir is not the NULL pointer, tempnam uses the pathname pointed to by
 *   dir as the directory,
 *
 *   otherwise, tmpdir uses the value of the TMPDIR environment variable if
 *   the variable is defined,
 *
 *   otherwise the directory defined by P_tmpdir in the stdio.h header file
 *   if that directory is writable by the caller,
 *
 *   otherwise, tempnam will use "/tmp" as a last resort.
 */


static char  seed[4]="AAA";

/*
 * cpdir - copy <str> into <buf>, removing the trailing directory separator
 *         if necessary.
 */

static char *
cpdir(char *buf, char *str)
{
	char *p, pbrk;

	if(str != NULL) {
		strcpy(buf, str);
		p = buf + strlen(buf) -1;
		pbrk = strchr(buf,':') ? ':' : '/';  /* for GS/OS */
		if(*p == pbrk) *p = '\0';
	}
	return(buf);
}

/*
 * tempnam
 *	dir    -- use this directory please (if non-NULL)
 *	prefix -- use this (if non-NULL) as filename prefix
 */

char *
tempnam (const char *dir, const char *prefix)
{
	register char *p, *q, *tmpdir, pbrk;
	int tl=0, dl=0, pl;

	/* create a buffer <p> that's as large as necessary */
	pl = strlen(P_tmpdir);
	if( (tmpdir = getenv("TMPDIR")) != NULL ) tl = strlen(tmpdir);
	if( dir != NULL ) dl = strlen(dir);
	if( (p = malloc((unsigned int)(max(max(dl,tl),pl)+16))) == NULL )
		return(NULL);
	*p = '\0';

#define PERM W_OK

	if( (dl == 0) || (access( cpdir(p, dir), PERM) != 0) )
		if( (tl == 0) || (access( cpdir(p, tmpdir), PERM) != 0) )
			if( access( cpdir(p, P_tmpdir), PERM) != 0 )
				if( access( cpdir(p, "/tmp"), PERM) != 0 )
					return(NULL);

	pbrk = strchr(p,':') ? ':' : '/';
	q = p + strlen(p);
	*q++ = pbrk;
	*q = '\0';
	if(prefix) {
		*(p+strlen(p)+5) = '\0';
		(void)strncat(p, prefix, 5);
	}

	strcat(p, seed);
	strcat(p, "XXXXXX");

	q = seed;
	while(*q == 'Z') *q++ = 'A';
	++*q;

	if(*mktemp(p) == '\0') return(NULL);
	return(p);
}
