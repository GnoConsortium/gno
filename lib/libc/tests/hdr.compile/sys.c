/*
 * $Id: sys.c,v 1.1 1997/02/28 05:12:56 gdr Exp $
 */

#include "test.h"

/*
 * This set has no #include dependancies in sys, so the order doesn't
 * matter.
 */
#include <sys/cdefs.h>
#include <sys/dirent.h>
#include <sys/errno.h>
#include <sys/ioccom.h>
#include <sys/ttydefaults.h>
#include <sys/ttydev.h>
#include <sys/unistd.h>
#include <sys/syslimits.h>

/* This set has _some_ dependancies in sys, so watch the order. */
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/filio.h>
#include <sys/time.h>
#include <sys/mount.h>
#include <sys/ports.h>
#include <sys/select.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/stat.h>
#include <sys/syslog.h>
#include <sys/times.h>
#include <sys/ttycom.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <sys/ttychars.h>
#include <sys/ioctl_compat.h>
#include <sys/ioctl.h>
#include <sys/termios.h>
#include <sys/tty.h>
#include <sys/param.h>

int i;
