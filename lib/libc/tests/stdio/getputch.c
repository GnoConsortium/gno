/*
 * This test checks to see if fgetc(3) returns a non-zero high byte
 * for anything but EOF.
 *
 * Usage:
 *      Try it first on a text file.  It should behave like cat(1):
 *           getputch < getputch.c > out
 *      Next try it on a binary file:
 *           getputch < getputch > out
 *      If fputc has problems with binary files you should see diagnostic
 *      output.  Note that in the second case you cannot do a cmp(1) on
 *      the files and expect it to succeed since both stdin and stdout are
 *      in "newline translation mode".
 *
 * $Id: getputch.c,v 1.1 1997/09/05 06:46:31 gdr Exp $
 *
 * Dave Tribby, August 1997.
 */

#pragma debug 25
#pragma lint -1

#include <stdio.h>

int main (int argc, char *argv[]) {
	int ch, hibyte, oldhi = -1;
  int retcode = 0;

  while ((ch = fgetc(stdin)) != EOF) {
	   if (ch > 255) {
	      hibyte = (ch >> 8) & 255;
	      if (hibyte != oldhi) {
	         fprintf(stderr,"  fgetc() high byte = 0x%02X\n", hibyte);
	         oldhi = hibyte;
           retcode = -1;
        }
        ch &= 0xFF;
     }
     (void) fputc(ch, stdout);
  }
  return retcode;
}
