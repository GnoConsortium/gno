*	$Id: sccf.asm,v 1.1 1998/02/02 08:18:04 taubert Exp $

	case	on

* Install an interrupt handler into the SIM tool.  This uses the SIM
* library so it's really easy

InstallInt	START

	ph4	#IntVect
	pea	PortNum
	jsl	INSTALLINTVECT
	rts

oldVect	ENTRY
	dc	i4'0'
	END

* Remove our SCC interrupt handler from the Main Interrupt Vector

RemoveInt	START
	ph4	#IntVect
	pea	PortNum
	jsl	REMOVEINTVECT
	rts
	END

IntVect	START
	using	SerialData

	longa	off
	longi	off

	php
	long	x
; JAWAID!  *whap*
; check_stuff needs RR0 contents, not RR3!
;	tay		we need this for check_stuff
	bit	#Channel_Rx
	beq	t_Tx_Int

	phb
	phk
	plb
	phd
	long	m
	lda	SerialDP
	tcd
	short	m
; *whap*
;	sty	<lastInt	store the int value for check_stuff
lp1	ldy	<ibuf_head
	lda	#1
	sta	>PortControl
	lda	>PortControl
	pha

	lda	>PortData
	sta	<lastChar	; for post-processing
	sta	[in_buf],y
	iny
	cpy	<ibuf_size
	bne	noWrap1
	ldy	#0
noWrap1	sty	<ibuf_head
	ldx	<ibuf_mark
	inx
	stx	<ibuf_mark
	cpx	#3072
	bcc	noFCON

	jsr	turn_flow_off	hold your horses!

noFCON	jsr	check_stuff	; reload Y from dp at lp1

	pla
	and	#$60
	beq	weepers
	lda	#0
	sta	>PortControl	; error reset
	lda	#$30
	sta	>PortControl
	
weepers	lda	#0	; Reset high IUS
	sta	>PortControl
	lda	#$38
	sta	>PortControl

	lda	>PortControl
	bit	#1
	bne	lp1	; more data!
	pld
	plb
gohome	anop
	plp
	clc
	rtl

notOurIntr	anop
	plp
	sec
	rtl

t_Tx_Int	anop
	bit	#Channel_Tx
	beq   statusInt
	lda	<obuf_fcon
	bne	turnOffTxInt
;	phy
	phd
	long	m
	lda	>SerialDP
	tcd
	short	m
	phb
	phk
	plb
doTxChar	ldy	<obuf_mark
	beq	turnOffTxInt
	tya
	ldy	<obuf_tail
	lda	[out_buf],y
	sta	>PortData
	iny
	cpy	<obuf_size
	bne	noWrap
	ldy	#0
noWrap	sty	<obuf_tail
	ldx	<obuf_mark
	dex
	stx	<obuf_mark

	lda	#0	; Reset high IUS
	sta	>PortControl
	lda	#$38
	sta	>PortControl

	plb
	pld
;               ply
	bra   gohome
turnOffTxInt	lda   #%00101000
	sta	>PortControl
	lda	#0
	sta	<tx_in_progress
	plb
	pld
;	ply
	bra	gohome

* Must be a status interrupt
statusInt	anop
;	phy
	phd
	long	m
	lda	>SerialDP
	tcd
	short	m
	phb
	phk
	plb
	lda	>PortControl
	bit	#%00100000	; check status of CTS line
	bne	turniton

	lda	#1
	sta	<obuf_fcon
reset	lda	#0	; Reset high IUS
	sta	>PortControl
	lda	#$38
	sta	>PortControl

	plb
	pld
;               ply
	jmp   gohome
turniton	lda	<obuf_fcon
	beq	reset
	stz	<obuf_fcon
	jmp	doTxChar
	END

	longa	off
	longi	on

check_stuff	START
	using	SerialData

	lda	>PortControl
; *whap*
;	lda	<lastInt
	bit	DHUP
	bne	sighupit

	lda	<lastChar
	jsr	checkIntr
	bcs	goaway	; send a signal!

	lda	blockProc
	cmp	#-1
	beq	checkselect

	ldy	#2
	lda	[procPtr],y
	cmp	#pBlocked	; is it still blocked?
	bne	checkselect	; nope, leave it alone
	lda	#pReady
	sta	[procPtr],y	; restart the process

; check for select here.

checkselect	anop
	long	m
	lda	>dtty+t_select_proc
	cmp	#$FFFF
	beq	done

* someone is selecting on us, so call selwakeup with the process ID
* and our collision flag
	pha

	lda	>dtty+privFlags
	and	#TS_RCOLL
	pha
	jsl	>dtty+t_selwakeup
	lda	>dtty+privFlags
	and	#TS_RCOLL.EOR.$FFFF
	sta	>dtty+privFlags
	lda	#$FFFF
	sta	>dtty+t_select_proc

done	anop
	short	m
goaway	anop
	rts
* we lost carrier detect, so send a sighup signal
sighupit	anop
	long	m
	lda	#$FFFF
	sta	>$E00408
	pea	1	; push signal number
	lda	>dtty+t_devNum
	pha		; push our device number
	jsl	dtty+t_sendSignal		
*	short	m	; see done, above
	bra	done
	END

	longa	on
	longi	on

* ReadBuffer and WriteBuffer expect the D register to be set properly
* (i.e., to SerialDP)

ReadBuffer	START
	using	SerialData

	php
	short	m
	sei
	ldy	<ibuf_tail
	cpy   <ibuf_head
	beq	nodata
	lda	[in_buf],y
	iny
	cpy	<ibuf_size
	bcc	noWrap
	ldy	#0
noWrap	sty	<ibuf_tail
	long	m
	ldx	<ibuf_mark
	dex
	stx	<ibuf_mark
	pha
	lda	<fcon_status
	bne	fc_is_off
	cpx	#1024
	bcs	fc_is_off
	short	m
	jsr	turn_flow_on
	long	m
fc_is_off	pla
	and	#$FF
	plp
	sec
	rts
nodata	long	m
	plp
	clc
	rts
	END

WriteBuffer	START
	using	SerialData

	tay		; save character in Y
tryagain	php
	short	m
	sei

	ldx	<obuf_mark
	beq	bufEmpty
	cpx	#BUF_SIZE-1
	beq	noRoom

addToBuffer	tya		; put character back in A
	ldy	<obuf_head
	sta	[out_buf],y
	iny
	cpy	<obuf_size
	bcc	noWrap
	ldy	#0
noWrap	sty	<obuf_head
	long	m
	inx
	stx	<obuf_mark
	bra	wasData
	longa	off
bufEmpty	anop
;               lda	>PortControl
;	lda	>PortControl
;	bit	#Tx_empty          see if something is in the Tx latch
;	beq	addToBuffer	and if so stick char in the buffer
	lda	<tx_in_progress
	bne	addToBuffer
	tya
	sta	>PortData
	lda	#1
	sta	<tx_in_progress

wasData	plp
	sec
	rts
noRoom	long	m
	plp
	bra	tryagain
	plp
	clc
	rts
	END

WriteByteOld	START
	subroutine (2:ch),0

	lda	ch
	short	m
	pha

flowCheck	lda   >PortControl       ; check the CTS line to make sure
	bit	#%00100000         ; the printer can accept data...
	beq	flowCheck          ; if not, then wait for it

txfull	lda	>PortControl       ; make sure the transmit buffer
	bit	#%00000100         ; is empty
	beq	txfull

	pla
	sta	>PortData          ; write our data!
	long	m

	return
	END

InitSCC	START

	php
	sei
	short	m

	lda	>PortControl
	lda	#$09
	sta	>PortControl
	
	lda	#%00000010+PortReset	channel reset, turn intrs off
	sta	>PortControl
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	
	ldx	#0
a_ian_loop	lda   async_data,x
	cmp	#$FF
	beq	a_ian_leap
	sta	>PortControl

* What's the timer for?
	
	inx
	bra	a_ian_loop

a_ian_leap	anop
	long	m
	plp
	rtl

async_data	dc  i1'$04,$44'	X16 clock mode, 1 stop bit
	dc	i1'$02,$00'	no interrupt vector
	dc	i1'$03,%11100000'	rx 8 bits/char, rx off, auto enables
	dc	i1'$05,$62'	DTR = 0, tx 8 bits/char, tx off, RTS = 1
	dc	i1'$09,%0010'	no reset, all ints off
	dc	i1'$0A,$00'	NRZ data encoding
	dc	i1'$0B,Reg11'	xtal on, rx clock = brg out, tx clock = brg_out
initBaudLo	ENTRY
	dc	i1'$0C,$04'	low byte of brg time constant
initBaudHi	ENTRY
	dc	i1'$0D,$00'	high byte of brg time constant
	dc	i1'$0E,$80'	source = brg, brg clock = xtal, brg off
	dc	i1'$0E,$01'	brg on, brg clock = xtal
	dc	i1'$03,%11100001'	rx 8 bits/char, rx on
	dc	i1'$05,DTR'	DTR = 0, tx 8 bits/char, tx on, RTS = 1
	dc	i1'$0F,%00101000'	interrupt on CTS status change
*	dc	i1'$01,%00010001'	rx char ints enable
*	dc	i1'$09,%00001010'	master interrupt enable
	dc	i1'$FF'
	END

InitSCC2	START
	using	SerialData

	php
	sei
	short	m

	lda	>PortControl
	lda	#1
	sta	>PortControl
	lda	#%00010010	; intr on all rx and tx
	sta	>PortControl
	lda	#9
	sta	>PortControl
	lda	#%1010
	sta	>PortControl

; hum, just for kicks...
	lda	#0
	sta	>PortControl	; error reset
	lda	#$30
	sta	>PortControl
	lda	#0	; Reset high IUS
	sta	>PortControl
	lda	#$38
	sta	>PortControl

	plp
	rts
	END

	longa	on
	longi	on
DeInitSCC	START
	using	SerialData

* Turn off the SCC's interrupt generation completely
	php
	sei
	short	m
	lda	>PortControl
	lda	#9
	sta	>PortControl
	lda	#%0010
	sta	>PortControl

	lda	#1
	sta	>PortControl
	lda	#0
	sta	>PortControl

	lda	>PortData
	lda	#%11010000
	sta	>PortControl
	nop
	nop
	nop
	nop
	nop
	lda	#%00101000
	sta	>PortControl
	nop
	nop
	nop
	nop
	nop
	lda	#9
	sta	>PortControl
	lda	#%1010
	sta	>PortControl

;	lda	>SerFlag
;	and	#%11111000
;	sta	>SerFlag
;	lda	>ourSerFlag
;	and	#%11111000
;	sta	>ourSerFlag

	long	m
	plp
	rts
	END

StartSerial	START
	using	SerialData
result	equ	0
	subroutine (4:dp,4:inb,4:outb),2

	stz	result
	lda	inb
	ldy	#in_buf
	sta	[dp],y
	lda	inb+2
	ldy	#in_buf+2
	sta	[dp],y
	lda	outb
	ldy	#out_buf
	sta	[dp],y
	lda	outb+2
	ldy	#out_buf+2
	sta	[dp],y

	phd

	lda	dp
	sta	>SerialDP
	tcd		; 'return' will restore this

	lda	#0
	sta	<ibuf_mark
	sta	<obuf_mark
	sta	<ibuf_head
	sta	<obuf_head
	sta	<ibuf_tail
	sta	<obuf_tail
	sta	<obuf_fcon
	sta	<tx_in_progress
	sta	<lastInt
	sta	<lastChar
	lda	#4096
	sta	<ibuf_size
	sta	<obuf_size

	lda	#1
	sta	<fcon_status
	lda	#FCON_RTS
	sta	<fcon_type

	lda	#DTR
	sta	<reg5Status

	jsl	InitSCC
	jsr	InstallInt
	sta	|temp	tells if there was an error
	cmp	#0
	bne	handlerErr
	jsr	InitSCC2	only start intrs if handler was
handlerErr	pld		properly installed
	lda	temp
	sta	result
	return 2:result
temp	dc	i2'0'
	END

FlushOutQ	START
	using	SerialData

	phd
	lda	>SerialDP
	tcd
lp	lda	<obuf_mark
	bne	lp
	pld
	rts
	END

EndSerial	START
	subroutine (0:foo),0
	jsr	FlushOutQ
	jsr	DeInitSCC	; turn off interrupts, etc.
	jsr	RemoveInt
	return
	END

* usually called at interrupt time, requires that D point to the driver's
* direct page and expects short memory and long index
	longa	off
	longi	on
turn_flow_off	START
	lda	fcon_status
	beq   alreadyOff	flow is already off
	lda	fcon_type
	beq	noFlowControl	they don't want flowcontrol
	cmp	#FCON_XON	xon/xoff?
	bne	doRTS
	lda	#'S'-64
	jsr	priorityWrite
	stz	fcon_status
noFlowControl	anop
alreadyOff	rts
doRTS	lda	#5
	sta   >PortControl
	lda	#DTR+$80
	sta	>PortControl
	sta	<reg5Status
	stz	fcon_status
	rts
	END

turn_flow_on	START
	lda	fcon_status
	bne   alreadyOn	flow is already on
	lda	fcon_type
	beq	noFlowControl	they don't want flowcontrol
	cmp	#FCON_XON	xon/xoff?
	bne	doRTS
	lda	#'Q'-64
	jsr	priorityWrite
	lda	#1
	sta	fcon_status
noFlowControl	anop
alreadyOn	rts
doRTS	lda	#5
	sta   >PortControl
	lda	#DTR
	sta	>PortControl
	sta	<reg5Status
	lda	#1
	sta	fcon_status
	rts
	END

	longa	on
	longi	on
DTRON	START
	using	SerialData

	phd
	lda	>SerialDP
	tcd
	php
	short	m
	sei
	lda	#5
	sta	>PortControl
	lda	#DTR
	sta	>PortControl
	sta	<reg5Status
	plp
	long	m
	pld
	rts
	END

DTROFF	START
	using	SerialData

	phd
	lda	>SerialDP
	tcd
	php
	sei
	short	m
	lda	#5
	sta	>PortControl
	lda	#DTR+$80
	sta	>PortControl
	sta	<reg5Status
	plp
	long	m
	pld
	rts
	END

BRKON	START
	rts		we really need to do something here :)
	END

BRKOFF	START
	rts
	END

priorityWrite	START
	brk	$10
	rts
	END

	longa	on
	longi	on
ComInit	start
	using	SerialData

	ldy	#sg_ispeed
	lda	[dTermioPtr],y
	and	#$00FF
	bne	dontCP

	pha
	ph2	#CtlPanBaud
	_ReadBParam
	pla
	asl	a
	tax
	lda	RevBDTab,x
dontCP	ldy	#sg_ispeed
	short	m
	sta	[dTermioPtr],y
	ldy	#sg_ospeed
	sta	[dTermioPtr],y
	long	m

	and	#$FF
	asl	a
	tax
	short	m
	lda	baudTbl,x	set the initial baud rate
	sta	|initBaudLo+1	(in the register setup table)
	xba
	sta	|initBaudHi+1
	long	m

;               jsr   SetWord	we only support 8N1 right now!
;	jsr	InstComplete	install completion routine! ack!
	rts
	END

* Set the baud rate, and parity for a connection
SetWord	START
	using	SerialData

	php
	sei
	ldy	#sg_ispeed
	lda	[dTermioPtr],y
	and	#$0F
	asl	a
	tax

	lda	>baudTbl,x
	sta	CurBaud

* reset the baud rate in the SCC

	tax
	short	m
	lda	>PortControl
	lda   #$0C
	sta   >PortControl
	txa
	sta   >PortControl
	lda   #$0D
	sta   >PortControl
	txa
	xba
	sta   >PortControl
	long	m

realFIN	plp
	rts
	end
