**************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*
* $Id: mmdebug.asm,v 1.2 1998/04/24 15:38:33 gdr-ftp Exp $
*
**************************************************************************

	mcopy	/obj/gno/bin/gsh/mmdebug.mac

dummy	start		; ends up in .root
	end

	setcom 60

~NEW	START
hand	equ	0
ptr	equ	4
_~NEW	name

	subroutine (4:size),8

	NewHandle (size,~USER_ID,#$C018,#0),hand
               lda	[hand]
	sta	ptr
	ldy	#2
	lda	[hand],y
	sta	ptr+2
	return 4:ptr
	END

~DISPOSE	START
hand	equ	0
checkptr	equ	4
_~DISPOSE	name

	subroutine (4:ptr),8

	FindHandle ptr,hand
               lda	[hand]
	sta	checkptr
	ldy	#2
	lda	[hand],y
	sta	checkptr+2
	eor	ptr+2
	eor	ptr
	eor	checkptr
	beq	okay

               brk	$55

okay	DisposeHandle hand
	return
	END
