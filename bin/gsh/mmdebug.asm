**************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*
* $Id: mmdebug.asm,v 1.3 1998/08/03 17:30:23 tribby Exp $
*
**************************************************************************
*
* MMDEBUG.ASM
*   By Tim Meekins
*   Rewritten by Dave Tribby for GNO 2.0.6
*
* Alternate routines for ~NEW and ~DISPOSE with extra debugging
*
* Note: text set up for tabs at col 16, 22, 41, 49, 57, 65
*              |     |                  |       |       |       |
*	^	^	^	^	^	^	
**************************************************************************
*
* Interfaces defined in this file:
*
*
**************************************************************************

	mcopy	/obj/gno/bin/gsh/mmdebug.mac

dummymmdebug	start		; ends up in .root
	end

	setcom 60

DB~NEW	START

hand	equ	0	Handle allocated by memory manager
ptr	equ	hand+4	Pointer dereference of handle
space	equ	ptr+4

	subroutine (4:size),space

	lda	size	If size is 0,
	ora	size+2
	bne	newh
	brk	$A1	 allocation error #1.
	bra	foul
;
; Allocate memory block of requested size using NewHandle
;
newh	anop

	pha		Reserve space for
	pha		 returned handle address.
	ph4	size	Amount of memory to allocate.
	lda	~USER_ID	Add the "auxID" tag
	eor	#$0D00	 to the user ID so we
	pha		  can identify our allocations.
	pea	$C018
	pea	0
	pea	0
	tool	$0902	NewHandle (size,~USER_ID,#$C018,#0)
	plx		Store handle at "hand",
	stx	hand	 preserving contents of
	plx		  possible error code
	stx	hand+2	   in accumulator.
                  
	bcc	deref	If there is a MM error,
	brk	$A2	 allocation error #2.
foul	stz	ptr
	stz	ptr+2
	bra	goback

deref	lda	[hand]	Dereference the handle.
	sta	ptr
	ldy	#2
	lda	[hand],y
	sta	ptr+2

;
; Fill memory with recognizable pattern
;
	lda	#"~~"
	ldy	size
dec_index	dey
	dey
	bmi	full
	sta	[ptr],y
	bra	dec_index

full	short	a	In case size was odd,
	sta	[ptr]	 be sure the first byte
	long	a	  gets the pattern.

goback	return 4:ptr	Return pointer to caller.

	END


DB~DISPOSE	START
hand	equ	0
checkptr	equ	4
space	equ	checkptr+4

	subroutine (4:ptr),space
;
; First check: is address > $007FFFFF?
;
	lda	ptr+2
	and	#$FF80
	beq	findit
	lda	ptr
	ldx	ptr+2
	brk	$D1	Deallocate error #1.
	bra	goback

;
; Second check: is there a valid handle to this pointer?
;
findit	FindHandle ptr,hand

	lda	hand	Error if FindHandle
	ora	hand+2	 returned 0.
	bne	ckptr
	brk	$D2	Deallocate error #2.
	bra	goback

ckptr	lda	[hand]	Dereference the
	sta	checkptr	 found handle.
	ldy	#2
	lda	[hand],y
               sta	checkptr+2

	cmp	ptr+2	If the pointer isn't
	bne	errD3	 the first byte
	lda	checkptr	  of the handle,
	cmp	ptr	   there is a problem.
	beq   chkid
errD3	brk	$D3
	bra	goback

chkid	ldy	#6	Get User ID word
	lda	[hand],y	 from handle header.
	eor	#$0D00	Remove aux ID of $0D00.
	cmp	~USER_ID
	beq	okay
	lda	[hand],y
	brk	$D4	Deallocate error #4.
	bra	goback

okay	DisposeHandle hand	Deallocate the memory.
	bcc	goback
	brk	$D5	Deallocate error #5.

goback	return

	END
