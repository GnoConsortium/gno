/*
 * Copyright 1996 Devin Reade <gdr@myrias.com>.
 * All rights reserved.
 *
 * For copying and distribution information, see the file "COPYING"
 * accompanying this file.
 *
 * $Id: install.h,v 1.1 1996/03/31 23:38:33 gdr Exp $
 */

#ifndef __GSOS__
#include <gsos.h>
#endif

/* these are from libc */
extern GSString255Ptr __C2GSMALLOC (char *s);
extern char * __GS2CMALLOC (GSString255Ptr g);
extern char * __GS2C (char *s, GSString255Ptr g);
extern int _mapErr (int err);

/* from basename.c */
extern char *dirname (char *path);
extern char *basename (char *path);

/* from c2gs.c */
extern GSString255Ptr __C2GS(char *s, GSString255Ptr g);

/* from copyfile.c */
extern char *copyfile (char *from, char *to);

/* from errnoGS.c */
extern unsigned short errnoGS;
extern char *strerrorGS (unsigned short num);
extern void perrorGS (char *format, ...);

/* from expandpath.c */
extern char *expandpath (char *path);

/* from stringGS.c */
extern void strcpyGSString255 (GSString255Ptr to, GSString255Ptr from);
extern void strcatGSString255 (GSString255Ptr to, GSString255Ptr from);
extern int  strcmpGSString255 (GSString255Ptr a, GSString255Ptr b);
                                                                
