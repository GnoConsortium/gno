#ifdef __CCFRONT__
#include <14:pragma.h>
#endif

#define DSORT
#include "common.h"

#include <stdlib.h>
#include <unistd.h>

#define KILLTEMP fclose(fp1); fclose(fp2); fclose(fp3); fclose(fp4); \
                 unlink(file1); unlink(file2); unlink(file3); unlink(file4); \
                 free(file1); free(file2); free(file3); free(file4)
#define KILLARRAY(a,i) { \
                          size_t j; \
                          for (j=0;j<i;j++) free(a[j]); \
                          free(a); \
                       }

static char *errmsg1 = 
   "disksort: write failed on fp1 during construction phase";
static char *errmsg2 =
   "disksort: write failed on fp2 during construction phase";
static char *errmsg3 = "disksort: couldn't allocate scratch buffers";
static char *errmsg4 = "disksort: couldn't reopen temp files";
static char *errmsg5 = "disksort: read on temp file failed";

/*
 * int disksort (char *infile, size_t linecount, size_t linelength);
 *
 * Pre:  <infile> is the name of the input file.  <linecount> is the
 *       maximum number of text lines we should try to sort in _memory_
 *       at any one time.  If it is zero, then DEFAULT_LINECOUNT is used.
 *       <linecount> may be much smaller than the actual linecount of
 *       <infile>.  <linelength> - 1 is the maximum length of line from
 *       <infile> that disksort will recognise.  If <linelength> is zero,
 *       then DEFAULT_LINELENGTH is used.  Global out_fp is an open stream.
 *
 * Post: The file refered to by <infile> is sorted lexicographically 
 *       by lines.  If a line is longer than <linelength>, then any extra 
 *       characters in that line will be truncated.  On success, disksort
 *       returns zero.  On failure, disksort returns -1.  The sorted output
 *       is printed to out_fp, and <infile> is unchanged.
 *
 * Note: This routine is based on a polyphase merge sort using four
 *       temporary files.
 *
 * Uses Globals:
 *       fp1, fp2, fp3, fp4, file1, file2, file3, file4, out_fp
 */

int disksort (char *infile, size_t linecount, size_t linelength) {
  FILE *in_fp;                          /* input file pointer */
  char lastfile;                        /* to where did we last write? */
  size_t runcount;                      /* how many runs make up infile? */
  FILE *fpA, *fpB, *fpC, *fpD;
  char *tempout;                        /* the name of the final temp file */
  char **array;
  size_t i;
  char *strA, *strB;

  /*
   *
   * PHASE ZERO:  Initialization
   *
   */

  if (initdisksort() != 0) return -1;  /* already printed error msgs */

  /*
   * Open the input file for reading
   */

  if ((in_fp=fopen(infile,"r"))==NULL) {
    if (v_flag) perror("disksort: couldn't open input file");
    KILLTEMP;
    return -1;
  }

  /*
   * Get the size of the array of strings we will sort, and create it
   */

  if (linecount == 0) linecount = DEFAULT_LINECOUNT;
  if (linelength == 0) linelength = DEFAULT_LINELENGTH;
  if ((array = malloc (linecount * sizeof(char *))) == NULL) {
    if (v_flag) perror("disksort: couldn't allocate array");
    fclose(in_fp);
    KILLTEMP;
    return -1;
  }
  for (i = 0; i<linecount; i++) {
    if ((array[i] = malloc (linelength * sizeof(char))) == NULL) {
      if (v_flag) perror("disksort: couldn't allocate array elements");
      KILLARRAY(array,i);
      fclose(in_fp);
      KILLTEMP;
      return -1;
    }
  }
    
  /*
   * PHASE I:
   *
   * Read runs from input file, sort each run, dump to first two
   * temp files, alternating between file1 & file2.
   */
  

  lastfile = 'B';
  runcount = 0; 
  
  while(!feof(in_fp)) {

    /* read in a block that can be sorted in core memory */
    for (i=0; i<linecount; i++) {
      if (fgets(array[i],linelength,in_fp)==NULL) {
        if (feof(in_fp)) {
          array[i][0] = '\0'; /* end of file */
          --i;                /* reduce it by one so that sortarray() works */
          break;
        } else {
          if (v_flag) perror(errmsg5);  /* file error */
          KILLARRAY(array,linecount);
          fclose(in_fp);
          KILLTEMP;
          return -1;
        }
      }
    }

    /* sort it */
    sortarray(array,i);

    /* print it out to one of the temp files and add the end-of-line DELIM */
    if (lastfile == 'B') {
      for (i=0; (i<linecount) && (array[i][0]!='\0'); i++)
        if ((fprintf(fp1,"%s",array[i])==EOF) && v_flag) perror(errmsg1);
      lastfile = 'A';
      if ((fprintf(fp1,"%c\n",DELIM)==EOF) && v_flag) perror(errmsg1);
    } else {     /* lastfile == 'A' */
      for (i=0; (i<linecount) && (array[i][0]!='\0'); i++)
        if ((fprintf(fp2,"%s",array[i])==EOF) && v_flag) perror(errmsg2);
      lastfile = 'B';
      if ((fprintf(fp2,"%c\n",DELIM)==EOF) && v_flag) perror(errmsg2);
    }
  }

  /* clean up Phase I */
  fclose(in_fp);
  rewind(fp1);
  rewind(fp2);

  /*
   * merge the files -- at this point, files fp1 and fp2 contain
   * multiple runs of <linecount> records.  Keep merging and
   */

  /* initialize this backwards because of the initial flip */
  fpA = fp2;  

  /* get some scratch strings for the merge */
  if (((strA=malloc(linelength))==NULL) || 
      ((strB=malloc(linelength))==NULL)) {
    if (v_flag) perror(errmsg3);
    return -1;
  }

  do {
    runcount = 0;

    /* flip the files so we can sort back the other way */
    if (fpA == fp1) {
      fpA = fp3;
      fpB = fp4;
      fp1 = freopen(file1,"w+",fp1);
      fp2 = freopen(file2,"w+",fp2);
      if ((fp1==NULL) || (fp2==NULL)) {
        if (v_flag) perror(errmsg4);
        return -1;
      }
      fpC = fp1;
      fpD = fp2;
    } else {
      fpA = fp1;
      fpB = fp2;
      fp3 = freopen(file3,"w+",fp3);
      fp4 = freopen(file4,"w+",fp4);
      if ((fp3==NULL) || (fp4==NULL)) {
        if (v_flag) perror(errmsg4);
        return -1;
      }
      fpC = fp3;
      fpD = fp4;
    }
    rewind(fpA);
    rewind(fpB);

    /*
     * Sort pairs of runs until EOF is reached
     */
    for (;;) {
      int mergeresult;

      /*
       * merge one run from each of fpA and fpB into fpC, then repeat
       * it but placing the result into fpD.
       */

      mergeresult = mergeone(fpA,fpB,fpC,strA,strB,linelength);
      if (mergeresult == 0) {
        runcount++;
        mergeresult = mergeone(fpA,fpB,fpD,strA,strB,linelength);
        if (mergeresult == 0) runcount++;
      } 
      if (mergeresult == -1) {
        /* both files at EOF */
        break;
      } else if (mergeresult == -2) {
        /* files in error; message already printed */
        KILLARRAY(array,linecount);
        KILLTEMP;
        return -1;
      }
      /* else normal merge; continue */
    }
  } while (runcount>1);

  /*
   * At this point, fpC contains the sorted file. (We hope ...)
   */
  if (fpC==fp1) tempout = file1;
  else if (fpC==fp2)  tempout = file2;
  else if (fpC==fp3)  tempout = file3;
  else /* fpC==fp4 */ tempout = file4;

  /*
   * clean up and exit
   */

  /* copy lines from fpC to infile except for the trailing DELIM */
  rewind(fpC);
  for (;;) {
    if (fgets(strA,linelength,fpC)==NULL) {
      if(v_flag) perror(errmsg5);
      return -1;
    }
    if ((strA[0]==DELIM) && (strA[1]=='\n')) break;
    if (fprintf(out_fp,"%s",strA)==EOF) {
      if (v_flag) perror("disksort: write on output file failed");
      return -1;
    }
  }

  free(strA);
  free(strB);
  KILLARRAY(array,linecount);
  KILLTEMP;

  return 0;
}
    
