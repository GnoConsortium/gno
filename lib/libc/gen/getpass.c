/*
 * Copyright (c) 1988 The Regents of the University of California.
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
 * GDR:  This file could probably be cleaned up a bit ...
 *
 * $Id: getpass.c,v 1.1 1997/02/28 05:12:44 gdr Exp $
 *
 * This file is formatted for tab stops every 8 characters.
 */
 
#ifdef __ORCAC__
segment "libc_gen__";
#endif

#pragma optimize 0
#pragma debug 0
#pragma memorymodel 0

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)getpass.c	5.9 (Berkeley) 5/6/91";
#endif /* LIBC_SCCS and not lint */

#include <sgtty.h>
#include <sys/signal.h>

#include <pwd.h>
#include <stdio.h>
#include <unistd.h>

#include <string.h>
#include <stddef.h>
#include <texttool.h>
#include <sys/fcntl.h>
#include <signal.h>
#undef echo

char *
getpass(const char *prompt) {
	struct sgttyb term;
	register int ch;
	register char *p;
	FILE *fp, *outfp;
	long omask;
	int echo;
	static char buf[_PASSWORD_LEN + 1];

	/*
	 * read and write to /dev/tty if possible; else read from
	 * stdin and write to stderr.
	 */
     /*	if ((outfp = fp = fopen(".tty", "w+")) == NULL) { */
	if ((outfp = fp = fopen(".tty", "r+")) == NULL) {
		outfp = stderr;            
		fp = stdin;
	}
	setbuf(outfp,NULL);
        /*
	 * note - blocking signals isn't necessarily the
	 * right thing, but we leave it for now.
	 */
	omask = sigblock(sigmask(SIGINT)|sigmask(SIGTSTP));
	/*(void)tcgetattr(fileno(fp), &term); */
	gtty(fileno(fp),&term);
        if (echo = (term.sg_flags & ECHO)) {
		term.sg_flags &= ~ECHO;
		stty(fileno(fp),&term);
                /*(void)tcsetattr(fileno(fp), TCSAFLUSH|TCSASOFT, &term);*/
	}
     /*	(void)fputs(prompt, outfp);*/
    /*	rewind(outfp);	*/		/* implied flush */
	write(fileno(outfp),prompt,strlen(prompt));
    /*    for (p = buf; (ch = getc(fp)) != EOF && ch != '\n';)  */
        for (p = buf; (ch = ReadChar(0)&0x7F) != 0 && ch != '\n' && ch != '\r';)
		if (p < buf + _PASSWORD_LEN)
			*p++ = ch;
	*p = '\0';
	(void)write(fileno(outfp), "\r", 1);
	if (echo) {
		term.sg_flags |= ECHO;
		stty(fileno(fp), &term);
	}
	(void)sigsetmask(omask);
	if (fp != stdin)
		(void)fclose(fp);
	return(buf);
}
