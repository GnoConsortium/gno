/*
 * libc/gno/map.c  -- various mapping functions
 *
 * $Id: map.c,v 1.2 1997/09/05 06:14:16 gdr Exp $
 *
 * This file is formatted with tabstops every 8 columns
 */

#ifdef __ORCAC__
segment "libc_gno__";
#endif
 
#pragma optimize 0
#pragma memorymodel 0
#pragma debug 0

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <gno/gno.h>

/* needed only during debugging */
#include <stdio.h>

#define GS_READ      0x0001
#define GS_WRITE     0x0002
#define GS_INVISIBLE 0x0004
#define GS_BACKUP    0x0020
#define GS_RENAME    0x0040
#define GS_DESTROY   0x0080
#define GS_MASK      0xFF18 /* complement of bitwise-OR of above GS- values */

static int __fileModeEmulation = 1;	/* do we emulate Unix mode parameters? */

/*
 * _getModeEmulation
 */

int
_getModeEmulation (void) {
	return __fileModeEmulation;
}

/*
 * _setModeEmulation
 */
 
int
_setModeEmulation (int newval) {
	int result;

	result = __fileModeEmulation;
	__fileModeEmulation = !(newval == 0);
	return result;
}

/*
 * _mapMode2GS -- Take a Unix mode and return the GS/OS equivalent.
 *
 * Devin Reade <gdr@myrias.com>
 */              

mode_t
_mapMode2GS(mode_t oldmode) {
	mode_t newmode = GS_BACKUP;

	if (__fileModeEmulation == 0) {
		return oldmode;
	}
	if (oldmode & S_IRUSR) {
		newmode |= GS_READ;
	}
	if (oldmode & S_IWUSR) {
		newmode |= GS_WRITE | GS_RENAME | GS_DESTROY;
	}
	return newmode;
}

/*
 * _mapMode2Unix -- Take a GS/OS mode and return the Unix equivalent.
 *
 * Devin Reade <gdr@myrias.com>
 */

mode_t
_mapMode2Unix(mode_t oldmode) {
	mode_t newmode = S_IXUSR | S_IXGRP | S_IXOTH;
	int mask;

	if (__fileModeEmulation == 0) {
		return oldmode;
	}
	if (oldmode & GS_READ) {
		newmode |= S_IRUSR | S_IRGRP | S_IROTH;
	}     
	if ((oldmode & GS_WRITE) && (oldmode & GS_RENAME) &&
  	    (oldmode & GS_DESTROY)) {
		newmode |= S_IWUSR | S_IWGRP | S_IWOTH;
	}                                               

	/* get the umask */
	umask((mask = umask(0)));
	return newmode & mask;
}

/*
 * _setPathMapping -- control whether or not we map pathnames
 *
 * Devin Reade <gdr@myrias.com>
 */

static int __force_slash = 0;	/* default: no pathname mapping */

int
_setPathMapping (int val) {
	int previous = __force_slash;

	__force_slash = (val != 0);
	return previous;
}

int
_getPathMapping (void) {
	return __force_slash;
}

/*
 * _mapPath -- map all occurances of ':' to '/'
 *
 * Devin Reade <gdr@myrias.com>
 */

char *
_mapPath (char *pathname) {
	char *p;
	int foundSlash, foundColon;

	if (! __force_slash) {
		return pathname;
	}

	/* validity check */
	foundSlash = foundColon = 0;
	for (p = pathname; *p != '\0'; p++) {
		if (*p == ':') foundColon = 1;
		if (*p == '/') foundSlash = 1;
	}
	if (foundSlash && foundColon) {
		return NULL;
	}
	for (p = pathname; *p != '\0'; p++) {
		if (*p == ':') *p = '/';
	}
	return pathname;
}

/*
 * _mapPathGS -- map all occurances of ':' to '/'
 *
 * Devin Reade <gdr@myrias.com>
 */

GSStringPtr
_mapPathGS (GSStringPtr pathname) {
	char *p, *q;
	int foundSlash, foundColon;

	if (! __force_slash || pathname->length == 0) {
		return pathname;
	}

	/* validity check */
	foundSlash = foundColon = 0;
	p = pathname->text;
	q = p + pathname->length;
	while (p < q) {
		if (*p == ':') foundColon = 1;
		if (*p == '/') foundSlash = 1;
		p++;
	}
	if (foundSlash && foundColon) {
		return NULL;
	}
	p = pathname->text;
	while (p < q) {
		if (*p == ':') *p = '/';
		p++;
	}
	return pathname;
}

/*
 * _mapErr --	Take a GS/OS error and maps it to a kernel errno.  If there
 *		is no direct mapping, then map it to EIO.
 */
 
int
_mapErr (int err) {
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
