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
 */

/*
 * part of cmp for GNO/ME v2.0.6
 *
 * $Id: misc.c,v 1.1 1997/10/03 05:13:23 gdr Exp $
 */

#ifndef __GNO__
#ifndef lint
static char sccsid[] = "@(#)misc.c	8.3 (Berkeley) 4/2/94";
#endif /* not lint */
#endif

#include <sys/types.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>

#include "extern.h"

void
#ifndef __STDC__
eofmsg(file)
	char *file;
#else
eofmsg(char *file)
#endif
{
	if (!sflag)
		warnx("EOF on %s", file);
	exit(DIFF_EXIT);
}

void
#ifndef __STDC__
diffmsg(file1, file2, byte, line)
	char *file1, *file2;
	off_t byte, line;
#else
diffmsg(char *file1, char *file2, off_t byte, off_t line)
#endif
{
	if (!sflag)
#ifndef __ORCAC__
		(void)printf("%s %s differ: char %qd, line %qd\n",
		    file1, file2, byte, line);
#else
		(void)printf("[%s] %s differ: %s char %ld, line %ld\n",
		    (rflag==2) ? "resfork" : "datafork", file1, file2, byte, line);
#endif
	exit(DIFF_EXIT);
}
