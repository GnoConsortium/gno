/*
 * $Id: expandpath.c,v 1.1 1997/10/30 04:57:25 gdr Exp $
 */

/* Change the arg types of GS/OS parameter blocks. Must be before #includes */
#define __USE_DYNAMIC_GSSTRING__

#include <types.h>
#include <gsos.h>
#include <errno.h>
#include <orca.h>
#include <gno/gno.h>
#include "contrib.h"

/*
 * expandpath
 *
 * Uses the GS/OS facilities to expand the pathname <path>.  On
 * success, returns a pointer to the malloc'd expanded path.  On
 * failure it will return NULL and set errno.
 *
 * Note that in using this function, all directory separators will
 * be converted to colons.
 *
 * *********** THIS ROUTINE IS NOT REENTRANT **************
 */

#define OUTBUF_QUANTUM	255

GSStringPtr
LC_ExpandPathGS (GSStringPtr path)
{
	static ExpandPathRecGS	expand;
	static ResultBufPtr	outBuf = NULL;
	static int outBufSize = 0;
	int i;
        
	expand.pCount = 2;
	expand.inputPath = path;
	expand.flags = 0x0000;
	if (outBuf == NULL) {
		outBufSize += OUTBUF_QUANTUM;
		outBuf = GOinit(outBufSize, NULL);
		if (outBuf == NULL) {
			errno = ENOMEM;
			return NULL;
		}
	}
	for(;;) {
		expand.outputPath = outBuf;
		ExpandPathGS(&expand);
		switch (_toolErr) {
		case 0:
			/* NULL-terminate it and return */
			(outBuf->bufString.text)[outBuf->bufString.length]
				= '\0';
			return &(outBuf->bufString);
			break;
		case buffTooSmall:
			break;
		default:
			errno = _mapErr(_toolErr);
			return NULL;
		}
		/* if we got here, the buffer wasn't large enough */
		
		outBufSize += OUTBUF_QUANTUM;
		outBuf = GOchange(outBuf, outBufSize, NULL);
        }
}

char *          
LC_ExpandPath (char *path)
{               
	GSStringPtr inStr, result;
	int e;

	if ((inStr = __C2GSMALLOC(path)) == NULL) {
		return NULL;
	}
	result = LC_ExpandPathGS(inStr);
	e = errno;
	GIfree(inStr);
	errno = e;
	if (result) {
		return result->text;
	} else {
		return NULL;
	}
}
