#ifdef __CCFRONT__
#include <14:pragma.h>
#endif

#include "common.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

/*  
 * char **loadarray (unsigned long n, char *filename, size_t maxlinelen);
 *
 * Pre:  <filename> is the name of a file containing <n> lines of text,
 *       the number of lines.  The file <filename> must be closed.
 *       <maxlinelen> is the length of the longest line in <filename>
 *
 * Post: Returns a pointer to an array of pointers to malloc'd strings, 
 *       where the strings are successive lines from the file <filename>.
 *       On return <filename> will be closed.  If loadarray() fails for
 *       any reason, it will return NULL.
 *
 * Warning:  The use of realloc() with a NULL pointer for the initial
 *       allocation may not be portable.  If this is not valid for your
 *       current libraries, then #define BROKEN_REALLOC.
 *
 * Uses Globals:
 *       v_flag
 */

char **loadarray (unsigned long n, char *filename, size_t maxlinelen) {
 
  char **result;
  unsigned long i,j;
  FILE *in_fp;
  static char *inbuf=NULL;
  static size_t previous_size = 0;
  char *p;


#ifndef BROKEN_REALLOC  /* realloc() is ANSI-compliant with NULL first arg */

  /* reallocate the input buffer if necessary */
  if (maxlinelen > previous_size) {
    if ((p = realloc(inbuf,maxlinelen+1)) == NULL) {
      if (v_flag) perror("loadarray: couldn't (re)allocate input buffer");
      return NULL;
    }
    previous_size = maxlinelen;
    inbuf = p;
  }

#else  /* BROKEN_REALLOC */

  /* reallocate the input buffer if necessary */
  if (maxlinelen > previous_size) {
    if (previous_size == 0) {
      if ((p = malloc(maxlinelen+1)) == NULL) {
        if (v_flag)  perror("loadarray: couldn't allocate input buffer");
        return NULL;
      }
    } else {
      if ((p = realloc(inbuf,maxlinelen+1)) == NULL) {
        if (v_flag) perror("loadarray: couldn't reallocate input buffer");
        return NULL;
      }
    }
    previous_size = maxlinelen;
    inbuf = p;
  }


#endif /* BROKEN_REALLOC */

  /* allocate the array */
  if ((result = malloc (n * sizeof(char *)))==NULL) {
    if (v_flag) perror("loadarray: couldn't allocate base array");
    return NULL;
  }

  /* set up the input stream */
  in_fp = fopen(filename,"r");
  if (in_fp == NULL) {          /* open failed */
    free(result);
    if (v_flag) perror("loadarray: couldn't open input file");
    return NULL;
  }

  /* allocate and copy elements */
  for (i=0; i<n; i++) {
    
    /* read into the buffer */
    if(fgets(inbuf,maxlinelen+1,in_fp)==NULL) {
      /* read failed; clean up and exit */
      if (v_flag) {
        if (ferror(in_fp)) perror("loadarray: read error on input file");
        else perror ("loadarray: premature EOF on input file");
      }
      for (j=0; j<i; j++) free(result[j]);
      free(result);
      fclose(in_fp);
      return NULL;
    }

    /* copy the buffer to the array */
    result[i] = malloc(strlen(inbuf)+1);
    if (result[i]==NULL) {
      /* malloc failed; clean up and exit */
      if (v_flag) perror("loadarray: couldn't duplicate buffer");
      for (j=0; j<i; j++) free(result[j]);
      free(result);
      fclose(in_fp);
      return NULL;
    }
    strcpy(result[i],inbuf);
  }
  
  fclose(in_fp);
  return result;
}

