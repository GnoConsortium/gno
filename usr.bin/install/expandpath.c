/*
 * Copyright 1996 Devin Reade <gdr@myrias.com>.
 * All rights reserved.
 *
 * For copying and distribution information, see the file "COPYING"
 * accompanying this file.
 *
 * $Id: expandpath.c,v 1.1 1996/03/31 23:38:32 gdr Exp $
 */

#include <types.h>
#include <gsos.h>
#include <errno.h>
#include <orca.h>
#include "install.h"

/*
 * expandpath
 *
 * Uses the GS/OS facilities to expand the pathname <path>.  On
 * success, returns a pointer to the malloc'd expanded path.  On
 * failure it will return NULL and set errno.
 *
 * Note that in using this function, all directory separators will
 * be converted to colons.
 *
 * Unfortunately, this routine uses a little over 0.5k of stack space ...
 */

char *
expandpath (char *path)
{
   ExpandPathRecGS expand;
  GSString255 inStr;
  ResultBuf255 outBuf;
  int i;

   if (__C2GS(path,&inStr) == NULL) {
      errno = EINVAL;
     return NULL;
  }
  expand.pCount = 3;
  expand.inputPath = &inStr;
  expand.outputPath = &outBuf;
  expand.flags = 0x0000;
  outBuf.bufSize = 255;
   ExpandPathGS(&expand);
  if ((i = toolerror()) != 0) {
      errno = _mapErr(i);
      return NULL;
  }
  return __GS2CMALLOC(&(outBuf.bufString));
}
