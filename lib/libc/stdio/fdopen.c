/*
 * fdopen(3) implementation.
 *
 * Devin Reade, April 1997
 *
 * This file is formatted with tab stops every 8 columns
 *
 * $Id: fdopen.c,v 1.1 1997/07/27 23:15:09 gdr Exp $
 */

#ifdef __ORCAC__
segment "libc_stdio";
#endif

#pragma optimize 0
#pragma debug 0
#pragma memorymodel 0

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

FILE *
fdopen(int fildes, const char *cmode)
{
	FILE *result;
	struct stat *statbuf;
	unsigned int mode, whence, isBinary;

	/* allocate buffers */
	if ((statbuf = malloc(sizeof(struct stat))) == NULL) {
		goto fail1;
	}
	if ((result = malloc(sizeof(FILE))) == NULL) {
		goto fail2;
	}
	if ((result->_base = malloc(BUFSIZ)) == NULL) {
		goto fail3;
	}

	/* stat the file descriptor */
	if ((fstat(fildes, statbuf)) == -1) {
		goto fail4;
	}

	/* extract the mode */
	result->_flag = _IOFBF | _IOMYBUF;
	mode = 0;
	whence = SEEK_SET;
	isBinary = 0;
	switch(*cmode) {
	case 'r':
		mode = _IOREAD;
		break;
	case 'a':
		whence = SEEK_END;
		/*FALLTHROUGH*/
	case 'w':
		mode = _IOWRT;
		break;
	default:
		errno = EINVAL;
		goto fail4;
	}
	cmode++;     
	switch (*cmode) {
	case 0:
		break;
	case 'b':
		isBinary = 1;
		cmode++;
		switch(*cmode) {
		case 0:
			break;
		case '+':
			mode = _IORW;
			break;
                default:
			errno = EINVAL;
			goto fail4;
		}
	case '+':
		mode = _IORW;
		/*
		 * We don't document this behavior, but we will allow
		 * the 'b' to come in the third position as well, since
		 * it appears that this is the case for the ORCA fopen
		 * implementation.
		 */
		cmode++;
		if (*cmode == 'b') {
			isBinary = 1;
		}
		break;
	default:
		errno = EINVAL;
		goto fail4;
	}
	if (!isBinary) {
		result->_flag |= _IOTEXT;
	}
	result->_flag |= mode;

	/*
	 * There is a bug in the beta v2.0.6 kernel that, when stat'ing
	 * a character special device, only the S_IFCHR bit is set.  A
	 * similar situation is occuring for pipes/sockets.
	 */
	if (S_ISSOCK(statbuf->st_mode) || S_ISCHR(statbuf->st_mode)) {
		goto pass;
	}

	/* verify that the req mode agrees with the file descriptor mode */
	if ((result->_flag & (_IORW | _IOWRT)) &&
	    !(statbuf->st_mode & S_IWUSR)) {
		errno = EPERM;
		goto fail4;
	}
	if ((result->_flag & (_IORW | _IOREAD)) &&
	    !(statbuf->st_mode & S_IRUSR)) {
		errno = EPERM;
		goto fail4;
	}
	if (lseek(fildes, 0L, whence) == -1) {
		if (errno != ENOTBLK) {
			goto fail4;
		}
	}

	/* set up remaining FILE struct members */
    pass:
	result->next = stderr->next;
	stderr->next = result;
	result->_ptr = result->_base;
	result->_end = NULL;
	result->_size = BUFSIZ;
	result->_cnt = 0L;
	result->_pbk[0] = -1;
	result->_pbk[1] = -1;
	result->_file = fildes;
	return result;

    fail4:
    	free(result->_base);
    fail3:
    	free(result);
    fail2:
    	free(statbuf);
    fail1:
    	return NULL;
}
