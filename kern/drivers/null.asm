*	$Id: null.asm,v 1.1 1998/02/02 08:18:00 taubert Exp $
************************************************************
*
* NULL device driver
*
************************************************************

	case	on
	mcopy	port.mac
	copy	../gno/inc/tty.inc

NullHeader	START

	ds	t_open
* For speed/simplicity we use a non-standard, non-documented line
* discipline for the .null device
	dc	i4'NullInit2'
	dc	i4'NullDeInit'
	dc	i4'NullIOCTL'
	dc	i4'NullRead'
	dc	i4'NullWrite'
	dc	i4'NullNoFunc'
	dc	i4'NullNoFunc'
	dc	i4'NullNoFunc'
	dc	i4'NullNoFunc'
	dc	i4'NullNoFunc'
	dc	i4'NullNoFunc'
	dc	i4'NullNoFunc'
	dc	i4'NullNoFunc'
	ds	t_signalIO-editInd
	dc	i4'NullNoFunc'		; t_signalIO
	dc	i2'$FFFF'		; t_select_proc
	dc	i4'NullSelect'		; t_select
	dc	i4'0'			; t_selwakeup jmp set during install

NullNoFunc	anop
	rtl
	END

NullInit2	START
result	equ	0
	subroutine (2:devNum),2
	stz	result
	return 2:result
	END

NullDeInit	START
	subroutine (2:devNum),0
	return
	END

NullWrite	START
count	equ	0
retval	equ	2
	subroutine (2:reqCount,4:dataPtr,2:devNum),4
	stz	retval
	lda	reqCount
	sta	count
	return 4:count
	END

NullRead	START
count	equ	0
retval	equ	2
	subroutine (2:reqCount,4:dataPtr,2:devNum),4
	stz	count
	lda	#$4C
	sta	retval
	return 4:count
	END

NullIOCTL	START
retval	equ   0
	subroutine (4:tioc,4:dataPtr,2:devNum),2
	stz   retval
	return 2:retval
	END

NullSelect	START
res	equ	0
	subroutine (2:ttyn,2:which,2:pid),2
	lda	#1
	sta	res
	return	2:res
	END
