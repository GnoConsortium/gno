/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifdef __ORCAC__
segment "libc_stdio";
#endif

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)tmpfile.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <paths.h>
#include <stdlib.h>
#include <string.h>
#include "local.h"

FILE *
tmpfile(void)
{
	sigset_t set, oset;
	FILE *fp;
	int fd, sverrno;
#define	TRAILER	"tmp.XXXXXX"
#ifndef __GNO__
	char buf[sizeof(_PATH_TMP) + sizeof(TRAILER)];
#else
	static char *buf = NULL;

	if (buf == NULL) {
		buf = malloc(sizeof(_PATH_TMP) + sizeof(TRAILER) + 2);
		if (buf == NULL) {
			return NULL;
		}
	}
#endif

	(void)memcpy(buf, _PATH_TMP, sizeof(_PATH_TMP) - 1);
	(void)memcpy(buf + sizeof(_PATH_TMP) - 1, TRAILER, sizeof(TRAILER));

	sigfillset(&set);
	(void)sigprocmask(SIG_BLOCK, &set, &oset);

	fd = mkstemp(buf);
	if (fd != -1)
#ifdef __GNO__
		__sUnlinkAtExit(buf);
#else
		(void)unlink(buf);
#endif

	(void)sigprocmask(SIG_SETMASK, &oset, NULL);

	if (fd == -1)
		return (NULL);

	if ((fp = fdopen(fd, "w+")) == NULL) {
		sverrno = errno;
		(void)close(fd);
		errno = sverrno;
		return (NULL);
	}
	return (fp);
}

#ifdef __GNO__

typedef struct __sUnlinkList_t {
	struct __sUnlinkList_t *next;
	char *name;
} __sUnlinkList_t;

static __sUnlinkList_t *unlinkList = NULL;

/*
 * __sUnlinkAtExit()	- maintains a linked list of file names that
 *			  must be deleted when the program exits.
 *
 * __sExitDoUnlinks()	- called at exit to delete the files
 */

void
__sUnlinkAtExit (const char *fname)
{
	__sUnlinkList_t *elem;

	/* make sure __sExitDoUnlinks() will be called */
	__cleanup = _cleanup;

	elem = malloc(sizeof(__sUnlinkList_t) + strlen(fname) + 1);
	if (elem == NULL) {
		return;		/* it won't get deleted; tough luck */
	}
	elem->next = unlinkList;
	elem->name = (char *)((unsigned long) elem + sizeof(__sUnlinkList_t));
	strcpy(elem->name, fname);
	unlinkList = elem;
	return;
}

void
__sExitDoUnlinks(void)
{
	__sUnlinkList_t *elem;

	elem = unlinkList;
	while (elem != NULL) {
		unlink(elem->name);
		elem = elem->next;
	}
	return;
}

#endif
