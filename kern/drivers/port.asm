*	$Id: port.asm,v 1.1 1998/02/02 08:18:01 taubert Exp $
*
* 12/15/94 - select code added - Derek Taubert
*
	case	on
	mcopy	port.mac
	copy	../gno/inc/tty.inc
	copy	equates

SerialHeader	START

dtty	ENTRY
	ds	t_open
* Line Discipline entry points
	dc	i4'DInit'
	dc	i4'DDeInit'
	dc	i4'DIOCTL'
	dc	i4'0'	; ttread and ttwrite must be
	dc	i4'0'	; set up by InstallDriver
	dc	i4'D_mutex'
	dc	i4'D_demutex'
	dc	i4'D_out_enq'
	dc	i4'D_in_enq'
	dc	i4'D_out_deq'
	dc	i4'D_in_deq'
	dc	i4'D_size_inq'
	dc	i4'D_size_outq'
	ds	t_signalIO-editInd
	dc	i4'nullfunc'		; t_signalIO
	dc	i2'$FFFF'		; t_select_proc
	dc	i4'D_select'		; t_select
	dc	i4'0'			; t_selwakeup jmp set up during install
	END
D_in_enq	START
	rtl
	END
D_mutex	START
	rtl
	END
D_demutex	START
	rtl
	END

D_select	START
res	equ  0
	subroutine (2:ttyn,2:which,2:pid),2

	jsl	IncBusyFlag

	lda	#1
	sta	res
	lda	which	; which I/O to check?
	cmp	#SEL_READ
	bne	trywrite
	jsl	D_size_inq	; # bytes in in q
	cmp	#0
	bne	done

willwait	anop
* record that the process wants to do I/O
	lda	>dtty+t_select_proc	; see if someone's here already
	cmp	#$FFFF	; nope
	beq	nocollision
	cmp	pid		; is it us?
	beq	nocollision
	lda	>dtty+privFlags
	ora	#TS_RCOLL
	sta	>dtty+privFlags
	bra	none
nocollision	anop
	lda	pid	; set select_proc field to
	sta	>dtty+t_select_proc	; current process ID
	bra	none

trywrite	cmp	#SEL_WRITE
	bne	doexcept
	bra	done	; we can always write
;	jsl	D_left_outq	; # bytes avail in out q
;	cmp	#0
;	bne	done
;	bra	willwait

doexcept	anop	; there are no exceptions
none	lda	#0	; no data, return 0
	sta	res

done	jsl	DecBusyFlag
	return 2:res
	END

D_in_deq	START
	using	SerialData

	phb
	phk
	plb
	phd
	lda	>SerialDP
	tcd
	jsr	lowRead
	pld
	plb
	rtl
	END
D_out_deq	START
	rtl
	END

nullfunc	START
	lda	2,s
	sta	4,s
	lda	1,s
	sta	3,s
	pla
	rtl
	END

D_size_inq	START
	using	SerialData

	phd
	lda	>SerialDP
	tcd
	lda	<ibuf_mark
	pld
	rtl
	END

D_size_outq	START
	using	SerialData

	phd
	lda	>SerialDP
	tcd
	lda	<obuf_mark
	pld
	rtl
	END

;D_left_outq	START
;	using	SerialData
;
;	phd
;	lda	>SerialDP
;	tcd
;	lda	<obuf_size
;	sec
;	sbc	<obuf_mark
;	pld
;	rtl
;	END

* out_enq(int char)
D_out_enq	START
	using	SerialData

	phb
	phk
	plb
	phd
	lda	>SerialDP
	tcd
	lda	7,s
	jsr	WriteBuffer
	pld
	plb
	lda	2,s
	sta	4,s
	lda	1,s
	sta	3,s
	pla
	rtl
	END

lowRead	START

;	php		need to turn off interrupts to
;	sei		prevent the driver from hanging

WaitMore	anop
* Check for data in the buffer
	lda	<ibuf_mark	if a single char comes through
	bne	GotIt	while we're trying to sleep

* block process (will be unblocked by Read intr. completion routine)
	jsl	dtty+t_GetProcInd	; get pointer to process entry
	sta	procPtr
	stx	procPtr+2
;               php
;	sei
	ldy	#2                 ; set it to blocked state
	lda	#pBlocked
	sta	[procPtr],y
;	lda	tmpProcID  	; tell the intr handler which process
	lda	#1	; tell intr handler a process is asleep
	sta	blockProc	; to awaken
	ldy	#84	; index of waitDone field
	lda	#0
	sta	[procPtr],y	; set to zero, moron

	cop	$7F
;	plp
	ldy	#84
	lda	[procPtr],y
	beq	WaitMore	; thar's a character
	cmp	#1
	beq	WaitMore	
	lda	#-1
	sta	blockProc
	lda	#$7E43		; abort the transfer
	bra	intr

GotIt	anop
;	plp		restore interrupts
	lda	#-1
	sta	blockProc
	jsr	ReadBuffer
	
intr	anop
	rts
	END

DInit	START
	using	SerialData
hand	equ  0
ptr	equ  4
result	equ  12
bufHand	equ  14

	subroutine (2:devNum),18

	stz	result
*	ph4	#nhInit
*	_ErrWriteString
	ph4	#0
	ph4	#256
	ph2	dtty+t_userid
	ph2	#$C005
	ph4	#0
	_NewHandle
	cmp	#0
	beq	noerr

memErr	lda	#$54	; GS/OS out of memory
	sta	result
	jmp	goaway

noerr	pla
	sta	hand
	sta	DPHandle	
	pla
	sta	hand+2
	sta	DPHandle+2
	lda	[hand]
	sta	ptr
	sta	SerialDP
	ldy	#2
	lda	[hand],y
	sta	ptr+2

	pha
	pha
	ph4	#BUF_SIZE*2
	ph2	dtty+t_userid
	pea	$C008                  ; anywhere in memory
	pea	0	; no special location
	pea	0
	_NewHandle
	pla
	sta	bufHand
	sta	|bufHandle
	pla
	sta	bufHand+2
	sta	|bufHandle+2
	ora	bufHand
	bne	didAlloc

	ph4	>DPHandle
	_DisposeHandle
	bra	memErr

didAlloc	lda	[bufHand]
	tax
	ldy	#2
	lda	[bufHand],y
	sta	bufHand+2
	stx	bufHand

;	lda	#$20	; use CTS sighupping by default
	lda	#$00	; use no sighupping by default
	sta	>DHUP
	
	lda   SerialDP
	phd
	tcd
;              mv4   >dtermioPtr,dTermioPtr
	ld4	dtty,dTermioPtr
* Initialize default values for terminal settings
	ldy	#4

	lda	#CRMOD+ECHO
	sta	[dTermioPtr],y
	
	lda	#0
	short	m
* default baud rate either comes from the Control panel, or
* from a previous stty command. doing stty 0 on the port
* will force a reset to the control panel next time the port is opened

*	sta	[dTermioPtr]	; ispeed
*              ldy	#1
*	sta	[dTermioPtr],y     ; ospeed
	ldy	#t_intrc
	lda	#3
	sta	[dTermioPtr],y	; t_intrc
	ldy	#t_suspc
	lda	#'Z'-64
	sta	[dTermioPtr],y
	ldy	#t_quitc
	lda	#'\'-64
	sta	[dTermioPtr],y
	ldy	#t_startc
	lda	#'Q'-64
	sta	[dTermioPtr],y
	ldy	#t_stopc
	lda	#'S'-64
	sta	[dTermioPtr],y
	ldy	#t_eofc
	lda	#'D'-64
	sta	[dTermioPtr],y
	ldy	#t_brkc
	lda	#-1
	sta	[dTermioPtr],y
*	lda	#11
*	ldy	#t_ispeed
*	sta	[dTermioPtr],y
	long	m
* Initialize the port and parameters
	ldy	#privFlags
	lda	#0
	sta	[dTermioPtr],y
	ldy	#ws_row
	sta	[dTermioPtr],y
	iny2
	sta	[dTermioPtr],y
	iny2	
	sta	[dTermioPtr],y
	iny2	
	sta	[dTermioPtr],y
	jsr	ComInit	set the initial baud rate

	pld

	pea	0
	pei   (ptr)
	pei	(bufHand+2)
	pei	(bufHand)
	lda	bufHand
	clc
	adc	#BUF_SIZE
	tax
	lda	bufHand+2
	adc	#0
	pha
	phx
	jsl	StartSerial
	sta	result
	cmp	#0	error in startup?
	beq	goaway	Nope
	cmp	#7
	bcs	unknownErr
	asl	a	map SIM error to a reasonable
	tax		GS/OS driver error code
	lda	ErrorMap,x
	sta	result

	ph4	>DPHandle
	_DisposeHandle
	ph4	>bufHandle
	_DisposeHandle

goaway	return 2:result
unknownErr	sta  result	assume some other error
	bra	goaway
nhErr	str	'Modem:NewHandle failed'
nhInit	str	'Modem:Init'

ErrorMap	anop
	dc	i2'0'
	dc	i2'$29'	SIMAlreadyInst
	dc	i2'$27'	SIMInvalidAddr
	dc	i2'$29'	SIMATalkActive
	dc	i2'$27'	SIMNotInstalled
	dc	i2'$22'	SIMInvalidPort
	dc	i2'$26'	SIMNotFound

	END

DDeInit	START
	using	SerialData
	subroutine (2:devNum),0

*	ph4	#detxt
*	_ErrWriteString

	jsl	EndSerial

	ph4	>DPHandle
	_DisposeHandle
	ph4	>bufHandle
	_DisposeHandle

	return
detxt	str	'DeInit Modem'	
	END

DIOCTL	START
	using	SerialData
retval	equ  0
dpPtr	equ  2
	subroutine (4:tioc,4:dataPtr,2:devNum),6

	lda	SerialDP
	sta	dpPtr
	stz	dpPtr+2
	lda	tioc
	and	#$FF00
	xba
	cmp	#'f'
	beq	chkfile
	cmp	#'t'
	beq	tioctl
err	lda	#-1
	sta	retval
	bra	goaway
* FIONREAD
chkfile	lda	tioc
	and	#$7F
	cmp	#127
	bne	err

	ldy	#ibuf_mark
	lda	[dpPtr],y	how many bytes in input buffer?
	sta	[dataPtr]	lda is an atomic operation :-)

	stz   retval
	bra	goaway
tioctl	anop
	lda	tioc
	and	#$7F
	cmp	#23
	bcc	okay2	
	and	#$7F
	eor	#$7F
	cmp	#26
	bcc	okay1
	jmp	invalid
okay1	anop
	asl	a
	asl	a
	inc	a
	inc	a
	tax
	jmp	(tNTable,x)
okay2	anop
	asl	a
	asl	a
	inc	a
	inc	a
	tax
	jmp	(tPTable,x)
goaway	return 2:retval

tPTable	anop
	dc    i2'0',a2'TIOCGETD'
	dc    i2'1',a2'TIOCSETD'
	dc    i2'2',a2'invalid'
	dc    i2'3',a2'invalid'
	dc    i2'4',a2'invalid'
	dc    i2'-1',a2'invalid'
	dc    i2'-1',a2'invalid'
	dc    i2'-1',a2'invalid'
	dc    i2'8',a2'TIOCGETP'
	dc    i2'9',a2'TIOCSETP'
	dc    i2'10',a2'TIOCSETN'
	dc    i2'-1',a2'invalid'
	dc    i2'-1',a2'invalid'
	dc    i2'-1',a2'invalid'
	dc    i2'-1',a2'invalid'
	dc    i2'-1',a2'invalid'
	dc	i2'16',a2'TIOCFLUSH'
	dc    i2'-1',a2'invalid'
	dc    i2'-1',a2'invalid'
	dc    i2'-1',a2'invalid'
	dc    i2'-1',a2'invalid'
	dc	i2'21',a2'TIOCSHUP'
	dc	i2'22',a2'TIOCGHUP'

tNTable	anop
	dc    i2'127',a2'invalid'
	dc    i2'126',a2'invalid'
	dc    i2'125',a2'invalid'
	dc    i2'124',a2'invalid'

	dc    i2'123',a2'TIOCSBRK'
	dc    i2'122',a2'TIOCCBRK'
	dc    i2'121',a2'TIOCSDTR'
	dc    i2'120',a2'TIOCCDTR'

	dc    i2'119',a2'invalid'
	dc    i2'118',a2'invalid'
	dc    i2'117',a2'invalid'
	dc    i2'116',a2'invalid'
	dc    i2'115',a2'TIOCOUTQ'
	dc    i2'114',a2'TIOCSTI'
	dc    i2'113',a2'invalid'
	dc    i2'-1',a2'invalid'
	dc    i2'111',a2'TIOCSTOP'
	dc    i2'110',a2'TIOCSTART'

	dc    i2'109',a2'invalid'
	dc    i2'108',a2'invalid'
	dc    i2'107',a2'invalid'
	dc    i2'106',a2'invalid'
	dc    i2'-1',a2'invalid'
	dc    i2'104',a2'invalid'
	dc    i2'103',a2'invalid'

invalid	lda	#-1
	sta	retval
	jmp	goaway
*****************************************************
TIOCSHUP	anop
	lda	[dataPtr]
	sta	>DHUP
;	jsr	InstComplete
	stz	retval
	jmp	goaway
TIOCGHUP	anop
	lda	>DHUP
	sta	[dataPtr]
	stz	retval
	jmp	goaway

TIOCGETD	anop
TIOCSETD	anop
TIOCSETN	anop
TIOCGETP	stz	retval
	jmp	goaway

TIOCSETP	anop
	phd
	lda	>SerialDP
	tcd
	jsr	SetWord	$$$ SETWORD SET BAUD RATE
	pld
	stz	retval
	jmp	goaway

TIOCFLUSH	anop
	php
	sei
	lda	dataPtr
	ora	dataPtr+2
	bne	notNull
	ldx	#0
notNull	lda	[dataPtr]	get the type of operation
	tax
	cpx	#2
	beq	clrOutQ
	lda	#0	zero out the buffer information
	ldy	#ibuf_head
	sta	[dpPtr],y
	ldy	#ibuf_tail
	sta	[dpPtr],y
	ldy	#ibuf_mark
	sta	[dpPtr],y

clrOutQ	cpx	#1
	beq	doneFLUSH
	lda	#0	zero out the buffer information
	ldy	#obuf_head
	sta	[dpPtr],y
	ldy	#obuf_tail
	sta	[dpPtr],y
	ldy	#obuf_mark
	sta	[dpPtr],y

doneFLUSH	plp
	stz	retval
	jmp	goaway

TIOCOUTQ	ldy	#obuf_mark
	lda	[dpPtr],y
	sta	[dataPtr]
	stz	retval
	jmp	goaway

TIOCSTOP	anop	
TIOCSTART	anop
	lda	#-1
	sta	retval
	jmp	goaway	; we don't support these

TIOCSDTR	jsr   DTRON	; these don't need DP
	jmp   goaway

TIOCCDTR	jsr   DTROFF
	jmp   goaway

TIOCSBRK	jsr	BRKON
	stz	retval
	jmp	goaway
TIOCCBRK	jsr	BRKOFF
	stz	retval
	jmp	goaway

* simulate terminal input (this code is borrowed from INOUT.ASM/OurADB
TIOCSTI	anop	
; this chunk stores the byte in the buffer
	lda	[dataPtr]
	tax
	php
	sei
	phd
	lda	>SerialDP
	tcd
	ldy	<ibuf_head
	txa
	short	m
	sta	[in_buf],y
	long	m
	iny
	sty	<ibuf_head
	cpy	<ibuf_size
	bcc	stok
	ldy	#0
stok	sty	<ibuf_head
	inc	<ibuf_mark

; this chunk unblocks any process that was waiting on keyboard input

	lda	blockProc
	cmp	#-1
	beq	check4select

	ldy	#2
	lda	[procPtr],y
	cmp	#pBlocked	; is it still blocked?
	bne	done  	; nope, leave it alone
	lda	#pReady
	sta	[procPtr],y	; restart the process
; this was in the console code but not here
	lda	#-1
	sta	blockProc
;

; check for select here.

check4select	anop
	lda	>dtty+t_select_proc
	cmp	#$FFFF
	beq	done

* someone is selecting on us, so call selwakeup with the process ID
* and our collision flag
	pha

	lda	>dtty+privFlags
	and	#TS_RCOLL
	pha
	jsl	dtty+t_selwakeup
	lda	>dtty+privFlags
	and	#TS_RCOLL.EOR.$FFFF
	sta	>dtty+privFlags
	lda	#$FFFF
	sta	>dtty+t_select_proc

done	anop
	pld
	plp
	stz	retval
	jmp	goaway	
	END

checkIntr	START
	using	SerialData

	php
	long  ai
	and   #$7f
	pha
	short	m

	ldy	#sg_flags
	lda	[dTermioPtr],y
	bit	#$20	; RAW mode?
	beq	x9	; yep, no character checking
	brl	notty
x9	ldy	#t_quitc
	lda   [dTermioPtr],y
	cmp	#-1
	beq	x0
	cmp	1,s
	beq   gotQQ
x0	ldy	#t_suspc
	lda   [dTermioPtr],y
	cmp	#-1
	beq	x1
	cmp	1,s
	beq   gotZ
x1	ldy	#t_intrc
	lda   [dTermioPtr],y
	cmp	#-1
	beq	notty
	cmp	1,s
	beq   gotC
*x2	lda	>OutStopped
*	bne	x3
*	ldy	#t_stopc
*	lda   [dTermioPtr],y
*               cmp	#-1
*               beq	x3
*               cmp	1,s
*               beq   gotS
*x3	ldy	#t_startc
*	lda	[dTermioPtr],y
*               cmp	#-1
*               beq	notty
*               cmp   1,s
*               beq   gotQ
	bra   notty
*gotS           long	m
*	pla
*               lda   #1
*               sta   >OutStopped
*               plp
*               sec
*               rts
*gotQ           long	m
*	pla
*	lda   >OutStopped
*               beq   notQ
*               lda   #0
*               sta   >OutStopped
*               plp
*               sec
*               rts
*notQ	anop		;oops!
*               plp
*               clc
*               rts
gotQQ	long	m
	lda	#3
	bra	gotSIG
gotZ	long	m
	lda   #18
	bra   gotSIG
gotC	long	m
	lda   #2
gotSIG	anop
	phx
	phy
	pha
	
	lda 	<ibuf_head
	dec	a
	bpl	notNeg
	lda	<ibuf_size
	dec	a
notNeg	sta	<ibuf_head	; remove the character
	dec	<ibuf_mark

	lda	1,s
	pha		; push signal number
	ph2	>dtty+t_devNum	; push our device number
	jsl	dtty+t_sendSignal

	pla		; prolly don't need to, but what
	ply		; the hell...
	plx

	pla		; the character
	plp		; ready? Let's go!

	sec
	rts
notty	anop
	long	m
	pla
	plp
	clc
	rts
	END


SerialData	DATA
SerialDP	dc  i2'0'
DPHandle	dc  i4'0'
bufHandle	dc  i4'0'
DHUP	dc  i2'$00'
ourSerFlag	dc  i2'0'
CurBaud	dc  i2'0'

baudTbl	dc	i2'0'	0 - no baud! (hangup)
	dc	i2'$08FE'	1 - 50 baud
	dc	i2'$05FE'	2 - 75 baud
	dc	i2'$0415'	3 - 110 baud
	dc	i2'$035B'	4 - 134.5 baud
	dc	i2'$02FE'	5 - 150 baud
	dc	i2'$0000'	6 - 200 baud ($23E) or 57600
	dc	i2'$017E'	7 - 300 baud
	dc	i2'$00BE'	8 - 600 baud
	dc	i2'$005E'	9 - 1200 baud
	dc	i2'$003E'	10 - 1800 baud
	dc	i2'$002E'	11 - 2400 baud
	dc	i2'$0016'	12 - 4800 baud
	dc	i2'$000A'	13 - 9600 baud
	dc	i2'$0004'	14 - 19200 baud
	dc	i2'$0001'	15 - 38400 baud

* Firmware to UNIX baud-rate number conversion

RevBDTab	dc	i2'1'              50
	dc	i2'2'              75
	dc	i2'3'              110
	dc	i2'4'              134.5
	dc	i2'5'              150
	dc	i2'7'              300
	dc	i2'8'	600
	dc	i2'9'	1200
	dc	i2'10'	1800
	dc	i2'11'	2400
	dc	i2'0'	3600
	dc	i2'12'             4800
	dc	i2'0'	7200
	dc	i2'13'	9600
	dc	i2'14'	19200

	END
