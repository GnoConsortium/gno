*	$Id: pipe.asm,v 1.1 1998/02/02 08:19:37 taubert Exp $
**************************************************************************
*
* PIPE.ASM
*
*   by Jawaid Bazyar
*
*   takes proper care of refNums, so redirection is handled just like
*   Unix.  Also, implements the hooks necessary for pipes.
*
**************************************************************************
	mcopy m/pipe.mac
	case	on
	copy  global.equates
	copy	inc/kern.inc

* the maximum number of pipes is NPROC-1, and we need 4 sems per pipe,
* so NASMSEM is a good approximation

NASMSEM	gequ  NPROC*4+20
PIPESIZE	gequ  4096

rfPIPEREAD	gequ  1                  ;* read end of the pipe * 
rfPIPEWRITE	gequ  2                  ;* write end of the pipe * 
rfCLOSEEXEC	gequ  4                  ;* close this file on an exec() * 
rfP16NEWL	gequ  8                  ;* special prodos-16 newline mode * 

RRsize	gequ  6
RRrefnum	gequ  0
RRtype	gequ  2
RRcount	gequ  4

	copy	inc/gsos.inc

pipeData	DATA
refHandle	dc    i4'0'
refHandleSize	dc    i2'0'
	END

writeEvent	gequ  1

PRsize	gequ  32
PRnum	gequ  NPROC

* the new pipe record
pipeRecord	DATA
bufferH	dc    i4'0'              ; the pipe data handle
in	dc    i2'0'              ; write pointer
out	dc    i2'0'              ; read pointer
qflags	dc    i2'0'
RrnCount	dc    i2'0'              ; Why? Because special stuff has to
WrnCount	dc    i2'0'              ; happen in strange conditions

accessSem	dc    i2'0'              ; semaphores
needReadSem	dc    i2'0'
needWriteSem	dc    i2'0'
needRead	dc    i2'0'
needWrite	dc    i2'0'
writeLeft	dc    i2'0'
readLeft	dc    i2'0'
writeStatus	dc    i2'0'
dummy	dc	i2'0'
	ds    32*31              ; 32 pipes max- think about it
	END

**************************************************************************
*
*  FindRefnum(int rn, int type)
*    searches the table for a specified refnum.  It returns a pointer
*    to the entry, or NIL if no matching refnum is found.
*
**************************************************************************

FindRefnum	START
	using pipeData
tmpHandle	equ   0
tmpPtr	equ   4
retval	equ   8

	subroutine (2:refnum,2:type),12

	mv4   refHandle,tmpHandle

	lda   [tmpHandle]
	sta   tmpPtr
	ldy   #2
	lda   [tmpHandle],y
	sta   tmpPtr+2
	ldy   #0
findloop	lda   refnum
	cmp   [tmpPtr],y
	bne   nextone
	lda   type
	iny2
	cmp   [tmpPtr],y
	beq   gotit
	dey2
nextone	anop
	tya
	clc
	adc   #RRsize
	tay
	cpy   refHandleSize
	bcc   findloop
	stz   tmpPtr
	stz   tmpPtr+2
	bra   goaway
gotit	dey2
	tya
	clc
	adc   tmpPtr
	sta   tmpPtr
	lda   tmpPtr+2
	adc   #0
	sta   tmpPtr+2
goaway	anop
	return 4:tmpPtr
	END

InitRefnum	START
	using KernelStruct
	using pipeData

tmpHandle	equ   0
tmpPtr	equ   4

	subroutine (0:foo),8
	lda   #240
	sta   refHandleSize

;               NewHandle (#240,UserID,#0,#0),tmpHandle
	ph4   #0                 ; result space dammit!
	ph4   #240
	ph2   userID
	ph2   #8
	ph4   #0
	_NewHandle
	cmp   #0
	bne   handfailure
	pla
	sta   tmpHandle
	sta   refHandle
	pla
	sta   tmpHandle+2
	sta   refHandle+2

	ph4   tmpHandle          ; we must lock the handle or badness
	_HLock                   ; will occur- it could get purged!

	lda   [tmpHandle]
	sta   tmpPtr
	ldy   #2
	lda   [tmpHandle],y
	sta   tmpPtr+2

	ldy   #0
	tya
clrloop	anop
	sta   [tmpPtr],y
	iny2
	cpy   refHandleSize
	bcc   clrloop
	return
handfailure	anop
	jsr   writeacc
	ph4   #0
	jsl   PANIC
	END

GrowRefnum	START
	using pipeData

tmpHandle	equ   0
tmpPtr	equ   4
newsize	equ   8

	subroutine (0:foo),10

	ph4   refHandle
	_HUnlock

	lda   refHandleSize
	clc
	adc   #240
	sta   newsize

	pea   $0000
	pha
	ph4   refHandle
	_SetHandleSize
	cmp   #0
	beq   okay
	ph4   errtxt
	jsl   PANIC
okay	anop
	ph4   refHandle
	_HLock

	mv4   refHandle,tmpHandle
	lda   [tmpHandle]
	sta   tmpPtr
	ldy   #2
	lda   [tmpHandle],y
	sta   tmpPtr+2

	ldy   refHandleSize
	lda   #0
clrloop	anop
	sta   [tmpPtr],y
	iny2
	cpy   newsize
	bcc   clrloop
	return
errtxt	dc    c'Error growing kernel refnum table',i1'0'
	END


**************************************************************************
*
*  AddRefnum(int type, int refnum)
*   Adds a refnum/type to the refcount table.  If the refnum/type
*   already exists, a system panic will occur
*
**************************************************************************

AddRefnum	START
	using pipeData
ptr	equ   0

	subroutine (2:refnum,2:type),4
findEmpty	anop
	phy
	phx

	pei	(refnum)
	pei	(type)
	jsl   FindRefnum
	phx
	ora   1,s
	bne   addpanic
	pla

	ph2   #0
	ph2   #0
	jsl   FindRefnum
	sta   ptr
	stx   ptr+2
	ora   ptr+2
	bne   okay
	jsl   GrowRefnum
	bra   findEmpty
okay	anop
	ldy   #RRrefnum
	lda   refnum
	sta   [ptr],y
	ldy   #RRtype
	lda   type
	sta   [ptr],y
	ldy   #RRcount
	lda   #1
	sta   [ptr],y
	plx
	ply
	return
addstr	str   'AddRefnum:'
addpanic	ph4   #addp
	jsl   PANIC
addp	dc    c'AddRefnum: refnum/type already exists!',i'0'
	END

IncRefnum	START
ptr	equ   0

	subroutine (2:refnum,2:type),4

	phy
	phx

	ph2   refnum
	ph2   type
	jsl   FindRefnum
	sta   ptr
	stx   ptr+2
	ora   ptr+2
;              beq   goaway
	beq   errorpanic
	ldy   #RRcount
	lda   [ptr],y
	inc   a
	sta   [ptr],y

goaway	anop
	plx
	ply
	return
incstr	str   'IncRefnum:'
errorpanic	anop
	brk   $00
	ph4   #decerror
	jsl	PANIC
decerror	dc    c'IncRefnum- ref/type combo not found',i'0'
	END

**************************************************************************
*
*  DecRefnum(int refnum)
*
**************************************************************************

DecRefnum	START
ptr	equ   0
retval	equ   4

	subroutine (2:refnum,2:type),6
	phx

	stz   retval
	ph2   refnum
	ph2   type
	jsl   FindRefnum
	sta   ptr
	stx   ptr+2
	ora   ptr+2
	beq   errorpanic
	ldy   #RRcount
	lda   [ptr],y
	dec   a
	sta   [ptr],y
	sta   retval
	cmp   #0
	bne   goaway
	lda   #0
	ldy   #RRrefnum
	sta   [ptr],y            ; the count is down to zero
	ldy   #RRtype
	sta   [ptr],y
goaway	anop
	plx
	return 2:retval
addstr	str   'DecRefnum:'
errorpanic	anop
	brk   $00
	ph4   #decerror
	jsl   PANIC
decerror	dc    c'DecRefnum- ref/type combo not found',i'0'
	END

* an assembly semaphore 

asmSemTable	DATA
alloced	dc    i2'0'
semCount	dc    i2'0'
queuePtr	dc    i2'0'              ; we'll use indexen, not pointers
	dc    i2'0'
	ds    1184               ; (NASMSEM-1)*8 , NASMSEM = 128
	END

asmSemNew	START
retval	equ   0
	using asmSemTable
	subroutine (2:cnt),2

	php
	sei
	ldy   #0
loop	lda   alloced,y
	beq   gotit
	tya
	clc
	adc   #8
	tay
	cpy   #(NASMSEM*8)
	bra   loop
gotit	lda   #1
	sta   alloced,y
	lda   #0
	sta   queuePtr,y
	lda   cnt
	sta   semCount,y
	tya
	lsr   a
	lsr   a
	lsr   a
	inc   a
	ora	#$8000
	sta   retval
	plp
	return 2:retval
	END

asmSemDispose	START
	using asmSemTable
	using KernelStruct
retval	equ   0
	subroutine (2:sem),2

	php
	sei
	lda   sem
	and	#$7FFF
	dec   a
	asl   a
	asl   a
	asl   a
	tax
	lda   alloced,x
	beq   error
	stz	alloced,x	; deallocate the semaphore!
	
	lda   queuePtr,x
	tax
readyLoop	cpx   #0
	beq   done
	lda	ProcessState,x
	cmp	#pUnused	; it's gone, by god
	beq	skipProc

	lda   #pReady
	sta   ProcessState,x
skipProc	lda   queueLink,x
	tax
	bra   readyLoop
done	stz   retval
bye	plp
	return 2:retval
error	lda   #-1
	sta   retval
	bra   bye
	END

asmWait	START
	using asmSemTable
	using KernelStruct
	subroutine (2:sem),0

;               jsl   incBusy            ; ahh, critical section
	php
	sei
notowner	anop
	lda   sem
	and	#$7FFF
	dec   a
	asl   a
	asl   a
	asl   a
	tay

	lda   semCount,y            ; line for line from 'sem.c'
	dec   a
	sta   semCount,y
	bpl   goaway

* Add the current process to the queue.  Use the handy 'queueLink' field
* provided in the process structure to accomplish the link

	lda   queuePtr,y
	tax
	bne   loop
	lda   >curProcInd
	sta   queuePtr,y
	tax
	lda   #0
	sta   queueLink,x
	bra   nomo
loop	lda   queueLink,x
	beq   gotit
	tax
	bra   loop
gotit	lda   >curProcInd
	sta   queueLink,x
	tax
	lda   #0
	sta   queueLink,x

* Now that the process is enqueued, make it wait

nomo	ldx   curProcInd         ; 'suspend' the process
	lda   #pWait             ; put the process in a 'wait' state
	sta   ProcessState,x
; This will cause havoc with KERNkill and normal sem's EINTR
;               lda   sem
;               sta   psem,x
	short	m
	lda	>$E100FF
	pha
	lda	#0
	sta	>$E100FF
	cop   $7F
	pla
	sta	>$E100FF
	long	m
	plp		; restore interrupts
	bra   goaway2

goaway	anop
	plp
;               jsl   decBusy            ; leave critical section
goaway2	anop
	return
	END

asmSignal	START
	using asmSemTable
	using KernelStruct
	subroutine (2:sem),0

;               jsl   incBusy
	php
	sei
notowner	anop                     ; line for line from sem.c/ssignal
	lda   sem
	and	#$7FFF
	dec   a
	asl   a
	asl   a
	asl   a
	tay

	lda   semCount,y
	bpl   goaway             ; if >=0, no processes waiting

* Remove the next process from the queue and ready it for running

	lda   queuePtr,y
	pha                      ; store it for later
	tax
	lda   queueLink,x
	sta   queuePtr,y         ; that's pretty simple
	plx                      ; it's ready!
	lda	ProcessState,x
	cmp	#pUnused	; it's gone!
	beq	goaway
	lda   #pReady
	sta   ProcessState,x
goaway	anop
	lda   semCount,y
	inc   a
	sta   semCount,y
;               jsl   decBusy
	plp
	return
	END

*checkOA        START
*               short m
*loop           anop
*               lda   >$E0C061
*               bpl   loop
*loop2          anop
*               lda   >$E0C061
*               bmi   loop2
*               long  m
*               rts
*               END

* only for testing, we're hacking this.

newPipe	START
	using pipeRecord
	using KernelStruct
pipeind	equ   0
retval	equ   2
pipediv	equ   4
hand	equ   6
buf	equ   10

	subroutine (0:foo),14
	ldy   #0
	stz   pipediv
findlp	lda   bufferH+2,y
	ora   bufferH,y
	beq   gotone             ; found an empty one
	tya
	clc
	adc   #PRsize
	tay
	inc   pipediv
	cpy   #PRsize*PRnum
	bcc   findlp
	lda   #$FFFF
	sta   retval
	jmp   goaway

* We found an empty slot, now allocate the semaphores & memory
gotone	anop
	sty   pipeind

	ph2   #1
	jsl   asmSemNew
	ldy   pipeind
	sta   accessSem,y

	ph2   #0
	jsl   asmSemNew
	ldy   pipeind
	sta   needReadSem,y

	ph2   #0
	jsl   asmSemNew
	ldy   pipeind
	sta   needWriteSem,y

	lda	#0
	sta	needWrite,y
	sta	needRead,y
	sta	readLeft,y
	lda	#PIPESIZE-1
	sta	writeLeft,y

*              ph2   #0
*              jsl   asmSemNew
*              ldy   pipeind
*              sta   writeStatus,y
	lda   #1
	sta   WrnCount,y
	sta   RrnCount,y
	lda   #0
	sta   in,y
	sta   out,y
	sta   qflags,y
	ph4   #0
	ph4   #PIPESIZE
	ph2   userID
	ph2   #0008              ; may not use special memory
	ph4   #0
	_NewHandle
	ldy   pipeind
	pla
	sta   bufferH,y
	sta   hand
	pla
	sta   bufferH+2,y
	sta   hand+2

* zero out the pipe buffer so we can see what's going on better
	lda   [hand]
	sta   buf
	ldy   #2
	lda   [hand],y
	sta   buf+2
	ldy   #PIPESIZE-1
	lda   #0
	short m
lp	sta   [buf],y
	dey
	bpl   lp
	long  m

	lda   pipediv
	inc   a
	sta   retval
goaway	return 2:retval
	END

disposePipe	START
	using pipeRecord
	using KernelStruct
retval	equ   0
pipeind	equ   2

	subroutine (2:pipe),4

	lda   pipe
	jsl   calcPipeInd
	sta   pipeind

	tay

* push all the stack info we need in one big glob- no errors can occur

	lda   bufferH+2,y
	pha
	lda   bufferH,y
	pha
	lda   needWriteSem,y
	pha
	lda   needReadSem,y
	pha
	lda   accessSem,y
	pha
*              lda   writeStatus,y
*              pha

	jsl   asmSemDispose
	jsl   asmSemDispose
	jsl   asmSemDispose
*              jsl   asmSemDispose
	_DisposeHandle
	lda   #0
	ldy   pipeind
	sta   bufferH,y
	sta   bufferH+2,y
goaway	return
	END

IOreqCount	gequ  6
IOrefNum	gequ  0
IOdataBuf	gequ  2
IOtransCount	gequ  10


pipeHiRead	START
	using	pipeRecord

pipeInd	equ	0
locReadleft	equ  2
transLeft	equ  4
doCount	equ	8
curDataPtr	equ  10
pipeBuf	equ	14
pipeH	equ	18
didTransfer	equ  22
retVal	equ	26
usrOutInd	equ  28
NLenableMask	equ  30
numnewlines	equ  32
newlinebuf	equ  34
P16newlinech	equ  38	; note that these are only checked
tempChar	equ  40                 ; in 8-bit mode

	subroutine (4:pBlock,2:pipe,4:fdrec),42
* Initialize a bunch of crap

	lda   pipe
	jsl   calcPipeInd
	sta   pipeInd

* set up our local variables to do newline checking
	ldy   #FDNLenableMask
	lda   [fdrec],y
	sta   NLenableMask
	beq   nlOff              ; if NL's are off, save a few cycles
	ldy   #FDrefFlags
	lda   [fdrec],y
	and   #rfP16NEWL
	beq   doNormalNewL

	ldy   #FDNLnumChars
	lda   [fdrec],y
	sta   P16newlinech
	lda   #1
	sta   numnewlines
	stz   newlinebuf+2
	tdc
	clc
	adc   #P16newlinech
	sta   newlinebuf
	bra   nlOff

doNormalNewL	ldy   #FDNLnumChars      ; otherwise copy the info we need
	lda   [fdrec],y
	sta   numnewlines
	ldy   #FDNLtable
	lda   [fdrec],y
	sta   newlinebuf
	iny2
	lda   [fdrec],y
	sta   newlinebuf+2

nlOff	stz	retVal	; no error right now

* copy the # of bytes wanted to our "current count" counter
	ldy	#IOreqCount+2
	lda	[pBlock],y
	sta	transLeft+2
	ldy	#IOreqCount
	lda	[pBlock],y
	sta	transLeft
* move the data pointer to our "current pointer" pointer
	ldy	#IOdataBuf+2
	lda	[pBlock],y
	sta	curDataPtr+2
	ldy	#IOdataBuf
	lda	[pBlock],y
	sta	curDataPtr
	stz	didTransfer
	stz	didTransfer+2

l1	anop

;	lda	#hsHoldSig
;	jsl	setHoldSig	; hold signals until we tell you

* get access to the pipe record
	ldy	pipeInd
	lda	accessSem,y
	pha
	jsl	asmWait

* calculate number of bytes actually in the pipe that can be read
	ldy	pipeInd
	lda	bufferH,y
	sta	pipeH
	lda	bufferH+2,y
	sta	pipeH+2
	lda	readLeft,y
	sta	locReadleft	; temporary location

	lda	[pipeH]
	sta	pipeBuf
	ldy	#2
	lda	[pipeH],y
	sta	pipeBuf+2	

* determine how many bytes to transfer in this chunk
	lda	transLeft+2
	bne	gt1
	lda	transLeft
	cmp	locReadleft
	bcs	gt1
	lda	transLeft
	sta	doCount
	bra	lt1
gt1	lda	locReadleft
	sta	doCount
lt1	anop
	cmp	#0	; no data to read? what? morons!
	bne	havedata

* there's no data, so see if there are any writers left
	ldy	pipeInd
	lda   WrnCount,y
* there are writers, so go wait for data
	jne   wait4data
	lda	#$4C
	sta	retVal
	jmp	goaway
	
havedata	ldy	pipeInd
	lda	out,y
	tax
	stz	usrOutInd
	short	m
* main data copy loop
lp1	anop
	txy
* $$$ Squeeze the NewLine mode check in here somewhere
* (ptys will use NewLine's to implement the cooked tty mode)
	lda	[pipeBuf],y
	sta	tempChar
	ldy	usrOutInd
	sta	[curDataPtr],y
	inx
	cpx	#PIPESIZE
	bcc   nomod
	ldx   #0
nomod	anop
	iny
	sty	usrOutInd
	ldy	doCount
	dey
	sty	doCount

* this is the newline character check (note the massive overhead)
	lda   NLenableMask       ; if the mask is 0, we don't do NL
	bne   doNLcheck
	
	tya		; get the Z flag again
	bne   lp1
	bra	donexfer
doNLcheck	anop
	and   tempChar
	ldy   #0
lp	cpy   numnewlines
	beq   ckLastByte	; all out o' 
	cmp   [newlinebuf],y
	beq   gotnewline
	iny
	bra   lp
ckLastByte	ldy   doCount
	bne	lp1
	beq	donexfer

gotnewline	anop
;	bne	lp1   y != 0
	long	m
	lda	#1
	sta	retVal

* update our buffer pointers
donexfer	long	m
	ldy	usrOutInd
	sty	doCount	; reset this
	tya
	clc
	adc	curDataPtr
	sta	curDataPtr
	lda	curDataPtr+2
	adc	#0
	sta	curDataPtr+2
	
	tya
	clc
	adc	didTransfer
	sta	didTransfer
	lda	didTransfer+2
	adc	#0
	sta	didTransfer+2

	ldy	pipeInd
	txa
	sta	out,y

	lda	transLeft
	sec
	sbc   doCount
	sta	transLeft
	lda	transLeft+2
	sbc	#0
	sta	transLeft+2

	lda	readLeft,y
	sec
	sbc	doCount
	sta	readLeft,y
	lda	writeLeft,y
	clc
	adc	doCount
	sta	writeLeft,y

	lda	needWrite,y
	beq	noneedwrite
	dea
	sta	needWrite,y
	lda	needWriteSem,y     ; release a process that needs to
	pha	                   ; write into the pipe
	jsl	asmSignal

noneedwrite	anop
* anything left to transfer?
	lda	retVal
	cmp	#1
	bne	notNL
	stz	retVal
	bra	goaway
notNL	lda	transLeft
	ora	transLeft+2
	beq	goaway
	ldy	pipeInd
* tell a writer that we need more data, and block ourselves
wait4data	anop
	lda	needRead,y
	ina
	sta	needRead,y
* release the pipe structure
	lda	accessSem,y
	pha
	jsl	asmSignal
* wait for someone to write more data
* Note that if a writer terminates while we're waiting here,
* we'll go above, & read data, then come back here because the reader
* wants still more data. Then we hang. We need to check for a lack of
* writers again here
* We can only get here because there's not enough data in the pipe
* to satisfy the request.

;	lda	#hsNoHoldSig
;	jsl	setHoldSig	; don't hold signals
;	jsl	checkHeldSig	; did we get one?
;	lda	#hsHoldSig
;	jsl	setHoldSig	; hold signals until we tell you

	ldy	pipeInd
	lda	WrnCount,y
	beq	nomowrite
	lda	needReadSem,y
	pha
	jsl	asmWait
;	jsl	checkHeldSig

	jmp	l1
nomowrite	lda   #$4C
	sta	retVal
goaway	anop
* release the pipe (we're leaving, yea! Back to Kansas...)
	ldy	pipeInd
	lda	accessSem,y
	pha
	jsl	asmSignal
	lda	didTransfer
	ldy	#IOtransCount
	sta	[pBlock],y
	lda	didTransfer+2
	ldy	#IOtransCount+2
	sta	[pBlock],y

;	lda	#hsNoHoldSig
;	jsl	setHoldSig	; we're done, you can clobber us
;	jsl	checkHeldSig

	return 2:retVal
	END

pipeHiWrite	START
	using pipeRecord
pipeInd	equ	0
locWriteleft	equ  2
transLeft	equ  4
doCount	equ	8
curDataPtr	equ  10
pipeBuf	equ	14
pipeH	equ	18
didTransfer	equ  22
retVal	equ	26
usrInInd	equ  28

	subroutine (4:pBlock,2:pipe),30
* Initialize a bunch of crap

	lda   pipe
	jsl   calcPipeInd
	sta   pipeInd
	stz	retVal	; no error right now

* copy the # of bytes wanted to our "current count" counter
	ldy	#IOreqCount+2
	lda	[pBlock],y
	sta	transLeft+2
	ldy	#IOreqCount
	lda	[pBlock],y
	sta	transLeft
* move the data pointer to our "current pointer" pointer
	ldy	#IOdataBuf+2
	lda	[pBlock],y
	sta	curDataPtr+2
	ldy	#IOdataBuf
	lda	[pBlock],y
	sta	curDataPtr
	stz	didTransfer
	stz	didTransfer+2

l1	anop

;	lda	#hsHoldSig
;	jsl	setHoldSig

* get access to the pipe record
	ldy	pipeInd
	lda	accessSem,y
	pha
	jsl	asmWait

* calculate number of bytes actually in the pipe that can be read
	ldy	pipeInd
	lda	bufferH,y
	sta	pipeH
	lda	bufferH+2,y
	sta	pipeH+2
	lda	writeLeft,y
	sta	locWriteleft	; temporary location

	lda	[pipeH]
	sta	pipeBuf
	ldy	#2
	lda	[pipeH],y
	sta	pipeBuf+2	

* determine how many bytes to transfer in this chunk
	lda	transLeft+2
	bne	gt1
	lda	transLeft
	cmp	locWriteleft
	bcs	gt1
	lda	transLeft
	sta	doCount
	bra	lt1
gt1	lda	locWriteleft
	sta	doCount
lt1	anop

* see if there are any readers left
	ldy	pipeInd
	lda   RrnCount,y
* there are readers, so go check for data
	jne   check4data
* ack! there are no readers left!
	lda	needWriteSem,y
	pha	
	jsl	asmSignal	; for the next orphaned writer
	ldy	pipeInd
	lda	accessSem,y
	pha	
	jsl	asmSignal	; we need to relinquish pipe access

;	lda	#hsNoHoldSig	; so the signal below will get
;	jsl	setHoldSig	; delivered
;	jsl	checkHeldSig

* do not call checkHeldSig since we want this sent first

	pha
	pha
	jsl	KERNgetpid	; this is #pragma toolparms 1
	pea	13                ; SIGPIPE
	ph4	#errno
	jsl	KERNkill	; this is #pragma toolparms 1
	pla
	lda	#$27
	sta	retVal
	jmp	goaway

check4data	lda   doCount
	cmp	#0	; no room to write data?
	jeq	wait4data          ; no data, go wait for some
	
havedata	ldy	pipeInd
	lda	in,y
	tax
	stz	usrInInd
	short	m
* main data copy loop
lp1	anop
	ldy	usrInInd
	lda	[curDataPtr],y
	txy
	sta	[pipeBuf],y
	inx
	cpx	#PIPESIZE
	bcc	nomod
	ldx	#0
nomod	ldy	usrInInd
	iny
	sty	usrInInd
	ldy	doCount
	dey
	sty	doCount
	bne	lp1
* update our buffer pointers
	long	m
	ldy	usrInInd
	sty	doCount	; reset this
	tya
	clc
	adc	curDataPtr
	sta	curDataPtr
	lda	curDataPtr+2
	adc	#0
	sta	curDataPtr+2
	
	tya
	clc
	adc	didTransfer
	sta	didTransfer
	lda	didTransfer+2
	adc	#0
	sta	didTransfer+2

	ldy	pipeInd
	txa
	sta	in,y

	lda	transLeft
	sec
	sbc   doCount
	sta	transLeft
	lda	transLeft+2
	sbc	#0
	sta	transLeft+2

	lda	readLeft,y
	clc
	adc	doCount
	sta	readLeft,y
	lda	writeLeft,y
	sec
	sbc	doCount
	sta	writeLeft,y

	lda	needRead,y
	beq	noneedwrite
	dea
	sta	needRead,y
	lda	needReadSem,y     ; release a process that needs to
	pha	                   ; read from the pipe
	jsl	asmSignal

noneedwrite	anop
* anything left to transfer?
	lda	transLeft
	ora	transLeft+2
	beq	goaway
	ldy	pipeInd
* tell a reader that we need more room, and block ourselves
wait4data	lda   needWrite,y
	ina
	sta	needWrite,y

* release the pipe structure
	lda	accessSem,y
	pha
	jsl	asmSignal

;	lda	#hsNoHoldSig
;	jsl	setHoldSig
;	jsl	checkHeldSig
;	lda	#hsHoldSig
;	jsl	setHoldSig

* wait for someone to read more data
	ldy	pipeInd
	lda	needWriteSem,y
	pha
	jsl	asmWait
;	jsl	checkHeldSig
	jmp	l1

goaway	anop
* release the pipe (we're leaving, yea! Back to Kansas...)
	ldy	pipeInd
	lda	accessSem,y
	pha
	jsl	asmSignal
	lda	didTransfer
	ldy	#IOtransCount
	sta	[pBlock],y
	lda	didTransfer+2
	ldy	#IOtransCount+2
	sta	[pBlock],y

;	lda	#hsNoHoldSig
;	jsl	setHoldSig
;	jsl	checkHeldSig

	return 2:retVal
	END
	
* Input:
*    A = pipe number
* Output:
*    A = pipe index
*  pipeInd = (pipe - 1) * 32

calcPipeInd	START
	dec	a
	asl	a
	asl	a
	asl	a
	asl	a
	asl	a                      
	rtl
	END

incPipe	START
	using pipeRecord
pipeInd	equ   0
	subroutine (2:pipe,2:fl),2
	lda   pipe
	jsl   calcPipeInd
	sta   pipeInd
	tay
	lda   accessSem,y
	pha
	jsl   asmWait
	ldy   pipeInd
	lda   fl
	bit   #2
	bne   wc
	bit   #1
	bne   rc	; invalid happening
	brk	$00
rc	lda   RrnCount,y
	inc   a
	sta   RrnCount,y
	bra   goaway
wc	lda   WrnCount,y
	inc   a
	sta   WrnCount,y
goaway	anop
	lda   accessSem,y
	pha
	jsl   asmSignal
	return
	END

decPipe	START
	using pipeRecord
pipeInd	equ   0

	subroutine (2:pipe,2:fl),2
	lda   pipe
	jsl   calcPipeInd
	sta   pipeInd
	tay
	lda   accessSem,y
	pha
	jsl   asmWait
	ldy   pipeInd
	lda   fl
	bit   #2
	bne   wc
	bit   #1
	bne   rc	; don't know- this is a BAD thing
	brk	$00

rc	lda   RrnCount,y
	dec   a
	sta   RrnCount,y
	bne	goaway
	
	lda	needWrite,y
	beq	goaway
	dec	a
	sta	needWrite,y

	lda	needWriteSem,y
	pha
	jsl	asmSignal	; tell writers to go thru so they
	bra   goaway	; notice there's no more reader
wc	lda   WrnCount,y
	dec   a
	sta   WrnCount,y
*	sta	>$e00400
	bne	goaway
	lda	needRead,y
*	sta	>$e00402
	beq	goaway
	dec	a
	sta	needRead,y
g1	lda   needReadSem,y
	pha
	jsl   asmSignal

goaway	anop
	ldy   pipeInd
	lda   accessSem,y
	pha
	jsl   asmSignal
	return
	END
