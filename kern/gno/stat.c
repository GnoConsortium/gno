/*	$Id: stat.c,v 1.1 1998/02/02 08:19:02 taubert Exp $ */

/*

	stat.c

    Copyright 1992-1998, Procyon Inc.

    Many generations ago, Derek Taubert wrote the original version of
    this code.  There is still a family resemblance, but this great-great-
    great-grandchild also bears a striking resemblance to Jawaid Bazyar.
    I.e., this is a bastard piece of code.

    The many changes were made to facilitate its merge into the GNO Kernel,
    mostly dealing with reporting of pipes.
*/

segment "KERN2     ";
#pragma optimize 79

#include "proc.h"
#include "sys.h"
#include "kernel.h"
#include "/lang/orca/libraries/orcacdefs/stdio.h"
#include "/lang/orca/libraries/orcacdefs/string.h"
#include "/lang/orca/libraries/orcacdefs/stdlib.h"
#include <sys/stat.h>
#include <sys/errno.h>
#include <misctool.h>
#include <gsos.h>
#include <shell.h>

extern kernelStructPtr kp;
int inoPool = 1;

int
_mapErr (int err)
{
	int ret;

	if (!err) {
		return 0;
	}
	if ((err & 0xff00) == 0x4300) {
		/* GNO already mapped the error */
		return (err & 0x00ff);
	}
	switch (err) {
	    case 0x43:	ret = EBADF;	break;

	    case 0x44:
    	    case 0x45:
    	    case 0x46:	ret = ENOENT;	break;

    	    case 0x47:
	    case 0x50:	ret = EEXIST;	break;

    	    case 0x48:
    	    case 0x49:	ret = ENOSPC;	break;
            case 0x4A:	ret = ENOTDIR;	break;

    	    case 0x4B:
    	    case 0x4F:
    	    case 0x53:	ret = EINVAL;	break;
    	    case 0x54:	ret = ENOMEM;	break;
    	    case 0x4E:	ret = EACCES;	break;
    	    case 0x58:	ret = ENOTBLK;	break;
    	    default:	ret = EIO;	break;
	}
	return ret;
}

#pragma databank 1
int statCommon(const char *filename, struct stat *s_buf)
{
FileInfoRecGS fi;
DevNumRecGS getdev;
int e,entcount;
int dtype = 0;
GSString255Ptr fullpath,path;
extern void COPYC2GS(void*,void*);
extern GSString255Ptr gno_ExpandPath(GSString255Ptr, int, word);
unsigned long lsec;

    disableps();
    path = malloc(strlen(filename)+2);
    COPYC2GS(path,filename);
    fullpath = gno_ExpandPath(path,0,0);

    fi.pCount = 10;
    fi.optionList = NULL;
    fi.pathname = fullpath;
    GetFileInfoGS(&fi);
    if (_toolErr) {
        if (_toolErr == 0x58) dtype = 1; 
        else {
            nfree(path);
            enableps();
            return _mapErr(_toolErr);
        }
    }

    if (fullpath->text[0] == '.') {
        getdev.devName = (GSString32Ptr) fullpath;
        if (findDevice(fullpath)) {
            memset(s_buf,0,sizeof(struct stat));
            s_buf->st_mode = S_IFCHR;
            nfree(path); return 0;
        }
    }
    else {
        fullpath->text[fullpath->length] = 0;
        e = strpos(fullpath->text+1,':');
        if (e != -1) {
            if (e < fullpath->length) e++;
            fullpath->length = e;
        }
        getdev.devName = (GSString32Ptr) fullpath;
    }
    getdev.pCount = 2;
    GetDevNumberGS(&getdev);

    if (_toolErr) {
        enableps();
        return _mapErr(_toolErr);
    }
    s_buf->st_dev = getdev.devNum;

    /* this fakes an inode number.  For applications like diff,
       this will work since there are no file links under either
       ProDOS or HFS */

    s_buf->st_ino = inoPool++;

    if (!dtype) {
        s_buf->st_mode =
          ((fi.fileType == 0x0f) ? (S_IFDIR|S_IEXEC) : S_IFREG) |
          ((fi.access & 0x01) ? S_IREAD : 0) |
          ((fi.access & 0x02) ? S_IWRITE : 0) |
          (((fi.fileType == 0xff) || (fi.fileType == 0xb5) ||
            (fi.fileType == 0xb3)) ? S_IEXEC : 0);
        s_buf->st_nlink = 0;
        s_buf->st_uid = 0;
        s_buf->st_gid = 0;
        s_buf->st_rdev = 0;         /* device type */
        s_buf->st_size = fi.eof;

        lsec = ConvSeconds(1,0l,(Pointer)&fi.modDateTime);
 /*	2083363200. seconds difference */
        /*s_buf->st_atime = s_buf->st_mtime = lsec - 2756021998ul;*/

	/* a->i1 = a->i2 = b BROKEN in C 2.0.3 */
#if 0
        s_buf->st_atime = s_buf->st_mtime = lsec - 2078611200ul;
#else
	s_buf->st_mtime = lsec - 2078611200ul;
	s_buf->st_atime = s_buf->st_mtime;
#endif
        lsec = ConvSeconds(1,0l,(Pointer)&fi.createDateTime);            
        s_buf->st_ctime = lsec - 2078611200ul;

        s_buf->st_blksize = STAT_BSIZE;
        s_buf->st_blocks = fi.blocksUsed;
    } else {
        memset(s_buf,0,sizeof(struct stat));
        s_buf->st_dev = getdev.devNum;
        s_buf->st_mode = S_IFCHR;
    }
    enableps();
    return(0);
}

#pragma databank 1
#pragma toolparms 1

int KERNfstat(int *ERRNO, struct stat *s_buf, int fd)
{
RefInfoRecGS refInfo;
ResultBuf255Ptr gstring;
fdentryPtr fp;
int rc;
extern fdentryPtr getFDptr(int);

    disableps();
    if (kp->gsosDebug & 16)
    	fprintf(stderr,"fstat: fd: %d s_buf: %06lX\n",fd,s_buf);
    if ((fd == 0) || ((fp = getFDptr(fd)) == NULL) || (fp->refNum == 0)) {
    	*ERRNO = EBADF;
        enableps();
        return -1;
    }
    if (fp->refType == rtPIPE) {
        memset(s_buf,0,sizeof(struct stat));
        s_buf->st_mode = S_IFSOCK;
        enableps(); return 0;
    } else if (fp->refType == rtTTY) {
        memset(s_buf,0,sizeof(struct stat));
        s_buf->st_mode = S_IFCHR;
        enableps(); return 0;
    }
    gstring = malloc(sizeof(ResultBuf255));

    refInfo.pCount = 3;
    refInfo.refNum = fd;
    gstring->bufSize = 254;
    refInfo.pathname = gstring;
tryGet:
    GetRefInfoGS(&refInfo);
    if (_toolErr == 0x4F) {
        gstring = realloc(gstring,gstring->bufString.length+5);
        gstring->bufSize = gstring->bufString.length;
        goto tryGet;
    }
    if (_toolErr) {
        *ERRNO = _mapErr(_toolErr);
        enableps();
        return(-1);
    }
    gstring->bufString.text[gstring->bufString.length]=0;
    rc = statCommon(gstring->bufString.text,s_buf);
    if (rc) *ERRNO = rc;
    nfree(gstring);
    enableps();
    return (rc) ? -1 : 0;
}

int KERNlstat(int *ERRNO,struct stat *s_buf,const char *filename)
{
int rc;

    if (kp->gsosDebug & 16)
    	fprintf(stderr,"stat: lpath: %s s_buf: %06lX\n",filename,s_buf);
    rc = statCommon(filename,s_buf);
    if (rc) *ERRNO = rc;
    return (rc) ? -1 : 0;
}

int KERNstat(int *ERRNO,struct stat *s_buf,const char *filename)
{
int rc;

    if (kp->gsosDebug & 16)
    	fprintf(stderr,"stat: path: %s s_buf: %06lX\n",filename,s_buf);
    rc = statCommon(filename,s_buf);
    if (rc) *ERRNO = rc;
    return (rc) ? -1 : 0;
}
#pragma databank 0
#pragma toolparms 0

