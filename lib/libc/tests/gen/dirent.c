/*
 * Test by Devin Reade
 *
 * $Id: dirent.c,v 1.1 1997/02/28 05:12:52 gdr Exp $
 *
 */

#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

#define FAIL(arg) { fprintf(stderr,"%s (errno==%d)\n", arg, errno); exit(1); }

int main (int argc, char **argv) {
	DIR *dirp;
	struct dirent *ent;
	char *path;
	int count;
	long offset;

	if (argc != 2) {
		printf("usage: %s dir_name\n", argv[0]);
		exit(1);
	}
	path = argv[1];

	printf("opening %s\n", path);

	/* test opendir */
	if ((dirp = opendir(path)) == NULL) {
		FAIL("opendir failed");
	}

	/* test dirfd */
	printf("dirfd returns %d\n", dirfd(dirp));

	/* test readdir and telldir */
	count = 0;
	printf("current offset is %ld\n", telldir(dirp));
	while ((ent = readdir(dirp)) != NULL) {
		printf("\t%lu %hu %hu %hu %s\n", ent->d_fileno,
			ent->d_reclen, (unsigned short) ent->d_type,
			(unsigned short) ent->d_namlen, ent->d_name);
		if (count == 1) {
			offset = telldir(dirp);
			printf("count %d has offset %ld\n", count, offset);
		}
		count++;
	}

	/* test seekdir */
	printf("seeking to saved offset\n");
	seekdir(dirp, offset);
	if ((ent = readdir(dirp)) != NULL) {
		printf("\t%lu %hu %hu %hu %s\n", ent->d_fileno,
			ent->d_reclen, (unsigned short) ent->d_type,
			(unsigned short) ent->d_namlen, ent->d_name);
	}

	/* test rewinddir */
	printf("rewinding directory:\n");
	rewinddir(dirp);
	while ((ent = readdir(dirp)) != NULL) {
		printf("\t%lu %hu %hu %hu %s\n", ent->d_fileno,
			ent->d_reclen, (unsigned short) ent->d_type,
			(unsigned short) ent->d_namlen, ent->d_name);
	}

	/* test closedir */
	printf("closedir returns %d\n", closedir(dirp));
	return 0;
}
