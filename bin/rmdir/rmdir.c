/*
 * rmdir - remove directory
 *
 * ChangeLog:
 *	v1.1	- incorporated into GNO base distribution
 *		- added -p flag for POSIX conformance
 *		- moved rmdir(2) implementation to libc
 *	v1.0	- initial revision
 *
 * Version 1.1 by Devin Reade <gdr@myrias.ab.ca>
 */

#include <types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <err.h>
#include <gno/gno.h>
#include <gno/contrib.h>

#ifdef __STACK_CHECK__
static void
printStack (void) {
	fprintf(stderr, "stack usage: %d bytes\n", _endStackCheck());
}
#endif

static void
usage (void) {
	fprintf(stderr, "usage: rmdir [-p] directory ...\n");
	exit(1);
}

const char *nodup = "couldn't duplicate %s";

int
main(int argc, char **argv) {
	int c, result, pflag;
	char delim, *path, *root, *p, *q;

#ifdef __STACK_CHECK__
	_beginStackCheck();
	atexit(printStack);
#endif

	pflag = 0;
	while ((c = getopt (argc, argv, "p")) != EOF) {
		switch (c) {
		case 'p':
			pflag = 1;
			break;
		default:
			usage();
		}
	}

	if ((argc - optind) == 0) {
		usage();
	}
	result = 0;

	/* loop over all filenames */
	for (; optind<argc; optind++) {

		path = argv[optind];

		if (pflag == 0) {
			/*
			 * Just do the last directory component, and
			 * skip the mess below
			 */
			if (rmdir(path)!=0) {
				warn("%s skipped", path);
				result++;
			}

		} else {

			/* get the full pathname */
			if ((path = LC_ExpandPath (path)) == NULL) {
				warn("couldn't expand %s", argv[optind]);
				continue;
			}
			if ((q = strdup(path)) == NULL) {
				err(1, nodup, path);
			}
			path = q;

			/* what is the volume component? */
			q = (*path == ':') ? path+1 : path;
			q = strchr(q, ':');
			if (q != NULL) {
				*q = '\0';
			}

			if (*path == ':') {
				if ((root = strdup(path)) == NULL) {
					err(1, nodup, path);
				}
			} else {
				root = NULL;
			}
			if (q != NULL) {
				*q = ':';
			}
			
			for(;;) {
				if (*path == '\0') {
					/* deleted all the directories */
					break;
				}
				if ((root != NULL) && !strcmp(root, path)) {
					/* don't try to delete the volume */
					break;
				}
				if (rmdir(path)!=0) {
					warn("%s skipped", path);
					result++;
					break;
				}
				p = path + strlen(path) - 1;
				while (p >= path) {
					if (*p == ':') {
						*p = '\0';
						break;
					} else {
						*p-- = '\0';
					}
				}
			}	
		}
	}
	return result;
}
