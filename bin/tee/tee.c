/*
 * tee - send a copy of stdin to a file as well as stdout.
 *
 * Version 1.1 and later by Devin Reade <gdr@myrias.ab.ca>
 *
 * tee originally appeared with Gno v1.x, but was fully buffered.
 * This is a complete re-write which by default uses full buffering
 * for the output file, but _no_ buffering on stdin and stdout.
 * This buffering behavior can be changed slightly by the -b flag.
 *
 * $Id: tee.c,v 1.3 1996/09/09 06:12:16 gdr Exp $
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include "getopt.h"

#ifdef CHECK_STACK
void begin_stack_check(void);
int end_stack_check(void);

#define STACKSTART begin_stack_check()
#define STACKEND(n) { \
  fprintf(stderr,"stack usage: %d bytes\n",end_stack_check()); \
  return n; \
}
#else
#define STACKSTART
#define STACKEND(n) return n
#endif

#define BUFFERSIZE 512
#define USAGE { \
  fprintf(stderr,"%s %s\nUsage:\t%s %s\n",argv[0],versionstr,argv[0], \
          usagestr); \
  STACKEND(-1); \
}

char *versionstr = "version 1.2 by Devin Reade";
char *usagestr = "[ -abiV ] filename\n\
\t-a\tappend to filename\n\
\t-i\tignore SIGINT\n";

char buf2[BUFFERSIZE];

int
main(int argc, char **argv)
{
  int c, b_flag;
  char *mode;
  FILE *fp, *fp2;
  int characters;
  
  extern int optind;
  
  /*
   * initialization 
   */
  STACKSTART;
  mode = "w+";
  b_flag = 0;
  
  /*
   * parse the command line 
   */
  while ((c = getopt(argc, argv, "abiV")) != EOF) {
    switch (c) {
    case 'a':
      /* append to instead of truncate output file */
      mode = "a+";
      break;
      
    case 'b':
      /* do line buffering */
      b_flag++;
      break;
      
    case 'i':
      /* ignore SIGINT */
      signal(SIGINT, SIG_IGN);
      break;
      
    case 'V':
      /* FALLTHROUGH */
    default:
      USAGE;
      /* NOTREACHED */
    }
  }
  if ((argc - optind) < 1) {
    USAGE;
  }
  
  /*
   * open the output file 
   */
  if ((fp = fopen(argv[optind], mode)) == NULL) {
    perror("opening master file");
    STACKEND(1);
  }

  /*
   * loop until done with the first file 
   */
  if (b_flag) {
    /* line buffering */
    setvbuf(stdin,  NULL, _IOLBF, 1024);
    setvbuf(stdout, NULL, _IOLBF, 1024);
    characters = BUFFERSIZE;
  } else {
    /* no buffering */
    setvbuf(stdin,  NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    characters = 2;  /* a value of 2 gives one character reads */
  }

  /* poll until EOF seen on input or an error occurs */
  while (fgets(buf2, characters, stdin) != NULL &&
         fputs(buf2, stdout) != EOF &&
         fputs(buf2, fp) != EOF);
  fflush(fp);
  fflush(stdout);
  
  /*
   * make additional copies if necessary 
   */
  optind++;
  if (argc <= optind) {
    fclose(fp);
    STACKEND(0);
  }
  while (argc > optind) {
    size_t count;
    
    /* rewind the master file */
    rewind(fp);
    
    /* open the new file */
    if ((fp2 = fopen(argv[optind], mode)) == NULL) {
      perror("opening duplicate file");
      fclose(fp);
      STACKEND(1);
    }
    
    /* make the copy */
    while (!feof(fp) && !(ferror(fp))) {
      count = fread(buf2, sizeof(char), BUFFERSIZE, fp);
      
      if (count > 0) {
        fwrite(buf2, sizeof(char), count, fp2);
	
        if (ferror(fp2)) {
          perror("writing duplicate file");
          fclose(fp);
          fclose(fp2);
          STACKEND(1);
        }
      }
    }
    
    fclose(fp2);
    if (ferror(fp)) {
      perror("reading master file");
      fclose(fp);
      STACKEND(1);
    }
    optind++;
  }
  fclose(fp);
  STACKEND(0);
}
