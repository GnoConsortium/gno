
/* "center", (c) 1992 Marek Pawlowski.  v1.0
   Takes input from stdin, center's it, and puts it to
   stdout.

   v1.1 compiled for GNO Base Distribution.  Devin Reade, 15 February 1999

   Usage:  center [Columns] [File]

   Columns   The number of columns are to be considered
             "full-screen" for centering.  Default 80.
   File      Stream to read input from.  Default stdin.

   This utility is in the public domain, along with
   source code.  Munge at will.  Contact author for
   redistribution rights, or inclusion in package.
   Credit to Marek Pawlowski must be retained in
   modified source code.

   I'd like to see what you did to my source code, when
   you change it.  Correspond with the author (Marek
   Pawlowski) at the following Email addresses:

      marekp@pnet91.cts.com
      marekp@cerf.net

   $Id: center.c,v 1.2 1999/02/16 06:04:11 gdr-ftp Exp $

*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <gno/gno.h>

#define VERSION "1.1"

static void center(FILE *stream, int t);

char input[81];
char output[81];

static void usage(const char *file);

int
main(int argc, char **argv)
{
   int t, i, x, s, fflag, len, c;
   FILE *in;

   __REPORT_STACK();
   if (argc > 3) {
     usage(argv[0]);
   }

   s = 1;

   if(argc > 1) {
      len = strlen(argv[1]);
      for(x = 0 ; x <= (strlen(argv[1])-1) ; x++) {
         s = isdigit(argv[1][x]);
         t = atoi(argv[1]);
	 if (t>80) {
	   t = 80;
	 }
      }
   }

   else if (argc == 1)
      t = 80;

   fflag = 0;

   if (argc == 3) {
      in = fopen(argv[2], "r");
      if (!in) {
	fprintf(stderr, "%s: cannot open %s\n", argv[1], argv[2]);
	exit(1);
      }
      fflag = 1;
   }

   if(s != 0) {
      if(fflag == 1)
         center(in, t);
      else
         center(stdin, t);
   } else {
      usage(argv[0]);
   }
   return 0;
}

/* Function to call on other subroutines to result in a completely
   centered line! */

static void
center(FILE *stream, int t)
{
   int x, i, j;

   while(feof(stream) == 0) {
      fgets(input, 80, stream);

      i = (t - strlen(input)) / 2;

      for (j = 0; j<i; j++) {
	output[j] = ' ';
      }

      for(x = 0 ; x <= strlen(input) ; x++)
         output[i+x] = input[x];
      if(feof(stream) == 0)
           fputs(output, stdout);
   }
}

/* Function to display the usage of the utility */
static void
usage(const char *file)
{
   fprintf(stderr, "Usage: %s [columns] [file]\n", file);
   exit(1);
}
