*	$Id: gsos.asm,v 1.1 1998/02/02 08:19:23 taubert Exp $
**************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*   Derek Taubert
*
**************************************************************************
*
* GSOS.ASM
*   By Tim Meekins
*  and Jawaid Bazyar
*  and Derek Taubert (modified socket stuff)
*
* This file contains routines for patching GS/OS to intercept Shell
* calls and do special things with certain GS/OS calls.
*
* taken over by JB on 6/18/91
* 10/29/91 - extensive modifications to better support the data structures
*            of the fd (file descriptor) table
*
**************************************************************************

	copy  global.equates
	mcopy m/gsos.mac
	case	on
	copy	inc/gsos.inc
	copy	inc/tty.inc

GSOStable	START
	dc a2'GNOStandard'       ; $2001 Create
	dc a2'GNOStandard'       ; $2002 Destroy
	dc a2'NotImpGS'          ; $2003 OSShutDown
	dc a2'GNOChangePath'     ; $2004 ChangePath
	dc a2'GNOStandard'       ; $2005 SetFileInfo
	dc a2'GNOStandard'       ; $2006 GetFileInfo
	dc a2'NotImpGS'          ; $2007 JudgeName
	dc a2'NotImpGS'          ; $2008 Volume

	dc a2'GNOSetPrefix'      ; $2009 SetPrefix
	dc a2'PGGetPrefix'       ; $200A GetPrefix
	dc a2'GNOStandard'       ; $200B ClearBackupBit
	dc a2'NotImpGS'          ; $200C SetSysPrefs
	dc a2'NotImpGS'          ; $200D Null
	dc a2'GNOExpandPath'     ; $200E ExpandPath
	dc a2'NotImpGS'          ; $200F GetSysPrefs
	dc a2'GNOOpen'           ; $2010 Open
	dc a2'GNONewLine'        ; $2011 NewLine
	dc a2'GNORdWr'           ; $2012 Read
	dc a2'GNORdWr'           ; $2013 Write
	dc a2'PGClose'           ; $2014 Close
	dc a2'GNORefCommon'      ; $2015 Flush
	dc a2'GNORefCommon'      ; $2016 SetMark
	dc a2'GNORefCommon'      ; $2017 GetMark
	dc a2'GNORefCommon'      ; $2018 SetEOF
	dc a2'GNORefCommon'      ; $2019 GetEOF
	dc a2'GNOSetLevel'       ; $201A SetLevel
	dc a2'GNOGetLevel'       ; $201B GetLevel
	dc a2'GNORefCommon'      ; $201C GetDirEntry
	dc a2'NotImpGS'          ; $201D BeginSession
	dc a2'NotImpGS'          ; $201E EndSession
	dc a2'NotImpGS'          ; $201F SessionStatus
	dc a2'GNOStandard'	; $2020 GetDevNumber
	dc a2'NotImpGS'          ; $2021
	dc a2'NotImpGS'          ; $2022
	dc a2'NotImpGS'          ; $2023
	dc a2'NotImpGS'          ; $2024 Format
	dc a2'NotImpGS'          ; $2025 EraseDisk
	dc a2'NotImpGS'          ; $2026 ResetCache
	dc a2'GNOGetName'        ; $2027 GetName
	dc a2'NotImpGS'          ; $2028 GetBootVol
	dc a2'GNOQuit'           ; $2029 Quit
	dc a2'NotImpGS'          ; $202A GetVersion
	dc a2'NotImpGS'          ; $202B GetFSTInfo
	dc a2'NotImpGS'          ; $202C DInfo
	dc a2'NotImpGS'          ; $202D DStatus
	dc a2'NotImpGS'          ; $202E DControl
	dc a2'NotImpGS'          ; $202F DRead
	dc a2'NotImpGS'          ; $2030 DWrite
	dc a2'NotImpGS'          ; $2031 BindInt
	dc a2'NotImpGS'          ; $2032 UnbindInt
	dc a2'NotImpGS'          ; $2033 FSTSpecific
	dc a2'NotImpGS'          ; $2034 AddNotifyProc
	dc a2'NotImpGS'          ; $2035 DelNotifyProc
	dc a2'NotImpGS'          ; $2036 DRename
	dc a2'GNOGetStdRefNum'	; $2037 GetStdRefNum
	dc a2'NotImpGS'          ; $2038 GetRefNum
	dc a2'GNOGetRefInfo'     ; $2039 GetRefInfo
	dc a2'NotImpGS'	; $203A SetStdRefNum
	END

;
; Patch GS/OS
; This has been modified greatly to install itself through our special
; Permanent Init (PIF) file.  This will allow us to do GSBug OS call traps
; properly
;

GSOSinfo	DATA
storeHandle	dc    i4'0'
patchType	dc    i2'0'              ; 0 = no gnobug, 1 = gnobug
gnobugPath	str   '*:system:system.setup:gnobug'
	END

patchGSOS	START
	using GSOSinfo
handle	equ   0
nextHandle	equ   4
lookuid	equ   8
;
location	equ   0
attributes	equ   4
userID	equ   6
length	equ   8
last	equ   12
next	equ   16

	subroutine (0:foo),10
	pha
	ph4   #gnobugPath
	_GetUserID
	cmp   #0
	beq   okay1
	jmp   reggalar
okay1	pla
	sta   lookuid

	FindHandle #patchGSOS,handle

lb1	ldy   #last
	lda   [handle],y
	tax
	iny2
	ora   [handle],y
	beq   lb1a
	lda   [handle],y
	sta   handle+2
	stx   handle
	bra   lb1

lb1a	anop
lb2	lda   handle
	ora   handle+2
	jeq   reggalar
	ldy   #next
	lda   [handle],y
	sta   nextHandle
	iny2
	lda   [handle],y
	sta   nextHandle+2
	ldy   #userID
	lda   [handle],y
	cmp   lookuid
	beq   around1
	jmp   lb3
around1	anop
	ldy   #attributes
	lda   [handle],y
	cmp   #$C018             ; always, for PIF
	beq   around2
	jmp   lb3

lb3	mv4   nextHandle,handle
	jmp   lb2

around2	anop                     ; we've got the handle
	lda   [handle]
	sta   nextHandle
	ldy   #2
	lda   [handle],y
	sta   nextHandle+2

	lda   [nextHandle]
	sta   OldGSOS
	lda   [nextHandle],y
	sta   OldGSOS+2
	iny2
	lda   [nextHandle],y
	sta   OldGSOSSt
	iny2
	lda   [nextHandle],y
	sta   OldGSOSSt+2

	lda   #<OurGSOS
	ldy   #1
	sta   [nextHandle],y
	lda   #>OurGSOS
	iny
	sta   [nextHandle],y

	lda   #<StackGSOS
	ldy   #5
	sta   [nextHandle],y
	lda   #>StackGSOS
	iny
	sta   [nextHandle],y
	mv4   nextHandle,storeHandle
	lda   #1
	sta   patchType
	return

reggalar	anop
	pla
	stz   patchType
	mv4   >$E100A8,OldGSOS
	ld2   <OurGSOS,>$E100A8+1
	ld2   >OurGSOS,>$E100A8+2

	mv4   >$E100B0,OldGSOSSt
	ld2   <StackGSOS,>$E100B0+1
	ld2   >StackGSOS,>$E100B0+2
	return

	END

;
; UnPatch GS/OS
;
unpatchGSOS	START
	using GSOSinfo
patchPtr	equ   0
	subroutine (0:foo),4

	lda   patchType
	beq   oldType
	mv4   storeHandle,patchPtr

	ldy   #2
	lda   OldGSOS
	sta   [patchPtr]
	lda   OldGSOS+2
	sta   [patchPtr],y
	iny2
	lda   OldGSOSSt
	sta   [patchPtr],y
	iny2
	lda   OldGSOSSt+2
	sta   [patchPtr],y
	return
oldType	anop
	mv4   OldGSOS,>$E100A8
	mv4   OldGSOSSt,>$E100B0
	return
	END

OurGSOS	START
	php                      ; these six bytes are where
	phb                      ; the call info (cmdNum/pBlockPtr)
	long  ai
	phy                      ; get stored
	pha                      ;
	pha                      ; p/b reg contents
	pha                      ; y reg contents
	pha                      ; a reg contents
	lda   11,s               ; copy p,b, and y further down
	sta   5,s                ; so we can pull them off
	lda   9,s                ; before the call
	sta   3,s
	short a
	lda   15,s               ; set data bank to pBlock's
	pha                      ; bank (to use short-based)
	plb                      ; stack indirect adressing
	long  a
	ldy   #1
	lda   (13,s),y           ; cmdNum
	sta   7,s

	ldy   #5                 ; copy high word of pBlock
	lda   (13,s),y           ; pointer
	sta   11,s

	ldy   #3                 ; low word of pBlock pointer
	lda   (13,s),y
	sta   9,s
	lda   13,s
	clc
	adc   #6                 ; set rtl address to point after
	sta   13,s               ; the call information

	pla
	ply
	plb
	plp
	jsl   StackGSOS
	rtl
	END

StackGSOS	START
	using KernelStruct

	phb                      ;Save the data bank
	php                      ;Save the proc status
	long  ai
	phx
	phy
	phd
	phk
	plb
	cld
	ldx   curProcInd
	sta   exitCode,x
	lda   gsosDebug
	and   #1
	beq   noDebug
	ErrWriteChar #'('
	ErrWriteChar #'$'
	lda   12,s
	jsr   writeacc
	lda   gsosDebug
	and   #32
	beq   noDebug
	ErrWriteChar #','
	ErrWriteChar #'$'
	lda   16,s
	and   #$00FF
	jsr   writeacc
	lda   14,s
	jsr   writeacc
noDebug	anop
* check here to see if it's one of the call groups we handle
	lda   12,s
	and   #$FF00
	cmp   #0                 ; P16?
	beq   ok
	cmp   #$2000             ; GS/OS?
	beq   ok
	cmp   #$0100             ; ORCA?
	beq   ok
	pld                      ; nope, go to the real GS/OS entry
	ply                      ; point
	plx
	plp
	plb
* real stack entry point
OldGSOSSt	ENTRY
	dc    i4'0'
	brk   0
* real inline entry point
OldGSOS	ENTRY
	dc    i4'0'
	brk   0

ok	anop
	tsc
	sec
	sbc   #$1F
	tcd
	dec   a
	tcs
	lda   45
	sta   pBlock
	lda   47
	sta   pBlock+2
	lda   43
	sta   cmdNum
	lda   41
	sta   47
	lda   40
	sta   46
	lda   38
	sta   44
	lda   36
	sta   42
	lda   34
	sta   40
	lda   32
	sta   38

	jsl   incBusy
	lda   cmdNum
	and   #$FF00             ;Is it a shell command?
	cmp   #$0100
	beq   shellit
	cmp   #$0000             ; P16?
	beq   P16it
	jmp   GSOSit             ; must be GS/OS then!

P16it	anop

; create pointer to process entry for the current process

	lda   curProcInd
	clc
	adc   #|CKernData
	sta   procEnt
	lda   #^CKernData
	sta   procEnt+2
	lda   cmdNum
	and   #$00FF             ; mask off (nothing!)
	cmp   #$32+1
	bcc   rangeOK
	jmp   NotImpP16
rangeOK	anop
	asl   a
	tax
	jmp   (P16table-2,x)

GSOSit	anop
; create pointer to process entry for the current process

	lda   curProcInd
	clc
	adc   #|CKernData
	sta   procEnt
	lda   #^CKernData
	sta   procEnt+2

	lda   cmdNum
	and   #$00FF             ; mask off $2000
	cmp   #$3A+1
	bcc   range
	jmp   NotImpGS
range	anop
	asl   a
	tax
	jmp   (GSOStable-2,x)

shellit	anop
	lda   cmdNum
	and   #$40
	bne   orca2

	lda   #0
	sta   pCount
	bra   chkcmd
orca2	anop
	lda   [pBlock]
	sta   pCount
	lda   pBlock
	clc
	adc   #2
	sta   pBlock
	lda   pBlock+2
	adc   #0
	sta   pBlock+2

chkcmd	anop
	lda   cmdNum
	and   #$3F               ;Strip entry type 2 [don't ask]
	cmp   #$20	; new calls
	bcs   BadCommand

	asl   a
	tax
	jmp   (ShellCallTbl-2,x)


;  | p         pstat from before call
;  | acch
;  | accl
;  | p         pstat containing cznv result of cmp #1

;
; Not a valid command
;

BadCommand	anop
	lda   #1
; [6/29/91] removed all this duplicated code [jb]
;
; Return from a GSOS call
;
GSOSReturn	ENTRY
ShellReturn	ENTRY

	sta $1E+6                ; neat trick coming up
	tsc
	clc
	adc #$1E+6               ; so we can pull the result
	tcs
;               pla

	jsl   decBusy

;               pha
	lda   >gsosDebug
	and   #1
	beq   noDebug1
	lda   >gsosDebug
	and   #4
	beq   noDebug2
	lda   1,s
	beq   noDebug2           ; if no error, no message
	ErrWriteChar #15
	ErrWriteChar #'#'        ; error codes get a # sign
	ldy   #1
	lda   1,s                ; the error number (incognito)
	jsr   writeacc           ;
	ErrWriteChar #14
noDebug2	anop
	ErrWritechar #')'
	ErrWriteChar #13
noDebug1	anop
	pla

	pld
	ply
	plx
; insert from here
	pha
	cmp   #1
	lda	1,s
	php
	short m
	lda   1,s
	and   #%11000011
	sta   1,s
	lda   4,s
	and   #%00111100
	ora   1,s
	sta   4,s
	pla
	long  m
	lda   3,s
	xba
	sta   3,s                ; swap two bytes around
	pla
; to here
	plb                      ; modifies the fucking P!
	plp
	rtl

* Amazing how I've finally managed to merge all this code! Augh!

NotImpGS	ENTRY
NotImpP16	ENTRY
NotImp	ENTRY

	ph4   pBlock
	ph2   cmdNum
	jsl   OldGSOSSt
	jmp   GSOSReturn
	END

* New GetPrefix code
* instead of a handle, we'll use a small malloc'd record as follows:

PGGetPrefix	START
	using KernelStruct
pfxRec	equ   10
pfxString	equ   14

	ldy #prefixh-CKernData   ; copy the handle of the prefix rec
	lda [procEnt],y          ; out of the process table
	sta pfxRec               ; we dereference it later
	iny2
	lda [procEnt],y
	sta pfxRec+2

	lda   cmdNum
	and   #$FF00
	beq   ok2

	lda [pBlock]
	cmp #2
	beq ok1
	lda #$04
	jmp GSOSReturn

ok2	lda   [pBlock]
	bra   lookuppfx

ok1	ldy #2
	lda [pBlock],y

lookuppfx	anop
	inc	a
	asl   a
	asl	a
	tay
	iny2
	lda   [pfxRec],y
	sta   pfxString+2
	pha
	dey2
	lda   [pfxRec],y
	sta   pfxString
	pha

	ora	3,s
	bne	notnull
	pla
	pla
	ph4	#nullGSOS
	ld4	nullGSOS,pfxString

notnull	lda   cmdNum
	and   #$FF00
	beq   p16vers

	ldy   #6
	lda   [pBlock],y
	pha
	ldy   #4
	lda   [pBlock],y
	pha
	jsl   copygs2res
	jmp   GSOSReturn

* for P16 calls, we need to condition the returned pathname somewhat

p16vers	anop
	lda   [pfxString]
	beq	patherror	; length is zero, avoid major memtrashing
	tay
	iny
	short m

* Scan the prefix for a slash; if one is present, return a null pathname
* because after processing ':'s to '/'s the pathname would be totally
* incorrect.

slashloop	lda   [pfxString],y
	cmp   #'/'
	beq   patherror
	dey
	cpy   #2
	bcs   slashloop
	long  m
	ldy   #4
	lda   [pBlock],y
	sta   pfxString+2
	pha
	ldy   #2
	lda   [pBlock],y
	sta   pfxString
	pha
	jsl   copygs2p
	lda   [pfxString]
	and   #$00FF
	tay
	short m
fixloop	lda   [pfxString],y
	cmp   #':'
	bne   nofix
	lda   #'/'
	sta   [pfxString],y
nofix	dey
	bne   fixloop
	long  m

	lda   #0
	jmp   GSOSReturn
patherror	long  m
	pla                      ; remove input params
	pla                      ; store a zero as length,
	ldy   #4                 ; and don't return an error
	lda   [pBlock],y
	sta   pfxString+2
	ldy   #2
	lda   [pBlock],y
	sta   pfxString
	lda   #0
	short m
	sta   [pfxString]
	long  m
	jmp   GSOSReturn
nullGSOS	dc	i2'0'
	END

GNOSetPrefix	START
	using KernelStruct
pfxRec	equ   10
pfxInd	equ   14

pfxpCount	equ   16
pfxpfxNum	equ   18
pfxpathname	equ   20
newPath	equ   24

	ldy #prefixh-CKernData   ; copy the handle of the prefix rec
	lda [procEnt],y          ; out of the process table
	sta pfxRec               ; we dereference it later
	iny2
	lda [procEnt],y
	sta pfxRec+2

	lda [pBlock]
	cmp #2
	beq ok1
	lda #$04
	jmp GSOSReturn

ok1	anop
	pea	1	; don't do the named pfx expansion
	ldy   #2                 ; push the prefix number
	lda   [pBlock],y
	pha
	ldy   #6                 ; push the path to be expanded
	lda   [pBlock],y
	pha
	ldy   #4
	lda   [pBlock],y
	pha
	jsl gno_ExpandPath       ; call our C routine
	sta pfxpathname
	stx pfxpathname+2        ; store result
	cpx #$FFFF               ; did an error occur?
	bne noerr
	jmp GSOSReturn

noerr	anop
	ldy   #2
	lda   [pBlock],y
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

GNOChangePath	START
temp1	equ   10
temp2	equ   14
oldpathres	equ   18
newmem	equ   22

	ldy   #2
	lda   [pBlock],y
	sta   temp1
	iny2
	lda   [pBlock],y
	sta   temp1+2
	iny2
	lda   [pBlock],y
	sta   temp2
	iny2
	lda   [pBlock],y
	sta   temp2+2

	pea	0
	pea	0
	pushlong temp1
	jsl gno_ExpandPath
	cpx #$FFFF
	bne noerr
	jmp GSOSReturn
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

	phx
	pha                      ; push dest address

	ldy   #2
	sta   [pBlock],y
	iny2
	txa
	sta   [pBlock],y
	dey2
	ora   [pBlock],y
	beq   memoryErr

	jsl   copygsstr          ; copy the expanded path to temp

	pea	0
	pushword #0
	pushlong temp2
	jsl   gno_ExpandPath
	cpx   #$FFFF
	bne   noerr1
	ph4   newmem
	jsl   ~NDISPOSE
	jmp   GSOSReturn
noerr1	anop
	ldy   #6
	sta   [pBlock],y
	iny2
	txa
	sta   [pBlock],y

	movelong pBlock,pb
	lda   cmdNum
	sta   cn

	jsl OldGSOS
cn	dc    i2'0'              ; set to same call # we are
pb	dc    i4'0'
	pha                      ; return any error we get
	pushlong newmem
	jsl   ~NDISPOSE

interdict	anop
	ldy   #2
	lda   temp1
	sta   [pBlock],y
	iny2
	lda   temp1+2
	sta   [pBlock],y
	iny2
	lda   temp2
	sta   [pBlock],y
	iny2
	lda   temp2+2
	sta   [pBlock],y
	pla
	jmp GSOSReturn
memoryErr	pla
	pla
	pea   $0054
	bra   interdict
	END

GNOStandard	START
oldpath	equ   10
exppath	equ   14

	ldy #2
	lda [pBlock],y
	sta oldpath
	iny2
	lda [pBlock],y
	sta oldpath+2

	pea	0
	pushword #0
	pushlong oldpath
	jsl gno_ExpandPath
	cpx #$FFFF
	bne noerr
	jmp GSOSReturn
noerr	anop
	sta   exppath
	stx   exppath+2
	ph4   exppath
	jsl   findDevice
	cmp   #$FFFF
	beq   notdevice
	lda   #$0058              ; not a block device!
	bra   goaway

notdevice	anop
	ldy   #2
	lda   exppath
	sta   [pBlock],y
	ldy   #4
	lda   exppath+2
	sta   [pBlock],y

	ph4   pBlock
	ph2   cmdNum
	jsl   OldGSOSSt

	pha                      ; return any error we get

	lda	cmdNum
	cmp	#$2002
	beq	docff
	cmp	#$2005
	bne	noff
docff	anop
	ldx	oldpath+2
	lda	oldpath
	jsr	checkFF	

noff	anop
	ldy   #2
	lda   oldpath
	sta   [pBlock],y
	ldy   #4
	lda   oldpath+2
	sta   [pBlock],y
	pla
goaway	jmp   GSOSReturn
	END

checkFF	START
	using	KernelStruct

	tay

	lda   gsosDebug	; don't print the path twice
	pha
	stz	gsosDebug

	pea	1
	pea	0
	phx		; pathname pointer
	phy	
	jsl	gno_ExpandPath	; expand it again, but this time
	phx		; pointer to expanded pathname
	pha
	jsl	FindFF
	phx		; pointer to ff entry, if any
	pha
	ora	3,s	; null pointer means not in FF
	beq	noFF
	jsl	DeleteFF

goaway	pla
	sta	gsosDebug

	rts
noFF	pla
	pla
	bra	goaway
	END

PcheckFF	START
	using	KernelStruct

	tay

	lda   gsosDebug	; don't print the path twice
	pha
	stz	gsosDebug

	pea	1
	pea	0
	phx		; pathname pointer
	phy	
	jsl	p16_ExpandPath	; expand it again, but this time
	phx		; pointer to expanded pathname
	pha
	jsl	FindFF
	phx		; pointer to ff entry, if any
	pha
	ora	3,s	; null pointer means not in FF
	beq	noFF
	jsl	DeleteFF

goaway	pla
	sta	gsosDebug

	rts
noFF	pla
	pla
	bra	goaway
	END


GNOOpen	START
	using KernelStruct
temp1	equ   10
files	equ   14
fd	equ   18
fdPtr	equ   20
ttyn	equ   24

* Save the old pathname so we can restore before we exit

	ldy   #4
	lda   [pBlock],y
	sta   temp1
	iny2
	lda   [pBlock],y
	sta   temp1+2

	ldy   #openFiles-CKernData ; copy the handle of the prefix rec
	lda   [procEnt],y        ; out of the process table
	sta   files              ; we dereference it later
	iny2
	lda   [procEnt],y
	sta   files+2
	pea	0
	pushword #0
	pushlong temp1
	jsl   gno_ExpandPath
	cpx   #$FFFF
	bne   noerr
	jmp   GSOSReturn
noerr	anop
	phx
	pha
	phx
	pha
	jsl   findDevice
	cmp   #$FFFF
	jne   openTTY

	pla
	plx
	ldy   #4                 ; forgetting this is B A D (baaaad)
	sta   [pBlock],y
	txa
	iny2
	sta   [pBlock],y
	movelong pBlock,pb
	lda   cmdNum
	sta   cn

	ldy	#FDTLevel	; open the file with the proper
	lda	[files],y	; GS/OS level
	sta	dolevel+2
	ldy	#FDTLevelMode
	lda	[files],y
	sta	dolevel+4
	ph4	#dolevel
	pea	$201A	; SetLevelGS
	jsl	OldGSOSSt

	jsl   OldGSOS
cn	dc    i2'0'              ; set to same call # we are
pb	dc    i4'0'
	pha
	cmp   #0
	jne   error

; alloc a process file entry ONLY if GS/OS opened the file okay

	lda   [files]            ; inc the number of open files,
	inc   a	; allocFD does not do this
	sta   [files]

	pea	0                  ; push the (dp) address of
	tdc		; our local 'fd'
	clc
	adc	#fd
	pha
	jsl	allocFD
	sta	fdPtr
	stx	fdPtr+2

	ldy   #openFiles-CKernData
	lda   [procEnt],y
	sta   files
	iny2
	lda   [procEnt],y
	sta   files+2

	ldy   #2
	lda   [pBlock],y
	pha
	pea	FDgsos
	jsl   AddRefnum          ; add to our tracking table

	ldy   #2                 ; refNum returned by open
	lda   [pBlock],y
	ldy   #FDrefNum
	sta   [fdPtr],y          ; and store the new refnum in the list

	lda   fd                 ; retreive the fd number and
	ldy   #2                 ; return it in the pBlock
	sta   [pBlock],y
	ldy   #FDTLevel
	lda   [files],y
	ldy	#FDTLevelMode      ; handle internal levels
	ora	[files],y
	ldy   #FDrefLevel
	sta   [fdPtr],y          ; store file level with refnum

	lda   #0
	sta   1,s                ; no error! trick the stack
error	anop

	ldx	temp1+2
	lda	temp1
	jsr	checkFF

	ldy   #4
	lda   temp1
	sta   [pBlock],y
	iny2
	lda   temp1+2
	sta   [pBlock],y
	pla
	jmp   GSOSReturn
* this should be common between P16/GSOS
openTTY	anop
	inc   a
	sta   ttyn
	pla
	pla                      ; remove junk

	pei	(ttyn)
	pea   FDtty              ; we're looking for this type
	jsl   FindRefnum
	sta   fdPtr
	stx   fdPtr+2            ; if fdptr==NULL, then there are
	ora   fdPtr+2            ; no references to this TTY
	bne   notfirst

* create a reference to this TTY
	pei   (ttyn)
	pea   FDtty
	jsl   AddRefnum

* since this is the first open on this TTY, reassign this process' controlling
* tty to this terminal.  Also, make sure the terminal's process group is
* 0 (but not the process'- this allows the real controlling terminal to
* signal the process)
	
	ldx	curProcInd
	lda	ttyn
	dec	a
	sta	ttyID,x
	lda	ttyn
	dec	a
	asl	a
	tax
	stz	ttyStruct,x

* here, call the Init routine in the Device Driver
	lda   ttyn
	dec   a                  ; 0-n device number
	pha
	ldy	#t_open
	jsl   LineDiscDispatch
	cmp	#0
	bne	iserr	; open failed!
	bra   part2
notfirst	anop
* should check here for exclusive access bit. If set, then the
* open call will fail (before we change any counts)
	lda	ttyn
	jsl	checkExclTTY
	cpy	#0
	beq	not_excl
	lda	#$50
	bra	iserr99	; do NOT DecRefNum - we never inc'd it
iserr	pha
	pei	(ttyn)
	pea	FDtty
	jsl	DecRefnum	clean up after ourselves
	pla
iserr99	jmp	GSOSReturn	; the file is already open!

not_excl	pei   (ttyn)
	pea   FDtty
	jsl   IncRefnum          ; tell 'em we're here to party!

part2	anop                     ; no errors- 
	pea   0
	tdc
	clc
	adc   #fd
	pha                      ; push address of 'fd' (dp)
	jsl   allocFD
	sta   fdPtr
	stx   fdPtr+2
	ora   fdPtr+2
	bne   noerror9

	lda	#$42
	bra	iserr	; exit with error & cleanup

noerror9	anop
; re-dereference files since allocFD can modify it
	ldy   #openFiles-CKernData ; copy the handle of the prefix rec
	lda   [procEnt],y        ; out of the process table
	sta   files              ; we dereference it later
	iny2
	lda   [procEnt],y
	sta   files+2

	ldy   #FDrefNum
	lda   ttyn
	sta   [fdPtr],y          ; store device number
	ldy   #FDrefType
	lda   #FDtty
	sta   [fdPtr],y          ; and file descriptor type
	ldy   #FDTLevel
	lda   [files],y          ; and the file level
	ldy   #FDrefLevel
	sta   [fdPtr],y
	lda   #0
	ldy   #FDrefFlags        ; no flags for TTYs yet!
	sta   [fdPtr],y
	ldy   #FDNLenableMask    ; make sure newline mode is off
	sta   [fdPtr],y

	ldy   #FDTCount
	lda   [files],y
	inc   a
	sta   [files],y

	lda   fd                 ; retreive the fd number and
	ldy   #2                 ; return it in the pBlock
	sta   [pBlock],y
	lda   [pBlock]
	cmp   #5
	bcc   ttybyebye
	ldy   #8
	lda   [pBlock],y
	bne   storeAccess
	lda   #3
storeAccess	ldy   #12
	sta   [pBlock],y

ttybyebye	lda   #0
	jmp   GSOSReturn

dolevel	dc	i2'2'
	dc	i2'0'
	dc	i2'0'
	END

GNOSetLevel	START
	using  KernelStruct
files	equ    10

	ldy   #openFiles-CKernData ; copy the handle of the prefix rec
	lda   [procEnt],y        ; out of the process table
	sta   files              ; we dereference it later
	iny2
	lda   [procEnt],y
	sta   files+2
	lda   [pBlock]
	cmp   #1
	beq   okay
	cmp	#2
	beq	okay1
	lda   #$04
	bra   error
okay	anop
	pea	$8000	; default to 'user' mode
	bra	okay2
okay1	anop
	ldy	#4	; use the mode they passed us
	lda	[pBlock],y	
	and	#$8000
	pha

okay2	lda	1,s
	bit	#$8000
	beq	okay3
	ldy	#2
	lda	[pBlock],y
	bit	#$8000
	beq	okay3
	pla
	lda	#$59	
	jmp	GSOSReturn

okay3	ldy   #2
	lda   [pBlock],y
	ldy   #FDTLevel
	sta   [files],y
	pla
	ldy	#2	; calculate the real file level
	eor	[pBlock],y
	and	#$8000
	ldy	#FDTLevelMode
	sta	[files],y

	lda   #0
error	jmp   GSOSReturn
	END

GNOGetLevel	START
	using  KernelStruct
files	equ    10

	ldy   #openFiles-CKernData ; copy the handle of the prefix rec
	lda   [procEnt],y        ; out of the process table
	sta   files              ; we dereference it later
	iny2
	lda   [procEnt],y
	sta   files+2
	lda   [pBlock]
	cmp   #1
	beq   okay
	cmp	#2
	beq	okay1
	lda   #$04
	bra   error
okay	anop
	pea	$8000
	bra	okay2
okay1	anop
	ldy	#4
	lda	[pBlock],y	; levelmode is an input
	and	#$8000
	pha
okay2	ldy   #FDTLevel
	lda   [files],y
	ldy	#FDTLevelMode
	ora	[files],y
	eor	1,s
	ldy   #2
	sta   [pBlock],y
	pla
	lda   #0
error	jmp   GSOSReturn
	END

GNOExpandPath	START                    ; not to be confused with gno_EP
gep_flags	equ 10
outPath	equ 12
pathName	equ 16
in_ind	equ 20
out_ind	equ 22

	lda   #0
	sta   gep_flags
	lda   [pBlock]
	cmp   #2
	bcs   okay
	lda   #4
	jmp   GSOSReturn
okay	anop
	beq   okay1
	ldy   #$0A
	lda   [pBlock],y
	sta   gep_flags
okay1	anop
	ldy   #6
	lda   [pBlock],y
	sta   outPath
	iny2
	lda   [pBlock],y
	sta   outPath+2

	pea	1
	pea   $0000              ; prefix 0
	ldy   #4                 ; push the old pathname ptr
	lda   [pBlock],y
	pha
	dey
	dey
	lda   [pBlock],y         ; on the stack and call C expandpath
	pha
	jsl   gno_ExpandPath     ; ha!
	cpx   #$FFFF
	bne   noerr
	jmp   GSOSReturn
noerr	anop
	sta   pathName
	stx   pathName+2
	ldy   #6
	lda   [pBlock],y
	sta   outPath
	iny2
	lda   [pBlock],y
	sta   outPath+2

	lda   [pathName]
	clc
	adc	#4
	cmp   [outPath]
	beq   okay2
	bcc   okay2
	ldy   #2
	sta   [outPath],y
	lda   #$4F
	jmp   GSOSReturn
okay2	anop

	lda   [pathName]
	ldy   #2
	sta   [outPath],y
	tax                      ; count down with X reg
	beq   noCopy
	sty   in_ind
	lda   #4
	sta   out_ind

	short m
loop	anop
	ldy   in_ind
	lda   [pathName],y
	bit   gep_flags+1        ; hi bit into 'N' preg bit
	bpl   noupper
	cmp   #'a'
	bcc   noupper
	cmp   #'z'+1
	bcs   noupper
	sec
	sbc   #'a'-'A'
noupper	anop
	ldy   out_ind
	sta   [outPath],y
	long  m
	inc   in_ind
	inc   out_ind
	short m
	dex
	bne   loop

noCopy	anop
	long  m
	lda   #0
	jmp   GSOSReturn
	END

GNOQuit	START
	using KernelStruct
	ldx   curProcInd
	lda   flags,x
	ora   #%00001000         ; FL_NORMTERM
	sta   flags,x
	lda   [pBlock]
	cmp   #2
	bcs   doFlags
	pea   0
	bra   noFlags
doFlags	ldy   #6
	lda   [pBlock],y
	pha
noFlags	lda   [pBlock]
	cmp   #1
	bcs   doPname
	pea   0
	pea   0
	bra   noPname
doPname	ldy   #4
	lda   [pBlock],y
	pha
	ldy   #2
	lda   [pBlock],y
	pha
	ora   3,s
	beq   noPname
	jsl   gs2cstr
	phx
	pha                      ; push the coverted pathname
noPname	anop
	jsl   CommonQuit
	jmp   GSOSReturn         ; only returns on error
	END

* Flush         2015      +2
* GetDirEntry   201C      +2
* GetEOF        2019      +2        tty,pipe
* GetMark       2017      +2        tty,pipe
* GetRefInfo    2039      +2
* SetEOF        2018      +2        tty,pipe
* SetMark       2016      +2        tty,pipe

getFDptr	START 
	using KernelStruct
files	equ   0

	subroutine (2:rn),4
	lda	rn
	cmp	#32	; max # of open files
	bcs	invalidRN

	ldx   curProcInd
	lda   openFiles,x
	sta   files
	lda   openFiles+2,x
	sta   files+2
	lda   rn
	asl   a
	asl   a
	asl   a
	asl   a
	sec
	sbc   #FDsize-FDTTable
	clc
	adc   files
	sta   files
	lda   files+2
	adc   #0
	sta   files+2
goaway	return 4:files
invalidRN	stz   files
	stz	files+2
	bra   goaway
	END

mapRN	START
	using KernelStruct
files	equ   0
retval	equ   4
	subroutine (2:rn),6

	lda	rn
	cmp	#32
	bcs	invalidRN

	ldx   curProcInd
	lda   openFiles,x
	sta   files
	lda   openFiles+2,x
	sta   files+2
	lda   rn
	asl   a
	asl   a
	asl   a
	asl   a
	sec
	sbc   #FDsize-FDTTable
	tay
	lda   [files],y
	sta   retval
goaway	return 2:retval
invalidRN	stz   retval
	bra	goaway
	END

* search the specified file descriptor table for the physical
* refnum specified.  Returns 0 if the file wasn't found, or
* returns the refnum (index into table)

SearchFDTable	START
rn	equ	0
	
	subroutine (4:fdptr,2:refnum),2
	lda	#0
	sta	rn

loop	asl	a
	asl	a
	asl	a
	asl	a
	clc
	adc	#FDTTable	; don't forget the 6 bytes on top
	tay
	lda	[fdptr],y
	cmp	refnum
	bne	nxt
	iny
	iny
	lda	[fdptr],y
	cmp	#FDgsos
	beq	gotit

nxt	inc	rn
	lda	rn
	cmp	#32
	bcc	loop	

	stz	rn	; search failed, return 0
	bra	byebye
gotit	inc	rn	; rn + 1 is actual refnum
byebye	return 2:rn
	END

GNOGetRefInfo	START

oldRN	equ   10
fdPtr	equ   12

	ldy   #2
	lda   [pBlock],y
	beq	badRef	; refnum 0 is BAD
	sta   oldRN
	pha
	jsl	getFDptr
	sta	fdPtr
	stx	fdPtr+2
	ora	fdPtr+2
	beq	badRef	
	lda	[fdPtr]	
	cmp   #0
	bne   okay
badRef	lda   #$43
	jmp   GSOSReturn
okay	anop
	ldy	#FDrefType
	lda	[fdPtr],y
	cmp	#FDgsos
	beq	typeOkay
	cmp	#FDpipe
	beq	notGSOS

	ldy	#FDrefNum
	lda	[fdPtr],y
	dec	a
	asl	a
	asl	a
	tax
	
	lda	#3
	ldy	#4
	sta	[pBlock],y	

	lda	[pBlock]
	cmp	#3
	bcc	gohome

	lda	>DeviceNames+2,x
	pha
	lda	>DeviceNames,x
	pha
	ldy	#8
	lda	[pBlock],y
	pha
	ldy	#6
	lda	[pBlock],y
	pha
	jsl	copygs2res
gohome	lda	#0
	jmp	GSOSReturn	; basically ignore the call. no harm
typeOkay	lda	[fdPtr]
	ldy   #2
	sta   [pBlock],y
	ph4   pBlock
	ph2   cmdNum
	jsl   OldGSOSSt
	pha
	lda   oldRN
	ldy   #2
	sta   [pBlock],y
	pla
	jmp   GSOSReturn
notGSOS	lda	#$58
	jmp	GSOSReturn
	END

GNORefCommon	START
oldRN	equ   10
fdPtr	equ	12

	ldy   #2
	lda   [pBlock],y
	sta   oldRN
	pha
	bne	notzero
	lda	cmdNum
	cmp	#$2015
	jeq	FlushSpecial
	pla
	lda	#$43
	jmp	GSOSReturn

notzero	jsl	getFDptr
	sta	fdPtr
	stx	fdPtr+2
	lda	[fdPtr]	
	cmp   #0
	bne   okay
	lda   #$43
	jmp   GSOSReturn
okay	anop
	ldy	#FDrefType
	lda	[fdPtr],y
	cmp	#FDgsos
	beq	typeOkay
	jmp	notGSOS
	cmp	#FDpipe
	bne	notGSOS
	lda	#0
	jmp	GSOSReturn	; basically ignore the call. no harm
typeOkay	lda	[fdPtr]
	ldy   #2
	sta   [pBlock],y
	ph4   pBlock
	ph2   cmdNum
	jsl   OldGSOSSt
	pha
	lda   oldRN
	ldy   #2
	sta   [pBlock],y
	pla
	jmp   GSOSReturn
notGSOS	lda	#$58
	jmp	GSOSReturn

* Special case this: Flush() is the only routine here which can accept
* zero as a legal refnum; it should flush all open files, and we pass
* the call _unmodified_ into GS/OS for this reason.

FlushSpecial	anop
	pla
	ph4	pBlock
	ph2	cmdNum
	jsl	OldGSOSSt
	jmp	GSOSReturn
	END

GNONewLine	START
oldRN	equ   10
fdrec	equ   12
trueRN	equ   16

	ldy   #2
	lda   [pBlock],y
	sta   oldRN
	pha
	jsl   getFDptr
	sta   fdrec
	stx   fdrec+2
	ora	fdrec+2
	beq	badRef	; bad refnum
	ldy   #FDrefNum
	lda   [fdrec],y
	cmp   #0
	bne   okay
badRef	lda   #$43
	jmp   GSOSReturn
okay	anop
	sta   trueRN
	ldy   #FDrefType
	lda   [fdrec],y
	cmp   #FDgsos
	bne   nlPipe

	lda   trueRN
	ldy   #2
	sta   [pBlock],y
	ph4   pBlock
	ph2   cmdNum
	jsl   OldGSOSSt
	pha
	lda   oldRN
	ldy   #2
	sta   [pBlock],y
	pla
	jmp   GSOSReturn
nlPipe	ldy   #4
	lda   [pBlock],y
	ldy   #FDNLenableMask
	sta   [fdrec],y
	beq   noerror
	ldy   #6
	lda   [pBlock],y
	ldy   #FDNLnumChars
	sta   [fdrec],y
	cmp   #1
	bcc   doerror
	cmp   #256+1
	bcs   doerror

	ldy   #8
	lda   [pBlock],y
	tax
	iny2
	lda   [pBlock],y
	ldy   #FDNLtable+2
	sta   [fdrec],y
	dey2
	txa
	sta   [fdrec],y
	ldy   #FDrefFlags
	lda   [fdrec],y
	and   #%1111111111110111 ; ~rfP16NEWL
	sta   [fdrec],y

noerror	lda   #0
	jmp   GSOSReturn
doerror	ldy   #FDNLenableMask
	lda   #0
	sta   [fdrec],y
	lda   #$53
	jmp   GSOSReturn
	END

GNORdWr	START
	using KernelStruct
files	equ   10
fdrec	equ   14
rn	equ   18

	ldx   curProcInd
	lda   openFiles,x
	sta   files
	lda   openFiles+2,x
	sta   files+2
	ldy	#2
	lda	[pBlock],y
	pha	
	sta	rn
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
	ldy	#2
	sta	[pBlock],y
	ph4   pBlock
	ph2   cmdNum
	jsl   OldGSOSSt
	pha
	ldy	#2
	lda   rn
	sta   [pBlock],y
	pla
	jmp   GSOSReturn

*******************
doPipe	anop
	jsl   decBusy
;              ph4   (pBlock)+2
	lda   pBlock
	clc
	adc   #2
	tay
	lda   pBlock+2
	adc   #0
	pha
	phy
	ldy	#FDrefNum
	lda	[fdrec],y
	pha
	lda   cmdNum
	cmp   #$2012
	beq   pread
	jsl   pipeHiWrite
	bra   goback
pread	ph4   fdrec              ; we need this for newline
	jsl   pipeHiRead
goback	anop
	jsl   incBusy 
	jmp   GSOSReturn

*******************
doTTY	anop
	jsl   decBusy
	ldy   #8
	lda   [pBlock],y
	pha
	ldy   #6
	lda   [pBlock],y
	pha
	ldy   #4
	lda   [pBlock],y
	pha
	lda   cmdNum
	cmp   #$2012
	beq   tread
	ldy   #FDrefNum          ; tty devId number
	lda   [fdrec],y
	dec   a
	pha
	ldy	#t_write
	jsl   LineDiscDispatch
	bra   goaway2
tread	ldy   #FDrefNum          ; the tty device ID number
	lda   [fdrec],y
	dec   a
	pha
	ldy	#t_read
	jsl   LineDiscDispatch
goaway2	anop
	phx
	ldy	#12
	sta	[pBlock],y         ; to provide a way for TTYs to
	lda	#0	; return EOF properly
	ldy	#14
	sta	[pBlock],y
	jsl   incBusy
	pla
	jmp   GSOSReturn

*******************
doSocket	anop
	ldy	#FDrefNum
	lda	[fdrec],y
	pha
	ph2   cmdNum
;               ph4   pBlock
	pei	(pBlock+2)
	lda	pBlock
	inc	a
	inc	a
	pha
	jsl	SOCKrdwr
	jmp	GSOSReturn

	END

PGClose	START
	using KernelStruct

rn	equ   10
files	equ   12
fdptr	equ   16
errno	equ   20
cnt	equ   22
level	equ	24

	stz   errno
	ldy   #openFiles-CKernData ; get the address of the fd table
	lda   [procEnt],y
	sta   files
	iny2
	lda   [procEnt],y
	sta   files+2

	ldy   #0
	lda   cmdNum
	cmp   #$0014             ; P16 refNum is at offset 0
	beq   whichOS
	lda   [pBlock]
	cmp   #1
	beq   pCountOkay
	lda   #$04
	jmp   GSOSReturn
pCountOkay	ldy   #2                 ; GSOS refNum is at offset 2
whichOS	lda   [pBlock],y
	sta   rn
	beq   doClose0
	pha
	jsl	getFDptr
	stx	fdptr+2
	sta	fdptr
	ora	fdptr+2
	beq	refIsBad

	ph4	fdptr
	jsl   subClose
	cmp   #$43
	beq   dontdecr
	pha
	lda   [files]            ; decr the # of open files
	dec   a
	sta   [files]
	pla
dontdecr	jmp	GSOSReturn
refIsBad	lda	#$43
	jmp   GSOSReturn
doClose0	anop
	lda   files
	clc
	adc   #FDTTable
	sta   fdptr
	lda   files+2
	adc   #0
	sta   fdptr+2

	ldy	#FDTLevel	; calculate the actual
	lda	[files],y	; system level from the
	ldy	#FDTLevelMode	; user level setting
	ora	[files],y	; and the levelMode
	sta	level

	lda   [files]            ; number of entries
	sta   cnt
loop	anop
	lda   cnt
	beq   doneClose0
	lda   [fdptr]            ; zero refNum means empty entry
	beq   nextEntry

;	ldy	#FDTLevel	; handle 'internal' file levels
;	lda	[files],y	; like System 6
;	ldy	#FDTLevelMode
;	ora	[files],y
;	ldy	#FDrefLevel
;	cmp	[fdptr],y
;	bcs	noclose

	lda   level	; our calculated level
	ldy   #FDTLevel
	cmp   [files],y          ; 'global' file level
	bcc   noclose

	ph4   fdptr
	jsl   subClose
	cmp   #0
	beq   noerror
	sta   errno
noerror	anop
	lda   [files]            ; decr the # of open files
	dec   a
	sta   [files]
noclose	dec   cnt
nextEntry	lda   fdptr
	clc
	adc   #FDsize
	sta   fdptr
	lda   fdptr+2
	adc   #0
	sta   fdptr+2
	bra loop
doneClose0	lda   errno
	jmp   GSOSReturn
	END

subClose	START
CLpCount	equ   0
CLrefNum	equ   2
err	equ   4

	subroutine (4:fdptr),6
	stz   err
	ldy   #FDrefNum
	lda   [fdptr],y
	bne   validRN
	lda   #$43
	sta   err
	jmp   errorRet

validRN	anop
	ldy   #FDrefType
	lda   [fdptr],y

	asl	a
	tax
	jmp	(closetable,X)

closetable	dc  i2'subGSOS'
	dc	i2'subPipe'
	dc	i2'subTTY'
	dc	i2'subSocket'

*******************
subSocket	anop
	ldy   #FDrefNum
	lda   [fdptr],y
	pha
	pea   FDsocket
	jsl   DecRefnum
	cmp   #0	; there are still more references
	bne   subSock1	; to this file, don't close it!
	ldy   #FDrefNum
	lda   [fdptr],y
	pha
	jsl	SOCKclose
	sta	err
subSock1	jmp	goaway

*******************
subGSOS	anop
	ldy   #FDrefNum
	lda   [fdptr],y
	sta   CLrefNum
	pha

	lda   #1
	sta   CLpCount
	pea   FDgsos             ; we know refType == gsos
	jsl   DecRefnum
	cmp   #0                 ; there are still more references
	bne   goaway             ; to this file, don't close it!

	pea   $0000              ; stack is always in bank 0!
	phd
	pea   $2014              ; command number
	jsl   OldGSOSSt
	sta   err
	jmp   goaway

*******************
subPipe	anop

	ldy   #FDrefNum
	lda   [fdptr],y
	pha
	ldy   #FDrefFlags
	lda   [fdptr],y
	pha
	jsl   decPipe
	ldy   #FDrefNum
	lda   [fdptr],y
	pha
	pea   FDpipe
	jsl   DecRefnum
	cmp   #0
	bne   goaway
	ldy   #FDrefNum
	lda   [fdptr],y
	pha
	jsl   disposePipe

goaway	anop
	lda   #0
	ldy   #FDrefNum
	sta   [fdptr],y
	ldy   #FDrefType
	sta   [fdptr],y

errorRet	anop
	return 2:err

*******************
subTTY	anop
	ldy   #FDrefNum
	lda   [fdptr],y
	pha
	pea   FDtty
	jsl   DecRefnum
	cmp   #0
	bne   goaway

* Now, adjust the process group the terminal was in
	ldy	#FDrefNum
	lda	[fdptr],y
	dec	a
	asl	a
	tax
	lda	ttyStruct,x
	beq	nopgrp	; no pgrp was set
	dec	a	; adjust, since pgrps start at
	dec	a	; number 2
	asl	a
	tay
	lda	#0	; zero the pgrp
	sta	ttyStruct,x
	tyx
	lda   pgrpInfo,x	; decrement the pgrp count
	dec	a
	sta	pgrpInfo,x

nopgrp	ldy   #FDrefNum
	lda   [fdptr],y
	dec   a
	pha
	ldy	#t_close	; shutdown the device driver
	jsl   LineDiscDispatch

	bra   goaway

	END

GNOGetName	START
	using KernelStruct
pathptr	equ   10
resptr	equ   14
outind	equ   18
inind	equ   20
pathlen	equ   22

* Note that forked processes will get the name of their parent
	lda   [pBlock]
	beq   pcerr
	cmp   #3
	bcs   pcerr
	bra	goaround
pcerr	lda   #4
	jmp   GSOSReturn

goaround	anop
	ldy   #4
	lda   [pBlock],y
	sta   resptr+2
	ldy   #2
	lda   [pBlock],y
	sta   resptr

	ldx   curProcInd         ; get the full pathname of
	lda   procUserID,x       ; the process.
	pha
	pha		; space for tool call. ARGH!!!
	pha
	pea   $1
	_LGetPathname2
	pl4   pathptr
	lda   [pathptr]
	inc   a
	sta   pathlen
	tay
	short m
loop	lda   [pathptr],y
	cmp   #':'
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
	adc   #2                 ; minus the length word
	cmp   [resptr]           ; length of buffer?
	beq	okay	; <= is okay
	bcs   buffer2small
okay	anop
	ldy   #2
	sta   [resptr],y         ; store length of filename

	ldy   #4
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
	lda   [pBlock]
	cmp   #1
	beq   done
	ldx   curProcInd
	lda   procUserID,x
	ldy   #6
	sta   [pBlock],y
done	lda   #0
	jmp   GSOSReturn

buffer2small	ldy   #2
	sta   [resptr],y
	lda   #$4F
	jmp   GSOSReturn         ; buffer too small error
	END

GNOGetStdRefNum	START
	lda	[pBlock]
	cmp	#2
	beq	PCokay
	lda	#4
	jmp	GSOSReturn

PCokay	ldy	#2
	lda	[pBlock],y
	cmp	#10
	bcc	pfxerr
	cmp	#13
	bcs	pfxerr

	sec
	sbc	#9
	ldy	#4
	sta	[pBlock],y
	lda	#0
	jmp	GSOSReturn
pfxerr	lda	#$53
	jmp	GSOSReturn
	END
