/*
 * Copyright 1995-1998 by Devin Reade <gdr@trenco.gno.org>. For distribution
 * information see the README file that is part of the manpack archive,
 * or contact the author, above.
 *
 * $Id: util.c,v 1.2 1998/03/29 07:16:19 gdr-ftp Exp $
 */

#ifdef __ORCAC__
segment "util______";
#pragma noroot
#endif

#define __USE_DYNAMIC_GSSTRING__

#include <sys/types.h>
#include <sys/stat.h>
#include <types.h>
#include <gsos.h>
#include <ctype.h>
#include <string.h>
#include <sgtty.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <gno/gno.h>
#include "man.h"

/*
 * getManpath -- return a malloc'd copy of the MANPATH.  If MANPATH
 *               isn't defined in the environment, then use USRMAN, then
 *               MANDIR, then if all else fails, use the macro DEFAULT_MANPATH.
 *               If an error occurs, print the cause via perror and exit.
 */

char *getManpath(void) {
   char *manpath;

   manpath = getenv("MANPATH");
   if (manpath == NULL) manpath = getenv("USRMAN");
   if (manpath == NULL) manpath = getenv("MANDIR");
   if (manpath == NULL) manpath = DEFAULT_MANPATH;

   return LC_xstrdup(manpath);
}



/*
 * MakePathArray -- parse a path list and break it into a StringArray_t.
 *                  The original path is left unchanged.
 *
 *                  The delimiter between paths may either be a ' ' or a ':'.
 *                  The delimiter is assumed to be a ':' if <path> contains
 *                  both '/' and ':' characters but no ' ' character,
 *                  otherwise the delimiter is assumed to be a ' '.
 */

LC_StringArray_t
MakePathArray(char *path) {
	LC_StringArray_t	result;
	char		*delim, *p, *q;

	/* set the delimiter */
	if ((strchr(path,' ')==NULL) &&
	     strchr(path,':') &&
	     strchr(path,'/')) {
		delim = ":";
	} else {
		delim = " ";
	}

	/* build the array */
#ifndef __ORCAC__
	/* ORCA/C's strtok implementation doesn't modify the provided buffer */
	p = LC_xstrdup(path);
#else
	p = path;
#endif
	result = LC_StringArrayNew();
	q = strtok(p,delim);
	while (q != NULL) {
		LC_StringArrayAdd(result, q);
		q = strtok(NULL,delim);
	}

#ifndef __ORCAC__
	free(p);
#endif
	return result;
}

/*
 * strcasestr -- A case insensitive ("no-case") strstr
 *
 *             This is implemented using a convert-copy-to-single-case-
 *             then-strstr hack.
 *             It's speed could be increased by switching
 *             to a Boyer-Moore scan algorithm anytime strlen(substr) > 5,
 *             and using a straight-forward search for strlen(substr) <= 5.
 */

char *
strcasestr(char *str, char *substr) {

   char *strCopy, *substrCopy, *p;

   strCopy = LC_xstrdup(str);
   substrCopy = LC_xstrdup(substr);

   /* convert the strings */
   p = strCopy;
   while (*p) {
      *p = tolower(*p);
      p++;
   }
   p = substrCopy;
   while (*p) {
      *p = tolower(*p);
      p++;
   }

   p = strstr(strCopy,substrCopy);
   free(strCopy);
   free(substrCopy);
   return p;
}

/*
 * newerFile  -- Compares the "last modified" time of two paths.
 *               Returns:
 *                   the name of the newer file if both files exist
 *                   the second file if both exist and have the same mod time
 *                   the name of the existing file if one file doesn't exist
 *                   NULL and sets errno if neither file exists or if there
 *                      is an error.
 */

char *
newerFile(char *path1, char *path2) {

	static struct stat sbuf1, sbuf2;
	int e1, e2;

	/* stat the first file */
	if (stat(path1, &sbuf1) < 0) {
		if (errno == ENOENT) {
			e1 = ENOENT;
		} else {
			return NULL;
		}
	} else {
		e1 = 0;
	}

	/* stat the second file */
	if (stat(path2, &sbuf2) < 0) {
		if (errno == ENOENT) {
			e2 = ENOENT;
		} else {
			return NULL;
		}
	} else {
		e2 = 0;
	}

	/* one or both don't exist? */
	if (e1 && e2) {
		return NULL;
	}
	if (e1) {
		return path2;
	}
	if (e2) {
		return path1;
	}

	/* both exist */
   	return (sbuf1.st_mtime > sbuf2.st_mtime) ? path1 : path2;
}                        

/*
 * getcharraw() - return the next character from stdin without waiting
 *                for a CR to be hit.  Returns '\0' and sets errno
 *                on an error.
 */

#define FAILED_CHAR '\0';

char
getcharraw(void) {
   short oldmode;
   struct sgttyb s;
   int count;
   char c;

   /* obtain old terminal mode */
   if (gtty(STDIN_FILENO,&s) == -1) return FAILED_CHAR;
   oldmode = s.sg_flags;

   /* set terminal to CBREAK and obtain keystroke */
   s.sg_flags |= CBREAK;
   if (stty(STDIN_FILENO,&s) == -1) return FAILED_CHAR;
   count = read (STDIN_FILENO, &c, 1);

   /* reset old terminal mode */
   s.sg_flags = oldmode;
   if ((stty(STDIN_FILENO,&s) == -1) || (count == -1)) return FAILED_CHAR;
   return c;
}

/*
 * getFileType -- Get the file type and auxillary file type of a file.
 *                On success it returns a pointer to an internal buffer
 *                containing the file type and aux type.  On failure
 *                it returns NULL and sets errno.
 */

fileType *getFileType (char *file) {

   static FileInfoRecGS record;
   static fileType result;
   int i;

   /* set the parameters */
   record.pCount = 4;
   if ((record.pathname = __C2GSMALLOC(file)) == NULL) {
      return NULL;
   }

   /* get the info */
   GetFileInfoGS(&record);
   i = _toolErr;
   GIfree(record.pathname);
   if (i) {
      errno = _mapErr(i);
      return NULL;
   }

   /* set the return value */
   result.type = record.fileType;
   result.auxtype = record.auxType;

   return &result;
}
 
