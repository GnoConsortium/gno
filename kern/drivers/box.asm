*	$Id: box.asm,v 1.2 1999/03/19 07:18:00 gdr-ftp Exp $
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
* BOX.ASM
*   By Jawaid Bazyar
*
* Contains routines for drawing boxes using MouseText
*
**************************************************************************

	case	on
	mcopy	../drivers/box.mac
	copy	../drivers/console.equates

dummyBox start			; ends up in .root and ignored
	end


* EraseBox takes the same parameters as DrawBox, except the Accum
*  is the character to fill the box with.

EraseBox	START

	sta	EraseChar
	lda	<IODP_CV	; save cursor location
	pha
	lda	<IODP_CH
	pha

	lda	<IODP_TopMar
	sta	CurCV
	lda	<IODP_RightMar
	sec
	sbc	<IODP_LeftMar
	ina
	sta	TempWidth
VLoop	lda	CurCV
	sta	<IODP_CV
	jsr	VTAB
	lda	<IODP_LeftMar
	sta	<IODP_CH

HLoop	lda	EraseChar
	ora	#$80
	ldy	<IODP_CH
	jsr	StorChar
	iny
	sty	<IODP_CH
	cpy	<IODP_RightMar
	beq	HLoop
	bcc	HLoop

	inc	CurCV
	lda	CurCV
	cmp	<IODP_BotMar
	beq	VLoop
	bcc	VLoop

	pla
	sta	<IODP_CH
	pla
	sta	<IODP_CV
	jsr	VTAB        ;restore cursor location
	rts
EraseChar	dc  i2'0'
TempWidth	dc  i2'0'
CurCV	dc  i2'0'
	END
