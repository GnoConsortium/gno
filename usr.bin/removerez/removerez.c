/*
 * This is supposed to emulate the behavior of the ORCA/Shell script
 * 'removerez'.
 *
 * Devin Reade, February 1998
 *
 * $Id: removerez.c,v 1.1 1998/03/31 03:32:59 gdr-ftp Exp $
 */

#define __USE_DYNAMIC_GSSTRING__

#include <types.h>
#include <errno.h>
#include <stdlib.h>
#include <err.h>
#include <stdio.h>
#include <gsos.h>
#include <unistd.h>
#include <gno/gno.h>
#include <gno/contrib.h>

int
main (int argc, char **argv) {
	static FileInfoRecGS finfo;
	int i, quiet, usage, verbose;
	char *filename, *tempfile, *dname;
	GSStringPtr filenameGS, tempfileGS;
	unsigned short copyflags;

	__REPORT_STACK();

	/* initialization */
	quiet = usage = verbose = 0;
	copyflags = LC_COPY_DATA | LC_COPY_KEEPBUF | LC_COPY_BACKUP;
	while ((i = getopt(argc, argv, "qv")) != EOF) {
		switch(i) {
		case 'v':
			quiet = 0;
			verbose = 1;
			break;
		case 'q':
			quiet = 1;
			verbose = 0;
			break;
		default:
			usage = 1;
		}
	}
	if (usage || (argc-optind)<2) {
		errx(EXIT_FAILURE, "usage: %s file [...]\n", argv[0]);
	}
	for (i=optind; i<argc; i++) {
		/*
		 * Create a GSString version of the filename and temporary
		 * file.  We assume that we're not going to be operating
		 * on a large number of files, and therefore that the cost
		 * of the mallocs and frees are inconsequential.
		 */
		filename = argv[i];
		if (verbose) {
			warnx("working on %s", filename);
		}
		dname = dirname(filename);
		if ((tempfile = tempnam(dname, "rr.")) == NULL) {
			err(EXIT_FAILURE, "couldn't create temp file name");
		}
		tempfileGS = __C2GSMALLOC(tempfile);
		filenameGS = __C2GSMALLOC(filename);

		/*
		 * A bit of an optimization here.  Make sure there is
		 * actually a resource fork before we try to remove it.
		 */
		finfo.pCount = 5;
		finfo.pathname = filenameGS;
		GetFileInfoGS(&finfo);
		if (_toolErr) {
			errno = _mapErr(_toolErr);
			warn("couldn't stat %s", filename);
			continue;
		}
		if (finfo.storageType != 0x05) {
			if (!quiet) {
				warnx("%s is not an extended file; skipped",
				      filename);
			}
			continue;
		}

		/*
		 * Copy only the data fork to the temporary file. Retain
		 * all attributes.
		 */
		if (LC_CopyFileGS(filenameGS, tempfileGS, copyflags) == NULL) {
			unlink(tempfile);
			err(EXIT_FAILURE, "copy of %s failed", filename);
		}

		/* unlink the original and rename the temporary one back */
		if (unlink(filename) != 0) {
			err(EXIT_FAILURE, "unlink of %s failed", filename);
		}
		if (rename(tempfile, filename) != 0) {
			err(EXIT_FAILURE, "unable to rename %s to %s; manual "
			    "recovery required", tempfile, filename);
		}
		
	    again:
	    	/* clean up before the next loop */
	    	free(tempfile);
	    	GIfree(tempfileGS);
	    	GIfree(filenameGS);
	}
	return EXIT_SUCCESS;
}
