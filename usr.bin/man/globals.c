/*
 * Copyright 1995 by Devin Reade <gdr@myrias.com>. For distribution
 * information see the README file that is part of the manpack archive,
 * or contact the author, above.
 */

#include <types.h>
#include "man.h"

/*
 * The compression suffixes and how to uncompress the files.
 * If you use ".l" (that's "ell") as a suffix, you will break the
 * algorithm for dereferencing aroff "links".
 */

compressionType compressArray[] = {
   { ".Z",  "compress -cd" },
   { ".F",  "freeze -cd" },
   { ".gz", "gzip -cd" },
   { NULL,  NULL }
};

char linebuf[BUFFERSIZE];
char linebuf2[BUFFERSIZE];
char *manpath;

Section sections[] = {
   { "1",      "1" },
   { "2",      "2" },
   { "3",      "3" },
   { "3f",     "3f" },
   { "4",      "4" },
   { "5",      "5" },
   { "6",      "6" },
   { "7",      "7" },
   { "8",      "8" },
   { "new",    "n" },    /* the words "new", "local", "public",  */
   { "local",  "l" },    /* and "old" can be abbreviated on the  */
   { "public", "p" },    /* command line with "n", "l", "p", and */
   { "old",    "o" },    /* "o", respectively                    */
   { NULL, NULL }        /* MUST be NULL-terminated! */
};
