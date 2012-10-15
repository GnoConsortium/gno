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

#ifdef __XXORCAC__

char *
stpncpy(char * __restrict dst, const char * __restrict src, size_t n)
{
	#define LONG_M rep #0x20
	#define SHORT_M sep #0x20


	// copy at most n characters.
	// if strlen(src) < n, zero-fill the remainder (but return ptr to first null)

	asm {

		SHORT_M
		ldy #0
		ldx <n+2 	// max # of banks to copy
		beq partial

		// potentially copy a full bank.
bloop:
		// partially unrolled, factor = 4
		lda [src],y
		sta [dest],y
		beq done
		iny

		lda [src],y
		sta [dest],y
		beq done
		iny

		lda [src],y
		sta [dest],y
		beq done
		iny

		lda [src],y
		sta [dest],y
		beq done
		iny


		bne bloop

		inc <src+2
		inc <dest+2
		dec <n+2
		dex
		bne bloop

partial:
		// copy a partial bank
		ldy #0
		ldx <n
		beq maxn
ploop:
		lda [src],y
		sta [dest],y
		beq done
		iny
		dex
		bne ploop
	
maxn:	
		// at this point, n bytes have been copied and no NULL was found.
		// if stpncpy does not terminate with a NULL, it returns dst+n

		// n+2 is 0.
		LONG_M
		clc
		lda <n
		adc <dst
		sta <dst
		lda #0
		adc <dst+2
		sta <dst+2
		stz <n
		stz <n+2 // will already be 0.
		bra r 

done:
		// at this point, a NULL character was found
		// however, dest[y+1].. dest[n-1] must be 0 filled.
		LONG_M
		tya
		clc
		adc <dst
		sta <dst
		lda #0
		adc <dst+2
		sta <dst+2

		// update n for bzero.
		sec
		tya
		sbc <n
		sta <n
		sbc <n+2
		sta <n+2

r:
	}

	// really only need to do this if n > 1
	if (n) bzero(dst, n);
	return dst;
}

#else

char *
stpncpy(char * __restrict dst, const char * __restrict src, size_t n)
{

	for (; n--; dst++, src++) {
		if (!(*dst = *src)) {
			char *ret = dst;
			while (n--)
				*++dst = '\0';
			return (ret);
		}
	}
	return (dst);
}

#endif