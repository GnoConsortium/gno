/*
 * This test tries to execute a shell script directly (via execv(2)).
 * This *should* work.  Under GNO v2.0.4 it doesn't.
 *
 * Devin Reade, 1997
 *
 * $Id: script.c,v 1.1 1997/07/27 23:58:57 gdr Exp $
 */

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

int 
main (int argc, char **argv) {

  char *array[2];

  if (argc != 2) {
    printf("usage: %s /fully/qualified/script/name\n", argv[0]);
    return 1;
  }

  array[0] = argv[1];
  array[1] = NULL;

  execv(argv[1], array);
  printf("exec of %s failed: %s\n", argv[1], strerror(errno));
  return 1;
}
