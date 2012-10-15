/*
 * Implementation by Devin Reade.
 *
 * $Id: strerror.c,v 1.3 1998/02/04 15:16:19 gdr-ftp Exp $
 *
 * This file is formatted with tab stops every 8 columns.
 */

#ifdef __ORCAC__
segment "libc_gen__";
#endif

#include <sys/errno.h>		/* for ELAST */

const char * const sys_errlist[] = {

/* the following are used by both GNO and the Orca/Shell */

	"unknown error",			/*  0 */
	"domain error",				/*  1 */
	"result too large",			/*  2 */
	"not enough memory",			/*  3 */
	"no such file or directory",		/*  4 */
	"I/O error",				/*  5 */
	"invalid argument",			/*  6 */
	"bad file descriptor",			/*  7 */
	"too many open files",			/*  8 */
	"permission denied",			/*  9 */
	"file exists",				/* 10 */
	"no space left on device",		/* 11 */

/* the following are GNO-specific */

	"operation not permitted",		/* 12 */
	"no such process",			/* 13 */
	"interrupted system call",		/* 14 */
	"arg list too long",			/* 15 */
	"exec format error",			/* 16 */
	"no child processes",			/* 17 */
	"resource unavailable",			/* 18 */
	"not a directory",			/* 19 */
	"inappropriate ioctl for device",	/* 20 */
	"broken pipe",				/* 21 */
	"illegal seek",				/* 22 */
	"block device required",		/* 23 */
	"is a directory",			/* 24 */
	"not a socket",				/* 25 */
	"destination address required",		/* 26 */
	"message too long",			/* 27 */
	"wrong protocol for socket",		/* 28 */
	"protocol not available",		/* 29 */
	"protocol not supported",		/* 30 */
	"socket type not supported",		/* 31 */
	"operation not supported on socket",	/* 32 */
	"protocol family not supported",	/* 33 */
	"address family not supported",		/* 34 */
	"address already in use",		/* 35 */
	"can't assign requested address",	/* 36 */
	"network is down",			/* 37 */
	"network is unreachable",		/* 38 */
	"network dropped connection on reset",	/* 39 */
	"connection aborted",			/* 40 */
	"connection reset by peer",		/* 41 */
	"no buffer space available",		/* 42 */
	"socket is already connected",		/* 43 */
	"socket is not connected",		/* 44 */
	"can't send after socket shutdown",	/* 45 */
	"too many references: can't splice",	/* 46 */
	"connection timed out",			/* 47 */
	"connection refused",			/* 48 */
	"operation would block",		/* 49 */
	"operation now in progress",		/* 50 */
	"operation already in progress",	/* 51 */
	"bad address",				/* 52 */
	"no such device",			/* 53 */
	"host is down",				/* 54 */
	"no route to host",			/* 55 */
#define SYS_NERR 56	/* 55 + 1 for zeroth entry */
};

#if (ELAST + 1 != SYS_NERR)
#error message table out of sync
#endif

const int
sys_nerr = SYS_NERR;

const char * const *
_errnoText = sys_errlist;	/* backward compatible */

