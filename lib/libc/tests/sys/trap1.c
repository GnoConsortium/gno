/*
 * Test by Devin Reade
 *
 * $Id: trap1.c,v 1.2 1997/02/28 05:43:54 gdr Exp $
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gno/gno.h>

#define FAIL() { printf("TEST FAILED\n"); exit (1); }
#define PASS() { printf("test passed\n"); exit (0); }

#define VERSION_TRIPLE(val) \
	(((val)&0xFF00) >> 8), \
	(((val)&0x00F0) >> 4), \
	(((val)&0x000F) >> 0)

extern int _toolErr;

int main (int argc, char **argv) {
  u_short kernelVersion;

  kernStatus();
  if (_toolErr) {
  	printf("kernStatus returned %d\n", _toolErr);
     FAIL();
  }

	kernelVersion = kernVersion();
  if (_toolErr) {
	   printf("kernVersion failed with code %d\n", _toolErr);
     FAIL();
  } else if (kernelVersion < 0x0204) {
	   printf("there are no tests for kernel version %d.%d.%d\n",
		       VERSION_TRIPLE(0x0204));
	} else {
	   printf("running tests for kernel version %d.%d.%d\n",
		       VERSION_TRIPLE(0x0204));
  }

	printf("SYSTEM CALL\t\t\tRETURN VALUE\n");

	printf("getpid\t\t\t\t%d\n", getpid());
  printf("setdebug(%d)\t\t\t%d\n", dbgSYSCALL, setdebug(dbgSYSCALL));
  printf("setdebug(%d)\t\t\t%d\n", 0, setdebug(0));
  return 0;    
}
