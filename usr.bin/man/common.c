/*
 * Copyright 1995 by Devin Reade <gdr@myrias.com>. For distribution
 * information see the README file that is part of the manpack archive,
 * or contact the author, above.
 */

segment "common____";

#include <types.h>
#include <string.h>
#include "man.h"

/*
 * getSuffixIndex
 *
 * return the index into compressArray of the appropriate
 * suffix/decompresser.  If there is no match, return -1.
 */

int getSuffixIndex(char *name) {
   char *p;
   int i;

   for (i=0; compressArray[i].suffix != NULL; i++) {
      p = strstr(name,compressArray[i].suffix);
      if (p && !*(p + strlen(compressArray[i].suffix))) return i;
   }
   return -1;
}
