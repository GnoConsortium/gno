************************************************************
*
* FULL device driver
*
************************************************************

	case	on
	mcopy	port.mac
	copy	../gno/inc/tty.inc


Header	START

	ds	t_open
* For speed/simplicity we use a non-standard, non-documented line
* discipline for the .null device
	dc	i4'Init2'
	dc	i4'DeInit'
	dc	i4'IOCTL'
	dc	i4'Read'
	dc	i4'Write'
	dc	i4'NoFunc'
	dc	i4'NoFunc'
	dc	i4'NoFunc'
	dc	i4'NoFunc'
	dc	i4'NoFunc'
	dc	i4'NoFunc'
	dc	i4'NoFunc'
	dc	i4'NoFunc'
	ds	t_signalIO-editInd
	dc	i4'NoFunc'		; t_signalIO
	dc	i2'$FFFF'		; t_select_proc
	dc	i4'Select'		; t_select
	dc	i4'0'			; t_selwakeup jmp set during install

NoFunc	anop
	rtl
	END

Init2	START
result	equ	0
	subroutine (2:devNum),2
	stz	result
	return 2:result
	END

DeInit	START
	subroutine (2:devNum),0
	return
	END

Write	START
* 
*  returns ENOSPC / volumeFull
*
count	equ	0
retval	equ	2
	subroutine (2:reqCount,4:dataPtr,2:devNum),4
	lda #$48
	sta retval
	stz	count
	return 4:count
	END

Read	START
*
* .full read returns infinite zeros.
*
count	equ	0
retval	equ	2
	subroutine (2:reqCount,4:dataPtr,2:devNum),4
	stz	retval
	lda	reqCount
	sta	count

	lsr a
	beq last
	tax
	ldy #0
	lda #0
loop	anop
	sta [dataPtr],y
	iny 
	iny
	dex
	bne loop

last	anop
* at this point:
* a = 0
* c = 1 if reqCount is odd.
	bcc out
	
	short m
	ldy reqCount
	dey
	sta [dataPtr],y
	long m

out	anop
	return 4:count
	END

IOCTL	START
retval	equ   0
	subroutine (4:tioc,4:dataPtr,2:devNum),2
	stz   retval
	return 2:retval
	END

Select	START
res	equ	0
	subroutine (2:ttyn,2:which,2:pid),2
	lda	#1
	sta	res
	return	2:res
	END

