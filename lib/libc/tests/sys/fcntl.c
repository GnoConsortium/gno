/*
 * tests fcntl (F_GETFL) for stdin, stdout, stderr
 * 
 * $Id: fcntl.c,v 1.1 1997/09/21 18:11:42 gdr Exp $
 */

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int
main(int argc, char **argv) {

  printf("starting\n");
  printf("flags for stdin  are 0x%x\n", fcntl(STDIN_FILENO, F_GETFL));
  printf("flags for stdout are 0x%x\n", fcntl(STDOUT_FILENO, F_GETFL));
  printf("flags for stderr are 0x%x\n", fcntl(STDERR_FILENO, F_GETFL));
  return 0;
}
