#ifdef __CCFRONT__
#include <14:pragma.h>
#endif
/*
 * dsort -- sort a text file on disk lexicographically
 *
 * Synopsis:
 *          dsort [-hvV?] [-l length] [-n lines] [-o outfile]
 *                [-t path1[,path2[,path3[,path4]]]] infile
 *
 * Options:
 *          -h -?         -- print version and usage info, then exit
 *          -l <length>   -- use a line length of <length>
 *          -n <m>        -- sort <m> lines in memory.
 *          -o <outfile>  -- sorted output to <outfile> rather than
 *                           to stdout
 *          -t <pathlist> -- use <pathlist> (up to four paths) as the locations
 *                           of temp files. <pathlist> is of the form:
 *                           path1[,path2[,path3[,path4]]].  If any of these
 *                           are not specified, dsort will attempt to use
 *                           the system default temp path.
 *          -v            -- verbose operation
 *          -V            -- print version information
 */


#define DEFFUNC
#define DSORT
#include "common.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "/usr/include/getopt.h"    /* GNU version */

extern int  optind;
extern char *optarg;
extern int  errno;

static char *versionstring="\
Version 1.0 by Devin Reade\n";

static char *usagestring="\
dsort -- Sort a text file on disk lexicographically\n\
\n\
Synopsis:\n\
\tdsort\t[-hvV?] [-l length] [-n lines] [-o outfile]\n\
\t\t[-t path1[,path2[,path3[,path4]]]] infile\n\
\n\
Options:\n\
\t-h -?\t\t-- Print version and usage info, then exit.\n\
\t-l <length>\t-- Set the maximum line length to <length>.\n\
\t-n <m>\t\t-- Set the number of lines to sort in memory to <m>.\n\
\t-o <outfile>\t-- Dump sorted output to <outfile> rather\n\
\t\t\t   than to stdout.\n\
\t-t <pathlist>\t-- Set the paths to use for the location of\n\
\t\t\t   scratch files. Paths are delimited by \',\' characters.\n\
\t-v\t\t-- Verbose operation.\n\
\t-V\t\t-- Print version information.\n";

             

int main (int argc, char **argv) {

  size_t lc, i;
  char   *outfile;          /* the name of the output file, if nec */
  char   **array;           /* an array of strings; for sorting */
  size_t maxlinelen;        /* length of longest line in current file */
  size_t maxlinecount;      /* max number of lines we want to allow */
  char   *tbuffer;          /* buffer containing the temp paths */
  short  failed=0;          /* any errors found? */
  int    c;
  short  errflag=0;
  short  l_flag=0;
  short  n_flag=0;
  short  o_flag=0;
  short  t_flag=0;
  short  V_flag=0;
  /* v_flag defined in common.h */

#ifdef DEBUG
  begin_stack_check();
#endif

  /*
   * parse the command line
   */

  while ((c= getopt(argc,argv,"hl:n:o:t:vV?")) != EOF)
    switch (c) {
    case 'l':      /* use this as the maximum line length */
      l_flag++;
      errno = 0;
      maxlinelen = (size_t) atol(optarg);
      if (errno = ERANGE) maxlinelen = DEFAULT_LINELENGTH;
      break;
    case 'n':      /* sort this number of lines in memory */
      n_flag++;
      errno = 0;
      maxlinecount = (size_t) atol(optarg);
      if (errno == ERANGE) maxlinecount = DEFAULT_LINECOUNT;
      break;
    case 'o':      /* redirect sorted output to file */
      o_flag++;
      outfile = optarg;
      break;
    case 't':      /* define locations of temp files */
      t_flag++;
      if ((tbuffer=malloc(strlen(optarg)+1))==NULL) {
        perror("couldn't allocate temporary buffer; using default");
        break;
      }
      strcpy(tbuffer,optarg);
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
    fprintf (stderr,"\n%s\n%s\n",usagestring,versionstring);
    return -1;
  }
  if (V_flag) fprintf(stderr,"\n%s\n",versionstring);

  if (!l_flag) maxlinelen = DEFAULT_LINELENGTH;
  if (!n_flag) maxlinecount = DEFAULT_LINECOUNT;
  if (v_flag) fprintf(stderr,
    "Sorting %lu lines in memory.\nMaximum recognised line length = %lu\n",
    maxlinecount,maxlinelen);
  if (o_flag) {
    if ((out_fp = fopen(outfile,"w")) == NULL) {
      if (v_flag) perror("open on output file failed");
      return -1;
    }
  } else out_fp = stdout;

  tpath1 = NULL;
  tpath2 = NULL;
  tpath3 = NULL;
  tpath4 = NULL;
  if ((t_flag) && (tbuffer!=NULL)) {
    char *tp = tbuffer;

    /* set tpath1 */
    tpath1 = tp;
    while (*tp && (*tp!=',')) tp++;
    if (*tp) {
      *tp++ = '\0'; /* terminate tpath1 */
      if (v_flag) fprintf(stderr,"Will try to use temp directory %s\n",tpath1);

      /* set tpath2 */
      tpath2 = tp;
      while (*tp && (*tp!=',')) tp++;
      if (*tp) {
        *tp++ = '\0'; /* terminate tpath2 */
        if (v_flag)
          fprintf(stderr,"Will try to use temp directory %s\n",tpath2);
                                                                               
        /* set tpath3 */
        tpath3 = tp;
        while (*tp && (*tp!=',')) tp++;
        if (*tp) {
          *tp++ = '\0'; /* terminate tpath3 */
          if (v_flag)
            fprintf(stderr,"Will try to use temp directory %s\n",tpath3);
                                                                     
          /* set tpath4 */
          tpath4 = tp;
          while (*tp && (*tp!=',')) tp++;
          *tp = '\0'; /* terminate tpath4 */
          if (v_flag)
            fprintf(stderr,"Will try to use temp directory %s\n",tpath4);
        }                                                              
      }
    }
  } else if (v_flag) fprintf(stderr,"Using default temp path\n");

  /* do the sort */
  if (argc - optind == 1) {
    c = disksort(argv[optind],maxlinecount,maxlinelen);
  } else {
    fprintf(stderr,"\n%s\n%s\n",usagestring,versionstring);
    c = -1;
  }

  if (t_flag && (tbuffer)) free(tbuffer);

#ifdef DEBUG
  fprintf(stderr,"%s stack usage:  %d bytes\n",argv[0],end_stack_check());
#endif

  return c;
}
