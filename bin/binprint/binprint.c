/* 
    BINPRINT.C

    Displays files in hex-dump format (with ascii subdisplay if desired)

    v1.3  Incorporated sources into GNO base build.  Reformatted sources,
	  prototyped functions, added calls to fflush(3). (Devin Reade)
    v1.2  Optimized and fixed input from terminal bug (Phil Vandry)
    v1.1  Added stacksize directive (Jawaid Bazyar)
    v1.0  Original version by Derek Taubert

    $Id: binprint.c,v 1.3 1999/01/16 18:35:57 gdr-ftp Exp $
*/

#define VERSION "1.3"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <gno/gno.h>	/* for stack checking routines */

unsigned int doline(char *dest, char *source, unsigned int actual,
		    unsigned int cols);
static void printGood(long off, unsigned char *buf, int real, int form);

unsigned char *buffer2;

int
main(int argc, char **argv)
{
  int duh;
  int a;
  int c, errflg = 0, columns = 16;
  size_t pos = 0;
  unsigned char *buffer;

  __REPORT_STACK();
  while ((c = getopt(argc,argv, "Vc:")) != EOF)
    switch (c) {
    case 'c' :
      columns = atoi(optarg);
      break;
    case 'V' :
      fprintf(stdout, "binprint v%s for GNO/ME\n", VERSION);
      exit(0);
    default : 
      errflg++;
    }
  if (errflg) {
    fprintf(stderr,"usage: binprint [-c<columns>] files...\n");
    exit(2);
  }
  argv += optind;
  if (columns < 8) columns = 8;
  if ((buffer2 = (unsigned char *)malloc((size_t)(columns*4)+1)) == NULL) {
    fprintf(stderr,"Cannot allocate buffer space\n");
    exit(1);
  }
  buffer = (unsigned char *)(buffer2+(columns*3));
  if (optind == argc) {
    duh = STDIN_FILENO;
    goto action;
  }
  for (;optind<argc;optind++) {
    if ((duh = open(*argv,O_RDONLY|O_BINARY)) == -1) {
      fprintf(stderr,"error opening %s\n",*argv);
      exit(1);
    }
    printf("\n%s\n",*argv);
    fflush(stdout);	/* we write to STDOUT_FILENO directly */
  action:   
    while ((a = (int)read(duh, buffer,
			  (size_t)(columns * sizeof(unsigned char)))) != 0) {
      printGood(pos,buffer,a,columns);
      pos += a;
    }
    close(duh);
  }
  free(buffer);
  free(buffer2);
  exit(0);
}

static void
printGood(long off, unsigned char *buf, int real, int form)
{
  if (!real) return;
#if 0
    printf("%8lX: ",off);
    fflush(stdout);
#endif
  
  /* The following is a hack required because of buffering by the stdio
     libraries. I wish it was not necesary... */
  
  {
    static char puthere[11];
    int howmany;
    
    howmany = sprintf(puthere,"%8lX: ",off);
    write(STDOUT_FILENO, puthere, (size_t)howmany);
  }
  
  write(STDOUT_FILENO, buffer2, (size_t)doline(buffer2,buf,form,real));
}
