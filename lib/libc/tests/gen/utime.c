/*
 * Test by Devin Reade
 *
 * $Id: utime.c,v 1.2 1998/02/04 07:26:53 gdr-ftp Exp $
 */

#include <sys/types.h>
#include <utime.h>
#include <stdio.h>
#include <err.h>
#include <time.h>
#include <gno/gno.h>

int main(int argc, char **argv) {
  struct utimbuf utb;

	if (argc != 2) {
	   errx(1, "usage: %s filename", __prognameGS());
  }

  if ((utb.actime = time(NULL)) == -1) {
	   err(1, "time failed! (shouldn't happen)");
  }
  utb.modtime = utb.actime;

  if (utime(argv[1], &utb) == -1) {
	   err(1, "utime for %s failed", argv[1]);
  }
  printf("passed\n");
  printf("access time:       %s\n", ctime(&utb.actime));
  printf("modification time: %s\n", ctime(&utb.modtime));
  return 0;
}
