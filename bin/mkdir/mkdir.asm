***********************************************************************
*
* MKDIR.ASM - Version 1.0
* Written by Tim Meekins
* Copyright (C) 1991 by Procyon, Inc.
* This program is hereby donated to the public domain.
*
* This program creates a new directory in the current (or specified)
* directory.
*
* TODO:
*   o Add -p option.
*   o Should I support creating multiple directories like Unix?
*
* HISTORY:
*   1.0  11/29/91 First version.
*
**************************************************************************

	keep	mkdir
               mcopy mkdir.mac

mkdir	START
	jml	~GNO_COMMAND
	END

main	START

arg	equ	0
retval	equ	arg+4
space	equ	retval+2

	subroutine (2:argc,4:argv),space

	stz	retval

	lda	argc
	dec	a
	bne	part2
	ErrWriteCString #usage
	jmp	error
part2	dec	a
	beq	part3
	ErrWriteCString #oneerr
	jmp	error

part3	ldy	#4
	lda	[argv],y
	sta	arg
	iny2
	lda	[argv],y
	sta	arg+2

	ldy	#0
	short	a
loop	lda	[arg],y
	beq	part4
	sta	pathname+2,y
	iny
	bra	loop

part4	long	a
	sty	pathname
	Create createparm
	bcc	done

	sta	errval

	ErrWriteCString #errleadin
	Error errval
		
error	inc	retval

done	return 2:retval

createparm	dc	i2'5'
createpath	dc	i4'pathname'
	dc	i2'$C3'
	dc	i2'$0F'
	dc	i4'$0000'
	dc	i2'$0D'

errval	dc	i2'0'

usage	dc	c'Usage: mkdir directory.',h'0d0a00'
oneerr	dc	c'mkdir: too many arguments.',h'0d0a00'
errleadin	dc	c'mkdir: ',h'00'

pathname	dc	i2'0'
	ds	256	;I dread any person entering a pathname
;			;this long!

	END
