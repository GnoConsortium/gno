/*
 * Copyright 1996 Devin Reade <gdr@myrias.com>.
 * All rights reserved.
 *
 * For copying and distribution information, see the file "COPYING"
 * accompanying this file.
 *
 * $Id: basename.c,v 1.1 1996/03/31 23:38:30 gdr Exp $
 */

#include <string.h>
#include <stdio.h>
#include "install.h"

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
basename (char *path)
{
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
dirname (char *path)
{
   char delim, *p;
  static char dir[FILENAME_MAX];

  strncpy(dir,path,FILENAME_MAX-1);
  dir[FILENAME_MAX-1] = '\0';
  delim = strchr(dir,':') ? ':' : '/';
  p = strchr(dir,delim);
  if (p == NULL) {
      *dir = '\0';
  } else {
      *p = '\0';
  }
  return dir;
}
