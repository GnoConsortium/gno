/*
 * Copyright 1996 Devin Reade <gdr@myrias.com>.
 * All rights reserved.
 *
 * For copying and distribution information, see the file "COPYING"
 * accompanying this file.
 *
 * $Id: install.h,v 1.2 1996/09/03 03:54:59 gdr Exp $
 */

#ifndef __GSOS__
#include <gsos.h>
#endif

/* these are from libc */
extern GSString255Ptr __C2GSMALLOC (char *s);
extern char * __GS2CMALLOC (GSString255Ptr g);
extern char * __GS2C (char *s, GSString255Ptr g);
extern int _mapErr (int err);
