;
; Stack checking routines by Phillip Vandry <vandry@cam.org>.  Added
; to GNO by Devin Reade <gdr@myrias.com>.  See the man page for details.
;
; $Id: stack.asm,v 1.1 1997/02/28 05:12:47 gdr Exp $
;

	keep	stack

	mcopy	stack.mac
	case	on

* This test function goes into "stack.ROOT", which is not used

main	start
	jsl	_beginStackCheck
	ldx	#16
	jsr	recurse
	jsl	_endStackCheck
	rtl
recurse	dex
	beq	outs
	jsr	recurse
outs	rts
	end

_beginStackCheck start libc_gno__
	using	stack_str
	phb
	pha
	pha
	pea	0
	phd
	~FindHandle *
	tsc
	phd
	tcd
	pha
	pha
	pei	3
	pei	1
	~GetHandleSize *
	pla
	sta	>stack_size
	pla
	lda	[1]

* leave 256 bytes at the beginning for SANE
	clc
	adc	#256

	tay		; start of handle
	sta	>stack_loc
	ldx	#0
	tsc
	dec	a
	dec	a
	sta	3
	pea	0
	plb
	plb
lppp	anop
	txa
	and	#%111
	tax
	lda	>stack_str,x
	sta	|0,y
	iny
	inx
	cpy	3
	bcc	lppp
	pld
	pla
	pla
	plb
	rtl
	end

_endStackCheck start libc_gno__
	using	stack_str
	phb
	pea	0
	plb
	plb
	lda	>stack_loc
	tay
	ldx	#0
	lda	|0,y
	cmp	>stack_str,x
	beq	lp2
	lda	#$ffff	; gone over
	bra	terminate

lp2	anop
	txa
	and	#%111
	tax
	lda	|0,y
	cmp	>stack_str,x
	bne	gotend
	inx
	iny
	bra	lp2
gotend	anop		; Y = Stack not used
	tya
	sec
	sbc	>stack_loc
	pha
	lda	>stack_size
	sec
	sbc	1,s
	ply
terminate plb
	rtl
	end

	setcom 50

stack_str	privdata	libc_gno__

		dc	c'StackChkS'
stack_size	ds	2
stack_loc	ds	2
		end
