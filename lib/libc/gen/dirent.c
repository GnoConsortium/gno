/*
 * The original version of these routines (for GNO v2.0.5 and earlier)
 * were by Derek Taubert and Jawaid Bazyar.  Reimplemented from scratch
 * by Devin Reade.
 *
 * $Id: dirent.c,v 1.2 1997/09/21 06:04:59 gdr Exp $
 *
 * This file is formatted with tab stops every 8 characters.
 */
 
#ifdef __ORCAC__
segment "libc_gen__";
#endif

#define __LIBC_DIRENT	/* needed for decls in <dirent.h> */

#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <gsos.h>
#include <dirent.h>
#include <errno.h>
#include <gno/gno.h>

DIR *
opendir (char *filename) {
	DIR *dirp;
	OpenRecGS *openRecPtr;
	int err;

	/*
	 * the original version of this code used an ExpandDevices call
	 * to expand filename before the call to OpenGS
	 */
	 
	/* do the allocations */
        if ((dirp = malloc(sizeof(DIR))) == NULL) {
                return NULL;
        }
        if ((dirp->dd_ent = malloc(sizeof(DirEntryRecGS))) == NULL) {
        	free(dirp);
        	return NULL;
        }
        if ((dirp->dd_ent->name = (ResultBuf255Ptr) GOinit (255, NULL))
	     == NULL) {
	     	free(dirp->dd_ent);
        	free(dirp);
        	return NULL;
	}        
        if ((dirp->dd_data = malloc(sizeof(struct dirent))) == NULL) {
        	free(dirp->dd_ent->name);
        	free(dirp->dd_ent);
        	free(dirp);
        	return NULL;
        }
        if ((openRecPtr = malloc(sizeof(OpenRecGS))) == NULL) {
        	free(dirp->dd_data);
        	free(dirp->dd_ent->name);
        	free(dirp->dd_ent);
        	free(dirp);
        	return NULL;
        }
        if ((openRecPtr->pathname = (GSString255Ptr) __C2GSMALLOC(filename))
	     == NULL) {
        	free(openRecPtr);
        	free(dirp->dd_data);
        	free(dirp->dd_ent->name);
        	free(dirp->dd_ent);
        	free(dirp);
        	return NULL;
        }

        /* open what is believed to be a directory */
        openRecPtr->pCount = 8;
	openRecPtr->requestAccess = readEnable;
	openRecPtr->resourceNumber = 0;
	OpenGS(openRecPtr);
	if ((err = _mapErr(_toolErr)) == 0) {
		if (openRecPtr->storageType != 0x0d &&	/* subdirectory */
		    openRecPtr->storageType != 0x0f) {	/* volume directory */
		    err = ENOTDIR;
		    openRecPtr->pCount = 1;
		    CloseGS(openRecPtr);		/* cheat a bit */
		}
	}
	if (err) {
		free(openRecPtr->pathname);
        	free(openRecPtr);
        	free(dirp->dd_data);
        	free(dirp->dd_ent->name);
        	free(dirp->dd_ent);
        	free(dirp);
        	errno = err;
        	return NULL;
        }

        /* if we're here, filename has been opened and it's a directory */
        dirp->dd_fd = openRecPtr->refNum;
        dirp->dd_ent->pCount = 7;
        dirp->dd_ent->refNum = dirp->dd_fd;
	free(openRecPtr->pathname);
       	free(openRecPtr);
       	return dirp;
}

struct dirent *
readdir (DIR *dirp) {
	struct dirent *result;
	int err;

	/* get the info from GS/OS */
	dirp->dd_ent->base = 1;
	dirp->dd_ent->displacement = 1;
        GetDirEntryGS(dirp->dd_ent);
	if ((err = _mapErr(_toolErr)) != 0) {
		return NULL;
	}

	/* copy it into the user-usable buffer */
	result = dirp->dd_data;
	result->d_fileno	= dirp->dd_ent->entryNum;
	result->d_reclen	= sizeof(struct dirent);
	switch (dirp->dd_ent->fileType) {
	case 0x0f:
		result->d_type	= DT_DIR;
	default:
		result->d_type	= DT_REG;
	}
	result->d_namlen = (char) dirp->dd_ent->name->bufString.length;
	memcpy(result->d_name, dirp->dd_ent->name->bufString.text,
	       (size_t) result->d_namlen);
	result->d_name[result->d_namlen] = '\0';

	return result;
}

void
rewinddir (DIR *dirp) {
	dirp->dd_ent->base = 0;
	dirp->dd_ent->displacement = 0;
        GetDirEntryGS(dirp->dd_ent);
}

long
telldir (DIR *dirp) {
	return (long) dirp->dd_ent->entryNum;
}

void
seekdir (DIR *dirp, long loc) {
	dirp->dd_ent->base = 0;
	dirp->dd_ent->displacement = loc;
        GetDirEntryGS(dirp->dd_ent);
}

int
closedir (DIR *dirp) {
	int closerec[2];
	int err;

	closerec[0] = 1;
	closerec[1] = dirp->dd_ent->refNum;
	CloseGS(closerec);
	err = _mapErr(_toolErr);
	free(dirp->dd_data);
	free(dirp->dd_ent->name);
	free(dirp->dd_ent);
	free(dirp);
	if (err) {
		errno = err;
		return -1;
	} else {
		return 0;
	}
}
