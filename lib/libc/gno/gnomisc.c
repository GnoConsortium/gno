/*
 * $Id: gnomisc.c,v 1.2 1997/09/21 06:07:24 gdr Exp $
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

static char *unknown = "(unknown)";
static GetNameRecGS namerec = { 1, NULL };

char *
__prognameGS (void) {
	namerec.pCount = 1;
	if (namerec.dataBuffer == NULL) {
		namerec.dataBuffer =
			(ResultBuf255Ptr) GOchange (NULL, NAME_MAX, NULL);
		if (namerec.dataBuffer == NULL) {
			return unknown;
		}
		GetNameGS(&namerec);
		if (_toolErr) {
			GOfree(namerec.dataBuffer);
			namerec.dataBuffer = NULL;
			return unknown;
		}
		/* NULL-terminate it */
		namerec.dataBuffer->bufString.text
			[namerec.dataBuffer->bufString.length] = '\0';
	}
	return namerec.dataBuffer->bufString.text;
}

void
WriteGString (GSStringPtr gp) {
	/* perhaps this should call WriteGS directly */
	write(STDERR_FILENO, gp->text, gp->length);
}
