/*
 * Memory allocation-related routines that call exit(3) if they fail.
 *
 * Devin Reade, September 1997
 *
 * $Id: xalloc.c,v 1.1 1997/10/03 04:49:40 gdr Exp $
 */

#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <contrib.h>

void *
LC_xmalloc (size_t size) {
  char *result;

  if ((result = malloc(size)) == NULL) {
    err(1, "malloc failed");
    exit(EXIT_FAILURE);
  }
  return (void *) result;
}

void *
LC_xrealloc (void *ptr, size_t size) {
  char *result;

  if ((result = realloc(ptr, size)) == NULL) {
    err(1, "realloc failed");
    exit(EXIT_FAILURE);
  }
  return (void *) result;
}

char *
LC_xstrdup (const char *str) {
  char *result;

  if ((result = strdup(str)) == NULL) {
    err(1, "strdup failed");
    exit(EXIT_FAILURE);
  }
  return result;
}

