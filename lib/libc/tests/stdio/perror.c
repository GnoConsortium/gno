/*
 * Test by Devin Reade
 *
 * $Id: perror.c,v 1.2 1997/07/28 00:20:37 gdr Exp $
 */

#pragma debug 25

#include <stdio.h>
#include <errno.h>

int main (int argc, char **argv) {
  int i;
  
  fprintf(stderr, "ELAST is %d\n", ELAST);
  if (argc > 1) {
    for (i=0; i<sys_nerr; i++) {
      fprintf(stderr, "loop %d\t\"%s\"\n", i, sys_errlist[i]);
    }
  } else {
    for (i=0; i<sys_nerr; i++) {
      fprintf(stderr, "loop %d\t", i);
      errno = i;
      perror("testing perror");
    }
  }                                    
  return 0;
}
