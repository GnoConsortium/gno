/*
 * Usage:  newer file1 file2
 *
 * Returns zero if both files exist and file1 is newer than file1, otherwise
 * returns 1.  If file2 does not exist, an error message is printed
 * as well.
 *
 * Devin Reade, November 1997
 *
 * $Id: newer.c,v 1.1 1997/11/24 05:46:29 gdr Exp $
 */

#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

int 
main (int argc, char **argv) {
  struct stat sbuf1, sbuf2;
  char *file1, *file2;

  if (argc != 3) {
    fprintf(stderr, "Usage: %s file1 file2\n", argv[0]);
    return 1;
  }
  file1 = argv[1];
  file2 = argv[2];

  if (stat(file2, &sbuf2) < 0) {
    fprintf(stderr, "couldn't stat %s: %s\n", file2, strerror(errno));
    return 1;
  }

  if (stat(file1, &sbuf1) < 0) {
    return 1;
  }

  return (sbuf1.st_mtime > sbuf2.st_mtime) ? 0 : 1;
}
