/*
 * Copyright 1996 Devin Reade <gdr@myrias.com>.
 * All rights reserved.
 *
 * For copying and distribution information, see the file "COPYING"
 * accompanying this file.
 *
 * $Id: copyfile.c,v 1.1 1996/03/31 23:38:31 gdr Exp $
 */

#include <errno.h>
#include <gsos.h>
#include <orca.h>
#include <string.h>
#include <stdlib.h>
#include "install.h"

/* the chunk size in which we copy files */
#define COPY_BUFFER_SIZE 1024

/*
 * copyfile
 *
 * copy a file from the pathname <from> to the location <to>, which
 * may be a directory.  Ensure that file types and other information
 * (except for the backup bit) is matched.
 *
 * Returns NULL and sets errno on failure.  On success, returns a
 * pointer to an internal buffer containing the final pathname.
 *
 * +++ THIS ROUTINE IS NOT REENTRANT +++
 */

char *
copyfile (char *from, char *to)
{
   static char buffer[COPY_BUFFER_SIZE];
   static FileInfoRecGS inforec;
  static OpenRecGS openrec;
  static ExpandPathRecGS expandrec;
  static ResultBuf255 resultbuf;
  static struct {
      Word pCount;
     Word refNum;
     Longword dataBuffer;
     Longword requestCount;
     Longword transferCount;
     Word cachePriority;
  } iobuf;
  static struct {
      Word pCount;
     Word refNum;
  } closerec;
  static GSString255 fromGS, toGS;
  static char *result = NULL;        /* we only use this if our path is */
                                     /* exactly 255 chars long */
  size_t len1, len2;
  Word refNumIn, refNumOut;          /* GS/OS ref numbers for I/O */
   int isDir, i, j, k, done;
  char *p, *q, *r;

  /* concheck and convert filenames to GSString255 type */
  if (!from || !to ||
      ((len1 = strlen(from)) > 254) ||
      ((len2 = strlen(to))   > 254) )
  {
      errno = EINVAL;
     return NULL;
  }
  fromGS.length = len1;
  toGS.length = len2;
  strcpy(fromGS.text,from);
  strcpy(toGS.text,to);

   /* expand the original file name */
   expandrec.pCount = 3;
  expandrec.inputPath = &fromGS;
  expandrec.outputPath = &resultbuf;
  expandrec.flags = 0x0000;
  resultbuf.bufSize = 255;
  ExpandPathGS(&expandrec);
  if ((i = toolerror()) != 0) {
      errno = _mapErr(i);
     return NULL;
  }
  strcpyGSString255(&fromGS,&(resultbuf.bufString));

   /* expand the destination name */
   expandrec.pCount = 3;
  expandrec.inputPath = &toGS;
  expandrec.outputPath = &resultbuf;
  expandrec.flags = 0x0000;
  resultbuf.bufSize = 255;
  ExpandPathGS(&expandrec);
  if ((i = toolerror()) != 0) {
      errno = _mapErr(i);                                        
     return NULL;
  }
  strcpyGSString255(&toGS,&(resultbuf.bufString));
  
  /* find out if <to> is a directory */
   inforec.pCount = 5;
  inforec.pathname = &toGS;
  GetFileInfoGS(&inforec);
  i = toolerror();
  switch(i) {
  case 0:
   isDir = ((inforec.storageType == 0x0D) ||
              (inforec.storageType == 0x0F)) ? 1 : 0;
      break;
  case fileNotFound:
      isDir = 0;
     break;
   default:
      errno = _mapErr(i);
     return NULL;
  }

  /* it's a directory? tack on the file name */
  if (isDir) {
   
     /* expand the directory name */
      expandrec.pCount = 3;
     expandrec.inputPath = &toGS;
     expandrec.outputPath = &resultbuf;
     expandrec.flags = 0x0000;
     resultbuf.bufSize = 255;
     ExpandPathGS(&expandrec);
     if ((i = toolerror()) != 0) {
         errno = _mapErr(i);
        return NULL;
     }

     /* tack on the final component */
      p = basename(from);
     len1 = strlen(p);
     if (len1 + toGS.length + 1 > 255) {
         errno = EINVAL;
        return NULL;
     }
     q = &(toGS.text[toGS.length]);
     r = p;
     *q++ = ':';
     for (i=0; i<len1; i++) {
         *q++ = *r++;
     }
     toGS.length += len1 + 1;
   }

  /* check to see it's not the same file */
  if (strcmpGSString255(&fromGS, &toGS) == 0) {
      errno = EINVAL;
     return NULL;
  }

  /* get the file info of the original file */
   inforec.pCount = 7;
  inforec.pathname = &fromGS;
  GetFileInfoGS(&inforec);
  if ((i = toolerror()) != 0) {
      errno = _mapErr(i);
     return NULL;
  }

  /* destroy the old target file if it exists */
  inforec.pCount = 1;
  inforec.pathname = &toGS;  DestroyGS(&inforec);
  i = toolerror();
  switch(i) {
  case 0:
  case fileNotFound:
      break;
  default:
      errno = _mapErr(i);
     return NULL;
  }

   /* create the new file */
   inforec.pCount = 5;
  inforec.pathname = &toGS;
  CreateGS(&inforec);
  if ((i = toolerror()) != 0) {
      errno = _mapErr(i);
     return NULL;
  }

  /* copy both forks, if necessary */
  for (i=0; i< ((inforec.storageType == extendedFile) ? 2 : 1); i++) {

   /* open the input file */
     openrec.pCount = 4;
     openrec.pathname = &fromGS;
     openrec.requestAccess = readEnable;
     switch (i) {
     case 0:
         openrec.resourceNumber = 0x0000;
        break;
     case 1:
         openrec.resourceNumber = 0x0000;
        break;
     }
     OpenGS(&openrec);
     if ((j = toolerror()) != 0) {
         errno = _mapErr(j);
        return NULL;
     }
     refNumIn = openrec.refNum;
   
   /* open the output file */
     openrec.pathname = &toGS;
     openrec.requestAccess = writeEnable;
     OpenGS(&openrec);
     if ((j = toolerror()) != 0) {
         closerec.pCount = 1;
        closerec.refNum = refNumIn;
        CloseGS(&closerec);
         errno = _mapErr(j);
        return NULL;
     }
     refNumOut = openrec.refNum;

     /* transfer the data */
     done = 0;
     iobuf.pCount = 5;
     iobuf.dataBuffer = (Longword) &buffer;
     iobuf.cachePriority = cacheOn;
     while (!done) {

         iobuf.refNum = refNumIn;
         iobuf.requestCount = COPY_BUFFER_SIZE;
        ReadGS(&iobuf);
        k = toolerror();
        switch (k) {
        case 0:
            break;
        case eofEncountered:
            done = 1;
           break;
        default:
            closerec.pCount = 1;
         closerec.refNum = refNumIn;
         CloseGS(&closerec);
         closerec.refNum = refNumOut;
         CloseGS(&closerec);
            errno = _mapErr(k);
           return NULL;
         }

        iobuf.refNum = refNumOut;
        iobuf.requestCount = iobuf.transferCount;
        WriteGS(&iobuf);
        if ((k = toolerror()) != 0) {
            closerec.pCount = 1;
         closerec.refNum = refNumIn;
         CloseGS(&closerec);
         closerec.refNum = refNumOut;
         CloseGS(&closerec);
            errno = _mapErr(k);
           return NULL;
        }
     }     /* end loop over buffering */
      closerec.pCount = 1;
   closerec.refNum = refNumIn;
   CloseGS(&closerec);
   closerec.refNum = refNumOut;
   CloseGS(&closerec);
  }        /* end loop over forks */

  /* set file information to match original file */
   inforec.pCount = 7;
  inforec.pathname = &toGS;
  SetFileInfoGS(&inforec);
  if ((i = toolerror()) != 0) {
      errno = _mapErr(i);
     return NULL;
   }
  if (toGS.length == 255) {
      if (result) free(result);
     result = __GS2CMALLOC(&toGS);
     return result;
  }
  toGS.text[toGS.length]='\0';
  return toGS.text;
}
