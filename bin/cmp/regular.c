/*-
 * Copyright (c) 1991, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
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
 *
 * $Id: regular.c,v 1.1 1997/10/03 05:13:23 gdr Exp $
 */

#ifndef lint
static char sccsid[] = "@(#)regular.c	8.3 (Berkeley) 4/2/94";
#endif /* not lint */

#include <sys/param.h>
#ifndef __GNO__
#include <sys/mman.h>
#endif
#include <sys/stat.h>

#include <err.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "extern.h"

#define ROUNDPAGE(i) ((i) & ~pagemask)

void
#ifndef __STDC__
c_regular(fd1, file1, skip1, len1, fd2, file2, skip2, len2)
	int fd1, fd2;
	char *file1, *file2;
	off_t skip1, len1, skip2, len2;
#else
c_regular(int fd1, char *file1, off_t skip1, off_t len1, int fd2, char *file2,
	off_t skip2, off_t len2)
#endif
{
#ifndef __GNO__
	u_char ch, *p1, *p2;
#else
	u_char ch, p1, p2;
#endif
	off_t byte, length, line;
	int dfound;
	off_t pagemask, off1, off2;

	if (sflag && len1 != len2)
		exit(1);

	if (skip1 > len1)
		eofmsg(file1);
	len1 -= skip1;
	if (skip2 > len2)
		eofmsg(file2);
	len2 -= skip2;

	pagemask = (off_t)getpagesize() - 1;
	off1 = ROUNDPAGE(skip1);
	off2 = ROUNDPAGE(skip2);

	length = MIN(len1, len2);
#ifndef __ORCAC__
	if (length > SIZE_T_MAX)
		return (c_special(fd1, file1, skip1, fd2, file2, skip2));
#else
	if (length > SIZE_T_MAX) {
		c_special(fd1, file1, skip1, fd2, file2, skip2);
		return;
	}
#endif

#ifndef __GNO__
	if ((p1 = (u_char *)mmap(NULL,
	    (size_t)length, PROT_READ, MAP_SHARED, fd1, off1)) == (u_char *)MAP_FAILED)
		err(ERR_EXIT, "%s", file1);

	madvise(p1, length, MADV_SEQUENTIAL);
	if ((p2 = (u_char *)mmap(NULL,
	    (size_t)length, PROT_READ, MAP_SHARED, fd2, off2)) == (u_char *)MAP_FAILED)
		err(ERR_EXIT, "%s", file2);
	madvise(p2, length, MADV_SEQUENTIAL);

	dfound = 0;
	p1 += skip1 - off1;
	p2 += skip2 - off2;
	for (byte = line = 1; length--; ++p1, ++p2, ++byte) {
		if ((ch = *p1) != *p2)
			if (lflag) {
				dfound = 1;
				(void)printf("%6qd %3o %3o\n", byte, ch, *p2);
			} else
				diffmsg(file1, file2, byte, line);
				/* NOTREACHED */
		if (ch == '\n')
			++line;
	}

	if (len1 != len2)
		eofmsg (len1 > len2 ? file2 : file1);
	if (dfound)
		exit(DIFF_EXIT);
#else
	dfound = 0;
	lseek(fd1, skip1, SEEK_SET);
	lseek(fd2, skip2, SEEK_SET);
	for (byte = line = 1; length--; ++byte) {
		read(fd1, (void *) &p1, 1);
		read(fd2, (void *) &p2, 1);
		if (p1 != p2)
			if (lflag) {
				dfound = 1;
				(void)printf("%6qd %3o %3o\n", byte, ch, p2);
			} else
				diffmsg(file1, file2, byte, line);
				/* NOTREACHED */
		if (p1 == '\n')
			++line;
	}

	if (len1 != len2)
		eofmsg (len1 > len2 ? file2 : file1);
	if (dfound)
		exit(DIFF_EXIT);
#endif

}
