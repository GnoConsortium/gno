/*
 * Tests fgetln(3) routine -- interactively.
 * 
 * Devin Reade, 1997.
 * 
 * $Id: fgetln.c,v 1.1 1997/07/27 23:50:59 gdr Exp $
 */

#include <stdio.h>

int 
main(int argc, char **argv) {
  char *p;
  size_t len;
  
  while ((p = fgetln(stdin, &len)) != NULL) {
    printf("%d:%s", len, p);
  }
  return 0;
}
