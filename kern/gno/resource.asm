*	$Id: resource.asm,v 1.1 1998/02/02 08:19:44 taubert Exp $
**************************************************************************
*
* RESOURCE.ASM
*
*   v1.0 (11/17/91)
*   by Jawaid Bazyar
*
*   This file patches the Resource Manager to take proper care of
*   refNum's/FDs in our new environment, and also keeps track of the
*   CurResourceApp value- it makes sure that the Resource Manager
*   always has correct info about the App making a call
*
**************************************************************************

	case	on
	mcopy	m/resource.mac

OldRMtable	DATA
oldOpenFiles	dc  i4'0'
rmStackPtr	dc  i2'0'
rmStack	dc	16i4'0'
rmFuncs	dc	60i4'0'
	END

	copy	inc/gsos.inc

* We set the BootInit function to a null function to avoid calling the
* real ResourceManager BootInit function- which could be rather catastrophic

* We'll patch the StartUp and ShutDown functions when we track CurResourceApp.
* The new 6.0.1 OpenResourceFileByID() calls OpenResourceFile to do its
* dirty work, so leaving it as-is won't cause any problems.

NewRMtable	DATA
rmsize	dc	i4'0'	; copy this from old table
	dc	i4'NULLTOOLFUNC-1'	; BootInit
	dc	i4'NewRMSU-1'	; StartUp
	dc	i4'NewRMSD-1'	; ShutDown
	dc	i4'0'	; Version
	dc	i4'0'	; Reset
	dc	i4'0'	; Status
	dc	i4'0'
	dc	i4'0'

	dc	i4'NewRMFunc-1'	; 9
	dc	i4'NewORF-1'       ; A
	dc	i4'NewRMFunc-1'	; B
	dc	i4'NewRMFunc-1'    ; C
	dc	i4'NewRMFunc-1'	; D
	dc	i4'NewRMFunc-1'	; E
	dc	i4'NewRMFunc-1'	; F
	dc	i4'NewRMFunc-1'	; 10
	dc	i4'NewRMFunc-1'	; 11
	dc	i4'NewRMFunc-1'	; 12
	dc	i4'NewSCRA-1'	; 13
	dc	i4'NewRMFunc-1'	; 14
	dc	i4'NewRMFunc-1'	; 15
	dc	i4'NewRMFunc-1'	; 16
	dc	i4'NewRMFunc-1'	; 17
	dc	i4'NewRMFunc-1'	; 18
	dc	i4'NewRMFunc-1'	; 19
	dc	i4'NewRMFunc-1'	; 1A
	dc	i4'NewRMFunc-1'	; 1B
	dc	i4'NewRMFunc-1'	; 1C
	dc	i4'NewRMFunc-1'	; 1D
	dc	i4'NewRMFunc-1'	; 1E
	dc	i4'NewGOFRN-1'	; 1F
	dc	i4'NewRMFunc-1'	; 20
	dc	i4'NewRMFunc-1'	; 21
	dc	i4'NewRMFunc-1'	; 22
	dc	i4'NewRMFunc-1'	; 23
	dc	i4'NewRMFunc-1'	; 24
	dc	i4'NewRMFunc-1'	; 25
	dc	i4'NewRMFunc-1'	; 26
	dc	i4'NewRMFunc-1'	; 27
	dc	i4'NewRMFunc-1'	; 28
	dc	i4'NewRMFunc-1'	; 29
	dc	i4'NewRMFunc-1'	; 2A
	dc	i4'NewRMFunc-1'	; 2B
	dc	i4'NewRMFunc-1'	; 2C
	dc	i4'NewRMFunc-1'	; 2D
	dc	i4'NewRMFunc-1'	; 2E
	dc	i4'NewRMFunc-1'	; 2F

	END
	
patchRM	START
	using	OldRMtable
	using NewRMtable
tsPtr	equ	0
	subroutine (0:foo),4
	pha
	pha
	ph2	#0
	ph2	#$1E
	_GetTSPtr
	pl4	tsPtr

	lda	[tsPtr]
	sta	rmFuncs
	sta	rmsize
	tax
	ldy	#2
	lda	[tsPtr],y
	sta	rmFuncs,y
	sta	rmsize+2
	iny2
copyloop	cpx	#0
	beq	donecopy
	lda	[tsPtr],y
	sta	rmFuncs,y
	iny2
	lda	[tsPtr],y
	sta	rmFuncs,y
	iny2
	dex
	bra	copyloop
donecopy	anop	
	ph2	#0
	ph2	#$1E
	ph4	#NewRMtable
	_SetTSPtr
	return
	END

unPatchRM	START
	using OldRMtable
tsPtr	equ	0
	subroutine (0:foo),4
	pha
	pha
	ph2	#0
	ph2	#$1E
	_GetTSPtr
	pl4	tsPtr

	lda	[tsPtr]
	tax
	ldy	#4
copyloop	cpx	#0
	beq	donecopy
	lda	rmFuncs,y
	sta	[tsPtr],y
	iny2
	lda	rmFuncs,y
	sta	[tsPtr],y
	iny2
	dex
	bra	copyloop
donecopy	anop	
	return
	END


* Makes sure that the CurResourceApp setting is correct.

SetCurRes	START
	using	KernelStruct

	lda	>curProcInd
	tax
	lda	>resapp,X
	beq	okay	; no resource thingy set
	cmp	>CurResApp
	beq	okay	; cur is the one we want

	pha
	ldx	#$131E
	jsl	$E10000
;	_SetCurResourceApp

okay	rts
	END


* We keep a stack of return addresses, since to pre-and-post patch these
* tool calls, we have to displace one of the two return addresses already
* there.
* We also copy the current process' openFiles pointer to a safe temp,
* and install the Kernel Null process' open files list.  All Resource
* manager calls are made with the Kernel's file list, so it will always
* have all files accessible

NewRMFunc	START
	using	OldRMtable
	using KernelStruct

	phb	; 11,s	; three bytes of storage
	pha 	; 9,s               for calling the old RM routine

	php	; 8,s  	; prolly not necessary, but fuck it
	phb                      ; 7,s
	phk
	plb                     
	phx                      ; 5,s
	phy                      ; 3,s
	pha		; 1,s

* get the last return address on the stack (at 9,s) and save it in
* our return stack

	lda	rmStackPtr
	bne	noSwitch
	ldx	curProcInd
	lda	openFiles,x
	sta	oldOpenFiles
	lda	openFiles+2,x
	sta	oldOpenFiles+2
	lda	openFiles	; Kernel open file list
	sta	openFiles,x
	lda	openFiles+2
	sta	openFiles+2,x	;
	lda	rmStackPtr
noSwitch	anop
	asl	a
	asl	a
	tax
	lda	12,s	
	sta	rmStack,x
	lda	13,s
	sta	rmStack+1,x
	inc	rmStackPtr

* Now, put our return address on the stack instead

	lda	#(rmReturn-1)|-8
	sta	13,s
	lda	#rmReturn-1
	sta	12,s	

	jsr	SetCurRes

* Finally, shove the address of the old function to call onto the
* stack and RTL to it.

	lda	5,s	; X
	and	#$FF00             ; mask off tool #
	xba                      ; swap
	asl	a
	asl	a
	tax
	lda	rmFuncs+1,x
	sta	10,s
	lda	rmFuncs,x
	sta	9,s
	pla
	ply
	plx
	plb
	plp
	rtl
rmReturn	anop
* we push three space bytes - here we'll store the original second rtl
* address, and rtl out of our patch

	phb	; 11,s	; three bytes of storage
	pha 	; 9,s               for calling the old RM routine

	php		; 8,s
	phb                      ; 7,s
	phk
	plb                     
	phx                      ; 5,s
	phy                      ; 3,s
	pha		; 1,s
	
	lda	rmStackPtr
	dec	a
	sta	rmStackPtr
	bne	noSwitch2
	ldx	curProcInd
	lda	oldOpenFiles
	sta	openFiles,x
	lda	oldOpenFiles+2
	sta	openFiles+2,x
	lda	rmStackPtr
noSwitch2	anop
	asl	a
	asl	a
	tax
	lda	rmStack+1,x
	sta	10,s
	lda	rmStack,x
	sta	9,s
	pla
	ply
	plx
	plb
	plp
	rtl
	END

*  Resource Manager GetOpenFileRefNum patch
*
*  call old GetOpenFileRefNum (result in 'ref')
*  search current process' file table for existence of 'ref'.
*  If the refNum is found, then return the file descriptor
*  else, find the ref in the kernel's file table, and copy
*  the entry to a new entry in the process' file descriptor
*  table, and return the new FD.

NewGOFRN	START
	using	OldRMtable
	using KernelStruct
kernFDPtr	equ  1
procFDPtr	equ  5
ref	equ	9

	phb	; 11,s	; three bytes of storage
	pha 	; 9,s               for calling the old RM routine

	php	; 8,s  	; prolly not necessary, but fuck it
	phb                      ; 7,s
	phk
	plb                     
	phx                      ; 5,s
	phy                      ; 3,s
	pha		; 1,s

* get the last return address on the stack (at 9,s) and save it in
* our return stack

	lda	rmStackPtr
	bne	noSwitch
	ldx	curProcInd
	lda	openFiles,x
	sta	oldOpenFiles
	lda	openFiles+2,x
	sta	oldOpenFiles+2
	lda	openFiles	; Kernel open file list
	sta	openFiles,x
	lda	openFiles+2
	sta	openFiles+2,x	; is attacted to process
	lda	rmStackPtr
noSwitch	anop
	asl	a
	asl	a
	tax
	lda	12,s	
	sta	rmStack,x
	lda	13,s
	sta	rmStack+1,x
	inc	rmStackPtr

* Now, put our return address on the stack instead

	lda	#(rmReturn-1)|-8
	sta	13,s
	lda	#rmReturn-1
	sta	12,s	

* Finally, shove the address of the old function to call onto the
* stack and RTL to it.

	jsr	SetCurRes

	ldx	#$1F*4
	lda	rmFuncs+1,x
	sta	10,s
	lda	rmFuncs,x
	sta	9,s
	pla
	ply
	plx
	plb
	plp
	rtl
rmReturn	anop
* we push three space bytes - here we'll store the original second rtl
* address, and rtl out of our patch

	phb	; 11,s	; three bytes of storage
	pha 	; 9,s               for calling the old RM routine

	php		; 8,s
	phb                      ; 7,s
	phk
	plb                     
	phx                      ; 5,s
	phy                      ; 3,s
	pha		; 1,s
	
	lda	rmStackPtr
	dec	a
	sta	rmStackPtr
	bne	noSwitch2
	ldx	curProcInd
	lda	oldOpenFiles
	sta	openFiles,x
	lda	oldOpenFiles+2
	sta	openFiles+2,x
	lda	rmStackPtr
noSwitch2	anop
	asl	a
	asl	a
	tax
	lda	rmStack+1,x
	sta	10,s
	lda	rmStack,x
	sta	9,s
**********
	lda	1,s
	beq	noerror
	jmp	doerror
noerror	lda	15,s	; get the ref value returned by
	tay                      ; the real GetOpenFileRefNum
	phd
	tsc
	sec
	sbc	#10
	tcs
	tcd

	sty	ref
	
	ldx	curProcInd	; push the process' fd table ptr
	lda	openFiles+2,x
	sta	procFDPtr+2
	pha
	lda	openFiles,x
	sta	procFDPtr
	pha
	ph2	ref
	jsl	SearchFDTable

	cmp	#0
	bne	gotrefnum

	lda	[procFDPtr]        ; increment the file count, we're
	inc	a                  ; opening a new one up!
	sta	[procFDPtr]	

*	lda	openFiles+2	; kernel file table pointer
*	pha	
*	lda	openFiles
*	pha	
*	ph2	ref
*	jsl	SearchFDTable

	mv4	openFiles,kernFDPtr
	lda	ref
	dec	a
	asl	a
	asl	a
	asl	a
	asl	a
	clc
	adc	#FDTTable
	tay
	lda	[kernFDPtr],y
	cmp	#0
	beq	rmpanic

	phy		; save the index across this call
	
	pha		; push the mapped refNum
	pea	FDgsos
	jsl	IncRefnum          ; inc the system-wide count

	pla		; from above phy
	clc
	adc	kernFDPtr
	sta	kernFDPtr
	lda	#0
	adc	kernFDPtr+2
	sta	kernFDPtr+2	; create pointer to kern spot
	
	pea	0
	tdc
	clc
	adc	#ref
	pha		; push address of 'ref'
	jsl	allocFD	; process' entry must be restored
	sta	procFDPtr
	stx	procFDPtr+2

	ph4	#FDsize
	ph4	kernFDPtr
	ph4	procFDPtr
	jsl	memcpy
	lda	ref
gotrefnum	tay
	tsc
	clc
	adc	#10
	tcs
	pld
	tya
	sta	15,s
**********
doerror	pla
	ply
	plx
	plb
	plp
	rtl

rmpanic	ph4	#rmpanicstr
	jsl	PANIC
rmpanicstr	dc  c'RM: can''t find refNum in nullProc',i'0'
	END


* OpenResourceFile patch.
* watches for use of memory-based resource maps, and if encountered,
* maps the refNum passed in the resourceMap structure

NewORF	START
	using	OldRMtable
	using KernelStruct
resMapPtr	equ   11	; amt. of local space & parm offset
kernFDPtr	equ   1
procFDPtr	equ   5
ref	equ	9
resFileNum	equ   $10

	phb	; 11,s	; three bytes of storage
	pha 	; 9,s               for calling the old RM routine

	php	; 8,s  	; prolly not necessary, but fuck it
	phb                      ; 7,s
	phk
	plb                     
	phx                      ; 5,s
	phy                      ; 3,s
	pha		; 1,s

* get the last return address on the stack (at 9,s) and save it in
* our return stack

	lda	rmStackPtr
	bne	noSwitch
	ldx	curProcInd
	lda	openFiles,x
	sta	oldOpenFiles
	lda	openFiles+2,x
	sta	oldOpenFiles+2
	lda	openFiles	; Kernel open file list
	sta	openFiles,x
	lda	openFiles+2
	sta	openFiles+2,x	;
	lda	rmStackPtr
noSwitch	anop
	asl	a
	asl	a
	tax
	lda	12,s	
	sta	rmStack,x
	lda	13,s
	sta	rmStack+1,x
	inc	rmStackPtr

* Now, put our return address on the stack instead

	lda	#(rmReturn-1)|-8
	sta	13,s
	lda	#rmReturn-1
	sta	12,s	

* Finally, shove the address of the old function to call onto the
* stack and RTL to it.

	jsr	SetCurRes

	ldx	#$0A*4	; we know the tool number
	lda	rmFuncs+1,x
	sta	10,s
	lda	rmFuncs,x
	sta	9,s

	lda	15+4+3,s
	ora	15+6+3,s
	sta	mbrmFlag
	
	pla
	ply
	plx
	plb
	plp
	rtl
rmReturn	anop
* we push three space bytes - here we'll store the original second rtl
* address, and rtl out of our patch

	phb	; 11,s	; three bytes of storage
	pha 	; 9,s               for calling the old RM routine

	php		; 8,s
	phb                      ; 7,s
	phk
	plb                     
	phx                      ; 5,s
	phy                      ; 3,s
	pha		; 1,s
	
*****
* begin patch
*****
*9-11: second RTL
*12-14: first rtl
	
	cmp	#0
	jne	notopen	; didn't open the file!
	lda	mbrmFlag
	jeq	nomembase
	phd
	tsc
	sec
	sbc	#14
	tcs
	tcd

	pha		; push space for retval, dammit!
	pha
	lda	15+16+4,s	; return value (file ID)
	pha
	_GetMapHandle
	pla
	sta	kernFDPtr
	pla
	sta	kernFDPtr+2
	lda	[kernFDPtr]
	sta	resMapPtr
	ldy	#2
	lda	[kernFDPtr],y
	sta	resMapPtr+2

	mv4	oldOpenFiles,procFDPtr
	ldy	#resFileNum
	lda	[resMapPtr],y
	dec	a
	asl	a
	asl	a
	asl	a
	asl	a
	clc
	adc	#FDTTable
	tay
	tax
	lda	[procFDPtr],y
	beq	nofutzing	; it will fall thru to an error later
	sta	ref
	
	tay
	txa		; make a pointer to the fd entry
	clc
	adc	procFDPtr
	sta	procFDPtr
	lda	#0
	adc	procFDPtr+2
	sta	procFDPtr+2

	lda	openFiles+2
	pha
	lda	openFiles
	pha
	phy		; saved above
	jsl	SearchFDTable	; is it in the kernel table?
	cmp	#0	; find it?
	bne	gotit	; already have the guy
	
	pei	(ref)	; push the mapped refNum
	pea	0
	jsl	IncRefnum          ; inc the system-wide count

	pea	0	; allocate a new fd
	tdc
	clc
	adc	#ref
	pha		; push address of 'ref'
	jsl	allocFD	; process' entry must be restored
	sta	kernFDPtr
	stx	kernFDPtr+2
	ph4	#FDsize
	ph4	procFDPtr
	ph4	kernFDPtr
	jsl	memcpy
	lda	ref
gotit	ldy	#resFileNum
	sta	[resMapPtr],y
nofutzing	anop
	tsc
	clc
	adc	#14		
	tcs
	pld
nomembase	anop
notopen	anop

*****
* end patch
*****

	lda	rmStackPtr
	dec	a
	sta	rmStackPtr
	bne	noSwitch2
	ldx	curProcInd
	lda	oldOpenFiles
	sta	openFiles,x
	lda	oldOpenFiles+2
	sta	openFiles+2,x
	lda	rmStackPtr
noSwitch2	anop
	asl	a
	asl	a
	tax
	lda	rmStack+1,x
	sta	10,s
	lda	rmStack,x
	sta	9,s
	pla
	ply
	plx
	plb
	plp
	rtl
mbrmFlag	dc	i2'0'	
	END

NewSCRA	START
	using	OldRMtable
	using KernelStruct

	phb	; 11,s	; three bytes of storage
	pha 	; 9,s               for calling the old RM routine

	php	; 8,s  	; prolly not necessary, but fuck it
	phb                      ; 7,s
	phk
	plb                     
	phx                      ; 5,s
	phy                      ; 3,s
	pha		; 1,s

* get the last return address on the stack (at 9,s) and save it in
* our return stack

	lda	rmStackPtr
	bne	noSwitch
	ldx	curProcInd
	lda	openFiles,x
	sta	oldOpenFiles
	lda	openFiles+2,x
	sta	oldOpenFiles+2
	lda	openFiles	; Kernel open file list
	sta	openFiles,x
	lda	openFiles+2
	sta	openFiles+2,x	;
	lda	rmStackPtr
noSwitch	anop
	asl	a
	asl	a
	tax
	lda	12,s	
	sta	rmStack,x
	lda	13,s
	sta	rmStack+1,x
	inc	rmStackPtr

* Get the resapp id, for use after the call
	lda	18,s
	sta	>tempresapp

* Now, put our return address on the stack instead

	lda	#(rmReturn-1)|-8
	sta	13,s
	lda	#rmReturn-1
	sta	12,s	

* Finally, shove the address of the old function to call onto the
* stack and RTL to it.

;	jsr	SetCurRes	; duh

	ldx	#$13*4	; we know the call number
	lda	rmFuncs+1,x
	sta	10,s
	lda	rmFuncs,x
	sta	9,s
	pla
	ply
	plx
	plb
	plp
	rtl
rmReturn	anop
* we push three space bytes - here we'll store the original second rtl
* address, and rtl out of our patch

	phb	; 11,s	; three bytes of storage
	pha 	; 9,s               for calling the old RM routine

	php		; 8,s
	phb                      ; 7,s
	phk
	plb                     
	phx                      ; 5,s
	phy                      ; 3,s
	pha		; 1,s

* last bit of the patch
	cmp	#0
	bne	waserr
	lda	>curProcInd
	tax
	lda	>tempresapp
	sta	>CurResApp
	sta	>resapp,X

waserr	lda	rmStackPtr
	dec	a
	sta	rmStackPtr
	bne	noSwitch2
	ldx	curProcInd
	lda	oldOpenFiles
	sta	openFiles,x
	lda	oldOpenFiles+2
	sta	openFiles+2,x
	lda	rmStackPtr
noSwitch2	anop
	asl	a
	asl	a
	tax
	lda	rmStack+1,x
	sta	10,s
	lda	rmStack,x
	sta	9,s
	pla
	ply
	plx
	plb
	plp
	rtl
tempresapp	dc  i2'0'
CurResApp	ENTRY
	dc	i2'$401E'
	END

NewRMSU	START
	using	OldRMtable
	using KernelStruct

	phb	; 11,s	; three bytes of storage
	pha 	; 9,s               for calling the old RM routine

	php	; 8,s  	; prolly not necessary, but fuck it
	phb                      ; 7,s
	phk
	plb                     
	phx                      ; 5,s
	phy                      ; 3,s
	pha		; 1,s

* get the last return address on the stack (at 9,s) and save it in
* our return stack

	lda	rmStackPtr
	bne	noSwitch
	ldx	curProcInd
	lda	openFiles,x
	sta	oldOpenFiles
	lda	openFiles+2,x
	sta	oldOpenFiles+2
	lda	openFiles	; Kernel open file list
	sta	openFiles,x
	lda	openFiles+2
	sta	openFiles+2,x	;
	lda	rmStackPtr
noSwitch	anop
	asl	a
	asl	a
	tax
	lda	12,s	
	sta	rmStack,x
	lda	13,s
	sta	rmStack+1,x
	inc	rmStackPtr

* Get the resapp id, for use after the call
	lda	18,s
	sta	>tempresapp

* Now, put our return address on the stack instead

	lda	#(rmReturn-1)|-8
	sta	13,s
	lda	#rmReturn-1
	sta	12,s	

* Finally, shove the address of the old function to call onto the
* stack and RTL to it.

	jsr	SetCurRes

	ldx	#$02*4	; we know the call number
	lda	rmFuncs+1,x
	sta	10,s
	lda	rmFuncs,x
	sta	9,s
	pla
	ply
	plx
	plb
	plp
	rtl
rmReturn	anop
* we push three space bytes - here we'll store the original second rtl
* address, and rtl out of our patch

	phb	; 11,s	; three bytes of storage
	pha 	; 9,s               for calling the old RM routine

	php		; 8,s
	phb                      ; 7,s
	phk
	plb                     
	phx                      ; 5,s
	phy                      ; 3,s
	pha		; 1,s

* last bit of the patch
	cmp	#0
	bne	waserr
	lda	>curProcInd
	tax
	lda	>tempresapp
	sta	>CurResApp
	sta	>resapp,X


waserr	lda	rmStackPtr
	dec	a
	sta	rmStackPtr
	bne	noSwitch2
	ldx	curProcInd
	lda	oldOpenFiles
	sta	openFiles,x
	lda	oldOpenFiles+2
	sta	openFiles+2,x
	lda	rmStackPtr
noSwitch2	anop
	asl	a
	asl	a
	tax
	lda	rmStack+1,x
	sta	10,s
	lda	rmStack,x
	sta	9,s
	pla
	ply
	plx
	plb
	plp
	rtl
tempresapp	dc  i2'0'
	END

NewRMSD	START
	using	OldRMtable
	using KernelStruct

	phb	; 11,s	; three bytes of storage
	pha 	; 9,s               for calling the old RM routine

	php	; 8,s  	; prolly not necessary, but fuck it
	phb                      ; 7,s
	phk
	plb                     
	phx                      ; 5,s
	phy                      ; 3,s
	pha		; 1,s

* get the last return address on the stack (at 9,s) and save it in
* our return stack

	lda	rmStackPtr
	bne	noSwitch
	ldx	curProcInd
	lda	openFiles,x
	sta	oldOpenFiles
	lda	openFiles+2,x
	sta	oldOpenFiles+2
	lda	openFiles	; Kernel open file list
	sta	openFiles,x
	lda	openFiles+2
	sta	openFiles+2,x	;
	lda	rmStackPtr
noSwitch	anop
	asl	a
	asl	a
	tax
	lda	12,s	
	sta	rmStack,x
	lda	13,s
	sta	rmStack+1,x
	inc	rmStackPtr

* Get the resapp id, for use after the call
* Now, put our return address on the stack instead

	lda	#(rmReturn-1)|-8
	sta	13,s
	lda	#rmReturn-1
	sta	12,s	

* Finally, shove the address of the old function to call onto the
* stack and RTL to it.

	jsr	SetCurRes

	ldx	#$03*4	; we know the call number
	lda	rmFuncs+1,x
	sta	10,s
	lda	rmFuncs,x
	sta	9,s
	pla
	ply
	plx
	plb
	plp
	rtl
rmReturn	anop
* we push three space bytes - here we'll store the original second rtl
* address, and rtl out of our patch

	phb	; 11,s	; three bytes of storage
	pha 	; 9,s               for calling the old RM routine

	php		; 8,s
	phb                      ; 7,s
	phk
	plb                     
	phx                      ; 5,s
	phy                      ; 3,s
	pha		; 1,s

* last bit of the patch
	lda	>curProcInd
	tax
	lda	#0
	sta	>resapp,X


waserr	lda	rmStackPtr
	dec	a
	sta	rmStackPtr
	bne	noSwitch2
	ldx	curProcInd
	lda	oldOpenFiles
	sta	openFiles,x
	lda	oldOpenFiles+2
	sta	openFiles+2,x
	lda	rmStackPtr
noSwitch2	anop
	asl	a
	asl	a
	tax
	lda	rmStack+1,x
	sta	10,s
	lda	rmStack,x
	sta	9,s
	pla
	ply
	plx
	plb
	plp
	rtl
	END
