/*
 * $Id: copyfile.c,v 1.3 1998/04/10 21:36:08 gdr-ftp Exp $
 */

/* Change the arg types of GS/OS parameter blocks. Must be before #includes */
#define __USE_DYNAMIC_GSSTRING__

#include <types.h>	/* for _toolErr, among others */
#include <errno.h>
#include <gsos.h>
#include <orca.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <gno/gno.h>

#define NDEBUG
#include <assert.h>

#include "contrib.h"

#ifndef MAX
#define MAX(a,b) (((a)>(b)) ? (a) : (b))
#endif

#if 1
#define DEBUG(a)
#else
#include <stdio.h>
#define DEBUG(a) printf a
#endif

/*
 * copyfile
 *
 * copy a file from the pathname <from> to the location <to>, which
 * may be a directory.  Ensure that file types and other information
 * (except for the backup bit) is matched.
 *
 * Returns NULL and sets errno on failure.  On success, returns a
 * pointer to an internal buffer containing the final pathname.
 *
 * Possible errnos:
 *	EINVAL		Neither the LC_COPY_DATA nor the LC_COPY_REZ
 *			flags were set.
 *
 *	ENOMEM		Ran out of memory.
 *
 *	ENOENT		A request was made to copy only the resource
 *			fork, but the file does not have a resource fork.
 *
 *	(any errno returned from ExpandPathGS, GetFileInfoGS, DestroyGS,
 *	 CreateGS, OpenGS, CloseGS, SetFileInfoGS)
 *
 * +++ THIS ROUTINE IS NOT REENTRANT +++
 */

#define NAMELEN_QUANTUM	256

GSStringPtr
LC_CopyFileGS (GSStringPtr from, GSStringPtr to, unsigned short flags)
{
	static ResultBufPtr	expandedFrom = NULL;
	static size_t		expandedFromLen = 0;
	static ResultBufPtr	expandedTo = NULL;
	static size_t		expandedToLen = 0;
	static char *		buffer = NULL;
	static size_t		bufferLen = 0;
	static ExpandPathRecGS	expandrec;
	static FileInfoRecGS	inforec;
	static OpenRecGS	openrec;
 	static struct {
		Word pCount;
		Word refNum;
		Longword dataBuffer;
 		Longword requestCount;
		Longword transferCount;
		Word cachePriority;
	} iobuf;
	static struct {
		Word pCount;
		Word refNum;
	} closerec;
                
	ResultBufPtr	rbp;
	GSStringPtr	gsp;
	size_t		newlen;
	int		done, i, j, k, isDir, exists;
	char		*p, *q;
	Word		refNumIn, refNumOut, oldAccess;

	/* concheck */
	if ((flags & (LC_COPY_DATA | LC_COPY_REZ)) == 0) {
		errno = EINVAL;
		return NULL;
	}

	/*
	 * First time through?  We have to allocate our buffer.  Try
	 * 64k to start, then keep dividing by 2 until we can allocate
	 * one successfully.  Don't bother with anything less that 1k,
	 * just return an error.
	 */
	if (buffer == NULL) {
		newlen = 128;	/* we'll divide this at least once */
		do {
			newlen /= 2;
			if (newlen <= 1) {
				errno = ENOMEM;
				return NULL;
			}
			buffer = malloc(newlen * 1024);
		} while (buffer == NULL);
		bufferLen = newlen * 1024;

		/* do some other initializations */
		expandrec.pCount = 3;
	}
	
	/*
	 * Initialize the name result buffers, growing them if necessary.
	 */
	for (i=0; i<2; i++) {
		switch (i) {
		case 0:
			gsp = from;
			rbp = expandedFrom;
			newlen = expandedFromLen;
			break;
		case 1:
			gsp = to;
			rbp = expandedTo;
			newlen = expandedToLen;
			break;
		}
		done = 0;

		if (newlen < gsp->length) {
			while (newlen < gsp->length) {
				newlen += NAMELEN_QUANTUM;
			}
			rbp = GOchange(rbp, newlen, NULL);
			if (rbp == NULL) {
				return NULL;
			}
		}
		while (!done) {
			expandrec.inputPath = gsp;
			expandrec.outputPath = rbp;
			expandrec.flags = 0x0000;
			ExpandPathGS(&expandrec);
			switch ((j = _toolErr)) {
			case 0:
				/* NULL-terminate it */
				(rbp->bufString.text)[rbp->bufString.length]
					= '\0';
				done = 1;
				break;
			case buffTooSmall:
				newlen += NAMELEN_QUANTUM;
				rbp = GOchange(rbp, newlen, NULL);
				if (rbp == NULL) {
					done = 1;
					newlen -= NAMELEN_QUANTUM;
					errno = ENOMEM;
					/*
					 * we delay the return until after
					 * the next switch statement so that
					 * we can reset the static variables.
					 */
				}
				break;
			default:
				errno = _mapErr(j);
				return NULL;
			}
		}
		switch(i) {
		case 0:
			expandedFrom = rbp;
			expandedFromLen = newlen;
			break;
		case 1:
			expandedTo = rbp;
			expandedToLen = newlen;
			break;
		}
		if (rbp == NULL) {
			return NULL;
		}
	}

	/* find out if <to> is a directory */
	inforec.pCount = 5;
	inforec.pathname = &(expandedTo->bufString);
	GetFileInfoGS(&inforec);
	i = _toolErr;
	switch(i) {
	case 0:
		exists = 1;
		isDir = ((inforec.storageType == 0x0D) ||
			 (inforec.storageType == 0x0F)) ? 1 : 0;
		break;
	case fileNotFound:
		exists = 0;
		isDir = 0;
		break;
	default:
		errno = _mapErr(i);
		return NULL;
	}

	/* it's a directory? tack on the file name */
	if (isDir) {
		/* we know that the text field is null-terminated */
		p = expandedFrom->bufString.text +
			expandedFrom->bufString.length;
		if (p != expandedFrom->bufString.text) {
			--p;
		}
		/* p now points to the last legal char in the input name */
		i = 0;
		assert(*p != ':');
		q = expandedFrom->bufString.text;
		while ((p > q) && (*p != ':')) {
			--p;
			i++;
		}
		if (*p == ':') {
			p++;
		}
		/*
		 * p now points to the basename of expandedFrom and
		 * i is the length of that basename
		 */

		/* grow the resultbuf if necessary */
		if (expandedTo->bufString.length + i + 1 >
		    expandedTo->bufSize) {
			rbp = GOchange(expandedTo, expandedTo->bufString.length
				+ 1 + MAX(i, NAMELEN_QUANTUM), NULL);
			if (rbp == NULL) {
				errno = ENOMEM;
				return NULL;
			}
			expandedTo = rbp;
		}

		/* copy the basename over */
		q = expandedTo->bufString.text + expandedTo->bufString.length;
		*q++ = ':';
		memcpy(q, p, i+1);			/* one for the term */
		expandedTo->bufString.length += i + 1;	/* one for the ':' */
        }

	/* check to see it's not the same file */
	if (!strcmp(expandedFrom->bufString.text, expandedTo->bufString.text)) {
		errno = EINVAL;
		return NULL;
	}

	/* get the file info of the original file */
	inforec.pCount = 7;
	inforec.pathname = &(expandedFrom->bufString);
	GetFileInfoGS(&inforec);
	if ((i = _toolErr) != 0) {
		errno = _mapErr(i);
		return NULL;
	}
        oldAccess = inforec.access;
        
	if (((flags & LC_COPY_FORKS_MASK) == LC_COPY_REZ) &&
	    (inforec.storageType == extendedFile)) {
		errno = ENOENT;
		return NULL;
	}

	/* destroy the old target file if it exists and is not a directory */
	inforec.pCount = 1;
	inforec.pathname = &(expandedTo->bufString);
	DestroyGS(&inforec);
	i = _toolErr;
	switch(i) {
	case 0:
	case fileNotFound:
		break;
	default:
		errno = _mapErr(i);
		return NULL;
	}

DEBUG(("creating %s\n", expandedTo->bufString.text));
	/* create the new file */
	inforec.pCount = 5;
	inforec.pathname = &(expandedTo->bufString);
	inforec.access = 0x00c3;
	if ((flags & LC_COPY_REZ) == 0) {
		inforec.storageType = standardFile;
	}
	CreateGS(&inforec);
	if ((i = _toolErr) != 0) {
		errno = _mapErr(i);
		return NULL;
	}

DEBUG(("copying file\n"));
	/* copy both forks, if necessary */
	for (i=0; i<2; i++) {

		switch (i) {
		case 0:
			if ((flags & LC_COPY_DATA) == 0) {
				continue;
			}
			openrec.resourceNumber = 0x0000;
			break;
		case 1:
			if ((inforec.storageType != extendedFile) ||
			    ((flags & LC_COPY_REZ) == 0)) {
				continue;
			}
			openrec.resourceNumber = 0x0001;
			break;
		}

DEBUG(("doing fork %d (%s)\n", i, (i==0) ? "data", "rez"));

		/* open the input file */
		openrec.pCount = 4;
		openrec.pathname = &(expandedFrom->bufString);
		openrec.requestAccess = readEnable;
		OpenGS(&openrec);
		if ((j = _toolErr) != 0) {
			errno = _mapErr(j);
			return NULL;
		}
		refNumIn = openrec.refNum;

		/* open the output file */
		openrec.pathname = &(expandedTo->bufString);
		openrec.requestAccess = writeEnable;
		if (((flags & LC_COPY_REZ) == 0) &&
	            (openrec.storageType == 0x05)) {
	            	/* not copying resource fork; make it a standard file */
	            	openrec.storageType = 0x01;
		}
		OpenGS(&openrec);
		if ((j = _toolErr) != 0) {
			closerec.pCount = 1;
			closerec.refNum = refNumIn;
			CloseGS(&closerec);
			errno = _mapErr(j);
			return NULL;
		}
		refNumOut = openrec.refNum;

		/* transfer the data */
		done = 0;
		iobuf.pCount = 5;
		iobuf.dataBuffer = (Longword) buffer;
		iobuf.cachePriority = cacheOn;
		while (!done) {

			iobuf.refNum = refNumIn;
			iobuf.requestCount = bufferLen;
			ReadGS(&iobuf);
			k = _toolErr;
			switch (k) {
			case 0:
				break;
			case eofEncountered:
				k = 0;		/* no error condition */
				done = 1;
				break;
			default:
				/* leave k set as the error condition */
				done = 1;
				continue;
			}

			iobuf.refNum = refNumOut;
			iobuf.requestCount = iobuf.transferCount;
			WriteGS(&iobuf);
			if ((k = _toolErr) != 0) {
				/* leave k set as the error condition */
				done = 1;
			}
		}     /* end loop over buffering */

		closerec.pCount = 1;
		closerec.refNum = refNumIn;
		CloseGS(&closerec);
		closerec.refNum = refNumOut;
		CloseGS(&closerec);
		if (k != 0) {
			/* ending I/O before EOF is encountered? */
			errno = _mapErr(k);
			return NULL;
		}
	}	/* end loop over forks */

DEBUG(("setting file info\n"));

	/* set file information to match original file */
	inforec.pCount = 7;
	inforec.pathname = &(expandedTo->bufString);
	inforec.access = oldAccess;
	SetFileInfoGS(&inforec);
	if ((i = _toolErr) != 0) {
		errno = _mapErr(i);
		return NULL;
	}

	return &(expandedTo->bufString);
}

char *
LC_CopyFile (char *from, char *to, unsigned short flags)
{
	GSStringPtr fromGS, toGS, result;
	int e;

	if ((fromGS = __C2GSMALLOC(from)) == NULL) {
		return NULL;
	}
	if ((toGS = __C2GSMALLOC(to)) == NULL) {
		e = errno;
		GIfree(fromGS);
		errno = e;
		return NULL;
	}
	result = LC_CopyFileGS (fromGS, toGS, flags);
	e = errno;
	GIfree(fromGS);
	GIfree(toGS);
	errno = e;
#if 1
	/* get around a bug in ORCA/C v2.1.0 (reported) */
	if (result == NULL) {
		return NULL;
	} else {
		return result->text;
	}
#else
	return (result == NULL) ? NULL : result->text;
#endif
}
