#line 1 ":trenco4:gno.src:lib:libc:gen:testit.c"
#pragma lint -1
#pragma keep "testit"

#include <stdio.h>
#ifdef __GNO__
#include <unistd.h>
#endif
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>

int main (int argc, char **argv) {
	int fd1, fd2;

	if (argc != 2) {
	   printf("usage!\n");
     exit(1);
  }

  printf("trying first open\n");
  fd1 = open(argv[1], O_RDONLY);
  if (fd1 == -1) {
	   perror("open 1 failed\n");
     exit (1);
  }

  printf("trying second open\n");
  fd2 = open(argv[1], O_RDONLY);
  if (fd2 == -1) {
	   perror("open 2 failed\n");
     exit (1);
  }
   
	printf("done!\n");
  close(fd1);
  close(fd2);
	return 0;
}
