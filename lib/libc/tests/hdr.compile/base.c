/*
 * $Id: base.c,v 1.1 1997/02/28 05:12:55 gdr Exp $
 */

#include "test.h"
#include <sys/types.h>

/*
 * This set has no #include dependancies in the base directory, so the
 * order doesn't matter.
 */
#include <dirent.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <fnmatch.h>
#ifndef KERNEL
#include <fts.h>
#endif
#include <glob.h>
#include <grp.h>
#include <limits.h>
#include <netdb.h>
#include <paths.h>
#include <pwd.h>
#include <regexp.h>
#include <setjmp.h>

/*
 * we can't include both of these; they are mutually exclusive ways
 * of handling ttys
 */
#ifdef TEST_SGTTY
#include <sgtty.h>
#else
#include <termios.h>
#endif

#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <termcap.h>
#include <time.h>
#include <ttyent.h>
#include <types.h>
#include <unistd.h>
#include <utime.h>

/*
 * This set has _some_ dependancies in the base directory, so watch
 * the order.
 */
#include <curses.h>
#include <db.h>
#include <resolv.h>
#include <utmp.h>

/* a real mess ... */
#if 0
#include <appletalk.h>
#endif

int i;
