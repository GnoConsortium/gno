/*
 * Copyright 1996 Devin Reade <gdr@myrias.com>.
 * All rights reserved.
 *
 * For copying and distribution information, see the file "COPYING"
 * accompanying this file.
 *
 * $Id: c2gs.c,v 1.1 1996/03/31 23:38:31 gdr Exp $
 */

#include <types.h>
#include <gsos.h>
#include <string.h>
#include "install.h"

/*
 * __C2GS
 *
 * Converts a null-terminated C string into a class 1 GS/OS string.
 * Space for the GS/OS string must already be allocated, and the
 * length of s must not be more than 255 chars.
 *
 * If the s is too long, __C2GS will return NULL, otherwise it will
 * return the GS/OS string g.
 */

GSString255Ptr
__C2GS(char *s, GSString255Ptr g)
{
   size_t len;

  len = strlen(s);
  if (len > 255) return NULL;  /* the string won't fit */
  g->length = len;
  strncpy(g->text,s,255);
  return g;
}
