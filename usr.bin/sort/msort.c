#ifdef __CCFRONT__
#include <14:pragma.h>
#endif
/*
 * msort -- sort a text file in memory lexicographically
 *
 * Synopsis:
 *          msort [-hvV?] [-o outfile] [-n lines] file1 [file2 ...]
 *
 * Options:
 *          -h -?         -- print version and usage info, then exit
 *          -n <lines>    -- don't try to sort files over <lines> lines long
 *          -o <outfile>  -- sorted output to <outfile> rather than
 *                           to stdout
 *          -v            -- verbose operation
 *          -V            -- print version information
 */

#define DEFFUNC
#define MSORT
#include "common.h"

#include <limits.h>
#include <stdlib.h>
#include <errno.h>
#include "/usr/include/getopt.h"    /* GNU version */

extern int optind;
extern char *optarg;
extern int errno;

static char *versionstring="\
Version 1.0 by Devin Reade\n";

static char *usagestring="\
msort -- Sort a text file in memory lexicographically\n\
\n\
Synopsis:\n\
\tmsort [-hvV?] [-o outfile] file1 [file2 ...]\n\
\n\
Options:\n\
\t-h -?\t\t-- Print version and usage info, then exit.\n\
\t-n <m>\t\t-- Set the maximum number of lines per file to <m>.\n\
\t-o <outfile>\t-- Dump sorted output to <outfile> rather\n\
\t\t\t   than to stdout.\n\
\t-v\t\t-- Verbose operation.\n\
\t-V\t\t-- Print version information.\n";

int main (int argc, char **argv) {

  size_t lc, i;
  char   *outfile;          /* the name of the output file, if nec */
  char   **array;           /* an array of strings; for sorting */
  size_t maxlinelen;        /* length of longest line in current file */
  size_t maxlinecount;      /* max number of lines we want to allow */
  short  failed=0;          /* any errors found? */
  int    c;
  short  errflag=0;
  short  n_flag=0;
  short  o_flag=0;
  short  V_flag=0;
  /* v_flag defined in common.h */

#ifdef DEBUG
  begin_stack_check();
#endif

  /*
   * parse the command line
   */

  while ((c= getopt(argc,argv,"hn:o:vV?")) != EOF)
    switch (c) {
    case 'n':      /* don't try to sort if file is over n lines long */
      n_flag++;
      errno = 0;
      maxlinecount = (size_t) atol(optarg);
      if (errno == ERANGE) maxlinecount = DEFAULT_LINECOUNT;
      break;
    case 'o':      /* redirect sorted output to file */
      o_flag++;
      outfile = optarg;
      break;
    case 'v':      /* verbose */
      v_flag++;
      break;
    case 'V':      /* print version information */
      V_flag++;
      break;
    case '?':      /* fallthrough */
    case 'h':      /* fallthrough */
    default:       /* Display usage, version, and exit */
      V_flag++;
      errflag++;
      break;
    }

  /*
   * React to command line parameters
   */

  if (errflag) {
    fprintf(stderr,"\n%s\n%s\n",usagestring,versionstring);
    return -1;
  }
  if (V_flag) fprintf(stderr,"\n%s\n",versionstring);
  if (!n_flag) maxlinecount = DEFAULT_LINECOUNT;
  if (v_flag) fprintf(stderr,"Maximum lines per file = %lu\n",maxlinecount);

  if (o_flag) {
    if ((out_fp = fopen(outfile,"w")) == NULL) {
      if (v_flag) perror("open on output file failed");
      return -1;
    }
  } else out_fp = stdout;

  /* loop through files */
  for (; optind<argc; optind++) {

    /* get the line count */
    lc = linecount(argv[optind], &maxlinelen);
    if (lc>maxlinecount) {
      if (v_flag)
        fprintf(stderr,"%s too long for an in-memory sort -- file skipped\n",
                argv[optind]);
      failed = 1;
      continue;
    }

    /* load the array */
    array = loadarray (lc, argv[optind], maxlinelen);
    if (array == NULL) {
      if (v_flag) fprintf(stderr,"Ignoring file %s\n",argv[optind]);
      failed = 1;
      continue;
    }

    /* sort it */
    sortarray (array,lc);
    
    /* print the sorted file out and clean up the array */
    for (i=0; i<lc; i++) {
      fprintf(out_fp,"%s",array[i]);
      free(array[i]);
    }
    free(array);
  }

#ifdef DEBUG
  fprintf(stderr,"%s stack usage:  %d bytes\n",argv[0],end_stack_check());
#endif

  if (failed) return -1;
  else return 0;
}
