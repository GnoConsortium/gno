/*
 * chtyp
 *
 * Sets the file/aux type for GS/OS files.
 *
 * $Id: chtyp.c,v 1.1 1997/10/03 05:06:50 gdr Exp $
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gsos.h>

#include "ftypes.h"

void usage(void);
void version(void);

#ifdef __STACK_CHECK__
#include <gno/gno.h>

static void cleanup(void)
{
    (void) fprintf(stderr, "Stack Usage: %d.\n", _endStackCheck());
}
#endif

static GSString255 path;

int main(int argc, char **argv)
{
    char *ftype = NULL;
    char *atype = NULL;
    char *lang = NULL;
    char *left = NULL;
    int filet = -1;
    long auxt = -1;
    int ch;
    FileInfoRecGS finfo = {4, &path};

#ifdef __STACK_CHECK__
    atexit(cleanup);
    _beginStackCheck();
#endif

    while((ch = getopt(argc, argv, "t:a:l:vV")) != -1)
	switch(ch) {
	    case 't':
		if(lang) {
		    (void) fprintf(stderr,
				 	"chtyp: -t cannot be used with -l\n");
		    usage();
		}
		ftype = optarg;
		break;
	    case 'a':
		if(lang) {
		    (void) fprintf(stderr, 
					"chtyp: -a cannot be used with -l\n");
		    usage();
		}
		atype = optarg;
		break;
	    case 'l':
		if(ftype || atype) {
		    (void) fprintf(stderr, 
					"chtyp: -l cannot be used with %s%s\n",
					ftype ? "-t" : "-a", 
					ftype && atype ? " or -a." : ".");
		    usage();
		}
		lang = optarg;
		break;
	    case 'v':
	    case 'V':
		version();
		break;
	    default:
		usage();
		break;
	}

    argc -= optind;
    argv = argv + optind;

    if(argc == 0) {
	(void) fprintf(stderr, "chtyp: no files specified\n");
	usage();
    }

    if(lang) {
	if(find_lang(lang, &filet, &auxt) == -1) {
	    (void) fprintf(stderr, "chtyp: unknown language argument \"%s\".\n",
				lang);
	    usage();
	}
    }

    if(ftype) {
	filet = (int) strtol(ftype, &left, 0);
	if(errno == ERANGE || *left) 
	    if(find_type(ftype, &filet, &auxt) == -1) {
		(void) fprintf(stderr,
				"chtyp: invalid argument to -t option.\n");
		usage();
	    }
    }

    if(atype) {
	errno = 0;		/* be sure to reset errno! */
	auxt = strtol(atype, (char **) NULL, 0);
	if(errno == ERANGE) {
	    (void) fprintf(stderr, "chtyp: invalid argument to -a option.\n");
	    usage();
	}
    }

    ch = 0;
    while(argc) {
	strcpy(path.text, argv[ch]);
	path.length = strlen(argv[ch++]);
	GetFileInfoGS(&finfo);
	if(filet != -1)
	    finfo.fileType = filet;
	if(auxt != -1)
	    finfo.auxType = auxt;
	SetFileInfoGS(&finfo);
	argc--;
    }
}

static void usage(void)
{
    (void) fprintf(stderr,
     "Usage: chtyp { [-t type] [-a auxtype] } | { [-l lang] } file ...\n");
    exit(-1);
}

static void version(void)
{
    (void) fprintf(stderr,
		"V 2.0.1 Copyright 1997 by Evan Day, day@engr.orst.edu\n");
    usage();
}
