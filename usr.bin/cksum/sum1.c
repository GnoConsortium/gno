/*-
 * Copyright (c) 1991, 1993
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
 * Modified for GNO (Apple IIGS) by Dave Tribby, December 1997
 *
 * Constructs unacceptable to compiler are replaced using #ifndef __ORCAC__
 *
 * Changes not related to compiler are replaced using #ifndef __GNO__
 *
 * Added prototyped headers, surrounded by #ifndef __STDC__
 */


#ifndef __GNO__
#ifndef lint
static char sccsid[] = "@(#)sum1.c	8.1 (Berkeley) 6/6/93";
#endif /* not lint */
#endif /* not GNO */

#include <sys/types.h>
#include <unistd.h>

int
#ifndef __STDC__
csum1(fd, cval, clen)
	register int fd;
	u_long *cval, *clen;
#else
csum1(register int fd,
	u_long *cval,
	u_long *clen)
#endif
{
	register u_long total;
	register int nr;
#ifndef __GNO__
	register u_int crc;
#else
	register u_long crc;
#endif
	register u_char *p;
#ifndef __GNO__
	u_char buf[8192];
#else
	static u_char buf[8192];
#endif

	/*
	 * 16-bit checksum, rotating right before each addition;
	 * overflow is discarded.
	 */
	crc = total = 0;
	while ((nr = read(fd, buf, sizeof(buf))) > 0)
#if defined(__NOASM__) || !defined(__ORCAC__)
		for (total += nr, p = buf; nr--; ++p) {
			if (crc & 1)
				crc |= 0x10000;
			crc = ((crc >> 1) + *p) & 0xffff;
		}
#else
	/* Hand-optimized code for Apple IIGS */
        asm{
		ldx #0		; Use x-reg to index into buf.
		clc             ; total =
		lda nr          ;         nr
		adc total	;           + total
		sta total
		bcc nextchar	; If total overflows,
		inc total+2	;   increment high-order word.
	nextchar:
		clc
		lda crc		; Get current crc.
		bit #1		; If low-order bit is set,
		beq rotate	;   set carry flag for rotate.
		sec
	rotate:
		ror A		; Rotate right 1 bit.
		sta crc		; Temporarially store result.
		lda >buf,x	; Get next character
		and #0xff	;  (low-order byte only)
		clc		;   and add to crc.
		adc crc
		sta crc
		inx		; Bump pointer.
		dec nr		; Decrement byte counter.
		bne nextchar	; Stay in loop until done.
        }
#endif
	if (nr < 0)
		return(1);

	*cval = crc;
	*clen = total;
	return(0);
}
