#ifdef __CCFRONT__
#include <14:pragma.h>
#endif

#define DSORT
#include "common.h"

#include <string.h>

/*
 * int mergeone(FILE *fpA, FILE *fpB, FILE *fpC, char strA[], char strB[],
 *              size_t linelength);
 *
 * Pre:  fpA, fpB, and fpC are open file pointers.  The first should contain
 *       "runs" of data delimited by a line consisting of just the DELIM
 *       character, although either or both may be at EOF.  strA and strB
 *       are scratch character buffers, each of size linelength.
 *
 * Post: The first run on each of fpA and fpB are merge-sorted and added to
 *       fpC.  If either fpA or fpB are at EOF then the run from the other
 *       file pointer is simply concatenated onto fpC.  Mergeone will return
 *       zero if the merge was successful, -1 if both fpA and fpB are at
 *       EOF, and -2 if there was an error.  On return, the contents of
 *       strA and strB are undefined.
 *
 * Uses Globals:
 *       v_flag -- if set and an error occurs, a message will be printed
 *                 to stderr
 *       fp1,fp2,fp3,fp4 -- file pointers to the four scratch files
 *       file1,file2,file3,file4 -- names of the four scratch files
 */


int mergeone(FILE *fpA, FILE *fpB, FILE *fpC, char strA[], char strB[],
             size_t linelength) {

  short run_end_A = 0;
  short run_end_B = 0;

  /*
   * Load strA and strB with the first lines from fpA and fpB.  After
   * this, either file may be at EOF (but not error).
   */

  if ((fgets(strA,linelength,fpA)==NULL) && ferror(fpA)) {
    if (v_flag) perror("mergeone: Read error on fpA");
    return -2;
  }
  if ((fgets(strB,linelength,fpB)==NULL) && ferror(fpB)) {
    if (v_flag) perror("mergeone: Read error on fpB");
    return -2;
  }

  /*
   * merge fpA and fpB until we either get an EOF or a DELIM line
   */

  while (!feof(fpA) && !feof(fpB)) {

    /* test to see if our run is finished */
    if ((strA[0]==DELIM) && (strA[1]=='\n')) {
      run_end_A = 1;
      break;
    }
    if ((strB[0]==DELIM) && (strB[1]=='\n')) {
      run_end_B = 1;
      break;
    }

    if (strcmp(strA,strB) < 0) {

      /* print out the string to fpC */
      if (fprintf(fpC,"%s",strA) == EOF) {
        if (v_flag) perror("mergeone: Write error on fpC");
        return -2;
      }

      /* get another string from fpA */
      if ((fgets(strA,linelength,fpA)==NULL) && ferror(fpA)) {
        if (v_flag) perror("mergeone: Read error on fpA");
        return -2;
      }

    } else {

      /* print out the string to fpC */
      if (fprintf(fpC,"%s",strB) == EOF) {
        if (v_flag) perror("mergeone: Write error on fpC");
        return -2;
        if (v_flag) {
          /* say something */
        }
        return -2;
      }

      /* get another string from fpB */
      if ((fgets(strB,linelength,fpB)==NULL) && ferror(fpB)) {
        if (v_flag) perror("mergeone: Read error on fpB");
        return -2;
      }
    }
  }
    
  /*
   * We've come to the end of at least one of the runs, concatenate
   * the remainder on the output file
   */

  /* finish off fpA if necessary */
  while (!run_end_A && !feof(fpA)) {

    /* test to see if our run is finished */
    if ((strA[0]==DELIM) && (strA[1]=='\n')) {
      run_end_A = 1;
      break;
    }

    /* print out the string to fpC */
    if (fprintf(fpC,"%s",strA) == EOF) {
      if (v_flag) perror("mergeone: Write error on fpC");
      return -2;
    }

    /* get another string from fpA */
    if ((fgets(strA,linelength,fpA)==NULL) && ferror(fpA)) {
      if (v_flag) perror("mergeone: Read error on fpA");
      return -2;
    }
  }

  /* finish off fpB if necessary */
  while (!run_end_B && !feof(fpB)) {

    /* test to see if our run is finished */
    if ((strB[0]==DELIM) && (strB[1]=='\n')) {
      run_end_B = 1;
      break;
    }

    /* print out the string to fpC */
    if (fprintf(fpC,"%s",strB) == EOF) {
      if (v_flag) perror("mergeone: Write error on fpC");
      return -2;
    }

    /* get another string from fpB */
    if ((fgets(strB,linelength,fpB)==NULL) && ferror(fpB)) {
      if (v_flag) perror("mergeone: Read error on fpB");
      return -2;
    }
  }

  /*
   * At this point, both fpA and fpB are either at a run-end or at EOF, 
   * with no errors.  If at EOF, then don't append a DELIM character.
   */
  
  if (feof(fpA) && feof(fpB)) return -1;
  if (fprintf(fpC,"%c\n",DELIM) == EOF) {
    if (v_flag) perror("mergeone: Write error on fpC");
    return -2;
  }
  return 0;
}




        
