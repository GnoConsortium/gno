#ifdef __CCFRONT__
#include <14:pragma.h>
#endif

#define DSORT
#include "common.h"
#include <stdlib.h>
#include <unistd.h>

#define OPENMODE "w+"       /* the mode for create/read/write */

static char *errstring1="initdisksort: couldn't get temp name";
static char *errstring2="initdisksort: couldn't open temp file";

/*
 * int initdisksort(void);
 *
 * Pre:  None.
 * 
 * Post: Returns 0 on success, -1 on failure.
 *       On success:
 *         file1 through file4 are initialized as temp file names.
 *         fp1 through fp4 are open file pointers for file1 ... file4.
 *
 * Uses Globals:
 *       fp1, fp2, fp3, fp4
 *       file1, file2, file3, file4, 
 *       v_flag
 */

int initdisksort(void) {

  /*
   * Get the names for the temp files -- this is ponderous but necessary
   */

  if ((file1 = tempnam(tpath1,"dsort")) == NULL) {
    if (v_flag) perror(errstring1);
    return -1;
  }
  if ((file2 = tempnam(tpath2,"dsort")) == NULL) {
    if (v_flag) perror(errstring1);
    free(file1);
    return -1;
  }
  if ((file3 = tempnam(tpath3,"dsort")) == NULL) {
    if (v_flag) perror(errstring1);
    free(file1);
    free(file2);
    return -1;
  }
  if ((file4 = tempnam(tpath4,"dsort")) == NULL) {
    if (v_flag) perror(errstring1);
    free(file1);
    free(file2);
    free(file3);
    return -1;
  }

  /*
   * Open the temp files -- again ponderous but necessary
   */


  if ((fp1 = fopen(file1,OPENMODE))==NULL) {
    if (v_flag) perror(errstring2);
    free(file1);
    free(file2);
    free(file3);
    free(file4);
    return -1;
  }
  if ((fp2 = fopen(file2,OPENMODE))==NULL) {
    if (v_flag) perror(errstring2);
    unlink(file1);
    free(file1);
    free(file2);
    free(file3);
    free(file4);
    return -1;
  }
  if ((fp3 = fopen(file3,OPENMODE))==NULL) {
    if (v_flag) perror(errstring2);
    unlink(file1);
    unlink(file2);
    free(file1);
    free(file2);
    free(file3);
    free(file4);
    return -1;
  }
  if ((fp4 = fopen(file4,OPENMODE))==NULL) {
    if (v_flag) perror(errstring2);
    unlink(file1);
    unlink(file2);
    unlink(file3);
    free(file1);
    free(file2);
    free(file3);
    free(file4);
    return -1;
  }
   
  return 0;
}
