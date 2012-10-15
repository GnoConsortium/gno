/*-
 * Copyright (c) 2009 David Schultz <das@FreeBSD.org>
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
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <string.h>

#ifdef __ORCAC__

#define SHORTM sep #0x20
#define LONGM rep #0x20
#define SHORTX sep #0x10
#define LONGX rep #0x10
#define SHORTMX sep #0x30
#define LONGMX rep #0x30


size_t
strnlen(const char *s, size_t maxlen)
{
	size_t len;

	asm {
		stz <len
		stz <len+2

		ldy #0
		ldx <maxlen+2 // # of banks
		beq partial

		ldy #0
		// handle a full bank.  this is done 2-bytes at a time
bloop:
		lda [s],y
		bit #0x00ff
		beq done
		iny

		bit #0xff00
		beq done
		iny

		// partially unrolled.
		lda [s],y
		bit #0x00ff
		beq done
		iny

		bit #0xff00
		beq done
		iny

		// 3
		lda [s],y
		bit #0x00ff
		beq done
		iny

		bit #0xff00
		beq done
		iny	

		// 4
		lda [s],y
		bit #0x00ff
		beq done
		iny

		bit #0xff00
		beq done
		iny

		bne bloop
		inc <s+2
		inc <len+2
		dex
		bne bloop


partial:	// handle a partial bank.  This is done byte-by-byte.
		ldy #0
		ldx <maxlen
		beq done

		SHORTM
ploop:
		lda [s],y
		beq done
		iny
		dex
		bne ploop
		LONGM


done:
		sty <len

	}


	return (len);
}


#else

size_t
strnlen(const char *s, size_t maxlen)
{
	size_t len;

	for (len = 0; len < maxlen; len++, s++) {
		if (!*s)
			break;
	}
	return (len);
}

#endif
