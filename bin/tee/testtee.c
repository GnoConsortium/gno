/*
 * This is a test for tee(1) buffering.  It prints out LINES lines
 * of DIGITS digits each, sleeping for SLEEP seconds between digits.
 *
 * Usage:  ./testtee | ./tee [-b]
 *
 * $Id: testtee.c,v 1.1 1996/09/09 06:12:16 gdr Exp $
 */

#include <stdio.h>
#include <unistd.h>

#define LINES  3     /* number of lines to print */
#define DIGITS 4     /* digits per line */
#define SLEEP  1     /* seconds to sleep between digits */

int
main(int argc, char **argv)
{
  int i, j;

  setvbuf(stdout, NULL, _IONBF, 0);
  for (i=0; i<LINES; i++) {
    for (j=0; j<DIGITS; j++) {
      printf(" %d", j);
      sleep(SLEEP);
    }
    printf("\n");
  }
  return 0;
}
