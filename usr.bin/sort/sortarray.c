#ifdef __CCFRONT__
#include <14:pragma.h>
#endif

#include "common.h"

#include <string.h>

/*
 * void sortarray(char **array, unsigned long n);
 *
 * Pre:  <array> is a pointer to an array of pointers to NULL-terminated
 *       strings, and <n> is the number of elements in <array>
 *
 * Post: The strings in <array> are sorted lexicographically in ascending
 *       order, using the heapsort algorithm.  This is an in-place
 *       non-recursive sort with behavior O[n*lg(n)] both on average 
 *       and worst-case.
 */

void sortarray(char *array[], unsigned long n) {

  long l, j, ir, i;
  char *rra;

  if (n==1) return; /* no need to sort one element */
  --array;          /* fudge since the algorithm was designed */
                    /* for a unit-indexing */
  
  l = (n>>1) + 1;
  ir = n;
  
  /* 
   * The index l will be decremented from its initial value down to 0 during
   * the heap creation phase.  Once it reaches 0, the index ir will be
   * decremented from its initial value down to 0 during the heap selection
   * phase.
   */
  for (;;) {
    if (l > 1)              /* still in creation phase */
      rra = array[--l];
    else {                  /* in selection phase */
      rra= array[ir];       /* clear a space at the end of array */
      array[ir] = array[1]; /* retire the top of the heap into it */
      if (--ir == 1) {      /* done with the last promotion */
        array[1] = rra;
        return;
      }
    }
    i = l;           /* set up to sift down element rra to its proper place */
    j = l << 1;
    while (j<=ir) {
      if (j<ir && (strcmp(array[j],array[j+1])<0)) ++j;
      if (strcmp(rra,array[j])<0) {                     /* demote rra */
        array[i] = array[j];
        i = j;
        j += i;
      } else j = ir + 1;    /* this is rra's level; set j to terminate */
    }                       /* the sift-down */
    array[i] = rra;
  }
}
