/*
 * This shows the defined values for STDIN_FILENO, STDOUT_FILENO,
 * and STDERR_FILENO.  These will differ for GNO and ORCA.
 *
 * Devin Reade, 1997.
 *
 * $Id: showfds.c,v 1.1 1997/07/27 23:50:59 gdr Exp $
 */

#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv) {
  printf("stdin:   %d %d\n", STDIN_FILENO, fileno(stdin));
  printf("stdout:  %d %d\n", STDOUT_FILENO, fileno(stdout));
  printf("stderr:  %d %d\n", STDERR_FILENO, fileno(stderr));
  return 0;
}
