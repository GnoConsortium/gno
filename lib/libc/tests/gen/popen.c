/*
 * Test by Devin Reade
 *
 * $Id: popen.c,v 1.2 1997/09/21 16:33:49 gdr Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <err.h>

#define BUFFERSIZE 1024
char buffer[BUFFERSIZE];

int main (int argc, char **argv) {
  FILE *fp;
	int errflag = 0;
  int c;
  char *mode = NULL;

  while ((c = getopt(argc, argv, "rw")) != EOF) {
	   switch (c) {
     case 'r':
	      mode = "r";
        break;
     case 'w':
	      mode = "w";
        break;
     default:
	      warnx("unknown option: %c", c);
        errflag++;
     }
  }
  if (errflag) {
	   exit(1);
	}
  if ((argc - optind != 1) || (mode == NULL)) {
	   errx(1,"one flag and one argument required");
	}

  if ((fp = popen(argv[optind], mode)) == NULL) {
	   perror("popen failed");
     exit(1);
	   err(1, "popen failed");
  }
  fprintf(stderr, "popen passed, starting loop\n");
  while (fgets(buffer, BUFFERSIZE, fp) != NULL) {
	   fprintf(stderr, "T: %s", buffer);
  }
  fprintf(stderr, "now doing pclose\n");
	c = pclose(fp);
  fprintf(stderr, "pclose returned %d\n", c);

  return c;
}
