/*
 * tee - send a copy of stdin to a file as well as stdout.
 *
 * Version 1.1 and later by Devin Reade <gdr@myrias.ab.ca>
 *
 * tee originally appeared with GNO v1.x, but was fully buffered.
 * This is a complete re-write which by default uses full buffering
 * for the output file, but _no_ buffering on stdin and stdout.
 * This buffering behavior can be changed slightly by the -b flag.
 *
 * $Id: tee.c,v 1.4 1997/10/30 03:32:46 gdr Exp $
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <err.h>

#ifdef __STACK_CHECK__
#include <gno/gno.h>
static void
showstack (void) {
	fprintf(stderr,"stack usage: %d bytes\n", _endStackCheck());
	return;
}
#endif	/* __STACK_CHECK__ */

#define BUFFERSIZE 512

char *versionstr = "version 1.3 by Devin Reade";
char *usagestr = "[ -abiV ] filename\n\
\t-a\tappend to filename\n\
\t-i\tignore SIGINT\n";

char buf2[BUFFERSIZE];

static void	usage(const char *pname);

int
main(int argc, char **argv)
{
  int c, b_flag;
  char *mode, *master;
  FILE *fp, *fp2;
  int characters;
  
  extern int optind;
  
  /*
   * initialization 
   */
#ifdef __STACK_CHECK__
  _beginStackCheck();
  atexit(showstack);
#endif
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
      usage(argv[0]);
      /* NOTREACHED */
    }
  }
  if ((argc - optind) < 1) {
    usage(argv[0]);
  }
  
  /*
   * open the output file 
   */
  master = argv[optind];
  if ((fp = fopen(master, mode)) == NULL) {
    err(1, "couldn't open master file %s", master);
  }

  /*
   * loop until done with the first file 
   */
  if (b_flag) {
    /* line buffering */
    setvbuf(stdin,  NULL, _IOLBF, 1024);
    setvbuf(stdout, NULL, _IOLBF, 1024);
    characters = BUFFERSIZE;

    /* poll until EOF seen on input or an error occurs */
    while (fgets(buf2, characters, stdin) != NULL &&
           fputs(buf2, stdout) != EOF &&
           fputs(buf2, fp) != EOF);
  } else {
    /* no buffering */
    setvbuf(stdin,  NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    characters = 2;  /* a value of 2 gives one character reads */

    /* avoid ORCA/C v2.1.1b2 optimization problem */
#undef fgetc
#undef fputc

    /* poll until EOF seen on input or an error occurs */
    while (((c = fgetc(stdin)) != EOF) &&
           (fputc(c, stdout) != EOF) &&
           (fputc(c, fp) != EOF));
  }        

  fflush(fp);
  fflush(stdout);
  
  /*
   * make additional copies if necessary 
   */
  optind++;
  if (argc <= optind) {
    fclose(fp);
    exit(0);
  }
  while (argc > optind) {
    size_t count;
    
    /* rewind the master file */
    rewind(fp);
    
    /* open the new file */
    if ((fp2 = fopen(argv[optind], mode)) == NULL) {
      err(1, "couldn't open duplicate file %s", argv[optind]);
    }
    
    /* make the copy */
    while (!feof(fp) && !(ferror(fp))) {
      count = fread(buf2, sizeof(char), BUFFERSIZE, fp);
      
      if (count > 0) {
        fwrite(buf2, sizeof(char), count, fp2);
	
        if (ferror(fp2)) {
          err(1, "error writing duplicate file %s", argv[optind]);
        }
      }
    }
    
    fclose(fp2);
    if (ferror(fp)) {
      err(1, "error reading master file %s", master);
    }
    optind++;
  }
  fclose(fp);
  exit(0);
}

static void
usage (const char *pname) {
	fprintf(stderr,"%s %s\nUsage:\t%s %s\n", pname, versionstr,
		pname, usagestr);
	exit(1);
}
