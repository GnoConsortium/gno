/*
 * These routines were written by Devin Reade for GNO v2.0.4.
 *
 * $Id: basename.c,v 1.1 1997/02/28 05:12:43 gdr Exp $
 *
 * This file is formatted for tabs every 8 columns.
 */

#ifdef __ORCAC__
segment "libc_gen__";
#endif

#pragma optimize 0
#pragma debug 0
#pragma memorymodel 0

#include <string.h>
#include <stdio.h>
#include <unistd.h>

/*
 * basename
 *
 * returns the filename component of <path>.  If <path> contains colons,
 * they are assumed to be the directory separators, otherwise any '/' is
 * assumed to be a directory separator.
 *
 * If no directory separators are found, then the full path is returned.
 *
 * No check is done as to whether the pathname is valid on the any
 * given filesystem.
 */                 

char *
basename (const char *path) {
	char delim, *p;

	delim = strchr(path,':') ? ':' : '/';
	p = strrchr(path,delim);
	return p ? p+1 : path;
}

/*
 * dirname
 *
 * Returns a pointer to an internal buffer that contains a string that
 * matches the directory component
 * of <path>.  If <path> contains at least one ':', then it is assumed
 * that colons are directory separators, otherwise any '/' character
 * is treated as a directory separator.
 *
 * If <path> contains no pathname separators, then dirname() will
 * return an empty (zero-length) string.
 *
 * No check is done as to whether the pathname is valid on the any
 * given filesystem.
 */

char *
dirname (const char *path)
{
	char delim, *p;
	size_t len;
	static char dir[PATH_MAX];

	len = strlen(path);
	if (len >= PATH_MAX) {
		len = PATH_MAX -1;
		strncpy(dir,path,len);
		dir[len] = '\0';
	} else {
		strcpy(dir, path);
	}
	delim = strchr(dir,':') ? ':' : '/';
	p = strrchr(dir,delim);
	if (p == NULL) {
		*dir = '\0';
	} else {
		*p = '\0';
	}
	return dir;
}
