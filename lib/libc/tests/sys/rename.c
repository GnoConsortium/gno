/*
 * This file is formatted with tab stops every 8 columns.
 *
 * $Id: rename.c,v 1.1 1997/09/05 06:46:33 gdr Exp $
 *
 * Devin Reade, 1997
 */

#include <stdio.h>

int 
main(int argc, char **argv) 
{
  if (argc != 3) {
    printf("usage: %s <oldfile> <newfile>\n", argv[0]);
    return -1;
  }
  
  if (rename(argv[1], argv[2]) != 0) {
    perror("rename failed");
    return -1;
  }
  printf("passed\n");
  return 0;
}
