/*
 * Copyright 1996 Devin Reade <gdr@myrias.com>.
 * All rights reserved.
 *
 * For copying and distribution information, see the file "COPYING"
 * accompanying this file.
 *
 * $Id: inst.c,v 1.1 1996/03/31 23:38:33 gdr Exp $
 */

#include <stdio.h>
#include <getopt.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <types.h>
#include <sys/stat.h>
#include <orca.h>
#include "install.h"

/* actions */
#define NOCHANGE   0
#define ASSIGN    1
#define REMOVE    2
#define ADD       3

/* permissions */
#define S_USER    0700
#define S_GROUP   0070
#define S_OTHER   0007
#define S_ALL      0777
#define S_READ    0444
#define S_WRITE   0222
#define S_EXECUTE 0111

#define TYPE_TXT       0x04
#define TYPE_SRC       0xB0
#define TYPE_EXEC      0x00000006
#define TYPE_NONE      0x00000000

#define VERSION "1.0"
#define EMAIL   "<gdr@myrias.com>"

char *versionMsg = "Version %s by Devin Reade %s\n";
int  dFlag;

extern int mkdir(const char *);
extern int needsgno(void);
extern void begin_stack_check(void);
extern int  end_stack_check(void);

/*
 * usage
 *
 * display usage information and exit
 */

void
usage (void)
{
  fputs("Usage: install [-cdhsv] [-o owner] [-g group] [-m mode] ",stderr);
  fputs("source [...] dest\n\n",stderr);
  fputs("Options:\n",stderr);
  fputs("\t-c             Ignored.  (Backwards Unix compatibility)\n",stderr);
  fputs("\t-d             Create the specified directories\n",stderr);
  fputs("\t-g group       Specify group id (not implemented)\n",stderr);
  fputs("\t-h             Show usage information and exit.\n",stderr);
  fputs("\t-m mode        Specify (Unix) access mode\n",stderr);
  fputs("\t-o owner       Specify owner id (not implemented)\n",stderr);
  fputs("\t-s             Strip binary (not implemented)\n",stderr);
  fputs("\t-v             Show version number\n\n",stderr);
  fprintf(stderr,versionMsg,VERSION,EMAIL);
   exit(1);
}

/*
 * getmode
 *
 * set mode to the value corresponding to the permission bit string
 * <str>.  If the first char of <str> is a digit, then it is assumed
 * to be an octal number.  Otherwise it is assumed to be a string of
 * the form "ug+rx" (in the usual chmod(1) format).  Also sets action
 * to be the type of action to take, whether we're removing, adding,
 * or assigning the permission bits.
 *
 * If these assumptions don't hold, then return non-zero.  Returns
 * zero and sets mode on success.
 *
 * Since the IIgs currently doesn't have the concept of "group" and
 * "other" permissions, we take everything from the user permissions.
 */

int
getmode (char *str, unsigned long *mode, int *action)
{
  unsigned long who  = 0L;
  unsigned long perm = 0L;
  char *p, *q;

  /* octal number? */
  if (isdigit(*str)) {
    *action = ASSIGN;
    errno = 0;
    *mode = strtoul(str,NULL,8);
    return errno;
  }
  
  /* it's not an absolute octal; treat as a string */
  if (((p = strchr(str,'+')) == NULL) &&
      ((p = strchr(str,'-')) == NULL) &&
      ((p = strchr(str,'=')) == NULL)) {
    errno = EINVAL;
    return errno;
  }
  switch (*p) {
  case '+': *action = ADD;    break;
  case '-': *action = REMOVE; break;
  case '=': *action = ASSIGN; break;
  default:    assert(0);
  }

  /*
   * this condition should really be deduced from the umask, if it
   * were supported.
   */
  if (str == p) who |= S_USER;

  for (q = str; q<p; q++) {
    switch (*q) {
    case 'u':  who |= S_USER;    break;
    case 'g':  who |= S_GROUP;   break;
    case 'o':  who |= S_OTHER;   break;
    case 'a':  who |= S_ALL;     break;
    default:   errno = EINVAL;   return errno;
    }
  }

  for (q = p+1; *q; q++) {
    switch (*q) {
    case 'r':  perm |= S_READ;           break;
    case 'w':  perm |= S_WRITE;  break;
    case 'x':  perm |= S_EXECUTE;   break;
    case 's':  /* ignored */     break;
    default:   errno = EINVAL;           return errno;
    }
  }

  /* currently: ignore all but user permissions */
  if (!(who & S_USER)) {
    *action = NOCHANGE;
  }

  *mode = who & perm;
  return 0;
}

/*
 * mkdirs
 *
 * argv is assumed to be an array of argc pathnames.  mkdirs will
 * create all the listed directories, including their parents if
 * necessary.
 *
 * Returns zero on success.  Returns non-zero and prints a suitable
 * error message on failure.
 */

int
mkdirs (int argc, char **argv)
{
   static struct stat statbuf; /* reduce stack space */
   char *path, *p;
  size_t pathlen;
  int result = 0;
  int makeit;       /* do we try a mkdir()? */
  int abortpath;    /* we saw an error; don't both with rest of path */
  int i,j;
  int coloncount;

  /* loop over each of the pathnames in the array */
  for (i=0; i<argc; i++) {

      /* expand to a full pathname */
      if ((path = expandpath(argv[i]))==NULL) {
         perror(argv[i]);
        continue;
     }
     pathlen = strlen(path);
      
     /* is this pathname legal? */
     /* place a call to JudgeName() [custom] here */

     /* find out how many path components there are */
     coloncount = 0;
     p = path;
     while (*p) {
         if (*p == ':') {
            coloncount++;
        }
        p++;
     }
     p = path + 1;

     /* skip the volume name */
     assert((p = strchr(p,':'))!=NULL);
      p++;
     --coloncount;

     /* create each component in path */
     abortpath = 0;
     for (j=0; !abortpath && j<coloncount; j++) {
         if ((p = strchr(p,':')) == NULL) {
            p = path + pathlen;
        }
        *p = '\0';

        if (stat(path,&statbuf) != 0) {
            if (errno == ENOENT) {
               makeit = 1;
           } else {
               perror(path);
              makeit = 0;
              abortpath = 1;
              result = 1;
           }
        } else {
            makeit = 0;
           if (statbuf.st_mode & S_IFDIR == 0) {
               fprintf(stderr,"%s exists and is not a directory\n",path);
              abortpath = 1;
               result = 1;
           } /* else it exists and is a directory */
        }
   
         /* go ahead and create the directory */
        if (makeit && mkdir(path)) {
            perror(path);
           abortpath = 1;
            result = 1;
        }

         /* reinstate the ':' that we "nulled-out" */
        if (p != path + pathlen) {
         *p++ = ':';
        }
     }
     free(path);
  }
  return result;
}

/*
 * copyfiles
 *
 * <argv> is assumed to be an array of <argc> filenames.
 *
 * This routine copies all but the last specified file to the directory
 * or filename specified by the last filename.  If argc>2, the last element
 * _must_ be a directory.
 *
 * Returns zero on success.  On failure, returns the last non-zero errno
 * and prints error conditions to stderr.
 *
 * If action is not NOCHANGE, this routine will also set file permissions
 * as specified in the install(1) man page.  This may involve changing
 * the file type.
 */

static int
copyfiles (int argc, char **argv, int action, unsigned long mode)
{
   static FileInfoRecGS inforec;
  static GSString255   filenameGS;
  int i,j;
  int result=0;
  char *destination;
  Word newaccess;

  if (argc < 2) {
      errno = EINVAL;
     perror("internal error: not enough arguments to copyfiles()");
     return errno;
  }
  if (argc > 2) {
   
   /* find out if argv[argc-1] is a directory */

     if (__C2GS(argv[argc-1], &filenameGS) == NULL) {
         errno = EINVAL;
        perror("destination path too long");
        return errno;
     }
      inforec.pCount = 5;
     inforec.pathname = &filenameGS;
   GetFileInfoGS(&inforec);
   if ((errnoGS = toolerror()) != 0) {
        perrorGS("%s",argv[argc-1]);
         errno = _mapErr(errnoGS);
      return -1;
   }
   if ((inforec.storageType != 0x0D) && (inforec.storageType != 0x0F)) {
        errno = ENOTDIR;
         perror(argv[argc-1]);
        return errno;
     }
  }   

  --argc;
  for (i=0; i<argc; i++) {
      if ((destination = copyfile (argv[i],argv[argc])) == NULL) {
         errnoGS = toolerror();
         perrorGS("install of %s to %s",argv[i],argv[argc]);
         result = errno = _mapErr(errnoGS);
      }
     if (action == NOCHANGE) continue;

     /* get the file info for the source file */
     assert(__C2GS(argv[i],&filenameGS));
     inforec.pCount = 7;
     inforec.pathname = &filenameGS;
     GetFileInfoGS(&inforec);
     if ((errnoGS = toolerror()) != 0) {
         perrorGS("GetFileInfo for %s failed",argv[i]);
         result = errno = _mapErr(errnoGS);
     }

     /* modify the permissions as necessary */
     switch (action) {
     case ASSIGN:
         newaccess = 0xFFFF;
         if (!(mode & S_READ))  newaccess &= ~readEnable;
        if (!(mode & S_WRITE)) newaccess &= ~writeEnable;
         inforec.access &= newaccess;

        if ((mode & S_EXECUTE) &&
            (inforec.fileType == TYPE_TXT) || (inforec.fileType == TYPE_SRC)) {
            inforec.fileType = TYPE_SRC;
           inforec.auxType  = TYPE_EXEC;
        }
         break;

     case ADD:
         if (mode & S_READ)  inforec.access |= readEnable;
        if (mode & S_WRITE) inforec.access |= writeEnable;

        if ((mode & S_EXECUTE) &&
            (inforec.fileType == TYPE_TXT) || (inforec.fileType == TYPE_SRC)) {
            inforec.fileType = TYPE_SRC;
           inforec.auxType  = TYPE_EXEC;
        }
         break;                         

     case REMOVE:
         if (mode & S_READ)  inforec.access &= ~readEnable;
        if (mode & S_WRITE) inforec.access &= ~writeEnable;

        if ((mode & S_EXECUTE) &&
            (inforec.fileType == TYPE_TXT) || (inforec.fileType == TYPE_SRC)) {
            inforec.fileType = TYPE_TXT;
           inforec.auxType  = TYPE_NONE;
        }
         break;

     default:
         assert(0);
     }

     /* set the modified file info for the destination file */
     assert(__C2GS(destination,&filenameGS));
      SetFileInfoGS(&inforec);
     if ((errnoGS = toolerror()) != 0) {
         perrorGS("SetFileInfo for %s failed",destination);
         result = errno = _mapErr(errnoGS);
     }
  }
  return result;
}

/*
 * obvious ...
 */

int
main (int argc, char **argv)
{
  unsigned long mode;
  int c, nfiles;
   int action = NOCHANGE;

#ifdef CHECK_STACK
   begin_stack_check();
#endif

  if (needsgno()==0) {
      fprintf(stderr,"Requires GNO/ME\n");
     exit(1);
  }

  /* initialize */
  dFlag   = 0;
  mode    = 0L;
  
  /* parse command line */
  while ((c = getopt(argc,argv,"cdg:hm:o:sv")) != EOF) {
    switch (c) {
    case 'v':
      fprintf(stderr,versionMsg,VERSION,EMAIL);
      exit(1);
      break;
      
    case 'm':
      if (getmode(optarg,&mode,&action)) usage();
      break;
      
    case 'd':  dFlag++;
    case 'c': /* not implemented */
    case 'g':  /* not implemented */
    case 'o':  /* not implemented */
    case 's':  /* not implemented */
      break;
      
    case 'h':
    default:      usage();
    }
  }

  nfiles = argc - optind;

  if (dFlag) {
     if (nfiles < 1) usage();
    c = mkdirs(nfiles,&argv[optind]);
  } else {
     if (nfiles < 2) usage();
    c = copyfiles(nfiles, &argv[optind], action, mode);
  }

#ifdef CHECK_STACK
   fprintf(stderr,"stack usage: %d bytes\n",end_stack_check());
#endif

  return c;
}
