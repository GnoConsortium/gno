/*
 * fgetln(3) implementation.
 *
 * Devin Reade, April 1997
 *
 * This file is formatted with tab stops every 8 columns
 *
 * $Id: fgetln.c,v 1.2 1997/07/27 23:13:28 gdr Exp $
 */

#ifdef __ORCAC__
segment "libc_stdio";
#endif

#pragma optimize 0
#pragma debug 0
#pragma memorymodel 0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINESIZE 20

char *
fgetln (FILE *stream, size_t *len)
{
  static char *buffer = NULL;
  static size_t currentBufferSize = 0;
  char *p, *q;

  if (buffer == NULL) {
    currentBufferSize = LINESIZE;
    if ((buffer = malloc (currentBufferSize)) == NULL) {
      return NULL;
    }
  }


  p = fgets(buffer, currentBufferSize, stream);
  if (p == NULL) {
    return NULL;
  }

  for (;;) {
    if ((p = strchr(buffer, '\n')) != NULL) {
      break;
    }
    p = buffer + strlen(buffer);
    if ((q = realloc(buffer, currentBufferSize + LINESIZE)) == NULL) {
      return NULL;
    }
    buffer = q;
    currentBufferSize += LINESIZE;
    if (fgets(p, LINESIZE, stream) == NULL) {
      if (ferror(stream)) {
	return NULL;
      } else {
	break;
      }
    }
  }
  *len = strlen(buffer);
  return buffer;
}
