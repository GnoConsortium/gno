/*
 * Test by Devin Reade
 *
 * $Id: basename.c,v 1.1 1997/02/28 05:12:52 gdr Exp $
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int
main(int argc, char **argv)
{

	if (argc != 2) {
	   fprintf(stderr,"usage: %s filename\n", argv[0]);
     exit(1);
  }

  printf("basename(\"%s\") = \"%s\"\n", argv[1], basename(argv[1]));
  printf("dirname(\"%s\") = \"%s\"\n", argv[1], dirname(argv[1]));
  return 0;
}
