#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fts.h>
#include <assert.h>

int main (int argc, char **argv) {

	FTS *ftsp;
	FTSENT *entp;
	int options;

	if (argc < 2) {
		printf("usage: %s dirname\n", argv[0]);
		exit(1);
	}
	options = FTS_NOCHDIR | FTS_NOSTAT;
	ftsp = fts_open(&argv[1], options, NULL);
	if (ftsp == NULL) {
		printf("fts_open failed\n");
		exit(1);
	}
	
	while ((entp = fts_read(ftsp)) != NULL) {
		assert(entp->fts_name != NULL);
		printf("%s\n", entp->fts_name);
	}
	if (errno) {
		perror("fts_read return code");
	}

	options = fts_close(ftsp);
	printf("fts_close returned %d\n", options);
	return 0;
}
