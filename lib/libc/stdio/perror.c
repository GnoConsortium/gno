/*
 * Implementation by Devin Reade.
 *
 * $Id: perror.c,v 1.1 1997/02/28 05:12:49 gdr Exp $
 *
 * This file is formatted with tab stops every 8 columns.
 */

/* I have to do this until I can modify ORCALib */
#define sys_errlist	_gno_sys_errlist
#define sys_nerr	_gno_sys_nerr

#ifdef __ORCAC__
segment "libc_stdio";
#endif

#pragma databank 1
#pragma optimize 0
#pragma debug 0
#pragma memorymodel 0

#include <sys/errno.h>		/* for ELAST */
#include <stdio.h>		/* for remainder */

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

char *
strerror (int errnum)
{
	/*
	 * the size of buff must be greater than
	 *	strlen(sys_errlist[0]) + max number of digits in an int + 3
	 * ==  13 + 5 + 3 == 21
	 */
	static char buff[30];
	
	if (errnum > 0 || errnum < sys_nerr) {
		return sys_errlist[errnum];
	}
	sprintf(buff, "unknown error: %d", errnum);
	return buff;
}

/*
 * This implementation of perror should be replaced with one similar to
 * that for 4.4BSD, so that stdio doesn't need to get linked in.
 */
 
void
perror (char *s)
{
	char *s1, *s2;

	if (s == NULL) {
		s1 = s2 = "";
	} else {
		s1 = s;
		s2 = ": ";
	}
	if (errno <= 0 || errno >= sys_nerr) {
		fprintf(stderr, "%s%s%s: %d\n", s1, s2, sys_errlist[0], errno);
	} else {
		fprintf(stderr,"%s%s%s\n", s1, s2, sys_errlist[errno]);
	}
}

