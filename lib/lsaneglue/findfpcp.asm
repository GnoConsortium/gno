	case	on
	mcopy	findfpcp.mac

*
* Find the slot the FPE or NumberCruncher card is in
*
* Returns the slot number or -1 if FPCP card cannot be found.
*
* For slots 1-5 and 7, findfpcp() can find the card
* regardless of the Slot setting in the Control Panel.
* For slot 6, the Control Panel must be set to "Your Card"
* for the function to be able to find the card.
*
* Written in 1997 by Soenke Behrens, from Merlin code by
* Albert Chin-A-Young
* This code is hereby place into the Public Domain
*

*
* Dummy function to take care of findfpcp.root, which
* can then be discarded.
*

dummy	start
	end

****************************************************************
*
*  int findfpcp (void);
*
*  Find slot FPCP card is in
*
*  See also:  Floating-Point Coprocessor Manual by Albert
*  Chin-A-Young
*
****************************************************************
*

findfpcp start
SETINTC3ROM equ $e1c00a	; enable internal slot 3 ROM
SETSLOTC3ROM equ $e1c00b ; enable external slot 3 ROM
RDC3ROM	equ	$e1c017 ; bit 7 = 1 if slot c3 space enabled
SLTROMSEL equ	$e1c02d ; slot ROM select

	csub

	sei			Disable interrupts
	short	m		8-bit accumulator
	lda	#0		enable slot 3 ROM as FPCP
	sta	SETSLOTC3ROM 	 might be in slot 3
lab1	lda	RDC3ROM		wait for external slot 3
	bpl	lab1		 space to be enabled
	lda	SLTROMSEL	store previous value in Y
	tay
	ora	#%10110110	enable slot 1, 2, 4, 5, 7 ROM
	sta	SLTROMSEL
	long	m		16-bit accumulator

	ldx	#$c100		Start with slot 1
search	lda	$e00004,x	read slot at address $04,
	cmp	id_bytes	 $06 and $0b. If no match
	bne	next_slot	 is found, exit, else set up
	lda	$e00006,x	 base address
	cmp	id_bytes+2
	bne	next_slot
	lda	$e0000b,x
	cmp	id_bytes+4
	beq	found

next_slot txa			try next slot
	clc
	adc	#$100
	tax
	cmp	#$c000		last slot + $100
	bne	search

	lda	#$FFFF
	sta	slot_num	store -1 to
	bra	end		 indicate FPCP not found

found	txa			slot number of FPCP
	xba
	and	#$0f		strip slot number
	sta	slot_num	save slot number

	
end	short	m
	lda	#0		re-enable internal slot 3 ROM
	sta	SETINTC3ROM
lab2	lda	RDC3ROM		wait for internal slot 3
	bmi	lab2		 space to be enabled
	tya			re-enable other internal
	sta	SLTROMSEL	 slot ROM
	long	m

	cli			re-enable interrupts
	ret	2:slot_num

id_bytes anop
	dc	h'3838'
	dc	h'1818'
	dc	h'01af'

slot_num ds	2

	end
