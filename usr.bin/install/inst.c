/*
 * Copyright 1996-1997 Devin Reade <gdr@trenco.gno.org>.
 * All rights reserved.
 *
 * For copying and distribution information, see the file "COPYING"
 * accompanying this file.
 *
 * $Id: inst.c,v 1.5 1998/04/24 00:54:03 gdr-ftp Exp $
 */

#define VERSION "1.2"
#define EMAIL   "<gdr@trenco.gno.org>"

#define	__USE_DYNAMIC_GSSTRING__

#include <sys/types.h>
#include <types.h>
#include <sys/stat.h>
#include <gno/gno.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <orca.h>
#include <gsos.h>
#include <err.h>
#include <gno/contrib.h>

/* actions */
#define ACTION_CHANGE_MODE	0x0001
#define	ACTION_CHANGE_OWNER	0x0002
#define ACTION_CHANGE_GROUP	0x0004

/* permissions */
#define S_USER    0700
#define S_GROUP   0070
#define S_OTHER   0007
#define S_ALL     0777
#define S_READ    0444
#define S_WRITE   0222
#define S_EXECUTE 0111

char *versionMsg = "Version %s by Devin Reade %s\n";
int dFlag, verbose;

/*
 * usage
 *
 * display usage information and exit
 */

void
usage(void)
{
  fputs("Usage: install [-cdhsvV] [-o owner] [-g group] [-m mode] ", stderr);
  fputs("source [...] dest\n\n", stderr);
  fputs("Options:\n", stderr);
  fputs("\t-c             Ignored.  (Backwards Unix compatibility)\n", stderr);
  fputs("\t-d             Create the specified directories\n", stderr);
  fputs("\t-g group       Specify group id (not implemented)\n", stderr);
  fputs("\t-h             Show usage information and exit.\n", stderr);
  fputs("\t-m mode        Specify (Unix) access mode\n", stderr);
  fputs("\t-o owner       Specify owner id (not implemented)\n", stderr);
  fputs("\t-s             Strip binary (not implemented)\n", stderr);
  fputs("\t-V             Print each file as it is created (verbose).\n",
	stderr);
  fputs("\t-v             Show version number\n\n", stderr);
  fprintf(stderr, versionMsg, VERSION, EMAIL);
  exit(1);
}

/*
 * translateMode
 *
 * set mode to the value corresponding to the permission bit string
 * <str>.  If the first char of <str> is a digit, then it is assumed
 * to be an octal number.  Otherwise it is assumed to be a string of
 * the form "ug+rx" (in the usual chmod(1) format).
 *
 * If these assumptions don't hold, then return non-zero.  Returns
 * zero and sets mode on success.  mode may be modified, even on
 * failure.
 */

static int
translateMode(char *str, mode_t *mode)
{
	mode_t who = 0L;
	mode_t perm = 0L;
	char *p, *q;

	/* octal number? */
	if (isdigit(*str)) {
		errno = 0;
		*mode = strtoul(str, NULL, 8);
		return errno;
	}

	/* it's not an absolute octal; treat as a string */
	if (((p = strchr(str, '=')) == NULL) &&
	    ((p = strchr(str, '+')) == NULL) &&
	    ((p = strchr(str, '-')) == NULL)) {
		errno = EINVAL;
		return errno;
	}

	/*
	 * Since we use mode 0000 as our starting point, using '-' in the
	 * symbolic mode is equivalent to setting the mode to 0000 (absolute).
	 * Similarly, using '+' and '=' are equivalent.
	 *
	 * If '-' was specified, ignore the "ugo" and "rwx" parts of the string.
	 */
	if (*p == '-') {
		*mode = 0000;
		return 0;
	}
	*p++ = '\0';

	if (*str == '\0') {
		/* this should probably be derived from the umask */
		who |= S_ALL;
	} else {
		for (q = str; *q; q++) {
			switch (*q) {
			case 'u':	who |= S_USER;		break;
			case 'g':	who |= S_GROUP;		break;
			case 'o':	who |= S_OTHER;		break;
			case 'a':	who |= S_ALL;		break;
			default:	errno = EINVAL;		return errno;
			}
  		}
	}

	if (*p == '\0') {
		/* this should probaby be taken from the umask */
		perm |= S_READ | S_WRITE | S_EXECUTE;
	} else {
		for (; *p; p++) {
			switch (*p) {
			case 'r':	perm |= S_READ;		break;
			case 'w':	perm |= S_WRITE;	break;
			case 'x':	perm |= S_EXECUTE;	break;
			case 's':	perm |= S_EXECUTE;	break;
			default:	errno = EINVAL;		return errno;
			}
		}
	}

	*mode = who & perm;
	return 0;
}

/*
 * mkdirs
 *
 * argv is assumed to be an array of argc pathnames.  mkdirs will
 * create all the listed directories, including their parents if
 * necessary.
 *
 * Returns zero on success.  Returns non-zero and prints a suitable
 * error message on failure.
 */

int
mkdirs(int argc, char **argv)
{
  static struct stat statbuf;	/* reduce stack space */
  char *path, *p;
  size_t pathlen;
  int result = 0;
  int makeit;		/* do we try a mkdir()? */
  int abortpath;	/* we saw an error; don't both with rest of path */
  int i, j;
  int coloncount, isVolume;

  /* loop over each of the pathnames in the array */
  for (i = 0; i < argc; i++) {

    /* expand to a full pathname */
    if ((path = LC_ExpandPath(argv[i])) == NULL) {
      warn("couldn't expand the pathname of %s (ignored)", argv[i]);
      continue;
    }
    pathlen = strlen(path);

    /* is this pathname legal? */
    /* place a call to JudgeName() [custom] here */

    /* find out how many path components there are */
    coloncount = 0;
    p = path;
    while (*p) {
      if (*p == ':') {
	coloncount++;
      }
      p++;
    }

    /* skip the first part if it's a volume or device name */
    abortpath = 0;
    if ((*path == ':') || (*path == '.')) {
      j = 1;
      p = path + 1;
      if ((p = strchr(p, ':')) == NULL) {
        p = path + pathlen;
        abortpath = 1;
      } else {
        p++;
      }
    } else {
      j = 0;                                   
      p = path;
    }                                                          

    /* create each component in path */
    for (; !abortpath && j < coloncount; j++) {
      if ((p = strchr(p, ':')) == NULL) {
	p = path + pathlen;
      }
      *p = '\0';

      if (stat(path, &statbuf) != 0) {
	if (errno == ENOENT) {
	  makeit = 1;
	} else {
          warn("couldn't stat %s", path);
	  makeit = 0;
	  abortpath = 1;
	  result = 1;
	}
      } else {
	makeit = 0;
	if (statbuf.st_mode & S_IFDIR == 0) {
	  warnx("%s exists and is not a directory\n", path);
	  abortpath = 1;
	  result = 1;
	}			/* else it exists and is a directory */
      }

      /* go ahead and create the directory */
      if (makeit) {
      	if (verbose) {
      	  fprintf(stderr, "%s\n", path);
      	}
        if (mkdir(path)) {
	  warn("couldn't create directory %s", path);
	  abortpath = 1;
	  result = 1;
        }
      }
      /* reinstate the ':' that we "nulled-out" */
      if (p != path + pathlen) {
	*p++ = ':';
      }
    }
  }
  return result;
}

/*
 * copyfiles
 *
 * <argv> is assumed to be an array of <argc> filenames.
 *
 * This routine copies all but the last specified file to the directory
 * or filename specified by the last filename.  If argc>2, the last element
 * _must_ be a directory.
 *
 * Returns zero on success.  On failure, returns the last non-zero errno
 * and prints error conditions to stderr via the warn(3) routines.
 *
 * If the ACTION_CHANGE_MODE bit is set in <action>, chmod(2) is invoked
 * on the resulting file.  ACTION_CHANGE_USER and ACTION_CHANGE_GROUP
 * are currently ignored.
 */

static int
copyfiles(int argc, char **argv, int action, mode_t mode)
{
	static FileInfoRecGS inforec;
	GSStringPtr src, dest, newname;
        int total, i, result, printWhen;
	unsigned short flags;
#define PRINT_NEVER  0	/* used for printWhen (verbose mode) */
#define PRINT_BEFORE 1
#define PRINT_AFTER  2

	static const char *nodup = "couldn't duplicate %s file name \"%s\"";

	assert(argc > 1);

	/* initialization */
	result = 0;
	total = argc - 1;
	flags = LC_COPY_DATA | LC_COPY_REZ | LC_COPY_KEEPBUF;

	/* duplicate the destination name */
	if ((dest = __C2GSMALLOC(argv[total])) == NULL) {
		err(1, nodup, "destination", argv[total]);
		/*NOTREACHED*/
	}

	/*
	 * If we're copying more than one file, make sure the last
	 * is a directory.
	 */
	if (argc > 2) {
		inforec.pCount = 5;
		inforec.pathname = dest;
		GetFileInfoGS(&inforec);
		if (_toolErr != 0) {
			errno = _mapErr(_toolErr);
			err(1, "couldn't stat %s", dest->text);
			/*NOTREACHED*/
		}
		if ((inforec.storageType != 0x0D) &&
		    (inforec.storageType != 0x0F)) {
			errno = ENOTDIR;
			errx(1, "%s is not a directory", dest->text);
		}
	}

	/* copy each file */
	if (verbose) {
		printWhen = (argc == 2) ? PRINT_BEFORE : PRINT_AFTER;
	} else {
		printWhen = PRINT_NEVER;
	}
	for (i=0; i<total; i++) {
		if (printWhen == PRINT_BEFORE) {
			fprintf(stderr, "%s\n", argv[total]);
		}
	        if ((src = __C2GSMALLOC(argv[i])) == NULL) {
			err(1, nodup, "source", argv[i]);
			/*NOTREACHED*/
		}
		newname = LC_CopyFileGS(src, dest, flags);
		if (newname == NULL) {
			result = errno;
			warn("install of %s to %s failed", argv[i],
			     argv[total]);
		} else {
			if (printWhen == PRINT_AFTER) {
				fprintf(stderr, "%s\n", newname->text);
			}
			if (action & ACTION_CHANGE_MODE) {
				if (chmod(newname->text, mode) < 0) {
					result = errno;
					warn("couldn't change %s to mode %o",
					     newname->text, mode);
				}
			}
		}
		GIfree(src);
	}
	GIfree(dest);
	return result;
}

/*
 * obvious ...
 */

int
main(int argc, char **argv)
{
  mode_t newmode;
  int c, nfiles;
  int action = 0;

  __REPORT_STACK();
  
  if (needsgno() == 0) {
	errx(1, "requires GNO");
  }

  /* initialize */
  dFlag = 0;
  newmode = 0L;

  /* parse command line */
  while ((c = getopt(argc, argv, "cdg:hm:o:svV")) != EOF) {
    switch (c) {
    case 'v':
      errx(1, versionMsg, VERSION, EMAIL);
      /*NOTREACHED*/
      break;

    case 'm':
      action |= ACTION_CHANGE_MODE;
      if (translateMode(optarg, &newmode)) {
	usage();
      }
      break;

    case 'V':
      verbose=1;
      break;
      
    case 'd':  dFlag++;
    case 'c':			/* not implemented */
    case 'g':			/* not implemented */
    case 'o':			/* not implemented */
    case 's':			/* not implemented */
      break;

    case 'h':
    default:   usage();
    }
  }

  nfiles = argc - optind;
#if 1
  if (nfiles < (dFlag ? 1 : 2)) {
    usage();
  }

  c = 0;		/* using c as a temp variable */
  BeginSessionGS(&c);
  if (_toolErr) {
  	errno = _mapErr(_toolErr);
  	err(EXIT_FAILURE, "BeginSessionGS failed (0x%x)", _toolErr);
  }
  if (dFlag) {
    c = mkdirs(nfiles, &argv[optind]);
  } else {
    /* ignore new user and group for now */
    c = copyfiles(nfiles, &argv[optind], action, newmode);
  }
  action = 0;			/* using action as a temp variable */
  EndSessionGS(&action);
  if (_toolErr) {
  	errno = _mapErr(_toolErr);
  	err(EXIT_FAILURE, "EndSessionGS failed (0x%x)", _toolErr);
  }
#else

  if (dFlag) {
    if (nfiles < 1) {
      usage();
    }
    c = mkdirs(nfiles, &argv[optind]);
  } else {
    if (nfiles < 2) {
      usage();
    }
    /* ignore new user and group for now */
    c = copyfiles(nfiles, &argv[optind], action, newmode);
  }
#endif

  return c;
}
