*	$Id: select.asm,v 1.1 1998/02/02 08:19:46 taubert Exp $
*
*      select.asm
*
* modified socket stuff - Derek Taubert - 12/13/94
* added GS/OS file support and proper return value - DT - 2/6/96
* added timeout support - DT - 2/13/96
*

	mcopy	m/select.mac
	case	on
	copy	inc/tty.inc
	copy	inc/gsos.inc
	copy	inc/kern.inc
	copy	global.equates

IncBusyFlag	gequ  $E10064
DecBusyFlag	gequ  $E10068

DebugNames	gequ  1

*
*
*

TIselect	START KERN2
TIselect	name
	using	pipeRecord
	using	KernelStruct
	using	SelTimStruct

* locals
mask	equ   1                  1		used?
pPtr	equ   mask+4             5
fdtptr	equ   pPtr+4             9
fdPtr	equ   fdtptr+4           13		used?
slept	equ   fdPtr+4            17
r_mask	equ   slept+2            19
w_mask	equ   r_mask+4           23
x_mask	equ   w_mask+4           27,28,29,30
space	equ   x_mask+4-1

* intermediates
bsp	equ   x_mask+4           31
dsp	equ   bsp+1              32
rtl2	equ   dsp+2              34
rtl1	equ   rtl2+3             37

* args
errnoptr  equ  rtl1+3             40
timeout   equ  errnoptr+4         44
exceptfds equ  timeout+4          48
writefds  equ  exceptfds+4        52
readfds   equ  writefds+4         56
width     equ  readfds+4          60
retval    equ  width+2            62

	phd
	phb

	phk
	plb
	tsc
	sec
	sbc   #space
	tcd
	tcs

	stz	retval

	lda	>procPtr
	sta	pPtr
	lda	>procPtr+2
	sta	pPtr+2

	ldy	#openFiles-CKernData
	lda	[pPtr],y
	sta	fdtptr
	ldy	#(openFiles-CKernData+2)
	lda	[pPtr],y
	sta	fdtptr+2

	lda	readfds
	ora	readfds+2
	beq	checkwr1
	pei	(readfds+2)
	pei	(readfds)
	pei	(fdtptr+2)
	pei	(fdtptr)
	jsl	validate_mask
	cmp	#0
	bne	err_mask

checkwr1	lda   writefds
	ora	writefds+2
	beq	checkex1
	pei	(writefds+2)
	pei	(writefds)
	pei	(fdtptr+2)
	pei	(fdtptr)
	jsl	validate_mask
	cmp	#0
	bne	err_mask

checkex1	lda   exceptfds
	ora	exceptfds+2
	beq	enter_loop
	pei	(exceptfds+2)
	pei	(exceptfds)
	pei	(fdtptr+2)
	pei	(fdtptr)
	jsl	validate_mask
	cmp	#0
	beq	enter_loop
err_mask	lda   #EBADF
	sta	[errnoptr]
	lda	#-1
	sta	retval
	jmp	goaway

enter_loop	anop
	stz	slept
	jsr	getSelTimIndex
	lda	#0
	sta	>SelTimExpired,x
	lda	>curProcInd
	tax
	lda	#0
	sta	>p_waitvec,x	; just for kicks..
	sta	>p_waitvec+2,x

select_loop	anop
* First, reset the local select masks we're using
	lda	#0
	sta	r_mask
	sta	r_mask+2
	sta	w_mask
	sta	w_mask+2
	sta	x_mask
	sta	x_mask+2

* Set Process' "selecting" flag
	ldy	#flags-KernelStruct
	lda	[pPtr],y
	ora	#FL_SELECTING
	sta	[pPtr],y

* For each FD, poll the device by calling its select routine
* If the descriptor can't do the operation, select must record that
* this process wants to do I/O.
	lda	readfds
	ora	readfds+2
	beq	checkwr2
	pea	SEL_READ
	pei	(readfds+2)
	pei	(readfds)
	pei	(fdtptr+2)
	pei	(fdtptr)
	pea	0
	tdc
	clc
	adc	#retval
	pha
	jsl	poll_mask
	sta	r_mask
	stx	r_mask+2

checkwr2	lda   writefds
	ora	writefds+2
	beq	checkex2
	pea	SEL_WRITE
	pei	(writefds+2)
	pei	(writefds)
	pei	(fdtptr+2)
	pei	(fdtptr)
	pea	0
	tdc
	clc
	adc	#retval
	pha
	jsl	poll_mask
	sta	w_mask
	stx	w_mask+2

checkex2	lda   exceptfds
	ora	exceptfds+2
	beq	endpoll
	pea	SEL_EXCEPT
	pei	(exceptfds+2)
	pei	(exceptfds)
	pei	(fdtptr+2)
	pei	(fdtptr)
	pea	0
	tdc
	clc
	adc	#retval
	pha
	jsl	poll_mask
	sta	x_mask
	stx	x_mask+2

endpoll	php
	sei
	lda	>curProcInd
	tax
	lda	>flags,x
	bit	#FL_SELECTING
	bne	noloop
* Somebody cleared FL_SELECTING, we better check again
	plp
	jmp	select_loop

noloop	and	#FL_SELECTING.EOR.$FFFF
	sta	>flags,x

; retval slept	timeout	SelTimExpired
;   0	   0	   0	     X		sleep
;   0	   0	   1	     X		set alarm, sleep
;   0	   1	   0	     X		return -1, EINTR
;   0	   1	   1	     0		return -1, EINTR
;   0	   1	   1	     1		return 0
;   1	   X	   X	     X		return retval
	lda	retval
	bne	done1			; we have a fd, kaptain!
	lda	slept
	beq	tosleep
; check to see how k_sleep woke up
	lda	timeout
	ora	timeout+2
	beq	interup
	jsr	getSelTimIndex
	lda	>SelTimExpired,x
	beq	interup
	lda	#0
	sta	>SelTimExpired,x
done1	bra	done			; we timed out!
interup	plp
	lda	#EINTR
	sta	[errnoptr]
	lda	#-1
	sta	retval
	jmp	goaway

timeoutTMP	dc  i4'0'
; set up alarm here
tosleep	lda	timeout
	ora	timeout+2
	beq	noalrm
setalrm	anop
	MUL4	[timeout],#10,timeoutTMP
; FIXME: add microseconds to timeoutTMP too
	jsr	getSelTimIndex
	lda	timeoutTMP
	bne	nofudge
	inc	A			; so SelTimTimeout never starts at zero
nofudge	sta	>SelTimTimeout,x
	lda	timeoutTMP+2
	sta	>SelTimTimeout+2,x
	lda	>curProcInd
	sta	>SelTimPID,x

noalrm	lda	>truepid
	pha
	pea	0
	ph4	#selwait
	jsl	k_sleep

	inc	slept
	lda	timeout
	ora	timeout+2
	beq	noalrm2
; remove alarm here
	jsr	getSelTimIndex
	lda	#0
	sta	>SelTimTimeout,x
	sta	>SelTimTimeout+2,x

noalrm2	plp
	jmp	select_loop

done	plp
	ldy	#2
	lda	r_mask
	sta	[readfds]
	lda	r_mask+2
	sta	[readfds],y
	lda	w_mask
	sta	[writefds]
	lda	w_mask+2
	sta	[writefds],y
	lda	x_mask
	sta	[exceptfds]
	lda	x_mask+2
	sta	[exceptfds],y

goaway	anop
	tsc
	clc
	adc	#space
	tcs
	plb
	pld
	ldx	#0
	ldy	#22
	jmp	>tool_exit

	END


poll_mask	START KERN2
poll_mask	name
	using	KernelStruct
words_left	equ  0
cur_fd	equ	2
bitcount	equ  4
wordind	equ	6
fdPtr	equ	8
outmask	equ	12

	subroutine (2:which,4:mask,4:fdtptr,4:fdsrdyptr),16

	stz	outmask
	stz	outmask+2

	ldy	#FDTfdTableSize
	lda	[fdtptr],y	; table size, divide by 16
	lsr	a
	lsr	a
	lsr	a
	lsr	a
	sta	words_left
	stz	cur_fd
	stz	wordind
val_loop1	lda  #16
	sta	bitcount
	ldy	wordind
	lda	[mask],y
val_loop2	ror  a	; rotate into carry flag
	bcc	dontcheck1	; do not check this fd
	pha

	pei	(cur_fd)
	jsl	getFDptr
	sta	fdPtr
	stx	fdPtr+2

; dispatch to the driver's select routine, which will tell us whether there's
; data and do other sundry stuff

	lda	[fdPtr]	; get device number
	dec	a	; dec to get device index
	pha
	pei	(which)	; tell the driver which I/O operation
; had to add this parm - DT
	lda	>curProcInd	; current pid
	pha
;
	ldy	#FDrefType
	lda	[fdPtr],y
	cmp	#FDsocket
	beq	selsok1
	cmp	#FDgsos
	beq	selsok2
; otherwise, it is #FDtty
; need to load A with dev type here, not ref type - DT
	lda	[fdPtr]	; get device number
	dec	a	; dec to get device index
	ldy	#t_select
	jsl	LineDiscDispatch
	bra	donesel1

; we can always read and write #FDgsos, never except
selsok2	anop
	pla		; current pid
	pla		; which operation
	plx		; device number
	cmp	#SEL_EXCEPT
	beq	gsos_nox
	sec
	bra	donesel2
gsos_nox	clc
	bra	donesel2

selsok1	anop
	jsl	SOCKselect

donesel1	cmp	#1	; set carry properly
donesel2	bcc	notready
	lda	[fdsrdyptr]
	adc	#0
	sta	[fdsrdyptr]
	sec
notready	ldx	wordind
	ror	<outmask,x

	pla
dontcheck	inc  cur_fd
	dec	bitcount
	bne	val_loop2
	inc	wordind
	inc	wordind
	dec	words_left
	bne	val_loop1

goaway	return 4:outmask
dontcheck1	anop
	ldx	wordind
	ror	<outmask,x
	bra	dontcheck
	END


validate_mask	START KERN2
validate_mask	name
words_left	equ  0
cur_fd	equ	2
res	equ	4
bitcount	equ  6
wordind	equ	8
fdPtr	equ	10

	subroutine (4:mask,4:fdtptr),14

	stz	res
	ldy	#FDTfdTableSize
	lda	[fdtptr],y	; table size, divide by 16
	lsr	a
	lsr	a
	lsr	a
	lsr	a
	sta	words_left
	stz	cur_fd
	stz	wordind
val_loop1	lda  #16
	sta	bitcount
	ldy	wordind
	lda	[mask],y
val_loop2	ror   a	; rotate into carry flag
	bcc	dontcheck	; do not check this fd
	pha

	lda	cur_fd
	beq	badfd	; fd 0 does not exist
	pha
	jsl	getFDptr
	sta	fdPtr
	stx	fdPtr+2
	lda	[fdPtr]
	beq	badfd
	ldy	#FDrefType
	lda	[fdPtr],y
	cmp	#FDgsos
	beq	dn1
	cmp	#FDsocket
	beq	dn1
	cmp	#FDtty
	bne	badfd	; we only select on ttys & sockets

dn1	pla
dontcheck	inc   cur_fd
	dec	bitcount
	bne	val_loop2
	inc	wordind
	inc	wordind
	dec	words_left
	bne	val_loop1

goaway	return 2:res
badfd	pla
	lda	#1
	sta	res
	bra	goaway
	END


selwakeupdefer	START KERN2
selwakeupdefer	name
	subroutine (2:runflag,2:pid),0

	pei	(runflag)
	pei	(pid)
	ph4	#selwait
	jsl	k_remove
	return
	END


selwakeup	START KERN2
selwakeup	name
selwait	ENTRY
	using	KernelStruct
	subroutine (2:pid,2:collflag),0

	lda	collflag
	beq	nocollision

	ph4	#selwait
	jsl	k_wakeup

nocollision	ldx   pid

	lda   >p_waitvec,x
	cmp   #selwait
	bne   ck_coll
	lda   >p_waitvec+2,x
	cmp	#^selwait
	bne	ck_coll
	lda   >ProcessState,x
	cmp   #PS_SLEEP
	bne   ck_coll

	ph4	#selwakeupdefer
	lda	pid
	lsr	a
	lsr	a
	lsr	a
	lsr	a
	lsr	a
	lsr	a
	lsr	a
	pha
	pea	1
	jsl	defer
	bra	goaway

ck_coll	lda	>flags,x
	and	#FL_SELECTING.EOR.$FFFF
	sta	>flags,x

goaway	return
	END


SelTimStruct	DATA
SelTimTimeout	dc  i4'0'		; timeout in tenths of seconds
SelTimPID	dc  i2'0'		; pid argument to call selwakeup with
SelTimExpired	dc  i2'0'		; set by the HB routine when she blows
	ds	8*(NPROC-1)		; space for 31 more entries
	END


getSelTimIndex	START KERN2
getSelTimIndex	name
	using	KernelStruct
	lda	>truepid
	asl	a
	asl	a
	asl	a
	tax
	rts
	END


;KERNgetdtablesize START KERN2
;	using	KernelStruct
;
;pPtr	equ	0
;fdPtr	equ	4
;ret	equ	8
;
;	subroutine (4:errptr),10
;
;	mv4	>procPtr,pPtr
;	ldy	#openFiles-CKernData
;	lda	[pPtr],y
;	sta	fdPtr
;	ldy	#(openFiles-CKernData-2)
;	lda	[pPtr],y
;	sta	fdPtr+2
;
;	lda	#0
;	sta	[errptr]
;
;	ldy	#FDTfdTableSize
;	lda	[fdPtr],y
;	sta	ret
;	return 2:ret
;	END
;
;STACK	START ~_STACK
;	KIND	$12
;	ds	1024
;	END
