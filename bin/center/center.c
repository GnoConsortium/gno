
/* "center", (c) 1992 Marek Pawlowski.  v1.0
   Takes input from stdin, center's it, and puts it to
   stdout.

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

*/
#pragma stacksize 1024

#include <stdio.h>
#include <ctype.h>
#define VERSION "1.00"

char input[81];
char output[81];

main(argc, argv)
int argc;
char **argv;
{
   int t, i, x, s, fflag;
   FILE *in;

/* _INITGNOSTDIO();
   setvbuf(stdin,NULL,_IOLBF,256l);  */
   if (argc > 3)
      usage(argv[0]);

   s = 1;

   if(argc > 1) {
      for(x = 0 ; x <= (strlen(argv[1])-1) ; x++) {
         s = isdigit(argv[1][x]);
         t = atoi(argv[1]);
      }
   }

   else if (argc == 1)
      t = 80;

   fflag = 0;

   if (argc == 3) {
      in = fopen(argv[2], "r");
      if(!in) error(argv[1], argv[2]);
      fflag = 1;
   }

   if(s != 0) {
      if(fflag == 1)
         center(in, t);
      else
         center(stdin, t);
   }

   else
      usage(argv[0]);
}

/* Function to call on other subroutines to result in a completely
   centered line! */

center(stream, t)
FILE *stream;
int t;
{
   int x, i;

   while(feof(stream) == 0) {
      fgets(input, 80, stream);

      i = (t - strlen(input)) / 2;

      fillit(i, 0);

      for(x = 0 ; x <= strlen(input) ; x++)
         output[i+x] = input[x];
      if(feof(stream) == 0)
           fputs(output, stdout);
   }
}

/* Function to tell the person that the filename offered
   not be opened for reading */

error(name, file)
char *name;
char *file;
{
   fprintf(stderr, "%s: cannot open %s\n", name, file);
   exit(0);
}

/* Function to display the usage of the utility */

usage(file)
char *file;
{
   fprintf(stderr, "Usage: %s [columns] [file]\n", file);
   exit(0);
}

/* Function to fill in the left side of text with the appropriate
   number of spaces */

fillit(ntf, start)
int ntf;             /* Number To Fill */
int start;           /* Start filling at.. */
{
   int x;

   for(x = start ; x <= ntf ; x++) {
      output[x] = ' ';
   }
}
