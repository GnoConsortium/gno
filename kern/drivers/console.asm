*	$Id: console.asm,v 1.1 1998/02/02 08:17:54 taubert Exp $
************************************************************
*
* CONSOLE device driver
*
************************************************************

	case	on
	mcopy	../drivers/port.mac
	copy	../gno/inc/tty.inc
	copy	../drivers/console.equates
	copy	../drivers/kern.equates

**************************************************************************
consoleSem	gequ  $3C                ; output code semaphore

ConsoleHeader	START
contty	ENTRY
	ds	t_open
* Line Discipline entry points
	dc	i4'ConInit'
	dc	i4'ConDeInit'
	dc	i4'ConIOCTL'
	dc	i4'0'	; ttread and ttwrite must be
	dc	i4'0'	; set up by InstallDriver
	dc	i4'CON_mutex'
	dc	i4'CON_demutex'
	dc	i4'CON_out_enq'
	dc	i4'CON_in_enq'
	dc	i4'CON_out_deq'
	dc	i4'CON_in_deq'
	dc	i4'CON_size_inq'
	dc	i4'CON_size_outq'
	ds	t_signalIO-editInd
	dc	i4'CON_signalIO'
	dc	i2'$FFFF'		; noone selecting
	dc	i4'CON_select'
	dc	i4'0'			; t_selwakeup set during install
	END

CON_mutex	START KERN2
CON_demutex	ENTRY
CON_in_enq	ENTRY
CON_out_deq	ENTRY
	rtl
	END

CON_in_deq	START KERN2
;	using	InOutData
;	phd
;	lda	>InOutDP	
;	tcd
	jsl	KEYIN
;               pld
	rtl
	END

CON_select	START KERN2
res	equ  0
	subroutine (2:ttyn,2:which,2:pid),2

	lda	#1
	sta	res
	lda	which	; which I/O to check?
	cmp	#SEL_READ
	bne	trywrite
	jsl	CON_size_inq	; # bytes in in q
	cmp	#0
	bne	done

willwait	anop
* record that the process wants to do I/O
	lda	>contty+t_select_proc	; see if someone's here already
	cmp	#$FFFF	; nope
	beq	nocollision
	cmp	pid		; is it us?
	beq	nocollision
	lda	>contty+privFlags
	ora	#TS_RCOLL
	sta	>contty+privFlags
	bra	none
nocollision	anop
	lda	pid	; set select_proc field to
	sta	>contty+t_select_proc	; current process ID
	bra	none

trywrite	cmp	#SEL_WRITE
	bne	doexcept
	bra	done	; we can always write to console

doexcept	anop	; there are no exceptions on console
none	lda	#0	; no data, return 0
	sta	res
done	anop

	return 2:res
	END

CON_size_inq	START KERN2
	using	ADBData

	php
	sei
	short	m
	lda	>head
	sec
	sbc	>tail
	long	m
	and	#$00FF
	plp
	rtl
	END

CON_size_outq	START KERN2
	lda	#0
	rtl
	END
CON_out_enq	START KERN2
;	using	InOutData
;	phd
;	lda	>InOutDP
;	tcd
;	lda	6,s
	lda	4,s
	jsl	COUT
;	pld
	lda	2,s
	sta	4,s
	lda	1,s
	sta	3,s
	pla
	rtl
	END

* This function does nothing in the console driver
CON_signalIO	START KERN2
	lda	2,s
	sta	4,s
	lda	1,s
	sta	3,s
	pla
	rtl
	END

ConInit	START
dTermioPtr	equ  0
result	equ  4
	subroutine (2:devNum),6

	ld4	contty,dTermioPtr

	stz	result

* Initialize default values for terminal settings
	ldy	#sg_flags
	lda	#CRMOD+ECHO
	sta	[dTermioPtr],y
	short	m
	lda	#0
	sta	[dTermioPtr]	; ispeed
	ldy	#sg_ospeed
	sta	[dTermioPtr],y     ; ospeed
	ldy	#t_intrc
	lda	#'C'-64
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
	ldy	#t_dsuspc
	lda	#'Y'-64
	sta	[dTermioPtr],y
	ldy	#t_rprntc
	lda	#'R'-64
	sta	[dTermioPtr],y
	ldy	#sg_erase
	lda	#$7F
	sta	[dTermioPtr],y
	long	m

	ldy	#local
	lda	#LCRTERA+LCTLECH
	sta	[dTermioPtr],y

	ldy	#ws_row
	lda	#24
	sta	[dTermioPtr],y
	ldy	#ws_col
	lda	#80
	sta	[dTermioPtr],y
	ldy	#ws_xpixel
	lda	#0
	sta	[dTermioPtr],y
	ldy	#ws_ypixel
	sta	[dTermioPtr],y

	return 2:result
	END

ConDeInit	START
	subroutine (2:devNum),0

	return
	END

* Character translation & such is handled here, to keep it out
* of TextTool's silly assumptions. Fortunately, pipes donna have this
* prollem.

ConIOCTL	START KERN2
	using	ADBData
	using	InOutData
retval	equ   0
	subroutine (4:tioc,4:dataPtr,2:devNum),2

	lda	tioc
	and	#$FF00
	xba
	cmp	#'f'
	beq	chkfile
	cmp	#'t'
	beq	tioctl
err	lda	#-1
	sta	retval
	jmp	goaway
chkfile	lda	tioc
	and	#$7F
	cmp	#127
	bne	err
	php
	sei
	short	m
	lda	>head
	sec
	sbc	>tail
	long	m
	and	#$00FF
*	bpl	okey
*	eor	#$FFFF
*	inc	a
*okey	anop
	sta	[dataPtr]	
	plp
	stz   retval
	jmp	goaway
tioctl	anop
	lda	tioc
	and	#$7F
	cmp   #25
	bcc   okay2
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
	dc    i2'-1',a2'invalid'
	dc    i2'-1',a2'invalid'
	dc    i2'-1',a2'invalid'
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
	dc    i2'16',a2'TIOCFLUSH'
	dc    i2'-1',a2'invalid'
	dc    i2'-1',a2'invalid'
	dc	i2'19',a2'TIOCSETK'
	dc	i2'20',a2'TIOCGETK'
	dc	i2'-1',a2'invalid'
	dc	i2'-1',a2'invalid'
	dc	i2'23',a2'TIOCSVECT'
	dc	i2'24',a2'TIOCGVECT'

tNTable	anop
	dc    i2'127',a2'invalid'
	dc    i2'126',a2'invalid'
	dc    i2'125',a2'invalid'
	dc    i2'124',a2'invalid'

	dc    i2'123',a2'invalid'
	dc    i2'122',a2'invalid'
	dc    i2'121',a2'invalid'
	dc    i2'120',a2'invalid'

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

	dc    i2'0',a2'TIOCGETD'
	dc    i2'1',a2'TIOCSETD'
	dc    i2'-1',a2'invalid'
	dc    i2'-1',a2'invalid'
	dc    i2'-1',a2'invalid'
	dc    i2'-1',a2'invalid'
	dc    i2'-1',a2'invalid'
	dc    i2'-1',a2'invalid'
	dc    i2'8',a2'TIOCGETP'
	dc    i2'9',a2'TIOCSETP'
	dc    i2'10',a2'TIOCSETN'

TIOCGETD	anop
TIOCSETD	anop
TIOCSETN	anop
TIOCSETP	anop
TIOCGETP	stz	retval
	jmp	goaway

TIOCGVECT	anop
	php
	sei
	lda	>COUT+1
	sta	[dataPtr]
	ldy	#2
	lda	>COUT+3
	and	#$00FF
	sta	[dataPtr],y

	ldy	#4
	lda	>KEYIN+1
	sta	[dataPtr],y
	ldy	#6
	lda	>KEYIN+3
	and	#$00FF
	sta	[dataPtr],y
	plp
	stz	retval
	jmp	goaway

TIOCSVECT	anop
	php
	sei
	lda	[dataPtr]
	sta	>COUT+1
	ldy	#1
	lda	[dataPtr],y
	sta	>COUT+2

	ldy	#4
	lda	[dataPtr],y
	sta	>KEYIN+1
	iny
	lda	[dataPtr],y
	sta	>KEYIN+2
	plp
	stz	retval
	jmp	goaway

TIOCOUTQ	lda	#0
	sta	[dataPtr]
	stz	retval
	jmp	goaway
TIOCSTOP	lda	>OutStopped	
	bne	isStopped
	lda	#1
	sta	>OutStopped
isStopped	stz   retval
	jmp	goaway
TIOCSTART	lda   >OutStopped
	beq	isStopped
	lda	#0
	sta	>OutStopped
	bra	isStopped

TIOCFLUSH	php
	sei
	short	m
	lda	>tail
	sta	>head
	long	m
	plp
	stz	retval
	jmp	goaway

* simulate terminal input (this code is borrowed from INOUT.ASM/OurADB
TIOCSTI	anop	
	php
	sei
	phd
	lda	>InOutDP
	tcd
	short	m
	lda   >head
	tax
	lda	[dataPtr]
	sta   >keybuf,x
	ldy	#1
	lda   [dataPtr],y
	sta   >modbuf,x
	inx
	txa
	sta   >head

; this chunk unblocks any process that was waiting on keyboard input

	long 	ai
	lda   >blockCP
	cmp   #$FFFF
	beq   check4select

	ldy	#2
	lda	[IODP_procPtr],y
	cmp	#pBlocked	; is it still blocked?
	bne	done  	; nope, leave it alone
	lda	#pReady
	sta	[IODP_procPtr],y	; restart the process

	lda   #$FFFF
	sta   >blockCP

; check for select here.

check4select	lda   >contty+t_select_proc
	cmp	#$FFFF
	beq	done

* someone is selecting on us, so call selwakeup with the process ID
* and our collision flag
	pha

	lda	>contty+privFlags
	and	#TS_RCOLL
	pha
	jsl	contty+t_selwakeup
	lda	>contty+privFlags
	and	#TS_RCOLL.EOR.$FFFF
	sta	>contty+privFlags
	lda	#$FFFF
	sta	>contty+t_select_proc

done	anop
	pld
	plp
	stz	retval
	jmp	goaway	

TIOCSETK	anop
	lda	[dataPtr]
	sta	>keyMaps
	stz	retval
	jmp	goaway
TIOCGETK	anop
	lda	>keyMaps
	sta	[dataPtr]
	stz	retval
	jmp	goaway
	END

