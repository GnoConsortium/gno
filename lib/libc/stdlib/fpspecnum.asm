*
* Test an extended to see whether it is NaN or INF
*
* $Id: fpspecnum.asm,v 1.2 1997/12/21 20:12:27 gdr Exp $
*
* Soenke Behrens, August 1997
*     
	case	on
	mcopy	fpspecnum.mac

*
* Dummy function to take care of fpspecnum.root, which
* can then be discarded.
*

dummy	start
	copy :lang:orca:libraries:ainclude:e16.sane ; Apple-supplied SANE EQUs
	end

****************************************************************
*
*  int _isnan (extended x);
*
*  Check whether x is NaN, if so, return 1, otherwise, return 0.
*
****************************************************************
*

_isnan	start	libc_stdlb
result	equ	1
space	equ	result+2

	csub	(10:x),space

	short	m	clear the specific NaN code
	stz	x+6
	long	m

	lda	x	and do the compare
	cmp	nan_x
	bne	diff
	lda	x+2
	cmp	nan_x+2
	bne	diff
	lda	x+4
	cmp	nan_x+4
	bne	diff
	lda	x+6
	cmp	nan_x+6
	bne	diff
	lda	x+8
	cmp	nan_x+8
	bne	diff
	lda	#1
	sta	result
	bra	bye
diff	stz	result

bye	ret	(2:result)

nan_x	dc	h'0000000000000040FF7F' ; Hex encoding of a NaN
	end

****************************************************************
*
*  int _isinf (extended x);
*
*  Check whether x is INF, if so, return 1, otherwise, return 0.
*
****************************************************************
*

_isinf	start	libc_stdlb
result	equ	1
space	equ	result+2

	csub	(10:x),space
	short	m
	lda	x+9	get rid of sign bit
	and	#%01111111
	sta	x+9
	long	m

	lda	x	and do the compare
	cmp	inf_x
	bne	diff
	lda	x+2
	cmp	inf_x+2
	bne	diff
	lda	x+4
	cmp	inf_x+4
	bne	diff
	lda	x+6
	cmp	inf_x+6
	bne	diff
	lda	x+8
	cmp	inf_x+8
	bne	diff
	lda	#1
	sta	result
	bra	bye
diff	stz	result

bye	ret	(2:result)

inf_x	dc	h'0000000000000000FF7F' ; Hex encoding of +INF
	end
