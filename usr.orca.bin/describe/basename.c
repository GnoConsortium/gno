/*
 * $Id: basename.c,v 1.2 1997/09/24 06:34:57 gdr Exp $
 */

#include <string.h>
#include <stdio.h>
#include <unistd.h>

char *basename (char *name) {
   char *p, brk;

   /* checking for ':' is GS-specific */
   brk = (strchr(name,':')) ? ':' : '/';
   p = strrchr(name,brk);

   return ((p) ? p+1 : name);
}


#ifdef SHELLCOMD

int main (int argc, char **argv) {

   if (argc != 2) {
      fprintf(stderr,"Usage:  basename file_name\nVersion 1.0\n");
      return -1;
   }
   printf("%s\n",basename(argv[1]));
   return 0;
}

#endif  /* SHELLCOMD */
