/*
 * utime, utimes.  Implementation by Devin Reade.
 *
 * $Id: utime.c,v 1.1 1997/02/28 05:12:45 gdr Exp $
 *
 * This file is formatted with tab stops every 8 columns.
 */

#ifdef __ORCAC__
segment "libc_gen__";
#endif

#pragma optimize 0
#pragma debug 0
#pragma memorymodel 0

#include <sys/types.h>
#include <sys/time.h>
#include <utime.h>
#include <sys/errno.h>
#include <gno/gno.h>
#include <stdlib.h>
#include <string.h>
#include <types.h>
#include <gsos.h>

extern int _toolErr;

int
utime(const char *path, const struct utimbuf *buf)
{
	FileInfoRecPtrGS infoPtr;
	time_t t;
	struct tm *tmptr;
	int i;

	/* allocate structures */
	infoPtr = malloc (sizeof (FileInfoRecGS));
	if (infoPtr == NULL) {
		return -1;
	}
	infoPtr->pathname = (GSString255Ptr) __C2GSMALLOC(path);
	if (infoPtr->pathname == NULL) {
		i = errno;
		free(infoPtr);
		errno = i;
	return -1;
	}

	/* initialize structure and get current file info */
	infoPtr->pCount = 7;
	_toolErr = 0;
	GetFileInfoGS(infoPtr);
	if (_toolErr) {
		i = _mapErr(_toolErr);
		free(infoPtr->pathname);
		free(infoPtr);
		errno = i;
		return -1;
	}

	/* change the file creation time */
	if (buf == NULL) {
		time(&t);
		tmptr = localtime(&t);
	} else {
		tmptr = localtime(&(buf->actime));
	}
	infoPtr->createDateTime.second  = tmptr->tm_sec;
	infoPtr->createDateTime.minute  = tmptr->tm_min;
	infoPtr->createDateTime.hour    = tmptr->tm_hour;
	infoPtr->createDateTime.year    = tmptr->tm_year;
	infoPtr->createDateTime.day     = tmptr->tm_mday;
	infoPtr->createDateTime.month   = tmptr->tm_mon;
	infoPtr->createDateTime.weekDay = tmptr->tm_wday;

	/* change the file modification time */
	if (buf == NULL) {
		infoPtr->modDateTime = infoPtr->createDateTime;
	} else {
		tmptr = localtime(&(buf->modtime));          
		infoPtr->modDateTime.second  = tmptr->tm_sec;
		infoPtr->modDateTime.minute  = tmptr->tm_min;
		infoPtr->modDateTime.hour    = tmptr->tm_hour;
		infoPtr->modDateTime.year    = tmptr->tm_year;
		infoPtr->modDateTime.day     = tmptr->tm_mday;
		infoPtr->modDateTime.month   = tmptr->tm_mon;
		infoPtr->modDateTime.weekDay = tmptr->tm_wday;
	}

	/* write the info to the filesystem */
	infoPtr->storageType = 0x0000;
	SetFileInfoGS(infoPtr);
	i = (_toolErr) ? _mapErr(_toolErr) : 0;
	free(infoPtr->pathname);
	free(infoPtr);
	if (i != 0) {
		errno = i;
		return -1;
	}
	return 0;
}

int
utimes (const char *path, const struct timeval *tvp) {
	struct utimbuf tmpval;

	if (tvp) {
		tmpval.actime  = tvp[0].tv_sec;
		tmpval.modtime = tvp[1].tv_sec;
		return utime(path, &tmpval);
	} else {
		return utime(path, NULL);
	}
}
