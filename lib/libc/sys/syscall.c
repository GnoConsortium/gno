/*
 * libc/sys/syscall.c
 *
 * System Call (Trap) Interface.  This file contains those functions
 * which are in Chapter 2 but emulate system traps as opposed to directly
 * trapping to the GNO kernel.  The actual kernel trap calls are in trap.c
 * There are also a few support routines in here.
 *
 * Unless otherwise specified, see the respective man pages for details
 * about these routines.
 *
 * $Id: syscall.c,v 1.2 1997/07/27 23:33:36 gdr Exp $
 *
 * This file is formatted with tab stops every 3 columns.
 */

#ifdef __ORCAC__
segment "libc_sys__";
#endif

#pragma debug 0
#pragma memorymodel 0

/*
 * Use bits 0, 1, 2, 6 (== decimal 71) for optimization.  In Orca/C v2.1.0,
 * bits 4 and 5 are still reported to be buggy.
 *
 * Around variadic routines, we also add in optimization bit 3 (== 79).
 */

/* pragma optimize 71 */

#include <sys/syslimits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <types.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <gsos.h>
#include <string.h>
#include <stdio.h>
#include <orca.h>
#include <limits.h>
#include <signal.h>
#include <gno/gno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/mount.h>

/*
 * This is the maximum file name length allowed to be returned by GS/OS
 * in these stubs.
 */

#define GSOS_NAME_MAX PATH_MAX

/* these are prototyped in the Orca/C manual, but not in any header */
extern pascal void SystemQuitFlags (unsigned int);
extern pascal void SystemQuitPath (GSStringPtr);

/* these are the access bits to the GS/OS Create call */
#define GSOS_READ      0x0001
#define GSOS_WRITE     0x0002
#define GSOS_INVISIBLE 0x0004
#define GSOS_BACKUP    0x0020
#define GSOS_RENAME    0x0040
#define GSOS_DESTROY   0x0080

/* file types and, for EXEC, the auxtype */
#define TXT  0x04		/* text file */
#define BIN  0x06		/* binary file */
#define EXE  0xB5      /* shell command */
#define DIR  0x0F      /* directory */
#define S16  0xB3      /* system file (application) */
#define SRC  0xB0      /* SRC + EXEC = shell script */
#define EXEC 0x0006

#ifdef VERSION_CHECK
static void _libcPanic (const char *fmt, ...);
#endif

/*
 * _chdir
 *
 * chdir and fchdir are implemented in terms of this function.  Note that
 * <pathname> _might_ be changed by _chdir() -- if an error occurs setting
 * prefix 0, then the length of <pathname> will be set to zero.
 */

static int
_chdir(GSStringPtr pathname) {
	PrefixRecGS prefx;
  struct {			/* truncated version of GetFileInfoRec */
	   word pCount;
     GSString255Ptr pathname;
     word access;
     word fileType;
     longword auxType;
     word storageType;
  } shortFileInfo;
  int err;

  /* make sure it's a directory */
  shortFileInfo.pCount = 5;
  shortFileInfo.pathname = (GSString255Ptr) pathname;
  GetFileInfoGS(&shortFileInfo);
  if ((errno = _mapErr(_toolErr)) != 0) {
     return -1;
  }
  if (shortFileInfo.storageType != 0x0d &&		/* subdirectory */
  	 shortFileInfo.storageType != 0x0f)	{		/* volume directory */
     errno = ENOTDIR;
     return -1;
  }
      
  prefx.pCount = 2;
  prefx.prefixNum = 8;
  prefx.buffer.setPrefix = shortFileInfo.pathname;
  SetPrefixGS(&prefx);
	if ((errno = _mapErr(_toolErr)) != 0) {
     return -1;
  }

  prefx.prefixNum = 0;
  if (prefx.buffer.setPrefix->length < 64) {
	   SetPrefixGS(&prefx);
     if (_toolErr == 0) {
        return 0;
     }
  }
  prefx.buffer.setPrefix->length = 0;
	SetPrefixGS(&prefx);
  return 0;
}

#define CHMOD_MODE    1
#define CHMOD_TYPE    2
#define CHMOD_AUXTYPE 4

static int
_chmod (unsigned short op, GSStringPtr path, mode_t mode, unsigned short type,
        unsigned long auxtype) {
	FileInfoRecGS *infop;
  int err;

  if ((infop = malloc(sizeof(FileInfoRecGS))) == NULL) {
	   return -1;
  }
  infop->pCount = 4;
  infop->pathname = (GSString255Ptr) path;

  /* get the original data */
  GetFileInfoGS(infop);
  if (_toolErr) {
	   err = _toolErr;
     free(infop);
  	errno = _mapErr(err);
     return -1;
  }

  /*
   * Special case:  If the type is TXT or SRC, *and* the S_IXUSR bit
   * is set, *and* this is a "UNIX" mode, *and* no filetype or auxtype
   * was specified, then change the type to SRC and the AUXTYPE to EXEC.
   */
  if (((op & (CHMOD_TYPE | CHMOD_AUXTYPE)) == 0) &&
      (mode & S_IXUSR) &&
      _getModeEmulation() &&
      (infop->fileType == TXT || infop->fileType == SRC))
  {
	   infop->fileType = SRC;
     infop->auxType = EXEC;
  }

  /* modify it */
  if (op & CHMOD_MODE) {
 		infop->access = _mapMode2GS(mode);
  }
  if (op & CHMOD_TYPE) {
	   infop->fileType = type;
  }
  if (op & CHMOD_AUXTYPE) {
	   infop->auxType = auxtype;
  }

	/* set the info and return */
  SetFileInfoGS(infop);
	err = _toolErr;
  free(infop);
  if (err) {
  	errno = _mapErr(err);
     return -1;
  } else {
	   return 0;
  }
}

/*
 * _kernMinVersion
 *
 * This is used as an assert from within trap.asm (unless the global short
 * _kernDisableVersionCheck is set to a non-zero value.  It's argument is
 * the kernel version required by the given system call.  Returns on success.
 * On failure, it aborts the program with a suitable error message.
 *
 * It would be more efficient as an inline macro.
 *
 * THIS ROUTINE IS CURRENTLY DISABLED DUE TO PERFORMANCE CONCERNS.  SEE 
 * THE gno-devel MAILING LIST ARCHIVE FOR DETAILS.
 */

#ifdef VERSION_CHECK

unsigned short _kernDisableVersionCheck;

void
_kernMinVersion (unsigned int required) {
	static int gnoActiveKnown = 0;
  static u_short gnoVersion = 0;

  /* make sure GNO is active */
  if (! gnoActiveKnown) {
		kernStatus();
     if (_toolErr) {
	      _libcPanic("This program requires GNO.\n");
        /*NOTREACHED*/
     }
     gnoActiveKnown = 1;
  }

	/* get the current kernel version if we don't already have it */
  if (gnoVersion == 0) {
	   gnoVersion = kernVersion();
  }

  /* make sure our version meets the minimum required */
  if (required > gnoVersion) {
	   _libcPanic ("This program requires GNO v%d.%d.%d or later\n",
                 (required & 0xFF00) >> 8,
                 (required & 0x00F0) >> 4,
                 (required & 0x000F));
  }

	return;
}

#endif

/*
 * _setFdTranslation, _getFdTranslation
 *
 * Does newline translation occur for read/write calls on the specified
 * file descriptor?  Off by default.  Returns previous value for the
 * specified fd.
 *
 * If by any chance we manage to open more than OPEN_MAX files, then
 * translation is always off for those file descriptors.
 */

static int fdTranslationTable[OPEN_MAX];

#define _getFdTranslation(fd) \
	(((fd) >= 0) && ((fd) < OPEN_MAX) && fdTranslationTable[fd])

static int
_setFdTranslation(int fd, int isOn) {
	int oldval;

	if (fd < 0 || fd >= OPEN_MAX) {
	   return 0;
  }
  oldval = fdTranslationTable[fd];
  fdTranslationTable[fd] = (isOn != 0);
  return oldval;
}

/*
 * _statfs
 *
 * _statfs is a routine common to both statfs and fstatfs.  The first
 * parameter, gstr, is a pointer to a GSString containing the pathname.
 * Other than the type of the first argument, this call is identical
 * to the regular definition of statfs.
 */

static int
_statfs (GSStringPtr gstr, struct statfs *buf) {
	DevNumRecGS gd;
	VolumeRecGS vo;
  char printbuf[20];	/* device name in .dxx format */
  int err;

  /* get the volume number for the file name gstr */
  gd.pCount = 2;
  gd.devName = (GSString32Ptr) gstr;	/* does this work with a pathname? */
  GetDevNumberGS(&gd);
  if (_toolErr) {
  	errno = _mapErr(_toolErr);
     return -1;
 	}

  /* get the other volume info */
  vo.pCount = 6;
  sprintf(printbuf,".d%d", gd.devNum);
  vo.devName = (GSString32Ptr) __C2GSMALLOC(printbuf);
  vo.volName = (ResultBuf255Ptr) GOinit(32, NULL);
  VolumeGS(&vo);
  err = _toolErr;

  /* copy over our information */
  buf->f_type     = (long) vo.fileSysID; /* FST type */
  buf->f_bsize    = (long) vo.blockSize; /* size of blocks in filesystem */
  buf->f_blocks   = vo.totalBlocks; /* number of blocks on the volume */
  buf->f_bfree    = vo.freeBlocks;  /* number of free blocks */
  buf->f_bavail   = vo.freeBlocks;  /* none reserved for superuser */
  buf->f_files    = -1;				 /* undefined by this filesystem */
  buf->f_ffree    = -1;				 /* undefined by this filesystem */
  buf->f_fsid.hi  = 0;
  buf->f_fsid.lo  = gd.devNum;	    /* device number */
  buf->f_spare[0] = -1;
  buf->f_spare[1] = -1;
  buf->f_spare[2] = -1;
  buf->f_spare[3] = -1;
  buf->f_spare[4] = -1;
  buf->f_spare[5] = -1;
  buf->f_spare[6] = -1;

  /* clean up, set up return conditions */
  GOfree(vo.volName);
  free(vo.devName);
  if (err) {
  	errno = _mapErr(err);
     return -1;
  }
  return 0;
} 

/*
 * access -- a replacement for the GNO v2.0.4 one; this one will actually
 *           return 0 when testing X_OK on a directory.
 *
 * This function uses gotos.  Too bad; sometimes it's more efficient.
 */

int
access (const char *name, int mode) {
  FileInfoRecGS *recptr;
  GSStringPtr gptr;
  size_t len;
  int i;
  int result = 0;

  /* verify validity of args */
  if (!name || !*name) { /* for SYSV */
	   errno=ENOENT;
     return -1;
  }
  len = strlen(name);
  if (len >= USHRT_MAX || (mode & ~(R_OK|W_OK|X_OK|F_OK))) {
	   errno = EINVAL;      
     return -1;
  }

  /* allocate and initialize the GS/OS variables */
  if ((gptr = GIinit(len, name)) == NULL) {
	   errno = ENOMEM;
     return -1;
  }
  if ((recptr = malloc(sizeof(FileInfoRecGS))) == NULL) {
	   GIfree(gptr);
     errno = ENOMEM;
     return -1;
  }
  recptr->pCount = 4;
  recptr->pathname = (GSString255Ptr) gptr;

  /* get the info and check for errors */
  GetFileInfoGS(recptr);
  i = toolerror();
  if (i) {
  	errno = _mapErr(i);
     result = -1;
     goto done;
  }

  /* check read permission */
  if ((mode & R_OK) && !(recptr->access & GSOS_READ)) {
     errno = EACCES;
  	result = -1;
     goto done;
  }

  /* check write permission */
  if ((mode & W_OK) &&
      !((recptr->access & GSOS_WRITE) &&
        (recptr->access & GSOS_RENAME) &&
        (recptr->access & GSOS_DESTROY))) {
     errno = EACCES;
  	result = -1;
     goto done;
  }

  /*
   * Check execute mode.  This is true if:
   *    the file is a directory;
   *    the file is a shell command;
   *    the file is a shell script;
   *    the file is a S16 file;
   * But NOT if
   *    the file is a SYS or other type of file.
   */
  if ((mode & X_OK) &&
      !((recptr->fileType == EXE) ||
        (recptr->fileType == DIR) ||
        (recptr->fileType == SRC && recptr->auxType == EXEC) ||
        (recptr->fileType == S16))) {
     errno = EACCES;
     result = -1;
  }

done:
	GIfree(gptr);
  free(recptr);
  return result;
}

/*
 * chdir
 */

int
chdir (const char *pathname) {
	GSStringPtr pathnameGS;
  int result, err;

  if ((pathnameGS = __C2GSMALLOC(pathname)) == NULL) {
	   errno = ENOMEM;
     return -1;
  }
  result = _chdir(pathnameGS);
  err = errno;
  free(pathnameGS);
  if (result != 0) {
	   errno = err;
  }
  return result;
}

/*
 * chmod
 */

int
chmod (const char *pathname, mode_t mode) {
	GSStringPtr pathnameGS;
  int result, err;

  if ((pathnameGS = __C2GSMALLOC(pathname)) == NULL) {
	   errno = ENOMEM;
     return -1;
  }
  result = _chmod(CHMOD_MODE, pathnameGS, mode, 0, 0L);
  err = errno;
  free(pathnameGS);
  if (result != 0) {
	   errno = err;
  }
  return result;
}

/*  
 * close
 */
	 
int
close (int filds) {
  int cl[2] = {1, filds};
  int err;

	_setFdTranslation(filds, 0);
  CloseGS(cl);
  if (_toolErr) {
	   errno = _mapErr(_toolErr);
     return -1;
  }
  return 0;
}

/*
 * creat
 */
 
int
creat(const char *path, mode_t mode) {
	return open (path, O_CREAT | O_TRUNC | O_WRONLY, mode);
}    

/*
 * fchdir
 */

int
fchdir (int fd)
{
	RefInfoRecGS inforec;
  int err, result;

  /* get the pathname based on the file descriptor */
  inforec.pCount = 3;
  inforec.refNum = fd;
  inforec.pathname = (ResultBuf255Ptr) GOinit(GSOS_NAME_MAX, NULL);
  GetRefInfoGS (&inforec);
  if ((err = _mapErr(_toolErr)) != 0) {
	   GOfree(inforec.pathname);
     errno = err;
     return -1;
  }

  /* change directory */
  result = _chdir((GSStringPtr) &inforec.pathname->bufString);
  err = errno;
  GOfree(inforec.pathname);
  errno = err;
  return result;
}

/*
 * fchmod
 */

int
fchmod (int fd, mode_t mode)
{
	RefInfoRecGS inforec;
  int err, result;

  /* get the pathname based on the file descriptor */
  inforec.pCount = 3;
  inforec.refNum = fd;
  inforec.pathname = (ResultBuf255Ptr) GOinit(GSOS_NAME_MAX, NULL);
  GetRefInfoGS (&inforec);
  if ((err = _mapErr(_toolErr)) != 0) {
	   GOfree(inforec.pathname);
     errno = err;
     return -1;
  }

  /* change the mode */
  result = _chmod(CHMOD_MODE, (GSStringPtr) &inforec.pathname->bufString,
                  mode, 0, 0L);
  err = errno;
  GOfree(inforec.pathname);
  errno = err;
  return result;
}

/* 
 * fstatfs
 */

int
fstatfs (int fd, struct statfs *buf)
{
	RefInfoRecGS inforec;
  int err, result;

  /* get the pathname based on the file descriptor */
  inforec.pCount = 3;
  inforec.refNum = fd;
  inforec.pathname = (ResultBuf255Ptr) GOinit(GSOS_NAME_MAX, NULL);
  GetRefInfoGS (&inforec);
  if ((err = _mapErr(_toolErr)) != 0) {
	   GOfree(inforec.pathname);
     errno = err;
     return -1;
  }

  /* _statfs does the rest */
	result = _statfs((GSStringPtr) &inforec.pathname->bufString, buf);
  err = errno;
  GOfree(inforec.pathname);
  errno = err;
  return result;
}

/*
 * fsync
 */
 
int
fsync(int fd) {
	short ff[2];

  ff[0] = 1;
  ff[1] = fd;
  FlushGS(ff);
	if (_toolErr) {
  	errno = _mapErr(_toolErr);
     return -1;
  }
  return 0;
}

/*
 * ftruncate
 */

int
ftruncate(int fd, off_t length)
{
	SetPositionRecGS p;

 	p.pCount = 3;
  p.base = 0;
  p.refNum = fd;
  p.displacement = length;
  SetEOFGS(&p);
  if (_toolErr) {
  	errno = _mapErr(_toolErr);
     return -1;
	}
  return 0;
}

/*
 * getdtablesize
 */

int
getdtablesize (void) {
	return OPEN_MAX;
}

/*
 * getpgrp
 */
 
pid_t
getpgrp (void) {
	return _getpgrp(getpid());
}

/*
 * gettimeofday
 *
 * IIgs HACK!  HACK!  HACK!  We need a real gettimeofday!
 */
 
int
gettimeofday (struct timeval *tp, struct timezone *tzp) {
	tp->tv_sec = time(NULL);
	tp->tv_usec = 0l;
}

/*
 * lseek
 */
 
off_t
lseek(int filds, off_t offset, int whence) {
	SetPositionRecGS s;
	PositionRecGS m;
	EOFRecGS e;
	int err;
	
  e.pCount = m.pCount = 2;
  e.refNum = s.refNum = m.refNum = filds;
  GetEOFGS(&e);
  if (err = _mapErr(_toolErr)) {
  	errno = err;
     return -1L;
  }
  GetMarkGS(&m);

 	s.pCount = 3;
  s.base = 0;
  switch (whence) {
		case SEEK_SET:		s.displacement = offset; 					break;
  	case SEEK_CUR: 	s.displacement = m.position + offset;	break;
     case SEEK_END: 	s.displacement = e.eof + offset; 		break;
		default:
     	errno = EINVAL;
        return -1L;
 	}
  if (s.displacement < 0) {
  	errno = EINVAL;
     return -1L;
  }
  if (s.displacement > e.eof) {
  	SetEOFGS(&s);
     if (err = _mapErr(_toolErr)) {
     	errno = err;
        return -1L;
     }
 	}
  SetMarkGS(&s);
  if (err = _mapErr(_toolErr)) {
  	errno = err;
     return -1L;
 	}
  return s.displacement;
}

/*
 * mkdir
 */

int
mkdir(char *dirname)
{
	CreateRecGS cr;
  int err;

	cr.pCount = 5;
  cr.pathname = (GSString255Ptr) __C2GSMALLOC(dirname);
  if (cr.pathname == NULL) {
	   errno = ENOMEM;
     return -1;
  }
  cr.access = 0xC3;
  cr.fileType = 0x0F;
  cr.auxType = 0L;
  cr.storageType = 0x0D;
  CreateGS(&cr);
  err = _toolErr;
  free(cr.pathname);
  if (err) {
    errno = _mapErr(err);
    return -1;
 	}
  errno = 0;
  return 0;
}

/*
 * raise
 */

int
raise (int sig) {
	return kill (getpid(), sig);
}

/*              
 * read
 */

ssize_t
read (int filds, void *buf, size_t bytecount) {
	IORecGS iorec = {4, filds, buf, (long) bytecount, 0L};
  char *p;
  size_t i;
	int err;
  ssize_t result;

  /* read in the buffer */
 	ReadGS(&iorec);
  if (_toolErr == 0 || _toolErr == 0x4C) {
  	result = (size_t) iorec.transferCount;
  } else if (err = _mapErr(_toolErr)) {
  	errno = err;
     return -1;
  }

  /* translate newlines if necessary */
  if (_getFdTranslation(filds)) {
	   p = (char *) buf;
	   for (i = 0; i < result; i++, p++) {
	      if (*p == '\r') {
	         *p = '\n';
        }
     }
  }
}

/*
 * rexit
 */

void
rexit (int code) {
	SystemQuitFlags (0x4000);
  SystemQuitPath (NULL);
  exit(code);
}

/*
 * statfs
 */

int
statfs(char *path, struct statfs *buf) {
	ExpandPathRecGS ep;
	int err, result;

  /* get the full pathname of this file */
  ep.pCount = 3;
  if ((ep.inputPath = (GSString255Ptr) __C2GSMALLOC(path)) == NULL) {
	   return -1;
  }
  ep.outputPath = (ResultBuf255Ptr) GOinit(GSOS_NAME_MAX, NULL);
  if (ep.outputPath == NULL) {
	   err = errno;
	   free(ep.inputPath);
     errno = err;
     return -1;
  }
  ep.flags = 0;
  ExpandPathGS(&ep);
  if (_toolErr) {
     result = -1;
	   err = _mapErr(_toolErr);
  } else {
		result = _statfs((GSStringPtr) &ep.outputPath->bufString, buf);
     err = errno;
  }
  free(ep.inputPath);
  GOfree(ep.outputPath);
  errno = err;
  return result;
}

/*
 * truncate
 */

int
truncate(const char *path, off_t length)
{
	SetPositionRecGS p;
  int closerec[2];
	struct {
	   Word pCount;
     Word refNum;
     GSStringPtr pathname;
     Word requestAccess;
  } openrec;		/* abbreviated version of OpenRecGS */
  int err, result;

  /* open the file */
  openrec.pCount = 3;
  if ((openrec.pathname = __C2GSMALLOC(path)) == NULL) {
	   return -1;
  }   
  openrec.requestAccess = readWriteEnable;
  OpenGS(&openrec);
  err = _mapErr(_toolErr);
  free(openrec.pathname);
  if (err) {
     errno = err;
     return -1;
  }

  /* set up the close block */
  closerec[0] = 1;
  closerec[1] = openrec.refNum;

 	p.pCount = 3;
  p.base = 0;
  p.refNum = openrec.refNum;
  p.displacement = length;
  SetEOFGS(&p);
  if (_toolErr) {
  	errno = _mapErr(_toolErr);
     result = -1;
  } else {
	   result = 0;
  }
  CloseGS(closerec);
  return result;
}

/*
 * umask
 */
 
mode_t
umask (mode_t mask) {
	static mode_t currentMask = 0xFFFF;
  static int maskInitialized = 0;
  char *p;
  mode_t result;
  char maskStr[5];
  const char *umaskStr = "UMASK";

  /* initialize off of environment first time through -- hack */
  if (! maskInitialized) {
	   if ((p = getenv(umaskStr)) == NULL) {
	      currentMask = 022;
     } else {
	      currentMask = strtoul(p, NULL, 8);
     }
     maskInitialized = 1;
  }

	result = currentMask;
  currentMask = mask & 0777;
  maskStr[0] = '0';
  maskStr[1] = '0' + ((currentMask & 0700) >> 6);
  maskStr[2] = '0' + ((currentMask & 0070) >> 3);
  maskStr[3] = '0' + (currentMask & 0007);
  maskStr[4] = '\0';
  setenv(umaskStr, maskStr, 1);	/* ignore errors */
  return result;
}

/*
 * unlink
 */
 
int unlink(char *fname)
{
	/*
   * Orca/C doesn't specify what the "non-zero" return code is, so
   * force it to be -1.
   */
 	return (remove(fname) == 0) ? 0 : -1;
}

/*
 * When GNO supports wait4(2) (and assuming that it doesn't have waitpid()),
 * this routine can be changed to the following:
 *
 *  return (wait4(pid, istat, options, (struct rusage *)0));
 *
 * This implementation is flawed since it's not done in the kernel.  See
 * the BUGS section of the man page for details.
 */

pid_t
waitpid(pid_t pid, union wait *istat, int options)
{
   int result;
   pid_t pgid;

#if 1
	/*
   * there's a note in <unistd.h> about the implementation of
   * getpgrp() being buggy.
   */
   if (pid < -1 || pid == 0) {
	   fprintf(stderr,"waitpid: process groups not implemented.  Aborted.\n");
     abort();
   }
#endif

   /* We really need to do this in the kernel. */
   if (pid < -1) {
	   pgid = -pid;
   } else if (pid == 0) {
     pgid = _getpgrp(getpid());
   } else {
     pgid = -1;
   }

   for(;;) {
      result = wait(istat);
      if ((result == -1) ||                               
          (pid == result) ||
          ((pgid > 1) && (pgid == _getpgrp(result)))) {
    		return result;
      }
   }
}
 
/*
 * write
 */
 
ssize_t
write(int filds, void *buf, size_t bytecount) {
	IORecGS iorec = {4, filds, buf, (long) bytecount, 0L};
	int err;
  size_t i;
  char *p;

  /* translate newlines if necessary */
  if (_getFdTranslation(filds)) {
	   p = (char *) buf;
	   for (i = 0; i < bytecount; i++, p++) {
	      if (*p == '\n') {
	         *p = '\r';
        }
     }
  }

  /* write the file block */
  WriteGS(&iorec);
  if (err = _mapErr(_toolErr)) {
  	errno = err;
     return -1;
  }
  return (size_t) iorec.transferCount;
}

/* pragma optimize 79 */
#pragma optimize 8
#pragma debug 0

#ifdef VERSION_CHECK

/*
 * _libcPanic
 *
 * Get a message out to the user and exit.  This is at the end of the
 * file because of the higher optimization level required for variadic
 * functions.
 */

static void
_libcPanic (const char *fmt, ...) {
	va_list list;

  va_start(list, fmt);
	vfprintf(stderr, fmt, list);
  va_end(list);
  exit(EXIT_FAILURE);
}

#endif	/* VERSION_CHECK */

/*
 * open -- end of file because of higher optimization required
 */

int
open (const char *path, int oflag, ...) {
	OpenRecGS        openRec;
	CreateRecGS      createRec;
	SetPositionRecGS setMarkRec;
	va_list list;
	mode_t openmode;
  int err;                        /* saved errno */
  int result;                     /* returned value */
  size_t currentEof;              /* saved eof nec for append */

  /* grab extra parameter if necessary */
  va_start(list, oflag);
  if (oflag & O_CREAT) {
  	openmode = va_arg(list, mode_t);
  }
  err = 0;

  /* try to open the file */
 	openRec.pCount = 12;
  openRec.pathname = (GSString255Ptr) __C2GSMALLOC(path);
  if (openRec.pathname == NULL) {
	   va_end(list);
	   errno = ENOMEM;
	   return -1;		/* DON'T goto label 'done' ... spurious free() */
  }
  if ((oflag & O_ACCMODE) == O_RDONLY) {
	   openRec.requestAccess = readEnable;
  } else if ((oflag & O_ACCMODE) == O_WRONLY) {
	   openRec.requestAccess = writeEnable;
  } else if ((oflag & O_ACCMODE) == O_RDWR) {
	   openRec.requestAccess = readWriteEnable;
  } else {
	   openRec.requestAccess = 0;
  }
  openRec.resourceNumber = 0;	/* data fork */
  openRec.optionList = NULL;		/* no FST-specific info */

  OpenGS(&openRec);
  if ((_toolErr == 0) && (oflag & O_CREAT) && (oflag & O_EXCL)) {
     /* file already existed */
		close(openRec.refNum);
     err = EEXIST;
     result = -1;
     goto done;
  } else if ((_toolErr == 0) && (oflag & O_WRONLY) &&
             (openRec.storageType == 0x0d || openRec.storageType == 0x0f)) {
	   /* opening a volume directory or subdirectory for writing not permitted */
		close(openRec.refNum);
     err = EISDIR;
     result = -1;
     goto done;
  } else if ((err = _mapErr(_toolErr)) && (err == ENOENT)) {
     /* file doesn't exist -- create? */
		if (oflag & O_CREAT) {
	   	createRec.pCount = 3;
        createRec.pathname = openRec.pathname;
        createRec.access = _mapMode2GS(openmode);
        createRec.fileType = (oflag & O_BINARY) ? BIN : TXT;
        CreateGS(&createRec);
        if (err = _mapErr(_toolErr)) {
	   		result = -1;
        	goto done;
        }
        OpenGS(&openRec);
        if (err = _mapErr(_toolErr)) {
	   		result = -1;
        	goto done;
   		}
 		} else {
        /* no create and didn't exist -- error */
	   	result = -1;
	      goto done;
     }
 	} else if (err) {
	   /* unknown error on open */
	   result = -1;
	   goto done;
  }

  /* if we got here, the file is open */
  currentEof = openRec.eof;

  /* truncate the file if necessary */
  if ((oflag & O_TRUNC) && ((oflag & O_ACCMODE) != O_RDONLY)) {
	   ftruncate(openRec.refNum, 0L);
     currentEof = 0L;
  }

	/* append to file? */
  if ((oflag & O_APPEND) && ((oflag & O_ACCMODE) != O_RDONLY)) {
		setMarkRec.pCount = 3;
     setMarkRec.refNum = openRec.refNum;
     setMarkRec.base = 0;
     setMarkRec.displacement = currentEof;
     SetMarkGS(&setMarkRec);
     if (err = _mapErr(_toolErr)) {
	   	result = -1;
	      goto done;
    	}
  }

  /* success! */
  err = 0;
  result = openRec.refNum;
  if (oflag & O_TRANS) {
	   _setFdTranslation(result, 1);
  }

done:
	free(openRec.pathname);
  va_end(list);
	errno = err;
  return result;
}
