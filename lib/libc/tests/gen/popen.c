/*
 * Test by Devin Reade
 *
 * $Id: popen.c,v 1.1 1997/02/28 05:12:52 gdr Exp $
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
  char *mode;

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
  if (argc - optind != 1) {
	   errx(1,"one argument required");
	}

  if ((fp = popen(argv[optind], mode)) == NULL) {
	   err(1, "popen failed");
  }
  while (fgets(buffer, BUFFERSIZE, fp) != NULL) {
	   printf("T: %s", buffer);
  }
  printf("now doing pclose\n");
	c = pclose(fp);
  printf("pclose returned %d\n", c);

  return c;
}
