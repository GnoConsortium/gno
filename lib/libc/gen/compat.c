/*
 * These functions really belong in a separate library, libcompat, if
 * we're trying to be BSD-ish.  They are all obsolete functions.
 *
 * $Id: compat.c,v 1.1 1997/02/28 05:12:43 gdr Exp $
 *
 * This file is formatted with tabs every 8 characters.
 */

#ifdef __ORCAC__
segment "libc_gen__";
#endif

#pragma optimize 0
#pragma debug 0
#pragma memorymodel 0

#include <sgtty.h>

int
gtty(int filedes, struct sgttyb *argp) {
	return ioctl(filedes,TIOCGETP,argp);
}

int
stty (int filedes, struct sgttyb *argp) {
	return ioctl(filedes,TIOCSETP,argp);
}

