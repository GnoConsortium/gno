/*
 * rmdir - remove directory
 *
 * A quick and dirty utility for Gno.  This will delete all empty
 * directories given as arguments.  It will skip non-directory files
 * directories that aren't empty.
 *
 * If you don't compile with #define SHELL_COMD, then you just get the
 * rmdir(2) system call.
 *
 * Version 1.0 by Devin Reade <gdr@myrias.ab.ca>
 */

#include <gsos.h>
#include <orca.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define DIRECTORY 0x0F

extern GSString255Ptr __C2GSMALLOC(char *);
extern int _mapErr(int);
extern char *strerror(int errnum);
extern void begin_stack_check(void);
extern int end_stack_check(void);

typedef struct DestroyRecGS {
   Word pCount;
   GSString255Ptr pathname;
} DestroyRecGS, *DestroyRecPtrGS;

int rmdir (const char *path) {
   DestroyRecGS drec;
   FileInfoRecGS frec;
   int result;

   /* make a GSString copy of path */
   frec.pCount=3;
   if ((frec.pathname = __C2GSMALLOC(path)) == NULL) {
      errno = ENOMEM;
      return -1;
   }

   /* check to ensure that it's a directory */
   GetFileInfoGS(&frec);
   if ((result = toolerror())!=0) {
      errno = _mapErr(result);
      free(frec.pathname);
      return -1;
   }
   if (frec.fileType != DIRECTORY) {
      errno = ENOTDIR;
      free(frec.pathname);
      return -1;
   }

   /* it's a directory; try to delete it */
   drec.pCount=1;
   drec.pathname = frec.pathname;
   DestroyGS(&drec);
   if ((result = toolerror())!=0) {
      errno = _mapErr(result);
      free(frec.pathname);
      return -1;
   }

   /* it's been deleted.  Clean up and return */
   free(frec.pathname);
   return 0;
}

#ifdef SHELL_COMD

int main(int argc, char **argv) {
   int i, result;

#ifdef CHECK_STACK
   begin_stack_check();
#endif
         
   result = 0;
   for (i=1; i<argc; i++) {            /* loop over all filenames */

      if (rmdir(argv[i])!=0) {
         fprintf(stderr,"%s: %s: %s.  File skipped.\n",argv[0],argv[i],
                 strerror(errno));
         result = 1;
      }
   }

#ifdef CHECK_STACK
   fprintf(stderr,"stack usage: %d bytes\n",end_stack_check());
#endif

   return result;
}

#endif /* SHELL_COMD */
