*	$Id: p16.asm,v 1.1 1998/02/02 08:19:33 taubert Exp $
**************************************************************************
*
* P16.ASM
*   By Jawaid Bazyar
*
* This file contains routines for patching Prodos 16 calls to maintain
* the information the kernel needs to keep track of things.
*
**************************************************************************

	copy  global.equates
	mcopy m/p16.mac

	copy	inc/gsos.inc
	copy	inc/tty.inc
	case	on

* if you change FDsize, look for the commend "a * FDsize". You must
* change code there.

rfPIPEREAD	gequ  1                  ;* read end of the pipe * 
rfPIPEWRITE	gequ  2                  ;* write end of the pipe * 
rfCLOSEEXEC	gequ  4                  ;* close this file on an exec() * 
rfP16NEWL	gequ  8                  ;* special prodos-16 newline mode * 

P16table	START
	dc a2'P16Standard'       ; $2001 CREATE
	dc a2'P16Standard'       ; $2002 DESTROY
	dc a2'NotImpP16'         ; $2003
	dc a2'P16ChangePath'     ; $2004 CHANGE_PATH
	dc a2'P16Standard'       ; $2005 SET_FILE_INFO
	dc a2'P16Standard'       ; $2006 GET_FILE_INFO
	dc a2'NotImpP16'         ; $2007
	dc a2'NotImpP16'         ; $2008 VOLUME

	dc a2'P16SetPrefix'      ; $2009 SET_PREFIX
	dc a2'PGGetPrefix'       ; $200A GET_PREFIX

	dc a2'P16Standard'       ; $200B CLEAR_BACKUP_BIT
	dc a2'NotImpP16'         ; $200C
	dc a2'NotImpP16'         ; $200D
	dc a2'NotImpP16'         ; $200E
	dc a2'NotImpP16'         ; $200F
	dc a2'P16Open'           ; $2010 OPEN
	dc a2'P16NewLine'        ; $2011 NEWLINE
	dc a2'P16RdWr'           ; $2012 READ
	dc a2'P16RdWr'           ; $2013 WRITE
	dc a2'PGClose'           ; $2014 CLOSE
	dc a2'P16RefCommon'      ; $2015 FLUSH
	dc a2'P16RefCommon'      ; $2016 SET_MARK
	dc a2'P16RefCommon'      ; $2017 GET_MARK
	dc a2'P16RefCommon'      ; $2018 SET_EOF
	dc a2'P16RefCommon'      ; $2019 GET_EOF
	dc a2'P16SetLevel'       ; $201A SET_LEVEL
	dc a2'P16GetLevel'       ; $201B GET_LEVEL
	dc a2'P16RefCommon'      ; $201C GET_DIR_ENTRY
	dc a2'NotImpP16'         ; $201D
	dc a2'NotImpP16'         ; $201E
	dc a2'NotImpP16'         ; $201F
	dc a2'NotImpP16'         ; $2020 GET_DEV_NUM 
	dc a2'NotImpP16'         ; $2021 GET_LAST_DEV
	dc a2'NotImpP16'         ; $2022 READ_BLOCK
	dc a2'NotImpP16'         ; $2023 WRITE_BLOCK
	dc a2'NotImpP16'         ; $2024 FORMAT
	dc a2'NotImpP16'         ; $2025 ERASE_DISK
	dc a2'NotImpP16'         ; $2026
	dc a2'P16GetName'        ; $2027 GET_NAME
	dc a2'NotImpP16'         ; $2028 GET_BOOT_VOL
	dc a2'P16Quit'           ; $2029 QUIT
	dc a2'NotImpP16'         ; $202A GET_VERSION
	dc a2'NotImpP16'         ; $202B
	dc a2'NotImpP16'         ; $202C D_INFO
	dc a2'NotImpP16'         ; $202D
	dc a2'NotImpP16'         ; $202E
	dc a2'NotImpP16'         ; $202F
	dc a2'NotImpP16'         ; $2030
	dc a2'NotImpP16'         ; $2031 ALLOC_INTERRUPT
	dc a2'NotImpP16'         ; $2032 DEALLOC_INTERRUPT
	END

P16SetPrefix	START
	using KernelStruct

pfxRec	equ   10
pfxInd	equ   14

pfxpCount	equ   16
pfxpfxNum	equ   18
pfxpathname	equ   20
newPath	equ   24

	ldy 	#prefixh-CKernData   ; copy the handle of the prefix rec
	lda 	[procEnt],y          ; out of the process table
	sta 	pfxRec               ; we dereference it later
	iny2
	lda 	[procEnt],y
	sta 	pfxRec+2

	pea	1
	lda   [pBlock]
	pha                      ; pfx# for expandpath
	ldy   #4
	lda   [pBlock],y
	pha
	ldy   #2
	lda   [pBlock],y
	pha
	jsl	p16_ExpandPath	; call our C routine
	sta	pfxpathname
	stx	pfxpathname+2	; store result
	cpx	#$FFFF	; did an error occur?
	bne	noerr
	jmp	GSOSReturn

noerr	anop
	lda   [pBlock]
	inc	a
	asl   a
	asl	a
	sta   pfxInd
	tay
	iny2
	lda   [pfxRec],y
	pha
	dey2
	lda   [pfxRec],y
	pha
	ora	3,s
	bne	dodispose
	pla
	pla
	bra	nodispose

dodispose	jsl   ~NDISPOSE

nodispose	lda   [pfxpathname]
	inc   a
	inc   a
	inc   a
	pea   0
	pha
	jsl   ~NEW
	sta   newPath
	stx   newPath+2

	ldy   pfxInd             ; copy the addr of the new mem into the
	sta   [pfxRec],y         ; prefix record
	iny2
	txa
	sta   [pfxRec],y

	ph4   pfxpathname        ; copy the string
	ph4   newPath
	jsl   copygsstr
	lda   [newPath]
	inc   a
	tay
	lda   [newPath],y        ; check to see if there's a
	and   #$00FF
	cmp   #':'               ; separator at the end of the
	beq   alright
	short m                  ; prefix, and if not, then
	lda   #':'
	iny                      ; add one. Note that we alloced
	sta   [newPath],y        ; one extra byte for this possibility
	long  m
	dey
	tya
	sta   [newPath]
alright	lda   #0
	jmp   GSOSReturn
	END

P16Standard	START
oldPath	equ   10
pathName	equ   14
tmpLength	equ   18
exppath	equ   20

	lda 	[pBlock]
	sta 	oldPath
	ldy	#2
	lda 	[pBlock],y
	sta 	oldPath+2

	pea	0
	pushword #0
	pushlong oldPath
	jsl p16_ExpandPath
	cpx #$FFFF
	bne noerr
	jmp GSOSReturn
noerr	anop
	sta   exppath
	stx   exppath+2
*               ph4   exppath
	phx
	pha
	jsl   findDevice
	cmp   #$FFFF
	beq   notdevice
	lda   #$0058              ; not a block device!
	bra   goaway

notdevice	anop
	ldx   exppath+2
	lda   exppath
	ldy 	#0
	sta 	pathName
	inc 	a                    ; part of 'fix string'
	sta 	[pBlock],y
	bne 	incr
	inx
incr	txa
	iny2
	sta 	pathName+2         ; $$$ if the inx is taken, this is
	sta 	[pBlock],y         ; incorrect (pathname+2 is)
	lda 	[pathName]
	sta 	tmpLength
	xba
	sta	[pathName]
*	short m
*               ldy 	#1
*               sta 	[pathName],y
*               long  m

	ph4	pBlock
	ph2	cmdNum
	jsl	OldGSOSSt

*	movelong pBlock,pb       ; $$$ switch to stack-based
*               lda 	cmdNum
*               sta 	cn

*               jsl 	OldGSOS
*cn             dc 	i2'0'       	; set to same call # we are
*pb             dc 	i4'0'

	pha		; return any error we get
	lda 	tmpLength
	sta 	[pathName]

	lda	cmdNum
	cmp	#$0002
	beq	docff
	cmp	#$0005
	bne	noff
docff	anop
	ldx	oldPath+2
	lda	oldPath
	jsr	PcheckFF
noff	anop

	lda	oldPath
	sta	[pBlock]
	ldy	#2
	lda	oldPath+2
	sta	[pBlock],y
	pla
goaway	jmp	GSOSReturn
	END

P16Open	START
	using KernelStruct
temp1	equ   10
pathName	equ   14
tmpLength	equ   18
files	equ   20
fd	equ   24
fdptr	equ   26

	ldy   #2
	lda   [pBlock],y
	sta   temp1
	ldy   #4
	lda   [pBlock],y
	sta   temp1+2

	ldy   #openFiles-CKernData  ; copy the handle of the prefix rec
	lda   [procEnt],y        ; out of the process table
	sta   files              ; we dereference it later
	iny2
	lda   [procEnt],y
	sta   files+2

	pea	0
	ph2   #0
	ph4   temp1
	jsl   p16_ExpandPath
	cpx   #$FFFF
	bne   noerr
	jmp   GSOSReturn
noerr	anop
	ldy   #2
	sta   pathName
	inc   a
	sta   [pBlock],y
	bne   incr
	inx
incr	txa
	iny2
	sta   pathName+2
	sta   [pBlock],y

	lda   [pathName]
	sta   tmpLength
	xba
	sta   [pathName]

	movelong pBlock,pb
	lda cmdNum
	sta cn

	jsl   OldGSOS
cn	dc    i2'0'              ; set to same call # we are
pb	dc    i4'0'
	pha                      ; return any error we get
	cmp   #0
	bne   error

	lda   [pBlock]
	pha
	ph2   #0
	jsl   AddRefnum

	lda   [files]            ; inc the number of open files,
	inc   a
	sta   [files]

	pea	0	; allocate a file descriptor
	tdc
	clc	
	adc	#fd
	pha
	jsl	allocFD
	sta	fdptr
	stx	fdptr+2

; rederef the openFiles pointer
	ldy   #openFiles-CKernData
	lda   [procEnt],y
	sta   files
	iny2
	lda   [procEnt],y
	sta   files+2

	lda   [pBlock]
	ldy   #FDrefNum
	sta   [fdptr],y          ; and store the new refnum in the list
	lda   fd
	sta   [pBlock]           ; and store it
	ldy   #FDTLevel
	lda   [files],y
	ldy	#FDTLevelMode
	ora	[files],y
	ldy   #FDrefLevel
	sta   [fdptr],y          ; store file level with refnum

error	anop
	lda   tmpLength
	sta   [pathName]

	ldx	temp1+2
	lda	temp1
	jsr	PcheckFF

	ldy   #2
	lda   temp1
	sta   [pBlock],y
	ldy   #4
	lda   temp1+2
	sta   [pBlock],y
	pla
	jmp   GSOSReturn
	END

P16Quit	START
	using KernelStruct
	ldx   curProcInd
	lda   flags,x
	ora   #%00001000         ; FL_NORMTERM
	sta   flags,x

	ldy   #4                 ; push the flags word
	lda   [pBlock],y
	pha

	ldy   #2                 ; push the pathname
	lda   [pBlock],y
	pha
	ldy   #0
	lda   [pBlock],y
	pha
	ora   3,s
	beq   noPname
	jsl   p2cstr
	phx
	pha                      ; push the coverted pathname
noPname	anop
	jsl   CommonQuit
	jmp   GSOSReturn         ; only returns on error
	END

P16SetLevel	START
	using  KernelStruct
files	equ    10

	ldy   #openFiles-CKernData ; copy the handle of the prefix rec
	lda   [procEnt],y        ; out of the process table
	sta   files              ; we dereference it later
	iny2
	lda   [procEnt],y
	sta   files+2
okay	anop
	ldy   #FDTLevel
	lda   [pBlock]
	sta   [files],y
	lda	#$8000
	ldy	#FDTLevelMode
	sta	[files],y
	lda   #0
error	jmp   GSOSReturn
	END

P16GetLevel	START
	using  KernelStruct
files	equ    10

	ldy   #openFiles-CKernData ; copy the handle of the prefix rec
	lda   [procEnt],y        ; out of the process table
	sta   files              ; we dereference it later
	iny2
	lda   [procEnt],y
	sta   files+2
okay	anop
	ldy   #2
	lda   [files],y
	sta   [pBlock]
	lda   #0
error	jmp   GSOSReturn
	END

P16ChangePath	START
temp1	equ   10
temp2	equ   14
oldpathres	equ   18
newmem	equ   22

	pea	0
	pushword #0
	ldy   #2
	lda   [pBlock],y
	pha
	lda   [pBlock]
	pha                      ; push source onto stack
	jsl   p16_ExpandPath
	cpx   #$FFFF
	bne   noerr
	jmp   GSOSReturn
noerr	phx
	pha                      ; push source addr for copy
	stx   oldpathres+2
	sta   oldpathres

	pea   $0000
	lda   [oldpathres]
	clc
	adc   #2
	pha
	jsl   ~NEW
	stx   newmem+2
	sta   newmem
	stx   pathname+2
	sta   pathname

	phx
	pha                      ; push dest address

	ora   pathname+2
	beq   memoryErr

	jsl   copygsstr          ; copy the expanded path to temp

	pea	0
	pushword #0
	ldy   #6
	lda   [pBlock],y
	pha
	dey2
	lda   [pBlock],y
	pha                      ; push source onto stack
	jsl   p16_ExpandPath
	cpx   #$FFFF
	bne   noerr1
	ph4   newmem
	jsl   ~NDISPOSE
	jmp   GSOSReturn
noerr1	anop
	stx   newpathname+2
	sta   newpathname

	pushlong #chPathPB
	pea   $2004

	jsl OldGSOSSt
	pha                      ; return any error we get
	pushlong newmem
	jsl   ~NDISPOSE

interdict	anop
	pla
	jmp GSOSReturn
memoryErr	pla
	pla
	pea   $0054
	bra   interdict

chPathPB	anop
	dc    i2'2'
pathname	dc    i4'0'
newpathname	dc    i4'0'
	END

* Flush         2015      +2
* GetDirEntry   201C      +2
* GetEOF        2019      +2        tty,pipe
* GetMark       2017      +2        tty,pipe
* GetRefInfo    2039      +2
* SetEOF        2018      +2        tty,pipe
* SetMark       2016      +2        tty,pipe

P16RefCommon	START
oldRN	equ   10
fdPtr	equ	12

	lda   [pBlock]
	sta   oldRN
	pha
	cmp	#0
	bne	notzero
	lda	cmdNum	
	cmp	#$0015
	jeq	FlushSpecial
	pla
	lda	#$43
	jmp	GSOSReturn
	
notzero	jsl	getFDptr
	sta	fdPtr
	stx	fdPtr+2
	ora	fdPtr+2
	beq	refIsBad
	lda	[fdPtr]	
	cmp   #0
	bne   okay
refIsBad	lda   #$43
	jmp   GSOSReturn
okay	anop
	ldy	#FDrefType
	lda	[fdPtr],y
	cmp	#FDgsos
	beq	typeOkay
	jmp	notGSOS
typeOkay	lda	[fdPtr]
	sta   [pBlock]
	ph4   pBlock
	ph2   cmdNum
	jsl   OldGSOSSt
	pha
	lda   oldRN
	sta   [pBlock]
	pla
	jmp   GSOSReturn
notGSOS	lda	#$58
	jmp	GSOSReturn

FlushSpecial	anop
	ph4	pBlock
	ph2	cmdNum
	jsl	OldGSOSSt
	jmp	GSOSReturn
	END

P16NewLine	START
oldRN	equ   10
fdrec	equ   12
trueRN	equ   16

	lda   [pBlock]
	sta   oldRN
	pha
	jsl   getFDptr
	sta   fdrec
	stx   fdrec+2
	ora	fdrec+2
	beq	refIsBad
	ldy   #FDrefNum
	lda   [fdrec],y
	cmp   #0
	bne   okay
refIsBad	lda   #$43
	jmp   GSOSReturn
okay	anop
	sta   trueRN
	ldy   #FDrefType
	lda   [fdrec],y
	cmp   #FDpipe
	beq   nlPipe
	ldy   #FDrefNum

	lda   trueRN
	sta   [pBlock]
	ph4   pBlock
	ph2   cmdNum
	jsl   OldGSOSSt
	pha
	lda   oldRN
	sta   [pBlock]
	pla
	jmp   GSOSReturn
nlPipe	anop
	ldy   #2
	lda   [pBlock],y
	ldy   #FDNLenableMask
	sta   [fdrec],y
	beq   noerror

	ldy   #FDrefFlags
	lda   [fdrec],y
	and   #rfP16NEWL
	sta   [fdrec],y          ; tell 'em it's a P16 NewLine

	ldy   #4
	lda   [pBlock],y
	ldy   #FDNLnumChars
	sta   [fdrec],y

noerror	lda   #0
	jmp   GSOSReturn
	END

P16RdWr	START
	using KernelStruct
files	equ   10
fdrec	equ   14
rn	equ   18

	ldx   curProcInd
	lda   openFiles,x
	sta   files
	lda   openFiles+2,x
	sta   files+2
	lda   [pBlock]
	sta   rn
	pha
	jsl	getFDptr
	sta	fdrec
	stx	fdrec+2
	ora	fdrec+2
	bne	refIsOk
	lda	#$43
	jmp	GSOSReturn
refIsOk	ldy   #FDrefType
	lda   [fdrec],y
	asl	a
	tax
	jmp	(rdwrtable,X)
rdwrtable	dc  i2'doGSOS'
	dc	i2'doPipe'
	dc	i2'doTTY'
	dc	i2'doSocket'

*******************
doGSOS	anop
	ldy	#FDrefNum
	lda	[fdrec],y
	sta	[pBlock]
	ph4   pBlock
	ph2   cmdNum
	jsl   OldGSOSSt
	pha
	lda   rn
	sta   [pBlock]
	pla
	jmp   GSOSReturn

*******************
doPipe	anop
	jsl   decBusy
	ph4   pBlock
	ldy	#FDrefNum
	lda	[fdrec],y
	pha
	lda   cmdNum
	cmp   #$0012
	beq   pread
	jsl   pipeHiWrite
	bra   goback
pread	ph4   fdrec
	jsl   pipeHiRead
goback	anop
	jsl   incBusy
	jmp   GSOSReturn

*******************
doTTY	anop
	jsl   decBusy
	ldy   #6
	lda   [pBlock],y
	pha
	ldy   #4
	lda   [pBlock],y
	pha
	ldy   #2
	lda   [pBlock],y
	pha
	lda   cmdNum
	cmp   #$0012
	beq   tread
	ldy   #FDrefNum
	lda   [fdrec],y
	dec   a
	pha
	ldy	#t_write
	jsl   LineDiscDispatch
	bra   goaway2
tread	ldy   #FDrefNum
	lda   [fdrec],y
	dec   a
	pha
	ldy	#t_read
	jsl   LineDiscDispatch
goaway2	anop
	phx
	ldy	#10
	sta	[pBlock],y         ; to provide a way for TTYs to
	lda	#0
	ldy	#12
	sta	[pBlock],y	; bug! must clear hi word o'transCnt
	jsl   incBusy
	pla
	jmp   GSOSReturn

*******************
doSocket	anop
	ldy	#FDrefNum
	lda	[fdrec],y
	pha
	ph2   cmdNum
	ph4   pBlock
	jsl	SOCKrdwr
	jmp	GSOSReturn

	END

P16GetName	START
	using KernelStruct
pathptr	equ   10
resptr	equ   14
outind	equ   18
inind	equ   20
pathlen	equ   22

* Note that forked processes will get the name of their parent

	ldy   #2
	lda   [pBlock],y
	sta   resptr+2
	lda   [pBlock]
	sta   resptr

	ldx   curProcInd         ; get the full pathname of
	lda   procUserID,x       ; the process.
	pha
	pha		; space for tool call. ARGH!!!
	pha
	pea   $1
	_LGetPathname
	pl4   pathptr
	lda   [pathptr]
	and   #$00FF
	sta   pathlen
	inc   a
	tay
	short m
loop	lda   [pathptr],y
	cmp   #'/'
	beq   gotfname
	dey
	bra   loop
gotfname	long  m
	iny
	sty   inind
	lda   [pathptr]          ; get length
	sec
	sbc   inind              ; length of filename
	clc
	adc   #1                 ; minus the length word
	sta   [resptr]           ; store length of filename

	ldy   #1
	sty   outind
	short m
cploop	ldy   inind
	lda   [pathptr],y
	ldy   outind
	sta   [resptr],y
	ldy   inind
	cpy   pathlen  
	bcs   donecopy
	long  m
	inc   inind
	inc   outind
	short m
	bra   cploop
donecopy	long  m
	lda   #0
	jmp   GSOSReturn

	END
