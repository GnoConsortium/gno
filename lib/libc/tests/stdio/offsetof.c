/*
 * This shows the offset (expected and actual) of the _file member of
 * struct FILE for the stdio package.
 * 
 * Devin Reade, 1997.
 * 
 * $Id: offsetof.c,v 1.1 1997/07/27 23:50:59 gdr Exp $
 */

#include <stddef.h>
#include <stdio.h>

int main (int argc, char **argv) {

#ifdef __ORCAC_VERSION
  int version = __ORCAC_VERSION;
#else
  int version = 20;
#endif
  printf("the expected offsets are:\n");
  printf("\tv2.0.x:\t\t28\n");
  printf("\tv2.1.0:\t\t28\n");
  printf("\tv2.1.1b2:\t30\n");
  printf("for version %d, offsetof(_file) is %d\n", version,
	 offsetof(FILE, _file));
  
  return 0;
}
