/*
 * String Array implementation.  These routines dynamically handle an
 * array of NULL-terminated strings.  The array itself is also NULL-
 * terminated.
 *
 * Devin Reade, September 1997.
 *
 * $Id: strarray.c,v 1.2 1997/10/30 04:57:25 gdr Exp $
 */
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <err.h>
#include <contrib.h>

#define SLOTS_QUANTUM	64

/*
 * LC_StringArrayNew
 *
 * Creates and returns a new, empty string array.  The vector will be
 * allocated, but will have NULL as the first element.
 */

LC_StringArray_t
LC_StringArrayNew (void) {
  LC_StringArray_t result;
    
  result		= LC_xmalloc(sizeof(LC_StringArrayElem_t));
  result->lc_vec	= LC_xmalloc(SLOTS_QUANTUM * sizeof(char *));
  (result->lc_vec)[0]	= NULL;
  result->lc_alloced	= SLOTS_QUANTUM;
  result->lc_used	= 0;
  return result;
}

/*
 * LC_StringArrayAdd
 *
 * Adds a string <str> to the stringArray <array>.  Calling this
 * with <str>==NULL has no effect as the array is always NULL-terminated.
 */

void
LC_StringArrayAdd (LC_StringArray_t array, char *str) {
  if (array == NULL) {
    errx(1, "LC_StringArrayAdd was passed a NULL first argument");
  }
  if (array->lc_used > array->lc_alloced) {
    errx(1, "LC_StringArrayAdd: corrupted string array");
  }
    
  if (str == NULL) {
    return;
  }
    
  /* grow the size of the string array, if necessary */
  if (array->lc_alloced - array->lc_used < 2) {
    array->lc_alloced+=SLOTS_QUANTUM;
    if (array->lc_vec == NULL) {
      array->lc_vec = LC_xmalloc(array->lc_alloced * sizeof(char *));
    } else {
      array->lc_vec = LC_xrealloc(array->lc_vec, 
				  array->lc_alloced * sizeof(char *));
    }
  }
    
  /* add in the string */
  array->lc_vec[array->lc_used] = LC_xstrdup(str);
  array->lc_used++;
  array->lc_vec[array->lc_used] = NULL;
  return;
}

#if 0
/*
 * This routine has been disabled for now because it is subject to
 * buffer overflow.
 */

/*
 * LC_StringArrayAdd2
 *
 * This is like LC_StringArrayAdd, but instead of taking one string,
 * it takes printf()-like arguments
 */

void
LC_StringArrayAdd2 (LC_StringArray_t array, char *format, ...) {
  va_list ap;
    
  va_start(ap,format);
  vsprintf(buffer,format,ap);
  LC_StringArrayAdd(array,buffer);
  va_end(ap);
  return;
}
#endif


void
LC_StringArrayDelete (LC_StringArray_t array, char *str) {
  int i;
  
  for (i=0; i<array->lc_used; i++) {
    if (strcmp(array->lc_vec[i],str)) continue;
    
    /* found it; free up the memory */
    free(array->lc_vec[i]);
    for (; i<array->lc_used; i++) {
      array->lc_vec[i]=array->lc_vec[i+1];
    }
    break;
  }
  array->lc_used -= 1;
}


/*
 * LC_StringArrayClear -- free the memory allocated by LC_StringArrayAdd()
 *                     and reinitialize the associated variables.
 */

void
LC_StringArrayClear (LC_StringArray_t array) {
  int i;

  if (array == NULL) {
	   errno = EINVAL;
	   err(1, "LC_StringArrayClear passed NULL pointer");
  }
  if (array->lc_used == 0) {
    return;
  }
  if ((array->lc_vec == 0) || (array->lc_alloced == 0)) {
    errx(1, "LC_StringArrayClear: corrupted array");
  }
  for (i=0; i<array->lc_used; i++) {
    free((array->lc_vec)[i]);
  }
  free(array->lc_vec);
  array->lc_vec     = NULL;
  array->lc_alloced = 0;
  array->lc_used    = 0;
  return;
}

void
LC_StringArrayDestroy (LC_StringArray_t array) {
	LC_StringArrayClear(array);
  free(array);
}

/*
 * LC_StringArrayCat
 *
 * Concatenate the elements of a LC_StringArray_t into a single character
 * string.  Return a pointer to the malloc'd string.  The LC_StringArray_t
 * is not modified.
 */

char *
LC_StringArrayCat (LC_StringArray_t array, int insertSpace) {
  int i, j;
  size_t len, len2;
  char *result, *p, *q, **qq;
  
  len = 1;
  j = (insertSpace) ? 1 : 0;
  for (i=0; i<array->lc_used; i++) {
    len += strlen((array->lc_vec)[i]) + j;
  }
  result = LC_xmalloc(len * sizeof(char));
  *p = *result = '\0';
  j = array->lc_used;
  qq = array->lc_vec;
  for (i=0; i<j; i++) {
#if 1
    q = qq[i];
    len2 = strlen(q);
    strcpy(p, q);
    p += len2;
    if (insertSpace) {
      *p++ = ' ';
      *p = '\0';
    }
#else
    strcat(result,(array->lc_vec)[i]);
    if (insertSpace) {
      strcat(result," ");
    }
#endif
  }
  if (len > 1) {
    /* dump the extra space at the end of the string */
    result[len-1]='\0';
  }
  return result;
}
