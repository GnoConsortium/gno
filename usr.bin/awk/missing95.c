/* these routines are not part of win 95 and nt;
   they are included here so you can compile. */

#include <stdio.h>
FILE *popen(char *s, char *m) { return NULL; }
int pclose(FILE *f) { return 0; }
