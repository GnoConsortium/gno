/*
 * Too bad we don't have symlinks for GNO ...
 *	This file allows a program to use the *proper* name for this
 *	header file, while allowing the "real" header file to reside on
 *	a ProDOS volume.  This is good for speed and disk integrity,
 *	and gives a work-around for those sites which don't use HFS.
 *
 * $Id: dli_var.h,v 1.1 1997/02/28 04:41:52 gdr Exp $
 */

#ifndef _NET_DLI_VAR_H_
#include <net/dli.var.h>
#endif

