*	$Id: util.asm,v 1.1 1998/02/02 08:19:59 taubert Exp $
**************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*   Derek Taubert
*
**************************************************************************
*
* UTIL.ASM
*   By Tim Meekins
*   By Jawaid Bazyar
*
* Utility functions used by the kernel.  Mostly string manipulation
*  routines, with a strong tilt towards coversion between different types
*
**************************************************************************

	case	on
	mcopy m/util.mac

;=========================================================================
;
; Convert the accumulator to lower case.
;
;=========================================================================

TOLOWER	START KERN2

	if2   @,cc,#'A',done
	if2   @,cs,#'Z'+1,done
	add2  @,#'a'-'A',@

done	rts

	END

;=========================================================================
;
; Convert a c string to lower case
;
;=========================================================================

lowercstr	START KERN2

space	equ   0

	subroutine (4:p),space

	short a

	ldy   #0
loop	lda   [p],y
	beq   done
	if2   @,cc,#'A',next
	if2   @,cs,#'Z'+1,next
	add2  @,#'a'-'A',@
	sta   [p],y
next	iny
	bra   loop

done	long  a

	return

	END

;=========================================================================
;
; Get the length of a c string.
;
;=========================================================================

cstrlen	START KERN2

space	equ   0

	subroutine (4:p),space

	short a
	ldy   #0
loop	lda   [p],y
	beq   done
	iny
	bra   loop

done	long  a
	sty   p

	return  2:p

	END

;=========================================================================
;
; Allocate memory for a c string the same length as another string.
;
;=========================================================================

alloccstr	START KERN2

space	equ   0

	subroutine (4:p),space

	ph4   p                  ;get length of string
	jsl   cstrlen
	inc   a                  ;for terminator
	pea   0                  ;allocate memory
	pha
	jsl   ~NEW
	sta   p
	stx   p+2

	return 4:p

	END


;=========================================================================
;
; Copy a GS/OS string to a GS/OS result buffer. Assumes that the result
; address points to valid memory
;
;=========================================================================

copygs2res	START KERN2
bufString	equ  0
retval	equ  4

	subroutine (4:in,4:resultBuf),6
	lda	[in]
	ldy	#2
	sta	[resultBuf],y
	clc
	adc	#4	; string length, but add in buf size
	cmp	[resultBuf]
	beq	lenOkay
	bcc	lenOkay
	lda	#$004F	; bwa haha
	bra	goaway
lenOkay	lda	[in]	; we just want the length, bud
	tay	
	iny		; copy the length word also
	lda	resultBuf
	clc
	adc	#2
	sta	bufString
	lda	resultBuf+2
	adc	#0
	sta	bufString+2
	short a
loop	lda   [in],y
	sta   [bufString],y
	dey
	bpl   loop
	long  m
	lda	#0
goaway	sta	retval
	return 2:retval
	END

;=========================================================================
;
; appends one GS/OS string to another. assumes the destination is big
; enough to hold the result.
;
;=========================================================================
gsstrcat	START KERN2

iind	equ   0
oind	equ   iind+2
left	equ   oind+2
space	equ   left+2

	subroutine (4:s,4:d),space
	lda   [d]
	inc   a
	inc   a
	sta   oind
	lda   #2
	sta   iind
	lda   [s]
	sta   left
	clc
	adc   [d]
	sta   [d]

loop	short m
	ldy   iind
	lda   [s],y
	ldy   oind
	sta   [d],y
	long  m
	inc   iind
	inc   oind
	dec   left
	bne   loop
	return
	END

;=========================================================================
;
; Copy one GS/OS string to another. Assumes an alloccstr has been
; performed on destination.
;
;=========================================================================

copygsstr	START KERN2

space	equ   0

	subroutine (4:p,4:q),space

	lda   [p]
	inc   a
;               clc
;               adc   #1
	tay
	short a
loop	lda   [p],y
	sta   [q],y
	dey
	bpl   loop
done	long  a

	return

	END

;=========================================================================
;
; Copy one pascal string to another. Assumes an alloccstr has been
; performed on destination.
;
;=========================================================================

copypstr	START KERN2

space	equ   0

	subroutine (4:p,4:q),space

	lda   [p]
	and   #$00FF
	tay
	short a
loop	lda   [p],y
	sta   [q],y
	dey
	bpl   loop
done	long  a

	return

	END

;=========================================================================
;
; Copy one string to another. Assumes an alloccstr has been performed on
; destination.
;
;=========================================================================

copycstr	START KERN2

space	equ   0

	subroutine (4:p,4:q),space

	short a
	ldy   #0
loop	lda   [p],y
	beq   done
	sta   [q],y
	iny
	bra   loop
done	sta   [q],y
	long  a

	return

	END

;=========================================================================
;
; Converts a pascal string to a c string. This allocates memory for the
; new c string.
;
;=========================================================================

p2cstr	START KERN2

cstr	equ   0
space	equ   cstr+4

	subroutine (4:p),space

	lda   [p]
	and   #$FF
	inc   a
	pea   0
	pha
	jsl   ~NEW
	sta   cstr
	stx   cstr+2
	lda   [p]
	and   #$FF
	tax

	short a
	ldy   #0
loop	cpx   #0
	beq   done
	iny
	lda   [p],y
	dey
	sta   [cstr],y
	iny
	dex
	bra   loop

done	lda   #0
	sta   [cstr],y
	long  a

	return 4:cstr

	END

;=========================================================================
;
; Converts a GS/OS string to a c string. This allocates memory for the
; new c string.
;
;=========================================================================

gs2cstr	START KERN2

cstr	equ   0
space	equ   cstr+4

	subroutine (4:gs),space

	lda   [gs]
	inc   a
	pea   0
	pha
	jsl   ~NEW
	sta   cstr
	stx   cstr+2
	lda   [gs]
	tax

	short a
	ldy   #0
loop	cpx   #0
	beq   done
	iny
	iny
	lda   [gs],y
	dey
	dey
	sta   [cstr],y
	iny
	dex
	bra   loop

done	lda   #0
	sta   [cstr],y
	long  a

	return 4:cstr

	END


;=========================================================================
;
; Converts a c string to a pascal string. Does not allocate memory.
;
;=========================================================================

c2pstr	START KERN2

space	equ   0

	subroutine (4:cstr,4:p),space

	short a
	ldy   #0
loop	lda   [cstr],y
	beq   endstr
	iny
	sta   [p],y
	bra   loop
endstr	tya
	sta   [p]
	long  a

	return

	END

;=========================================================================
;
; Compare two c strings. Return 0 if equal.
;
;=========================================================================

cmpcstr	START KERN2

ch	equ   0
result	equ   ch+2
space	equ   result+2

	subroutine (4:p,4:q),space

	ld2   1,result

	ldy   #0
strloop	lda   [p],y
	and   #$FF
	beq   strchk
	sta   ch
	lda   [q],y
	and   #$FF
	cmp   ch
	bne   done
	iny
	bra   strloop
strchk	lda   [q],y
	and   #$FF
	bne   done

	stz   result

done	return  2:result

	END

writeacc	START
	phb
	phk
	plb
	phy
	phx
	pha
	sta tempacc
	xba
	lsr a
	lsr a
	lsr a
	lsr a
	and #$0F
	jsr hexdig
	lda tempacc
	xba
	and #$0F
	jsr hexdig
	lda tempacc
	lsr a
	lsr a
	lsr a
	lsr a
	and #$0F
	jsr hexdig
	lda tempacc
	and #$0F
	jsr hexdig
	pla                      ; preserve the accumulator
	plx
	ply
	plb
	rts
tempacc	dc i2'0'

hexdig	entry
	cmp #$A
	bcc add0
	clc
	adc #7
add0	anop
	clc
	adc #'0'
	ErrWriteChar @a
	rts
	END

;=========================================================================
;
; Copy a Pascal string to a GS/OS string
;
;=========================================================================

copyp2gs	START KERN2
slen	equ   0
	subroutine (4:p,4:gs),2

	lda   [p]
	and   #$00FF
	sta   [gs]
	sta   slen
	ldy   #0
	short m
loop	iny
	lda   [p],y
	iny
	sta   [gs],y
	dey
	cpy   slen
	bcc   loop
	long  m
	return
	END

;=========================================================================
;
; Copy a GS/OS string to a Pascal string
;
;=========================================================================

copygs2p	START KERN2
slen	equ   0
	subroutine (4:gs,4:p),2

	lda   [gs]
	and   #$00FF
	sta   [p]
	beq   done               ; 0 length- done!
	inc   a
	sta   slen
	ldy   #1
	short m
loop	iny
	lda   [gs],y
	dey
	sta   [p],y
	iny
	cpy   slen
	bcc   loop
	long  m
done	return
	END

;=========================================================================
;
; Copy a C string to a GS/OS result buffer
;
;=========================================================================

copyc2res	START KERN2
bufString	equ   0
clen	equ   4
in	equ   6
out	equ   8
space	equ   out+2

	subroutine (4:c,4:res),space

	ph4   c
	jsl   cstrlen
	ldy   #2
	sta   [res],y
	clc
	adc	#4
	cmp   [res]
	beq   lenokay
	bcc   lenokay
	lda   #$4F
	bra   goaway
lenokay	anop
	lda	[res],y
	sta   clen
	ldy   #0
	sty   in   
	ldy   #4
	sty   out
loop	anop
	short m
	ldy   in
	lda   [c],y
	ldy   out
	sta   [res],y
	long  m
	inc   in
	inc   out
	ldy   in
	cpy   clen
	bcs   done
	bra   loop
done	anop
	lda   #0
goaway	sta   clen
	return 2:clen
	END

GScaseEqual	START KERN2
retval	equ	0
lowera	equ   2
	subroutine (4:a,4:b),4

	lda	[a]
	cmp	[b]
	bne	notequal
	tay
	iny
loop	anop
	cpy 	#1
	beq	isequal            ; done with comparison
	lda	[a],y
	and	#$00FF
	jsr	TOLOWER
	sta	lowera
	lda	[b],y
	and	#$00FF
	jsr	TOLOWER
	cmp	lowera
	bne	notequal
	dey
	bra	loop
	
isequal	lda	#1
	sta	retval
goaway	return 2:retval
notequal	stz	retval
	bra	goaway
	END

COPYC2GS	START KERN2
	subroutine (4:c,4:gs),0
	
	ph4	c
	jsl	cstrlen
	sta	[gs]

	tay
	cpy	#0
	beq	done
	short	m
lp	lda	[c],y
	iny2
	sta	[gs],y
	dey2
	dey
	bpl	lp
done	long	m
	return
	END

~NDISPOSE	START KERN2
~NDISPOSE	name
	subroutine (4:ptr),0
	lda	ptr+2
	bit	#$FF80
	bne	badptr
	ora	ptr
	beq	zptr
	ph4	ptr
	jsl	~DISPOSE
	return
badptr	anop
	ph4	#bptrtxt
	jsl	PANIC
	brk	$00
bptrtxt	dc	c'Attempt to ~dispose bogus pointer',i1'0'
zptr	anop
	ph4	#zptrtxt
	jsl	PANIC
	brk	$00
zptrtxt	dc	c'Attempt to ~dispose null pointer',i1'0'
	END

	case	on
nfree	START KERN2
nfree	name
	subroutine (4:ptr),0
	lda	ptr+2
	bit	#$FF80
	bne	badptr
	ora	ptr
	beq	zptr
	ph4	ptr
	jsl	free
	return
badptr	anop
	ph4	#bptrtxt
	jsl	PANIC
	brk	$00
bptrtxt	dc	c'Attempt to free bogus pointer',i1'0'
zptr	anop
	ph4	#zptrtxt
	jsl	PANIC
	brk	$00
zptrtxt	dc	c'Attempt to free null pointer',i1'0'
	END

gnoSetHandleID	START KERN2
	subroutine (4:hand,2:id),0

	lda	id
	ldy	#6
	sta	[hand],y
	return
	END
