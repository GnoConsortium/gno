/*
 * This file is formatted with tab stops every 8 columns.
 *
 * $Id: unlink.c,v 1.1 1997/09/05 06:46:33 gdr Exp $
 *
 * Devin Reade, 1997
 */
#include <stdio.h>
#include <unistd.h>

int 
main (int argc, char **argv) 
{
  if (argc != 2) {
    printf("usage: %s filename\n", argv[0]);
    return 1;
  }
  printf("trying to unlink %s\n", argv[1]);
  if (unlink(argv[1]) == -1) {
    perror("unlink failed");
  }
  printf("done\n");
  return 0;
}
