/*
 * These functions really belong in a separate library, libcompat, if
 * we're trying to be BSD-ish.  They are all obsolete functions.
 *
 * $Id: compat.c,v 1.2 1997/09/21 06:04:59 gdr Exp $
 *
 * This file is formatted with tabs every 8 characters.
 */

#ifdef __ORCAC__
segment "libc_gen__";
#endif

#include <sgtty.h>

int
gtty(int filedes, struct sgttyb *argp) {
	return ioctl(filedes,TIOCGETP,argp);
}

int
stty (int filedes, struct sgttyb *argp) {
	return ioctl(filedes,TIOCSETP,argp);
}
