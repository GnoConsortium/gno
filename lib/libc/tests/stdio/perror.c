/*
 * Test by Devin Reade
 *
 * $Id: perror.c,v 1.1 1997/02/28 05:12:57 gdr Exp $
 */

#pragma debug 25

/* for now ... */
#define sys_nerr    _gno_sys_nerr
#define sys_errlist _gno_sys_errlist

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
