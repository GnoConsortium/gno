#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#if 0
#define STREAM stdout
#else
#define STREAM stderr
#endif

int main(int argc, char **argv) {
	int i;

  for (i=0; i<3; i++) {
	   fputc('x', STREAM);
     sleep(1);
  }
  fputc('\n', STREAM);
  exit(0);
}
