*	$Id: pty.asm,v 1.1 1998/02/02 08:19:40 taubert Exp $
****************************************************************
*
*  Pseudo-TTY device drivers and line discipline
*
*  Assumes 32 PTYs right now
*
****************************************************************

	case	on
	mcopy	m/pty.mac
	copy	inc/tty.inc

DebugNames	gequ  1
ptyBufSize	gequ  4096	!! MUST BE A POWER OF TWO !!
ptyDevStart	gequ  6

IncBusy	gequ  $E10064
DecBusy	gequ  $E10068

ptyDPindex	START KERN2
	dc	i2'ptyRecSize*0'
	dc	i2'ptyRecSize*0'
	dc	i2'ptyRecSize*1'
	dc	i2'ptyRecSize*1'
	dc	i2'ptyRecSize*2'
	dc	i2'ptyRecSize*2'
	dc	i2'ptyRecSize*3'
	dc	i2'ptyRecSize*3'
	dc	i2'ptyRecSize*4'
	dc	i2'ptyRecSize*4'
	dc	i2'ptyRecSize*5'
	dc	i2'ptyRecSize*5'
	dc	i2'ptyRecSize*6'
	dc	i2'ptyRecSize*6'
	dc	i2'ptyRecSize*7'
	dc	i2'ptyRecSize*7'
	dc	i2'ptyRecSize*8'
	dc	i2'ptyRecSize*8'
	dc	i2'ptyRecSize*9'
	dc	i2'ptyRecSize*9'
	dc	i2'ptyRecSize*10'
	dc	i2'ptyRecSize*10'
	dc	i2'ptyRecSize*11'
	dc	i2'ptyRecSize*11'
	dc	i2'ptyRecSize*12'
	dc	i2'ptyRecSize*12'
	dc	i2'ptyRecSize*13'
	dc	i2'ptyRecSize*14'
	dc	i2'ptyRecSize*14'
	dc	i2'ptyRecSize*15'
	dc	i2'ptyRecSize*15'
	dc	i2'ptyRecSize*16'
	dc	i2'ptyRecSize*16'
	dc	i2'ptyRecSize*17'
	dc	i2'ptyRecSize*17'
	dc	i2'ptyRecSize*18'
	dc	i2'ptyRecSize*18'
	dc	i2'ptyRecSize*19'
	dc	i2'ptyRecSize*19'
	dc	i2'ptyRecSize*20'
	dc	i2'ptyRecSize*20'
	dc	i2'ptyRecSize*21'
	dc	i2'ptyRecSize*21'
	dc	i2'ptyRecSize*22'
	dc	i2'ptyRecSize*22'
	dc	i2'ptyRecSize*23'
	dc	i2'ptyRecSize*23'
	dc	i2'ptyRecSize*24'
	dc	i2'ptyRecSize*24'
	dc	i2'ptyRecSize*25'
	dc	i2'ptyRecSize*25'
	dc	i2'ptyRecSize*26'
	dc	i2'ptyRecSize*26'
	dc	i2'ptyRecSize*27'
	dc	i2'ptyRecSize*27'
	dc	i2'ptyRecSize*28'
	dc	i2'ptyRecSize*28'
	dc	i2'ptyRecSize*29'
	dc	i2'ptyRecSize*29'
	dc	i2'ptyRecSize*30'
	dc	i2'ptyRecSize*30'
	dc	i2'ptyRecSize*31'
	dc	i2'ptyRecSize*31'
	END

PTY_signalIO	START KERN2
	txa		; put devNum in acc
	txy
	sec
	sbc	#ptyDevStart
	asl	a                  ; calculate index into table
	tax
	lda	>ptyDPindex,x
	clc	                   ; clc$$
	adc	>PTYDP
	phd
	tcd
	tya
	eor	6,s
	bit	#1
	bne	Bcheck4select

; check for select here.

Acheck4select	anop
	lda	>PTYSlaveHeader+t_select_proc
	cmp	#$FFFF
	beq	Adone

* someone is selecting on us, so call selwakeup with the process ID
* and our collision flag
	pha

	lda	>PTYSlaveHeader+privFlags
	and	#TS_RCOLL
	pha
	jsl	>PTYSlaveHeader+t_selwakeup
	lda	>PTYSlaveHeader+privFlags
	and	#TS_RCOLL.EOR.$FFFF
	sta	>PTYSlaveHeader+privFlags
	lda	#$FFFF
	sta	>PTYSlaveHeader+t_select_proc

Adone	anop
	pei	(p_bufAptr+2)
	pei	(p_bufAptr)
	jsl	k_wakeup
	bra	byebye

; check for select here.

Bcheck4select	anop
	lda	>PTYMastHeader+t_select_proc
	cmp	#$FFFF
	beq	Bdone

* someone is selecting on us, so call selwakeup with the process ID
* and our collision flag
	pha

	lda	>PTYMastHeader+privFlags
	and	#TS_RCOLL
	pha
	jsl	>PTYMastHeader+t_selwakeup
	lda	>PTYMastHeader+privFlags
	and	#TS_RCOLL.EOR.$FFFF
	sta	>PTYMastHeader+privFlags
	lda	#$FFFF
	sta	>PTYMastHeader+t_select_proc

Bdone	anop
	pei	(p_bufBptr+2)
	pei	(p_bufBptr)
	jsl	k_wakeup

byebye	anop
	pld
	lda	2,s
	sta	4,s
	lda	1,s
	sta	3,s
	pla
	rtl
	END

A_sizeq	START KERN2
_A_sizeq	name

	txa		; put devNum in acc
	sec
	sbc	#ptyDevStart
	asl	a                  ; calculate index into table
	tax
	lda	>ptyDPindex,x
	clc	                   ; clc$$
	adc	>PTYDP
	phd
	tcd
	lda	#4095
	sec
	sbc	p_Aleft
	pld
	rtl
	END

A_leftq	START KERN2
_A_leftq	name

	txa		; put devNum in acc
	sec
	sbc	#ptyDevStart
	asl	a                  ; calculate index into table
	tax
	lda	>ptyDPindex,x
	clc	                   ; clc$$
	adc	>PTYDP
	phd
	tcd
	lda	p_Aleft
	pld
	rtl
	END

B_sizeq	START KERN2
_B_sizeq	name

	txa
	sec
	sbc	#ptyDevStart
	asl	a	; calculate index into table
	tax
	lda	>ptyDPindex,x
	clc		; clc$$
	adc	>PTYDP
	phd
	tcd
	lda	#4095
	sec
	sbc	p_Bleft
	pld
	rtl
	END

B_leftq	START KERN2
_B_leftq	name

	txa
	sec
	sbc	#ptyDevStart
	asl	a	; calculate index into table
	tax
	lda	>ptyDPindex,x
	clc		; clc$$
	adc	>PTYDP
	phd
	tcd
	lda	p_Bleft
	pld
	rtl
	END

pty_mutex	START KERN2
_pty_mutex	name

	txa
	sec
	sbc	#ptyDevStart
	asl   a                  ; calculate index into table
	tax
	lda	>ptyDPindex,x
	clc	                   ; clc$$
	adc	>PTYDP
	phd
	tcd
	pei	(p_sem)
	jsl	>IncBusy
	jsl	asmWait
	pld
	rtl
	END

pty_demutex	START KERN2
_pty_demutex	name

	txa
	sec
	sbc	#ptyDevStart
	asl   a                  ; calculate index into table
	tax
	lda	>ptyDPindex,x
	clc	                   ; clc$$
	adc	>PTYDP
	phd
	tcd
	pei	(p_sem)
	jsl	asmSignal
	jsl	>DecBusy
	pld
	rtl
	END
*
*	ENQUEUE/DEQUEUE routines
*

A_deq	START KERN2
	using	KernelStruct
_a_deq	name

	txa
	sec
	sbc	#ptyDevStart
	asl   a                  ; calculate index into table
	tax
	lda	>ptyDPindex,x
	clc	                   ; clc$$
	adc	>PTYDP
	phd
	tcd

again	ldx	p_Aleft
	cpx	#4095
	beq	dowait
	ldy	p_Atail
	lda	[p_bufAptr],y	
	and	#$00FF
	pha
	tya
	inc	a
	and	#ptyBufSize-1	; wrap it around
	sta	p_Atail
	inx
	stx	p_Aleft
	pla
	pld
	rtl
dowait	anop
* free access semaphore
	pei	(p_sem)
	jsl	asmSignal

* wait on vector of buffer A; the line discipline code calls a low level
* signal_operation(read|write) to indicate what happened.  That way we
* don't vec_signal each and every time a character goes out.
	lda	>truepid
	pha
	pea	0
	pei	(p_bufAptr+2)
	pei	(p_bufAptr)
	jsl	k_sleep

	pei	(p_sem)
	jsl	asmWait
	bra	again
	END

B_deq	START KERN2
	using	KernelStruct
_b_deq	name

	txa
	sec
	sbc	#ptyDevStart
	asl   a                  ; calculate index into table
	tax
	lda	>ptyDPindex,x
	clc	                   ; clc$$
	adc	>PTYDP
	phd
	tcd

again	ldx	p_Bleft
	cpx	#4095
	beq	dowait
	ldy	p_Btail
	lda	[p_bufBptr],y	
	and	#$00FF
	pha
	tya
	inc	a
	and	#ptyBufSize-1	; wrap it around
	sta	p_Btail
	inx
	stx	p_Bleft
	pla
	pld
	rtl
dowait	anop
	pei	(p_sem)
	jsl	asmSignal
	lda	>truepid
	pha
	pea	0
	pei	(p_bufBptr+2)
	pei	(p_bufBptr)
	jsl	k_sleep
	pei	(p_sem)
	jsl	asmWait
	bra	again
	END

A_enq	START KERN2
	using	KernelStruct
char	equ  6
_a_enq	name

*	subroutine (2:char),2
*	'char' is 6,7,S
*	RTL address is 3,4,5,S
*	dp is 1,2,S

	txa
	sec
	sbc	#ptyDevStart
	asl   a                  ; calculate index into table
	tax
	lda	>ptyDPindex,x
	clc	                   ; clc$$
	adc	>PTYDP
	phd                      ; 1,s
	tcd

again	ldx	p_Aleft
	cpx	#0
	beq	dowait
	ldy	p_Ahead
	short	m
	lda	char,S
	sta	[p_bufAptr],y	
	long	m
	tya
	inc	a
	and	#ptyBufSize-1	; wrap it around
	sta	p_Ahead
	dex
	stx	p_Aleft
	pld
	lda	2,s
	sta	4,s
	lda	1,s
	sta	3,s
	pla
	rtl

dowait	anop
* free access semaphore
	pei	(p_sem)
	jsl	asmSignal

* wait on vector of buffer A; the line discipline code calls a low level
* signal_operation(read|write) to indicate what happened.  That way we
* don't vec_signal each and every time a character goes out.
	lda	>truepid
	pha
	pea	0
	pei	(p_bufAptr+2)
	pei	(p_bufAptr)
	jsl	k_sleep

	pei	(p_sem)
	jsl	asmWait
	bra	again
	END

B_enq	START KERN2
	using	KernelStruct
char	equ  6
_b_enq	name
*	subroutine (2:char),2

	txa
	sec
	sbc	#ptyDevStart
	asl   a                  ; calculate index into table
	tax
	lda	>ptyDPindex,x
	clc	                   ; clc$$
	adc	>PTYDP
	phd
	tcd

again	ldx	p_Bleft
	cpx	#0
	beq	dowait
	ldy	p_Bhead
	short	m
	lda	char,S
	sta	[p_bufBptr],y	
	long	m
	tya
	inc	a
	and	#ptyBufSize-1	; wrap it around
	sta	p_Bhead
	dex
	stx	p_Bleft
	pld
	lda	2,s
	sta	4,s
	lda	1,s
	sta	3,s
	pla
	rtl

dowait	anop
* free access semaphore
	pei	(p_sem)
	jsl	asmSignal

	lda	>truepid
	pha
	pea	0	; wakeup priority
	pei	(p_bufBptr+2)
	pei	(p_bufBptr)
	jsl	k_sleep
	pei	(p_sem)
	jsl	asmWait
	bra	again
	END

* 24 bytes each. 24*32 = 768 pages of direct page space
* Steal this from the kernel.

PTYSlaveHeader	START KERN2
	ds	t_open
* Line Discipline entry points
	dc	i4'PTYOpen'
	dc	i4'PTYClose'
	dc	i4'PTYIOCTL'
	dc	i4'ttread'
	dc	i4'ttwrite'
	dc	i4'pty_mutex'
	dc	i4'pty_demutex'
	dc	i4'B_enq'
	dc	i4'A_enq'
	dc	i4'B_deq'
	dc	i4'A_deq'
	dc	i4'A_sizeq'
	dc	i4'B_sizeq'
	ds	t_signalIO-editInd
	dc	i4'PTY_signalIO'
	dc	i2'$FFFF'		; noone selecting
	dc	i4'PTYS_select'
	jmp	>selwakeup

	END

PTYMastHeader	START KERN2
	ds	t_open
* Line Discipline entry points
	dc	i4'PTYOpen'
	dc	i4'PTYClose'
	dc	i4'PTYIOCTL'
	dc	i4'ttread'
	dc	i4'ptwrite'
	dc	i4'pty_mutex'
	dc	i4'pty_demutex'
	dc	i4'A_enq'
	dc	i4'B_enq'
	dc	i4'A_deq'
	dc	i4'B_deq'
	dc	i4'B_sizeq'
	dc	i4'A_sizeq'
	ds	t_signalIO-editInd
	dc	i4'PTY_signalIO'
	dc	i2'$FFFF'		; noone selecting
	dc	i4'PTYM_select'
	jmp	>selwakeup

	END

initPTY	START KERN2
ptyhand	equ  0
ptyptr	equ  4
_init_pty	name

	subroutine (0:foo),8

	pha
	pha
	pea	0
	pea	ptyRecSize*32
	lda	>~USER_ID
	pha
	pea	$C015
	pea	0
	pea	0
	_NewHandle
	pl4	ptyhand

	lda	[ptyhand]
	sta	>PTYDP
	sta   ptyptr
	ldy	#2
	lda	[ptyhand],y
	sta	>PTYDP+2
	sta   ptyptr+2

	ldy   #0
next	lda   #0
	sta   [ptyptr],y
	iny
	iny
	sta   [ptyptr],y
	tya
	clc
	adc   #ptyRecSize-2
	tay
	cmp   #ptyRecSize*32
	bcc   next

	return
	END

* If the devNum is even, this is the master we're opening.
* If the devNum is odd,  this is the slave we're opening.

PTYOpen	START KERN2
	using	KernelStruct
ptyDpPtr	equ  0
bufA	equ  4
bufB	equ  8
dTermioPtr	equ  12
result	equ  16
masterTerm	equ  18
slaveTerm	equ  22
	subroutine (2:devNum),26

	stz	result
	lda	devNum
	sec
	sbc	#ptyDevStart
	asl   a                  ; calculate index into table
	tax
	lda	>ptyDPindex,x
	clc	                   ; clc$$
	adc	>PTYDP
	sta	ptyDpPtr
	stz	ptyDpPtr+2
	ldy	#p_bufAptr
	lda	[ptyDpPtr],y
	ldy	#p_bufAptr+2
	ora	[ptyDpPtr],y
	beq	notInit	; this pty was not yet initialized

* at this point, the pty structures are already allocated and initialized.
* So, we need to signal the process that opened the other end of the pty
* to continue.
;	pei	(ptyDpPtr+2)
;	pei	(ptyDpPtr)
;	jsl   k_wakeup
	jmp	finish
notInit	anop

; if we're trying to open the Slave w/o the master being open, fail
; with an error $50.
	lda	devNum
	bit	#1
	beq	isMaster
	lda	#$50
	sta	result
	jmp	finish
; allocate master's and slave's TTY headers via malloc

isMaster	ph4	#ttyRecSize
	jsl	malloc
	stx	masterTerm+2
	sta	masterTerm
	ph4	#ttyRecSize
	jsl	malloc
	stx	slaveTerm+2
	sta	slaveTerm

; initialize them by copying data from the static copies here in KERN2

	lda	#ptwrite	; install custom write routine
	ldy	#t_write	; for the master
	sta	PTYMastHeader,y
	lda	#^ptwrite
	sta	PTYMastHeader+2,y

	ph4	#ttyRecSize
	ph4	#PTYMastHeader
	pei	(masterTerm+2)
	pei	(masterTerm)
	jsl	memcpy
	ph4	#ttyRecSize
	ph4	#PTYSlaveHeader
	pei	(slaveTerm+2)
	pei	(slaveTerm)
	jsl	memcpy

	lda	devNum
	and	#$FFFE	; clear the low bit
	asl	a
	asl	a
	tax
	lda	masterTerm+2    	; init the header pointers
	sta	>DeviceBlock+2,x
	lda   masterTerm
	sta	>DeviceBlock,x
	lda	slaveTerm+2
	sta	>DeviceBlock+6,x
	lda	slaveTerm
	sta	>DeviceBlock+4,x

	pea	0
	pea	ptyBufSize
	jsl	malloc	; alloc buffer A
	stx	bufA+2
	sta	bufA
	ldy   #p_bufAptr
	sta   [ptyDpPtr],y
	txa
	ldy   #p_bufAptr+2
	sta   [ptyDpPtr],y
	pea	0
	pea	ptyBufSize
	jsl	malloc	; alloc buffer B
	stx	bufB+2
	sta	bufB
	ldy   #p_bufBptr
	sta   [ptyDpPtr],y
	txa
	ldy   #p_bufBptr+2
	sta   [ptyDpPtr],y

* For purposes of interrupt chars & whatnot, both devices use the slave's
* ioctl structure, so initialize that.
* Initialize default values for terminal settings
	ldy	#sg_flags
	lda	#CRMOD+ECHO
	sta	[slaveTerm],y
	short	m
	lda	#0
	sta	[slaveTerm]	; ispeed
	ldy	#sg_ospeed
	sta	[slaveTerm],y     ; ospeed
	ldy	#t_intrc
	lda	#'C'-64
	sta	[slaveTerm],y	; t_intrc
	ldy	#t_suspc
	lda	#'Z'-64
	sta	[slaveTerm],y
	ldy	#t_quitc
	lda	#'\'-64
	sta	[slaveTerm],y
	ldy	#t_startc
	lda	#'Q'-64
	sta	[slaveTerm],y
	ldy	#t_stopc
	lda	#'S'-64
	sta	[slaveTerm],y
	ldy	#t_eofc
	lda	#'D'-64
	sta	[slaveTerm],y
	ldy	#t_brkc
	lda	#-1
	sta	[slaveTerm],y
	ldy	#t_dsuspc
	lda	#'Y'-64
	sta	[slaveTerm],y
	ldy	#t_rprntc
	lda	#'R'-64
	sta	[slaveTerm],y
	ldy	#sg_erase
	lda	#$7F
	sta	[slaveTerm],y
	long	m

	ldy	#privFlags
	lda	#0	; master can only be opened once
	sta	[slaveTerm],y
	lda	#EXCL
	sta	[masterTerm],y

	ldy	#local
	lda	#LCRTERA+LCTLECH
	sta	[slaveTerm],y

	ldy	#ws_row
	lda	#24
	sta	[slaveTerm],y
	ldy	#ws_col
	lda	#80
	sta	[slaveTerm],y
	ldy	#ws_xpixel
	lda	#0
	sta	[slaveTerm],y
	ldy	#ws_ypixel
	sta	[slaveTerm],y

	lda	#RAW
	ldy	#sg_flags
	sta	[masterTerm],y

	pea	1
	jsl	asmSemNew
	ldy	#p_sem
	sta	[ptyDpPtr],y

	lda	#0
	ldy	#p_Ahead
	sta	[ptyDpPtr],y
	ldy	#p_Atail
	sta	[ptyDpPtr],y
	ldy	#p_Bhead
	sta	[ptyDpPtr],y
	ldy	#p_Btail
	sta	[ptyDpPtr],y
	lda	#ptyBufSize-1
	ldy	#p_Aleft
	sta	[ptyDpPtr],y
	ldy	#p_Bleft
	sta	[ptyDpPtr],y

;	jsl	decBusy
;	lda	>truepid
;	pha
;	pea	0
;	pei	(ptyDpPtr+2)
;	pei	(ptyDpPtr)
;	jsl	k_sleep
;	jsl	incBusy

finish	anop
	return 2:result
PTYDP	ENTRY
	dc	i4'0'
	END

PTYClose	START KERN2
ptyDpPtr	equ  0
otherHeader	equ  4
devIndex	equ  8

	subroutine (2:devNum),10

	lda	devNum
	sec
	sbc	#ptyDevStart
	asl   a                  ; calculate index into table
	tax
	lda	>ptyDPindex,x
	clc	                   ; clc$$
	adc	>PTYDP
	sta	ptyDpPtr
	stz	ptyDpPtr+2

* only deallocate PTY information if the both ends of the PTY have been
* closed (see if the other PTY has been closed)

	lda	devNum
	asl	a
	asl	a
	sta	devIndex
	lda	devNum
	eor	#1	; the OTHER one, dork
	asl	a
	asl	a
	tax
	lda	#^PTYMastHeader
	cmp	>DeviceBlock+2,x
	bne	notClosed

	lda	>DeviceBlock,x
	cmp	#PTYMastHeader
	beq	isClosed
	cmp	#PTYSlaveHeader
	beq	isClosed

* If we're closing the master, send a SIGHUP to the PTYs process group
* If the slave was already closed, we assume there are no processes
* to kill and thus we don't fall through

notClosed	lda   devNum
	ror	a
	bcs	notMaster

	pea	1	; push signal number
	lda	devNum
	inc	a
	pha		; signal the TTY end
	jsl	PTYSlaveHeader+t_sendSignal
notMaster	bra   goaway

* Deallocate the pty buffers - only executed when both ends are
* closed
isClosed	ldy	#p_bufAptr+2
	lda	[ptyDpPtr],y
	pha
	ldy	#p_bufAptr
	lda	[ptyDpPtr],y
	pha
	jsl	nfree
	ldy	#p_bufBptr+2
	lda	[ptyDpPtr],y
	pha
	ldy	#p_bufBptr
	lda	[ptyDpPtr],y
	pha
	jsl	nfree

	lda	#0
	ldy	#p_bufAptr	; zero out this info
	sta	[ptyDpPtr],y
	ldy	#p_bufAptr+2
	sta	[ptyDpPtr],y

goaway	anop
* Deallocate the PTYs header
	ldx	devIndex
	lda	>DeviceBlock+2,x
	pha
	lda	>DeviceBlock,x
	pha
	jsl	free

* Reset the tty header pointer so we know we're closed
	ldx	devIndex
	lda	devNum
	ror	a
	bcs	resetSlave
	lda	#^PTYMastHeader
	sta	>DeviceBlock+2,x
	lda	#PTYMastHeader
	sta	>DeviceBlock,x
	bra	byebye
resetSlave	lda   #^PTYSlaveHeader
	sta	>DeviceBlock+2,x
	lda	#PTYSlaveHeader
	sta	>DeviceBlock,x

byebye	return
	END

PTYIOCTL	START KERN2
	using	KernelStruct
retval	equ  0
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

	ldx	devNum
	jsl   pty_mutex
	lda   devNum
	tax
	bit   #%00000001	; ARGH! IMMEDIATE! IMMEDIATE!
	beq   isMaster
	jsl   A_sizeq
	bra   notMaster
isMaster	jsl   B_sizeq
notMaster	anop
	sta	[dataPtr]
	ldx	devNum
	jsl   pty_demutex
	stz   retval
	jmp	goaway
tioctl	anop
	lda	tioc
	and	#$7F
	cmp   #21
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
	dc	i2'19',a2'invalid'
	dc	i2'20',a2'invalid'

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
	
TIOCOUTQ	anop
TIOCSTOP	anop
TIOCSTART	anop
TIOCFLUSH	anop
* simulate terminal input (this code is borrowed from INOUT.ASM/OurADB
TIOCSTI	anop
	stz	retval
	jmp	goaway	

	END

ptwrite	START KERN2
ttyPtr	equ  0
c	equ  4
xfer	equ  6
retval	equ  8
slavePtr	equ  10

	subroutine (2:length,4:buf,2:devNum),14

	lda	devNum
	jsr	fetchDevPtr	
	sta	ttyPtr
	stx	ttyPtr+2

	lda	devNum
	and	#%11111110	clear lo bit
	inc	a
	jsr	fetchDevPtr
	sta	slavePtr
	stx	slavePtr+2

	lda	length
	sta	xfer	
	stz	retval
	ldx	devNum
	ldy	#mutex
	jsl	ttyDispatch

px1	anop		we were in raw or cbreak mode

	lda	length
	jeq	pw1
	dea
	sta	length
	lda	[buf]
	and	#$00FF
	sta	c
	inc	buf
	bne	px2
	inc	buf+2
px2	anop
; check for special characters going to the slave, and process appropriately

	lda	c
	jsr	checkPtyIntr
	bcs   px1	; don't write char if it was a signal

	pei	(c)
	ldx	devNum
	ldy	#out_enq
	jsl	ttyDispatch
	jmp	px1
pw1	ldx	devNum
	ldy	#demutex
	jsl	ttyDispatch

	pea	0	; 0 means 'write occurred'
	ldx	devNum
	ldy	#t_signalIO
	jsl	ttyDispatch

	return 4:xfer

checkPtyIntr	anop

	php
	long  ai
	and   #$7f
	pha
	short	m

	ldy	#sg_flags
	lda	[slavePtr],y
	bit	#RAW	; RAW mode?
	beq	x9	; yep, no character checking
	brl	notty
x9	ldy	#t_quitc
	lda   [slavePtr],y
	cmp	#-1
	beq	x0
	cmp	1,s
	beq   gotQQ
x0	ldy	#t_suspc
	lda   [slavePtr],y
	cmp	#-1
	beq	x1
	cmp	1,s
	beq   gotZ
x1	ldy	#t_intrc
	lda   [slavePtr],y
	cmp	#-1
	beq	x2
	cmp	1,s
	beq   gotC
	bra	notty
x2	anop
;	lda	>OutStopped
;	bne	x3
;	ldy	#t_stopc
;	lda   [slavePtr],y
;               cmp	#-1
;               beq	x3
;               cmp	1,s
;               beq   gotS
x3	anop
;	ldy	#t_startc
;	lda	[slavePtr],y
;               cmp	#-1
;               beq	notty
;               cmp   1,s
;               beq   gotQ
;               bra   notty
;gotS           long	m
;	pla
;               lda   #1
;               sta   >OutStopped
;               plp
;               sec
;               rts
;gotQ           long	m
;	pla
;	lda   >OutStopped
;               beq   notQ
;               lda   #0
;               sta   >OutStopped
;               plp
;               sec
;               rts
notQ	anop		;oops!
	plp
	clc
	rts
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
	
	pha		; push signal number
	lda	devNum
	inc	a	; push our device number
	pha
	jsl	PTYSlaveHeader+t_sendSignal ; setup by InstallDriver

; flush internal editing buffers on interrupt character
	lda	#0
	ldy	#editInd
	sta	[slavePtr],y
	ldy	#editBegin
	sta	[slavePtr],y
	ldy	#st_flags
	sta	[slavePtr],y

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
sigtosend	dc i2'0'
	END

; eeeeewwwww, just one t_select_proc for all the ptys?
; we should probably store that on a per-pty basis...

PTYS_select	START KERN2
res	equ  0
	subroutine (2:ttyn,2:which,2:pid),2

	lda	#1
	sta	res
	lda	ttyn
	tax		; *_sizeq needs devNum in X
	lda	which	; which I/O to check?
	cmp	#SEL_READ
	bne	trywrite
	jsl	A_sizeq	; # bytes in in q
	cmp	#0
	bne	done

willwait	anop
* record that the process wants to do I/O
	lda	>PTYSlaveHeader+t_select_proc	; see if someone's here already
	cmp	#$FFFF	; nope
	beq	nocollision
	cmp	pid		; is it us?
	beq	nocollision
	lda	>PTYSlaveHeader+privFlags
	ora	#TS_RCOLL
	sta	>PTYSlaveHeader+privFlags
	bra	none
nocollision	anop
	lda	pid	; set select_proc field to
	sta	>PTYSlaveHeader+t_select_proc	; current process ID
	bra	none

trywrite	cmp	#SEL_WRITE
	bne	doexcept
	jsl	B_leftq	; # bytes avail in out q
	cmp	#0
	bne	done
	bra	willwait

doexcept	anop
; there are no exceptions on ptys - what about other side close?
none	lda	#0	; no data, return 0
	sta	res
done	anop

	return 2:res
	END

PTYM_select	START KERN2
res	equ  0
	subroutine (2:ttyn,2:which,2:pid),2

	lda	#1
	sta	res
	lda	ttyn
	tax		; *_sizeq needs devNum in X
	lda	which	; which I/O to check?
	cmp	#SEL_READ
	bne	trywrite
	jsl	B_sizeq	; # bytes in in q
	cmp	#0
	bne	done

willwait	anop
* record that the process wants to do I/O
	lda	>PTYMastHeader+t_select_proc	; see if someone's here already
	cmp	#$FFFF	; nope
	beq	nocollision
	cmp	pid		; is it us?
	beq	nocollision
	lda	>PTYMastHeader+privFlags
	ora	#TS_RCOLL
	sta	>PTYMastHeader+privFlags
	bra	none
nocollision	anop
	lda	pid	; set select_proc field to
	sta	>PTYMastHeader+t_select_proc	; current process ID
	bra	none

trywrite	cmp	#SEL_WRITE
	bne	doexcept
	jsl	A_leftq	; # bytes avail in out q
	cmp	#0
	bne	done
	bra	willwait

doexcept	anop
; there are no exceptions on ptys - what about other side close?
none	lda	#0	; no data, return 0
	sta	res
done	anop

	return 2:res
	END
