/*
 * tests simply that we can create a pipe.  It also fstat's the result.
 *
 * $Id: pipe1.c,v 1.1 1997/09/21 18:11:42 gdr Exp $
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
	int pdes[2], i, pval;
  struct stat sbuf;

  if (pipe(pdes) < 0) {
	   perror("pipe failed");
     exit(1);
  }

  for (i=0; i<2; i++) {
	   if (fstat(pdes[i], &sbuf) < 0) {
	      perror("stat failed");
	      exit(1);
     }
     printf("fd %d: st_mode is 0x%lx\n", i, (unsigned long) sbuf.st_mode);
  }
  return 0;
}
