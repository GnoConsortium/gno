/*
 * Too bad we don't have symlinks for GNO ...
 *	This file allows a program to use the *proper* name for this
 *	header file, while allowing the "real" header file to reside on
 *	a ProDOS volume.  This is good for speed and disk integrity,
 *	and gives a work-around for those sites which don't use HFS.
 *
 * $Id: ioctl_compat.h,v 1.1 1997/02/28 04:41:59 gdr Exp $
 */

#ifndef _SYS_IOCTL_COMPAT_H_
#include <sys/ioctl.compat.h>
#endif
