/*
 * $Id: creat.c,v 1.1 1997/02/28 05:12:57 gdr Exp $
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define FILE1 "data/creat1"
#define FILE2 "data/creat2"

int
main(int argc, char **argv) {
	int i;

  if ((i = creat(FILE1, 0644)) == -1) {
	   printf("mode 0644 failed: %d\n", errno);
     exit(1);
  } else {
	   close(i);
  }

  if ((i = creat(FILE2, 0444)) == -1) {
	   printf("mode 0444 failed: %d\n", errno);
     unlink(FILE1);
     exit(1);
  } else {
	   close(i);
  }

  unlink(FILE1);
  unlink(FILE2);
  return 0;
}  
  
