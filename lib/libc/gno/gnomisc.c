/*
 * $Id: gnomisc.c,v 1.3 1998/10/31 17:22:05 gdr-ftp Exp $
 *
 * This file is formatted with tabs every 8 characters.
 */

#ifdef __ORCAC__
segment "libc_gno__";
#endif

#pragma optimize 78	/* optimization breaks this file (79, 15 tried) */

#include <sys/syslimits.h>
#include <unistd.h>
#include <gsos.h>
#include <gno/gno.h>

int
needsgno(void) {
	kernStatus();
	if (_toolErr) {
		return 0;
	} else {
		return 1;
	}
}

char *	__progname = "(unknown)";
static GetNameRecGS namerec = { 1, NULL };

char *
__prognameGS (void) {
	namerec.pCount = 1;
	if (namerec.dataBuffer == NULL) {
		namerec.dataBuffer =
			(ResultBuf255Ptr) GOchange (NULL, NAME_MAX, NULL);
		if (namerec.dataBuffer == NULL) {
			/* we can't get it now, we likely can't get it later */
			namerec.dataBuffer = NULL;
			return __progname;
		}
		GetNameGS(&namerec);
		if (_toolErr) {
			GOfree(namerec.dataBuffer);
			namerec.dataBuffer = NULL;
			return __progname;
		}
		/* NULL-terminate it */
		namerec.dataBuffer->bufString.text
			[namerec.dataBuffer->bufString.length] = '\0';
		__progname = namerec.dataBuffer->bufString.text;
	}
	return __progname;
}

void
WriteGString (GSStringPtr gp) {
	/* perhaps this should call WriteGS directly */
	write(STDERR_FILENO, gp->text, gp->length);
}
