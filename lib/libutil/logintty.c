/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
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
 * This file is formatted for tab stops every 8 characters.
 *
 * $Id: logintty.c,v 1.1 1997/02/28 05:03:52 gdr Exp $
 */
 
#ifdef __ORCAC__
segment "libutil___";
#endif

#pragma optimize 0		/* was 73 */
#pragma debug 0
#pragma memorymodel 0

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)login_tty.c	1.2 (Berkeley) 6/21/90";
#endif /* LIBC_SCCS and not lint */

#include <sys/param.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <libutil.h>

#ifdef __ORCAC__
#include <types.h>
#include <texttool.h>
#include <stddef.h>
#include <gno/gno.h>
#endif

int
login_tty(int fd) {
        /* Creates a session and sets the process gruop ID */
        /*(void) setsid();*/
        tcnewpgrp(fd);
        settpgrp(fd);

        /* Set Controlling TTY for this proc to the terminal to
	 * which FD refers.
	 */
        if (ioctl(fd, TIOCSCTTY, (char *)NULL) == -1)
		return (-1);
	(void) dup2(fd, STDIN_FILENO);
	(void) dup2(fd, STDOUT_FILENO);
	(void) dup2(fd, STDERR_FILENO);
        /* Set up texttools I/O */
        SetInputDevice(3,(long)STDIN_FILENO);
        SetOutputDevice(3,(long)STDOUT_FILENO);
        SetErrorDevice(3,(long)STDERR_FILENO);
        if (fd > STDERR_FILENO)
		(void) close(fd);
	return (0);
}
