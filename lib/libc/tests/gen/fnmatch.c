/*
 * Test for fnmatch(3) written by Devin Reade and Steve Reeves
 *
 * $Id: fnmatch.c,v 1.2 1999/11/22 05:45:57 stever Exp $
 */

#include <fnmatch.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void
usage(char *progname)
{
	fprintf(stderr, "Usage: %s [flags] pattern filename...\n"
		"   Valid flags are: -noescape, -pathname, -period, "
		"-casefold, -leading_dir\n", progname);
	exit(1);
}

int
main (int argc, char **argv) {
	int result, flags, i;
	char *pat;

	if (argc < 3)
		usage(argv[0]);

	flags = 0;
	for (i = 1; i < argc; i++) {
		if (argv[i][0] != '-' || !strcmp(argv[i], "--"))
			break;
		if (!strcmp(argv[i], "-noescape"))
			flags |= FNM_NOESCAPE;
		else if (!strcmp(argv[i], "-pathname"))
			flags |= FNM_PATHNAME;
		else if (!strcmp(argv[i], "-period"))
			flags |= FNM_PERIOD;
		else if (!strcmp(argv[i], "-casefold"))
			flags |= FNM_CASEFOLD;
		else if (!strcmp(argv[i], "-leading_dir"))
			flags |= FNM_LEADING_DIR;
		else {
			fprintf(stderr, "%s: bad flag: %s\n", argv[0], argv[i]);
			usage(argv[0]);
		}
	}

	if (i+1 >= argc)
		usage(argv[0]);

	pat = argv[i];
	for (++i; i < argc; i++) {
		result = fnmatch(pat, argv[i], flags);
		printf("'%s' %s pattern '%s'\n", argv[i],
			(result == FNM_NOMATCH) ? "does not match" : "matches",
			pat);
	}

	return 0;
}
