/*
 * Copyright 1996 Devin Reade <gdr@myrias.com>.
 * All rights reserved.
 *
 * For copying and distribution information, see the file "COPYING"
 * accompanying this file.
 *
 * $Id: stringGS.c,v 1.1 1996/03/31 23:38:34 gdr Exp $
 */

#include <gsos.h>
#include "install.h"

/*
 * strcpyGSString255
 *
 * copies the GSString255 pointed to by <from> to that pointed
 * to by <to>
 */

void
strcpyGSString255 (GSString255Ptr to, GSString255Ptr from)
{
   int i;

  char *p = from->text;
  char *q = to->text;
  for (i=0; i<from->length; i++) *q++ = *p++;
   to->length = from->length;
  return;
}

/*
 * strcatGSString255
 *
 * concatenates the string <from> onto <to>, to a maximum of 255
 * chars total in <to>.
 */

void
strcatGSString255 (GSString255Ptr to, GSString255Ptr from)
{
   int i, count;

  char *p = from->text;
  char *q = to->text;
  q+= to->length;
   count = from->length;
  if (count > 255 - to->length) count = 255 - to->length;
  for (i=0; i<count; i++) {
      *q++ = *p++;
  }
  to->length += count;
  return;
}

/*
 * like strcmp(3), but for GSString255Ptr args.
 */

int
strcmpGSString255 (GSString255Ptr a, GSString255Ptr b)
{
   int i, count;
  char *p, *q;

  count = a->length - b->length;
  if (count) return count;

  p = a->text;
  q = b->text;
   for (i=0; i<count; i++, p++, q++) {
      if (*p == *q) continue;
     else if (*p > *q) return 1;
     else return -1;
  }
   return 0;
}
