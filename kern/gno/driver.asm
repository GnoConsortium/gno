*	$Id: driver.asm,v 1.1 1998/02/02 08:19:18 taubert Exp $
*  GNO- the only IIgs Operating System left!
************************************************************
*
*      Input/Output Drivers for TextTools
*
*      .null
*      GNO console
*      redirection supervisory driver
*      pipe interface driver for TextTools
*
************************************************************
*
*  Our texttool driver scheme supports two types- 
*  standard and supervisory.  Standard drivers must
*  support the same calls as a RAM-based texttools
*  driver.  Supervisory drivers must support In/Out/Err
*  calls.  They may call lower-level RAM-based protocol
*  drivers if they wish (and likely will).
*
************************************************************
*
* modified select stuff - Derek Taubert - 12/14/94
*

	mcopy	m/driver.mac
	case	on
	copy	global.equates
	copy	inc/tty.inc
	copy	inc/gsos.inc

getTTindex	START KERN2
getTTindex	name
	using KernelStruct

	lda   >truepid
	asl   a
	asl   a
	asl   a
	asl   a
	asl   a
	asl   a
	tax
	rts
	END

* given a GSstring as input, searches our device table for a match.
* if a match is found, the internal device number is returned.
* Note that these device numbers are the index+1, as 0 is not a valid
* pipe/refNum/device/.. number.  However, the TextTools use index+0,
* so keep this in mind.

findDevice	START KERN2
findDevice	name
	using KernelStruct
ind	equ   0
retval	equ   ind+2
space	equ   retval+2
	subroutine (4:gsosp),space

	ldy   #2
	lda   [gsosp],y
	and   #$FF
	cmp   #'.'               ; is this a driver name?
	bne   notadriver

	ph4   #dottty
	ph4   gsosp
	jsl   GScaseEqual
	cmp   #0
	beq   findloop1
	lda   >curProcInd
	tax
	lda   >ttyID,x
	sta   retval             ; get the 'controlling' tty #
	bra   goaway

findloop1	stz   ind
findloop	lda   ind
	asl   a
	asl   a
	tax
	lda   >DeviceNames+2,x
	ora   >DeviceNames,x
	beq   nodevice
	lda   >DeviceNames+2,x
	pha
	lda   >DeviceNames,x
	pha
	ph4   gsosp
	jsl   GScaseEqual
	cmp   #0
	beq   nodevice
* we have a device match.
	lda   ind
	sta   retval
	bra   goaway
nodevice	lda   ind
	inc   a
	cmp   #38
	bcs   notadriver
	sta   ind
	bra   findloop

notadriver	lda   #$FFFF
	sta   retval
goaway	return 2:retval
dottty	dc	i2'4'
	dc	c'.tty'
	END

getProcPtr	START
getProcPtr	name
	using	KernelStruct

	lda	>curProcInd
	tay
	clc
	adc	#KernelStruct
	ldx	#^KernelStruct
	rtl
	END

* Installs a driver into the system by adding the pointer
* to the driver's header block into the system driver table.

InstallDriver	START KERN2
InstallDriver	name
	subroutine (4:header,2:deviceNum,2:userID),0

	lda   deviceNum
	asl   a
	asl   a
	tax
	lda   header
	sta   >DeviceBlock,x
	lda   header+2
	sta   >DeviceBlock+2,x

	ldy   #t_devNum
	lda   deviceNum
	sta   [header],y

	ldy   #t_GetProcInd
	lda   tempGetPP
	sta   [header],y
	ldy   #t_GetProcInd+2
	lda   tempGetPP+2
	sta   [header],y

	ldy   #t_userid
	lda   userID
	sta   [header],y

	ldy	#t_sendSignal
	lda	tempQsig
	sta	[header],y
	ldy	#t_sendSignal+2
	lda	tempQsig+2
	sta	[header],y

	ldy	#t_BGCheck
	lda	tempBG
	sta	[header],y
	ldy	#t_BGCheck+2
	lda	tempBG+2
	sta	[header],y

	ldy	#t_selwakeup
	lda	tempSWU
	sta	[header],y
	ldy	#t_selwakeup+2
	lda	tempSWU+2
	sta	[header],y

; initialize to the standard line discipline

	lda	#0	; line discipline code
	ldy	#t_linedisc
	sta	[header],y
	ldy	#privFlags
	sta	[header],y
	ldy	#st_flags
	sta	[header],y
	lda	deviceNum
	beq	nullDev

	ldy	#t_read+2
	lda	[header],y
	ldy	#t_read
	ora	[header],y
	bne	skipR
	lda	#ttread	; standard LD read routine
	sta	[header],y
	lda	#^ttread
	ldy	#t_read+2
	sta	[header],y
skipR	anop
	ldy	#t_write+2
	lda	[header],y
	ldy	#t_write
	ora	[header],y
	bne	skipW
	lda	#ttwrite	; standard LD write routine
	sta	[header],y
	lda	#^ttwrite
	ldy	#t_write+2
	sta	[header],y
skipW	anop
; the driver's open routine must initialize the other line discipline
; vectors if they are not set up as static assignments (e.g. dc i4'CON_mutex')

; allocate the edit buffer.
	ph4	#4096
	jsl	malloc
	ldy	#editBuf
	sta	[header],y
	txa
	ldy	#editBuf+2
	sta	[header],y

nullDev	return
tempAlloc	jmp   >asmSemNew
tempDealloc	jmp   >asmSemDispose
tempWait	jmp   >asmWait
tempSignal	jmp   >asmSignal
tempBG	jmp   >BGCheck
tempQsig	jmp   >ttyQSignal
tempGetPP	jmp   >getProcPtr
tempSWU	jmp   >selwakeup
	END
