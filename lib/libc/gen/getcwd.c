#line 1 ":trenco4:gno.src:lib:libc:gen:getcwd.c"
/*
 * getwd originally by Derek Taubert.  First appeared in GNO v1.0 (?).
 * Modified for BSD 4.4 compatibility and dynamically allocated structs
 * by Devin Reade.
 *
 * $Id: getcwd.c,v 1.1 1997/02/28 05:12:44 gdr Exp $
 *
 * This file is formatted for tabs every 8 columns.
 */

#pragma optimize 0
#pragma debug 0
#pragma memorymodel 0

#ifdef __ORCAC__
segment "libc_gen__";
#endif

#include <sys/param.h>
#include <sys/types.h>
#include <types.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <orca.h>
#include <gno/gno.h>
#include <gsos.h>

char *
getcwd(char *pathname, size_t size) {
    PrefixRecGS *prefx;
    ResultBufPtr where;
    int e, allocated, i;
    char *result;

    if (size == 0 && pathname != NULL) {
    	errno = EINVAL;
    	return NULL;
    }
    if ((prefx = malloc (sizeof(PrefixRecGS))) == NULL) {
	return NULL;
    }
    if (pathname == NULL) {
    	size = MAXPATHLEN;
    	if ((pathname = malloc(size)) == NULL) {
	    e = errno;
    	    free(prefx);
    	    errno = e;
    	    return NULL;
    	}
    	allocated = 1;
    } else {
    	allocated = 0;
    }
    result = pathname;
    if ((where = GOinit (size, NULL)) == NULL) {
    	e = errno;
	free(prefx);
	if (allocated) free(pathname);
	errno = e;
	return NULL;
    }
	
    prefx->pCount = 2;
    prefx->buffer.getPrefix = (ResultBuf255Ptr) where;
    for (i=0; i<2; i++) {
	switch (i) {
	case 0:
	    prefx->prefixNum = 0;
	    break;
	case 1:
	    prefx->prefixNum = 8;
	    break;
	}
	GetPrefixGS(prefx);
	if (i == 0 && _toolErr == 0 && where->bufString.length == 0) {
	    /* prefix 0 not set */
	    continue;
	} else if ((e = _toolErr) != 0) {
	    e = (e == buffTooSmall) ? ERANGE : _mapErr(e);
 	    result = NULL;
 	    break;
	} else {
	    e = errno;
	    strncpy(pathname, where->bufString.text, where->bufString.length);
	    pathname[where->bufString.length] = 0;
    	    if (pathname[where->bufString.length-1] == ':') {
		pathname[where->bufString.length-1] = '\0';
	    }

	    /* convert the filename? */
	    if (_mapPath(pathname) == NULL) {
		e = EINVAL;
		result = NULL;
	    }
	    break;
        }    	
	/* NOTREACHED */
    }
    free(prefx);
    GOfree(where);
    if (allocated && result == NULL) {
    	free(pathname);
    }
    errno = e;
    return result;
}

char *
getwd (char *pathname) {
    return getcwd(pathname, MAXPATHLEN);
}
