/*
 * Copyright 1995 by Devin Reade <gdr@myrias.com>. For distribution
 * information see the README file that is part of the manpack archive,
 * or contact the author, above.
 */

segment "utilgs____";

#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <gsos.h>
#include <orca.h>
#include "util.h"

extern GSString255Ptr __C2GSMALLOC(char *);
extern int _mapErr(int);

/*
 * access -- a replacement for the standard Gno one; this one will actually
 *           return 0 when testing X_OK on a directory.
 *
 * This one still has a bug in it:  If the file is a shell command or sys
 * file, then X_OK will return zero, but not if it is a shell script (as
 * with 'chtyp -lexec filename').
 */

int access(char *name, int mode) {
   struct stat statbuf;
   int realmode=0;

   if (stat(name,&statbuf) == -1) return -1;

   /* check read permission */
   if ((mode & R_OK) && !(statbuf.st_mode & S_IREAD)) return -1;

   /* check write permission */
   if ((mode & W_OK) && !(statbuf.st_mode & S_IWRITE)) return -1;

   /*
    * Check execute mode.  Assume directories have execute permission
    * for GS/OS.
    */
   if (mode & X_OK) {
      if (statbuf.st_mode & (S_IFDIR | S_IEXEC)) return 0;
      else return -1;
   }

   return 0;   /* file merely exists */
}


/*
 * chdir -- Replacement for the one normally shipping with Gno.
 *          Returns -1 and sets errno on failure, instead of always
 *          returning zero.
 */

int chdir (const char *pathname) {
   
   PrefixRecGS record;
   struct stat statbuf;
   int result;

   errno = 0;
   if (stat(pathname,&statbuf) == -1) return -1;

   /* verify that it's a directory */
   if (!(statbuf.st_mode & S_IFDIR)) {
      errno = ENOTDIR;
      return -1;
   }

   /* change directory */
   record.pCount = 2;
   record.prefixNum = 0;   /* prefix 0 is the current directory */
   record.buffer.setPrefix = __C2GSMALLOC(pathname);
   if (record.buffer.setPrefix == (GSString255Ptr) NULL) {
      errno = ENOMEM;
      return -1;
   }
   SetPrefixGS(&record);

   /* verify success, clean up, and return */
   result = toolerror();
   free(record.buffer.setPrefix);
   if (result) {
      errno = EINVAL;
      return -1;
   }
   return 0;
}


/*
 * getFileType -- Get the file type and auxillary file type of a file.
 *                On success it returns a pointer to an internal buffer
 *                containing the file type and aux type.  On failure
 *                it returns NULL and sets errno.
 */

fileType *getFileType (char *file) {

   static FileInfoRecGS record;
   static fileType result;
   int i;

   /* set the parameters */
   record.pCount = 4;
   if ((record.pathname = __C2GSMALLOC(file)) == NULL) {
      errno = ENOMEM;
      return NULL;
   }

   /* get the info */
   GetFileInfoGS(&record);

   /* check for errors */
   i = toolerror();
   free(record.pathname);
   if (i) {
      errno = _mapErr(i);
      return NULL;
   }

   /* set the return value */
   result.type = record.fileType;
   result.auxtype = record.auxType;

   return &result;
}


#if 0
#include <stdio.h>

int main(int argc, char **argv) {
   char *p;
   char buf[1024];
   int i;
   fileType *ft;

   if (argc!=2) return -1;

   ft = getFileType(argv[1]);
   if (ft == NULL) {
      perror(argv[1]);
      exit(1);
   } else {
      printf("type =    %x\nauxtype = %lx\n",ft->type,ft->auxtype);
   }
   return 0;
}

   printf("F_OK: %d\n",access(argv[1],F_OK));
   printf("R_OK: %d\n",access(argv[1],R_OK));
   printf("W_OK: %d\n",access(argv[1],W_OK));
   printf("X_OK: %d\n",access(argv[1],X_OK));

   if (getwd(buf) == NULL) {
      perror("getwd failed");
      exit(-1);
   }
   printf("old directory: %s\n",buf);

   if ((i = chdir(argv[1])) !=0) perror ("chdir failed");
   printf("chdir returned %d\n",i);        
   
   if (getwd(buf) == NULL) {
      perror("getwd failed");
      exit(-1);
   }
   printf("new directory: %s\n",buf);
                                     
   return 0;
}

#endif /* 0 */
