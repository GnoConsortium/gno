/*
 * Test by Devin Reade
 *
 * $Id: stat.c,v 1.1 1997/02/28 05:12:58 gdr Exp $
 */


#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

#define FAIL(msg) {printf("%s\n", msg); exit(1); }

int main(int argc, char **argv) {
	struct stat sb;
  char *path;

	if (argc != 2) {
	   printf("usage: %s filename\n", argv[0]);
	   exit(1);
  }
  path = argv[1];

	if (stat (path, &sb) == -1) {
	   FAIL("stat failed");
  }

  printf("file\t= %s\n", path);
  printf("device\t= %#4x\n", sb.st_dev);
  printf("mode\t= %#4x\n", sb.st_mode);

  return 0;
}
