/*
 * Test by Devin Reade
 *
 * $Id: utime.c,v 1.1 1997/02/28 05:12:52 gdr Exp $
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
  return 0;
}
