*	$Id: kern.asm,v 1.2 1998/02/22 05:05:45 taubert Exp $

* GNO Multitasking environment
*  Low-level Kernel code
*  Context switching, tool patch vector, CDA patches,
*  process ID allocation, etc.
*  Copyright 1991-1998, Procyon Software
* added select() timeout support - DT - 2/13/96

	mcopy m/kern.mac
	copy  global.equates
	case	on

TIMESLICE	gequ  3
NUM_DEFER	gequ  16

;IncBusy        gequ  $E10064
;DecBusy        gequ  $E10068
BusyFlag	gequ  $E100FF

****************************************************************************
* Equates - Various equates required by these routines.
*
versionNumber	gequ $0100          ; The version number of this library.

dispatch1	gequ $e10000        ; The first toolbox dispatch vector.
dispatch2	gequ $e10004        ; The second toolbox dispatch vector.
ToolPointerTable	gequ $e103c0        ; Pointer to the active System TPT.
UserToolPointerTable	gequ $e103c8        ; Pointer to the active User TPT.
	
; Error return values from the routines InstallE10000 and RemoveE10000

noError	gequ $0000          ; Value returned if no error occurs.
badHeaderError	gequ $8001          ; Patch header wasn't valid.
headerNotFoundError	gequ $8002          ; Header to remove wasn't in the 
;                                          linked list.

****************************************************************************
* CheckPatch - Checks the passed toolbox dispatch vector to see if it
*              points to a valid patch.
*
* Input: Passed via the stack following C conventions.
*     newPatchAddr (long) - Address of the patch routine.
*
* Output:
*     If newPatchAddr is a valid patch -
*          Carry clear
*     If newPatchAddr is not a valid patch -
*          Carry set
*
CheckPatch	START

zprtl	equ   $01            ; The address for the rtl on our direct
;                                     page.
newPatchAddr	equ   zprtl+3        ; Address of patch (parameter to this
;                                     routine).

	tsc                  ; Make the stack the direct page after
;                                     saving
	phd                  ; the current direct page.
	tcd


	lda   newPatchAddr+2 ; Simple check to check for a valid
;                                     pointer.
	and   #$ff00
	bne   BadPatch       ; Wasn't zero, can't be a valid pointer.

	lda   [newPatchAddr]     ; Check for the first JML instruction.
	and   #$00ff
	cmp   #$005c
	bne   BadPatch

	ldy   #$04               ; Check for the second JML instruction.
	lda   [newPatchAddr],y
	and   #$00ff
	cmp   #$005c
	bne   BadPatch

	ldy   #$08               ; Check for the third JML instruction.
	lda   [newPatchAddr],y
	and   #$00ff
	cmp   #$005c
	bne   BadPatch

	ldy   #$0c               ; Check for the fourth JML instruction.
	lda   [newPatchAddr],y
	and   #$00ff
	cmp   #$005c
	bne   BadPatch

	ldy   #$10               ; Check for the rtl and phk instructions.
	lda   [newPatchAddr],y
	cmp   #$4b6b
	bne   BadPatch

	iny                    ; Check for the phk and pea instructions.
	lda   [newPatchAddr],y
	cmp   #$f44b
	bne   BadPatch

	clc                      ; Calculate the address of the rtl
;                                         instruction.
	lda   newPatchAddr
	adc   #$000f
	ldy   #$13               ; Check for address of the rtl
;                                         instruction.
	cmp   [newPatchAddr],y
	bne   BadPatch

GoodPatch	anop
	pld                      ; Restore the direct page and report
	clc                      ; that it was a good patch.
	rtl

BadPatch	anop
	pld                      ; Restore the direct page and report
	sec                      ; that something was wrong.
	rtl

	END

****************************************************************************
* InstallE10000   - Sets the jump vector at $e10000 and $e10004 to point to
*                 the passed new toolbox dispatch vector patch.  This routine
*                 also updates the linked lists so that more than one routine
*                 can be patched into the dispatch vectors.
*
* Input: Passed via the stack following C conventions.
*     newPatchAddr (long) - Address of the patch routine.
*

* Output:
*     If an error occurred -
*          Carry set, Accumulator contains one of the following error codes:
*               badHeaderError
*     If no error occurred and patch was installed successfully -
*          Carry clear, Accumulator contains zero.
*
InstallE10000	START

oldPatchAddr	equ   $01                ; Address of existing patch.
zprtl	equ   oldPatchAddr+4     ; The address for the rtl.
zpsize	equ   zprtl-oldPatchAddr ; Size of direct page we'll have on
;                                         the stack.
newPatchAddr	equ zprtl+3              ; Address of patch (parameter to
;                                         this routine).

	tsc                      ; Move the stack pointer to point beyond
	sec                      ; the direct page variables that we'll
	sbc   #zpsize            ; place on the stack.
	tcs
	phd                      ; Save the direct page register.
	tcd                      ; Set the direct page.
	php                      ; Disable interrupts
	sei

	pei   newPatchAddr+2     ; Check if patch header is valid.
	pei   newPatchAddr
	jsl   CheckPatch
	plx                      ; Remove the parameters from the stack.
	plx
	bcc   x1                 ; Report the badHeaderError if detected.
	ldy   #badHeaderError
	jmp   Exit

x1	lda   >dispatch1         ; Set up the next1Vector in the new patch.
	sta   [newPatchAddr]     ; The JML instruction and low byte.
	lda   >dispatch1+2
	ldy   #$02
	sta   [newPatchAddr],y   ; The middle and upper bytes.

	lda   >dispatch2         ; Set up the next2Vector in the new patch.
	ldy   #$04
	sta   [newPatchAddr],y   ; The JML instruction and low byte.
	lda   >dispatch2+2
	ldy   #$06
	sta   [newPatchAddr],y   ; The middle and upper bytes.

	lda   >dispatch1+3       ; See if there's already a patch in
;                                         dispatch1.
	and   #$00ff
	sta   oldPatchAddr+2
	pha                      ; High byte of possible header address.
	lda   >dispatch1+1
	sec
	sbc   #$0011
	sta   oldPatchAddr
	pha                      ; Low byte of possible header address.
	jsl   CheckPatch
	plx
	plx
	bcs   First              ; JIF this will be the first patch
;                                         installed.

	ldy   #$08               ; Set up the dispatch1Vector in the new
;                                         patch.
	lda   [oldPatchAddr],y
	sta   [newPatchAddr],y   ; The JML instruction and low byte.
	ldy   #$0a
	lda   [oldPatchAddr],y
	sta   [newPatchAddr],y   ; The middle and upper bytes.

	ldy   #$0c               ; Set up the dispatch2Vector in the new
;                                         patch.
	lda   [oldPatchAddr],y
	sta   [newPatchAddr],y   ; The JML instruction and low byte.
	ldy   #$0e
	lda   [oldPatchAddr],y
	sta   [newPatchAddr],y   ; The middle and upper bytes.
	
	bra   PatchIt            ; Now patch dispatch1 and dispatch2.

First	ldy   #$08               ; Set up the dispatch1Vector in the new
;                                         patch.
	lda   >dispatch1
	sta   [newPatchAddr],y   ; The JML instruction and low byte.
	ldy   #$0a
	lda   >dispatch1+2
	sta   [newPatchAddr],y   ; The middle and upper bytes.

	ldy   #$0c               ; Set up the dispatch2Vector in the new
;                                         patch.
	lda   >dispatch2
	sta   [newPatchAddr],y   ; The JML instruction and low byte.
	ldy   #$0e
	lda   >dispatch2+2
	sta   [newPatchAddr],y   ; The middle and upper bytes.

PatchIt	anop
	clc                      ; Calculate the address of the new
;                                         dispatch2.
	lda   newPatchAddr
	adc   #$0015
	sta   newPatchAddr
	xba
	and   #$ff00             ; Mask in the JML instruction.
	ora   #$005c
	sta   >dispatch2         ; The JML instruction and low byte.
	lda   newPatchAddr+1
	sta   >dispatch2+2       ; The middle and upper bytes.

	sec                      ; Calculate the address of the new
;                                         dispatch1.
	lda   newPatchAddr
	sbc   #$0004
	sta   newPatchAddr
	xba
	and   #$ff00             ; Mask in the JML instruction.
	ora   #$005c
	sta   >dispatch1         ; The JML instruction and low byte.
	lda   newPatchAddr+1
	sta   >dispatch1+2       ; The middle and upper bytes.

	ldy   #noError           ; Report that all went well.

Exit	plp                      ; Restore the interrupt state.
	pld                      ; Restore the previous direct page
;                                         register.
	tsc                      ; Restore the stack pointer.
	clc
	adc #zpsize
	tcs
	tya                      ; Value to return.
	beq noerr
	sec                      ; Report that there was an error.
	rtl
noerr	clc                      ; Report that there was no error.
	rtl

	END


****************************************************************************
* RemoveE10000 - Removes the specified patch from the dispatch1 and dispatch2
*                vectors and updates the linked lists for the remaining
*                toolbox patches.
*
* Input: Passed via the stack following C conventions.
*     patchToRemove (long) - Address of the patch to remove.
*
* Output:
*     If an error occurred -
*          Carry set, Accumulator contains one of the following error codes:
*               badHeaderError
*               headerNotFoundError
*     If no error occurred and patch was removed successfully -
*          Carry clear, Accumulator contains zero.
*
RemoveE10000	START

patchDispAddr	equ   $01                    ; Address of existing patch (and 1
;                                             extra byte).
prevHeader	equ   patchDispAddr+5        ; Used to search through the
;                                             linked list.
zprtl	equ   prevHeader+4           ; The address for the rtl.
zpsize	equ   zprtl-patchDispAddr    ; Size of direct page we'll have
;                                             on the stack.
patchToRemove	equ   zprtl+3                ; Address of patch (parameter to
;                                             this routine).

	tsc                          ; Move the stack pointer to point beyond
	sec                          ; the direct page variables that we'll
	sbc   #zpsize                  ; place on the stack.
	tcs
	phd                          ; Save the direct page register.
	tcd                          ; Set the direct page.
	php                          ; Disable interrupts
	sei

	pei   patchToRemove+2          ; Check if patch header we were asked to
	pei   patchToRemove            ; remove is a valid header.
	jsl   CheckPatch
	plx                      ; Remove the parameters from the stack.
	plx
	bcc   x1                 ; Report the badHeaderError if detected.
	ldy   #badHeaderError
	jmp   Exit

x1	clc                      ; Create the JML instruction that would
;                                         exist
	lda   patchToRemove      ; if the patchToRemove was installed.
	adc   #$0011
	sta   patchDispAddr+1
	lda   patchToRemove+2
	sta   patchDispAddr+3
	lda   patchDispAddr      ; Mask in the JML instruction.
	and   #$ff00
	ora   #$005c
	sta   patchDispAddr

	cmp   >dispatch1         ; Check if the patch to remove is the
;                                         first
	bne   NotFirstOne        ; patch installed.
	lda   >dispatch1+2
	cmp   patchDispAddr+2
	bne   NotFirstOne

	lda   [patchToRemove]    ; Restore the Dispatch1 vector.
	sta   >dispatch1
	ldy   #$02
	lda   [patchToRemove],y
	sta   >dispatch1+2


	ldy   #$04               ; Restore the Dispatch2 vector.
	lda [patchToRemove],y
	sta >dispatch2
	ldy #$06
	lda [patchToRemove],y
	sta >dispatch2+2

	bra NoErr                ; Everything went well.

NotFirstOne	anop
	sec                      ; Assume that whatever is in dispatch1 is
	lda >dispatch1+1         ; patch and get the address of its header.
	sbc #$0011
	sta prevHeader           ; Low and middle bytes.
	lda >dispatch1+3
	and #$00ff
	sta prevHeader+2         ; Upper byte of header address.

loop	pei prevHeader+2         ; Check if it really is a valid header.
	pei prevHeader
	jsl CheckPatch
	plx                      ; Remove the parameters from the stack.
	plx
	bcc x2                   ; Report that the patch that we asked to
	ldy #headerNotFoundError ; remove wasn't found.
	bra Exit

x2	lda [prevHeader]         ; See if this patch points to patch we
	cmp patchDispAddr        ; want to remove.
	bne nope
	ldy #$02
	lda [prevHeader],y
	cmp patchDispAddr+2
	bne nope

	lda [patchToRemove]      ; Restore the next1Vector.
	sta [prevHeader]
	ldy #$02
	lda [patchToRemove],y
	sta [prevHeader],y

	ldy #$04                 ; Restore the next2Vector.
	lda [patchToRemove],y
	sta [prevHeader],y
	ldy #$06
	lda [patchToRemove],y
	sta [prevHeader],y

	bra NoErr                ; Everything went well.

nope	ldy #$02                 ; Get the address of the next patch
;                                         header.
	lda [prevHeader],y
	tax
	lda [prevHeader]
	sta prevHeader
	stx prevHeader+2

	sec
	lda prevHeader+1
	sbc #$11
	sta prevHeader
	lda prevHeader+3
	and #$00ff
	sta prevHeader+2


	bra loop                 ; Now check this header.

NoErr	ldy #noError             ; Report that all went well.

Exit	plp                      ; Restore the interrupt state.
	pld                      ; Restore the previous direct page
;                                         register.
	tsc                      ; Restore the stack pointer.
	clc
	adc #zpsize
	tcs
	tya                      ; Value to return.
	beq xnoerr
	sec                      ; Report that there was an error.
	rtl
xnoerr	clc                      ; Report that there was no error.
	rtl
	END

toolV2Dat	DATA KERN2
t2Stack	ds	128*32
	END

****************************************************************************
* PatchHeader - Header required of all routines that will be patched
*               into the toolbox dispatch vectors.
*
* Note:  The code between next1Vector and NewDispatch2 must be included
*        for all calls.  The code below NewDispatch2 only needs to be
*        included for patches that want to post patch the calls.
*
PatchHeader	START
	using toolV2Dat
	using	KernelStruct

next1Vector	ENTRY                     ; Where dispatch1 should go when
;                                          finished.
	jml next1Vector                   ; (Filled in by InstallE10000).
next2Vector	ENTRY                      ; Where dispatch2 should go when 
;                                          finished.
	jml next2Vector                   ; (Filled in by InstallE10000).
dispatch1Vector	ENTRY                    ; Holds the JML instruction from 
;                                          $e10000.
	jml dispatch1Vector               ; (Filled in by InstallE10000).
dispatch2Vector	ENTRY                    ; Holds the JML instruction from 
;                                          $e10004.
	jml dispatch2Vector               ; (Filled in by InstallE10000).

anRtl	rtl                               ; An RTL instruction.  Its address 
;                                          will be
*                                        ; pushed on the stack for dispatch1
;                                          calls.

NewDispatch1	ENTRY                      ; Entry point for dispatch1 toolbox 
;                                          vector.

	phk                               ; Push program bank.
	pea anRtl-1                       ; Push the address of a RTL.

NewDispatch2	ENTRY                      ; Entry point for dispatch2 toolbox 
;                                          vector.

; The following code should be included in the PatchHeader if the patch wants
; to perform post patching.  This code will determine if the call that was 
; made actually exists and if it does, post patching can occur.  If the call 
; doesn't exist, any pre-call routines can be executed, but the post patching 
; shouldn't be attempted because the dispatcher will remove the second return 
; address from the stack, thus not returning to your post patching routines.
; Stack equates for this routine.

aLong	equ   $0001              ; A temporary long value.
oldDP	equ   aLong+4            ; Where the direct page is saved to.
oldTM	equ   oldDP+2            ; Where the tool call number is saved.

	phx                      ; Save the call that's being made.
	phd                      ; Save the current direct page.
	lda   >ToolPointerTable+2  ; Get the TPT to determine the number
	pha                      ; of tool sets loaded.
	lda   >ToolPointerTable
	pha
	tsc                      ; Set the direct page to the stack.
	tcd
	txa                      ; See if this tool set exits.
	and   #$00ff
	cmp   [aLong]            ; Is it larger than the number of tool
;                                         sets?
	bcs   noCall             ; JIF this tool set doesn't exist.
	asl   a
	asl   a
	tay                      ; Now get the pointer to the FPT.
	lda   [aLong],y
	tax
	iny
	iny
	lda   [aLong],y
	sta   aLong+2
	stx   aLong
	lda   oldTM              ; Get the function number.
	and   #$ff00
	xba
	cmp   [aLong]            ; Compare it to the number of entries in
;                                         table.
noCall	anop
	pla                      ; Remove aLong from the stack.
	pla
	pld                      ; Restore the original direct page.
	plx                      ; Recover the tool number.
	bcs   noCall2            ; don't pre patch, because we
*                                       ; cannot post-patch

; At this point the carry flag is set if the tool call doesn't exist and clear
; if the tool call exits.  No post patching must occur if the carry flag is 
; set.

	php
	sei
	phb
	phk
	plb                      ; 5,s
	pha                      ; 3,s
	phx                      ; 1,s

	jsl   incBusy

	ldx	curProcInd
	lda   t2StackPtr,x
	asl   a
	asl   a
	clc
	adc	curProcInd	; add current proc offset
	tax
	lda   7,s
	sta   >t2Stack,x
	short m
	lda   9,s
	sta   >t2Stack+2,x
	long  m
	ldx	curProcInd
	inc   t2StackPtr,x

* Now, put our return address on the stack instead

	short m
	lda   #^t2Return-1
	sta   9,s
	long  m
	lda   #t2Return-1
	sta   7,s
	plx
	pla
	plb
	plp
noCall2	anop
	jmp next2Vector          ; Go to the original $e10004 jump
;                                         instruction.
t2Return	anop
	phb
	pha                      ; space for return address

	php                      ; 6,s
	sei
	phb                      ; 5,s
	phk
	plb
	pha                      ; 3,s
	phx                      ; 1,s

	ldx	curProcInd
	lda   t2StackPtr,x
	dec   a
	sta   t2StackPtr,x
	asl   a
	asl   a
	clc
	adc	curProcInd
	tax
	short m
	lda   >t2Stack+2,x
	sta   9,s
	long  m
	lda   >t2Stack,x
	sta   7,s
	plx
	pla
	plb
	plp
	jmp   >decBusy
;               jsl   decBusy
;               rtl

	END

SaveAllPatch	START
	using	toolV2Dat
	using KernelStruct
	using IgnoreInfo

	php
	sei
	long  ai

	phb
	phk
	plb
	pha
	phx
	phy
;               jsl   incBusy

	lda   curProcInd
	sta   tcurProcInd
	lda   truepid
	sta   ttruepid
	lda   #0
	sta   curProcInd
	sta   truepid
	lda   gsosDebug
	sta   tgsosDebug
	stz   gsosDebug
	
	
* we need to move some tool stack info to make it look like the kernel
* process called "SaveAll".
	ldx	tcurProcInd
	lda	t2StackPtr,x
	dec	a
	sta	t2StackPtr,x
	asl	a
	asl	a
	clc
	adc	tcurProcInd
	tax
	lda	>t2Stack,x
	sta	tstack
	lda	>t2Stack+2,x
	sta	tstack+2
	
	lda	t2StackPtr
	asl	a
	asl	a
	tax
	lda	tstack
	sta	>t2Stack,x
	lda	tstack+2
	sta	>t2Stack+2,x
	lda	t2StackPtr
	inc	a
	sta	t2StackPtr

	pushword #$0000          ; system tool set
	pushword #$0C            ; texttools
	pushlong >oldTextTT      ; thar she blows
	_SetTSPtr
	ply
	plx
	pla
	plb
	plp
OLDSAVEALL	ENTRY
	jmp	>$000000

RestAllPatch	ENTRY
	php
	sei
	long  ai

	phb
	phk
	plb
	pha
	phx
	phy
	phd

	lda   tcurProcInd
	sta   curProcInd
	lda   ttruepid
	sta   truepid
	lda   tgsosDebug
	sta   gsosDebug

* we need to move some tool stack info to make it look like the interrupted
* process called "RestoreAll".
	lda	t2StackPtr
	dec	a
	sta	t2StackPtr
	asl	a
	asl	a
	tax
	lda	>t2Stack,x
	sta	tstack
	lda	>t2Stack+2,x
	sta	tstack+2
	
	ldx	tcurProcInd
	lda	t2StackPtr,x
	asl	a
	asl	a
	clc
	adc	tcurProcInd
	tax
	lda	tstack
	sta	>t2Stack,x
	lda	tstack+2
	sta	>t2Stack+2,x
	ldx	tcurProcInd
	lda	t2StackPtr,x
	inc	a
	sta	t2StackPtr,x

; set the flag to ignore the next occurrences of those
; particular tools that are grieving us right now.
; for ROM 01 this works, as they only try to restore IN OUT, not ERR.

	lda	#1
	sta	ignSetInDev
	sta	ignSetInGlo
	sta	ignSetOutDev
	sta	ignSetOutGlo
	pushword #$0000          ; system tool set
	pushword #$0C            ; texttools
	pushlong #TTtable        ; thar she blows
	_SetTSPtr

;               jsl   decBusy
	pld
	ply
	plx
	pla
	plb
	plp

;              ldx #0
;              ldy #0
;              jmp tool_exit

OLDRESTALL	ENTRY
	jmp	>$000000

savedCDAInfo	ENTRY
ttruepid	dc    i2'0'
tcurProcInd	dc    i2'0'
tgsosDebug	dc    i2'0'
tstack	dc	i4'0'
	END

cdaPatch	START
	using KernelStruct

	php
	lda   >reschedFlag
	inc   a
	sta   >reschedFlag
	lda   4,s
	sta   >rtlSave+2
	lda   1,s
	sta   4,s
	long  m
	lda   2,s
	sta   >rtlSave
	pla
	short m
	pla
	plp

;              jsl   oldCDAVect

	php
	php
	php
	php
	short m
	lda   >rtlSave+2
	sta   4,s
	long  m
	lda   >rtlSave
	sta   2,s
	lda   >reschedFlag
	dec   a
	sta   >reschedFlag
	plp
	rtl
rtlSave	dc    i4'0'
	END

; BrkVect
; kills the offending process
; gets info on the BRK by doing a GetInterruptState

BrkVect	START
	using KernelStruct

	sei
	clc                      ; (might not be necessary, but fuck it)
	xce
	phk
	plb
	long  ai
	jsl incBusy

	lda >curProcInd
	tax
	lda >lastTool,x
	sta >brkState+20

	pushlong #brkState
	pushword #$14
	_GetInterruptState

	lda 4,s                  ; copy PC into record
	sta brkState+13
	lda 2,s
	sta brkState+14
	tsc                      ; copy S into record
	sta brkState+6

	pushlong #brkState
	tsc
	pha
	jsl PRINTBRK
	bra brkproc
BrkVect1	ENTRY
	Using KernelStruct

	sei
	clc                      ; (might not be necessary, but fuck it)
	xce
	phk
	plb
	long  ai
	jsl incBusy

	lda >curProcInd
	tax
	lda >lastTool,x
	sta >brkState+20

	pushlong #brkState
	pushword #$14
	_GetInterruptState

	lda 2,s
	sta brkState+$0E
	short a
	lda 4,s
	sta brkState+$0D
	long  a

	pushlong #brkState
	tsc
	pha
	jsl PRINTBRK
;               jsr FlexBeep

; kill() now resets mutex to 0 if prog crashes in mutex
brkproc	anop
	tsc
	sta >$604

	pha
	pha
	jsl	KERNgetpid	; this is #pragma toolparms 1
	pea	9
	ph4	#killerr
	jsl	KERNkill	; this is #pragma toolparms 1
	pla
oops	bra	oops

killerr	dc    i2'0'
brkState	ds    32
	END

; since the kernel null process doesn't _resched, we won't get in 
; an 'exploding time slice' situation.  Make sure null process never calls
; _resched

CopVect	START
	using IntState
	using KernelStruct

	clc
	xce
	long  mx
	php
	phb

	sta >int_A               ; save accumulator
	lda #0                   ; 0 indicates COP
	sta >intType

* start saving off the registers
	short m
	pla
	sta >int_B               ; save data bank
	pla
	sta >int_P               ; save status of 'e' bit

	lda >$E0C02D
	sta >int_SLTROM
	lda >$E0C068
	sta >int_STATEREG
	long  a

* the value of s that's saved shouldn't differ from the value upon
* entry
	tsc
	sta >int_S               ; save stack pointer

	tdc
	sta >int_D               ; save direct page
	phk
	plb
	stx 	int_X
	sty 	int_Y                ; save X & Y
	lda 	>$E0C035
	and	#%1111111111110111         ; don't ever change SHR shadowing
	sta 	int_state
	jsl 	contextSwitch
; restore stuff
newViaCOP	ENTRY
	long	mx	; just for kicks
	lda	>$E0C035	; don't ever change SHR shadowing
	and	#%0000000000001000	; get old SHR shadow bit
	ora	>int_state
	sta 	>$E0C035
	lda >int_Y
	tay
	lda >int_X
	tax
	lda >int_D
	tcd

	short m
	lda >int_B
	pha
	plb
	lda >int_SLTROM
	sta >$E0C02D
	lda >int_STATEREG
	sta >$E0C068

	lda >int_P
	pha

	long  m
	lda >int_A
	plp
	xce
	rti
	END

	longa on
	longi on
InitKernel	START
	using KernelStruct
; TO CONSOLE
;               using InOutData
	phb
	phk
	plb

	move3 $E10071,oldBrk+1
;               move3 #BrkVect,$E10071

	pha
	pha
	pea   $5
	_GetVector
	pl4   oldCop

;               move3 $E10015,oldCop
	pea   $5
	ph4   #CopVect
	_SetVector
;               move3 #CopVect,$E10015

;               pha
;               _MMStartUp
;               pla
;               sta userID
; kp->userID set in main.c

; TO CONSOLE
;              jsl   InOutStart         ; good enough here....
	jsl   patchGSOS
	jsr   PatchText
	jsl   initvar
	jsl	patchRM

	ldx #0
loop	lda #0
	sta ParentProc,x
	sta ProcessState,x
	txa
	clc
	adc #procEntSize
	tax
	cpx #(procEntSize*NPROC)
	bcc loop
	lda #pRunning
	sta ProcessState
; let's do it, dude!

	pushlong #alarmHB
	_SetHeartBeat

	php                      ; jb 6/23/91
	sei                      ; jb 6/23/91
	move3 $E10069,oldDecBusy+1
	move3 #newDecBusy,$E10069
	move3 $E10011,oldInt+1
	move3 #IntHndl,$E10011
;               mv4   >$E10048,oldCDAvect
;               move3 #CDAPatch,>$E10049

	ph4   #PatchHeader
	jsl   InstallE10000
	ply
	ply
	plp                      ; jb 6/23/91
	plb
	rtl

DeInitKern	ENTRY
	pushlong #alarmHB
	_DelHeartBeat

	pea   $4                 ; Interrupt Manager
	ph4   oldInt+1
	_SetVector
	pea   $5                 ; COP Manager
	ph4   oldCop
	_SetVector

	ph4   #PatchHeader
	jsl   RemoveE10000
	ply
	ply
	move3 oldDecBusy+1,$E10069
	move3 oldBrk+1,$E10071
;               mv4   oldCDAvect,>$E10048

	jsl   unPatchRM
	jsl   unpatchGSOS
	jsr   UnPatchText
; TO CONSOLE
;              jsl   InOutEnd
	rtl
oldInt	ENTRY
	jmp >$000000
	dc i2'0'                 ; buffer space!
oldBrk	ENTRY
	jmp >$000000
oldCop	dc i4'0'
oldTool1	dc i4'0'
;oldCDAvect     ENTRY
;               dc i4'0'
oldDecBusy	ENTRY
	jmp >$000000
	END

mapPID	START
	using KernelStruct
retval	equ   0
	subroutine (2:pid),2

	ldx   #0
lp	anop
	lda   ProcessState,x
	beq   trynext            ; no process here, don't check
	lda   flpid,x
	cmp   pid
	beq   gotit
trynext	anop
	txa
	clc
	adc   #procEntSize
	cmp   #(procEntSize*NPROC)
	bcs   notfound
	tax
	bra   lp
gotit	txa
	lsr   a
	lsr   a
	lsr   a
	lsr   a
	lsr   a
	lsr   a
	lsr   a

goaway	anop
	sta   retval
	return 2:retval
notfound	lda   #-1
	bra   goaway             ; my stupidity amazes me. NOT gotit
	END

allocPID	START
	using KernelStruct
retval	equ   0
	subroutine (0:foo),2

	jsl   incBusy
lp0	anop
	ldx   #0
lp	anop
	lda   ProcessState,x
	beq   nextent
	lda   floatingPID
	cmp   flpid,x
	beq   trynext
nextent	anop                     ; inc x to next entry
	txa
	clc
	adc   #procEntSize
	cmp   #(procEntSize*NPROC)
	bcs   gotone
	tax
	bra   lp
trynext	lda   floatingPID
	inc   a
	bpl   inrange
	lda   #1
inrange	sta   floatingPID
	bra   lp0
gotone	lda   floatingPID
	sta   retval
	inc   a
	bpl   inrange2
	lda   #1
inrange2	sta   floatingPID
	jsl   decBusy
	return 2:retval
	END

IntState	DATA
int_A	dc i2'0'
int_X	dc i2'0'
int_Y	dc i2'0'
int_S	dc i2'0'
int_D	dc i2'0'
int_B	dc i2'0'

int_P	dc i2'0'                 ; needed to go back to emulation mode
intType	dc i2'0'
int_state	dc i2'0'
int_SLTROM	dc i1'0'
int_STATEREG	dc i1'0'
	END

IntHndl	START
	using IntState
	using KernelStruct

;               bvc nobrk
;               jsr FlexBeep
;               jmp BrkVect1
nobrk	anop
	clc
	xce
	bcs	ohmygod
	php
	short a
	pha
	lda $E1C046
	and #$08
	bne gotVBL
tofw	pla
	plp
ohmygod	xce
	jmp oldInt

gotVBL	anop

;              lda >reschedFlag
;              sta >$E00626
	lda >timeleft
	beq noMoreTime
	dec a
	sta >timeleft
	bne tofw

noMoreTime	anop
	lda #1                   ;1 indicates VBL
	sta >intType
	pla
;              sei

	long  ai
	sta >int_A               ; save accumulator

	short a
	lda >$E0C02D
	sta >int_SLTROM
	lda >$E0C068
	sta >int_STATEREG
	pla
	sta >int_P
	phb
	pla
	sta >int_B               ; save data bank
	long  a
	tsc
	sta >int_S               ; save stack pointer
	tdc
	sta >int_D               ; save direct page

	phk                      ; set up kernel's databank
	plb

	txa
	sta >int_X
	tya
	sta >int_Y               ; save X & Y

	ldx curProcInd           ; update the process'
	lda	ticks,x
	adc	#TIMESLICE
	sta	ticks,x	
	bcc nocarry
	inc ticks+2,x
nocarry	anop

	lda 	>$E0C035
	and	#%1111111111110111
	sta 	>int_state
	jsl 	contextSwitch
; restore stuff
newViaVBL	ENTRY
	long	mx
	lda	>$E0C035
	and	#%0000000000001000
	ora	>int_state
;	lda	>int_state
	sta 	>$E0C035
	lda >int_Y
	tay
	lda >int_X
	tax
	lda >int_D
	tcd

	short a
	lda >int_B
	pha
	plb
	lda >int_SLTROM
	sta >$E0C02D
	lda >int_STATEREG
	sta >$E0C068
	lda >int_P
	pha

	long a
	lda >int_A
	plp
	xce
	jmp oldInt
	END

* Databank MUST be set up before entry

contextSwitch	Start
	Using KernelStruct
	Using IntState

; there used to be a LONG here, but it's unnecessary

	lda >BusyFlag
	beq humph
	brl goaway

humph	anop

	lda #TIMESLICE
	sta timeleft             ; run me again please

	ldx curProcInd
	jsr GetInterruptState

	ldx curProcInd
	tsc
;              sta irq_S1,x

; if the process state isn't Running, the kernel wants to change it's status,
; so oblige it by not changing it to 'Ready'

	lda ProcessState,x
	cmp #pRunning            ; change of state?
	bne findit

spcl	anop
	lda #pReady
	sta ProcessState,x
findit	jsr FindNextEntry
	stx curProcInd
	txa
	clc
	adc	#KernelStruct
	sta	>procPtr
	lda	#^KernelStruct
	adc	#0
	sta	>procPtr+2

	txa
	lsr a
	lsr a
	lsr a
	lsr a
	lsr a
	lsr a                    ; divide by 128 to get true pid
	lsr a
	sta truepid
	
* Repair SANE's DP space
	phd
	lda	>$E103CA
	pha
	lda	>$E103C8
	pha
	tsc
	tcd
	ldy	#$A*4
	ldx	curProcInd
	lda	SANEwap,x
	sta	[1],y
	pla
	pla
	pld

	lda ProcessState,x
	cmp #pNew
	bne okay

; the following code is hopelessly obtuse, but it basically creates information
; on the stack to simulate an interrupt, so that the new process starts it's
; time slice like every other one does

	lda	#pRunning
	sta	ProcessState,x
	dec	irq_S,x	; make room on new stack
	dec	irq_S,x                    ;
	dec	irq_S,x                    ;
	dec	irq_S,x                    ;
	jsr	SetInterruptState          ; set up A,X,Y, etc
	lda	irq_D,x
	tcd
	lda	irq_S,x                    ; new proc's stack
	tcs                            ;
	lda	irq_P,x	;
	and	#%11111011	; make sure interrupts are on when
	sta	1,S	; we start the new process
	lda	irq_PC,x	; fake an interrupt
	sta	2,S	; on the new process' stack
	short	a
	lda	irq_K,x
	sta	4,s
	pha
	plb
	long	a
	lda	>intType
	bne	vbl
	jmp	newViaCOP                  ; this is required in order to
vbl	jmp	newViaVBL                  ; turn off VBL if one occurred

	longa on                       ; grr stupid stupid m/x 816
	longi on                       ; fricken bits

okay	anop
*                                       ; we'll return to COP or int hndl
	jsr SetInterruptState
	ldx curProcInd
	lda #pRunning
	sta ProcessState,x
;              lda irq_S1,x
;              clc
;              adc #3
	lda irq_S,x
	tcs
	lda >intType
	bne vbl1
	jmp newViaCOP                  ; this is required in order to
vbl1	jmp newViaVBL                  ; turn off VBL if one occurred

goaway	anop                     ; well, so much for that!
	rtl
temp	dc i4'0'
timState	dc i2'0'
stat	dc i2'0'
	END

	longa on
	longi on

GetInterruptState	START
	using KernelStruct
	using IntState

	lda int_A
	sta irq_A,x
	lda int_X
	sta irq_X,x
	lda int_Y
	sta irq_Y,x
	lda int_B
	sta irq_B,x
	lda int_P
	sta irq_P,x
	lda int_D
	sta irq_D,x
	lda int_S
	sta irq_S,x
	lda int_state
	sta irq_state,x
	lda int_SLTROM
	sta irq_SLTROM,x
	rts
	END

SetInterruptState	START
	using KernelStruct
	using IntState

	lda irq_A,x
	sta int_A
	lda irq_X,x
	sta int_X
	lda irq_Y,x
	sta int_Y
	lda irq_B,x
	sta int_B
	lda irq_P,x
	sta int_P
	lda irq_D,x
	sta int_D
	lda irq_S,x
	sta int_S
	lda irq_state,x
	sta int_state
	lda irq_SLTROM,x
	sta int_SLTROM
	rts
	END

; we need FindNextEntry to return x -> ProcessState,x
; When we're of a mind to, rewrite this to support priority scheduling
; (it will involve keeping a queue of 'Ready' processes).

FindNextEntry	Start
	Using KernelStruct
	ldx curProcInd
	txa
	clc
	adc #procEntSize
	tax
	cpx #(procEntSize*NPROC)
	bmi loop
	ldx #0
loop	lda ProcessState,x
	cmp #pReady
	beq found
	cmp #pNew
	beq found
	txa
	clc
	adc #procEntSize
	tax

	cpx #(procEntSize*NPROC)
	bcc loop
	ldx #0
	bra loop
found	rts
	End

* if X is -1 then the FindEmptyEntry call failed
* this needs to return offset in ProcessState otherwise

FindEmptyEntry	Start
	Using KernelStruct
	ldx #procEntSize
loop	lda ProcessState,x
	cmp #pUnused
	beq found
	txa
	clc
	adc #procEntSize
	tax
	cpx #(procEntSize*NPROC)
	bcc loop
notfound	ldx #$FFFF
found	rts
	End


; the NASTY assembly interface to the process tables
; currently, eight process entries are defined, and this must be set
; manually if NUMPROC in the system header files are changed. Kinda yucky,
; but then there's no other way.

KernelStruct	DATA
CKernData	ENTRY
ParentProc	dc    i2'-1'             ;0
ProcessState	dc    i2'0'              ;2
procUserID	dc    i2'0'              ;4
ttyID	dc    i2'0'              ;6
irq_A	dc    i2'0'              ;8
irq_X	dc    i2'0'              ;10
irq_Y	dc    i2'0'              ;12
irq_S	dc    i2'0'              ;14
irq_D	dc    i2'0'              ;16
irq_B	dc    i2'0'              ;18
irq_P	dc    i2'0'              ;20
irq_state	dc    i2'0'              ;22
irq_PC	dc    i2'0'	;24
irq_K	dc    i2'0'              ;26
psem	dc    i2'0'              ;28
prefixh	dc    i4'0'              ;30
argsp	dc    i4'0'              ;34
env	dc    i4'0'              ;38
siginfo	dc    i4'0'              ;42
irq_SLTROM	dc    i1'0'              ;46
irq_STATEREG	dc    i1'0'              ;47
resapp	anop
lastTool	dc    i2'0'              ;48
ticks	dc    i4'0'              ;50
flags	dc    i2'0'              ;54
openFiles	dc    i4'0'              ;56
pgrp	dc    i2'0'	;60
exitCode	dc    i2'0'              ;62
LInfo	dc    i4'0'              ;64
stoppedState	dc    i2'0'              ;68
alarmCount	dc    i4'0'              ;70
executeHook	dc    i4'0'	;74
queueLink	dc    i2'0'              ;78
waitq	dc    i4'0'              ;80
waitdone	dc    i2'0'              ;84
flpid	dc    i2'1'              ;86
retStack	dc	i4'0'              ;88
t2StackPtr	dc  i2'0'              ;92
p_uid	dc	i2'0'	;94
p_gid	dc	i2'0'	;96
p_euid	dc	i2'0'              ;98
p_egid	dc	i2'0'	;100
SANEwap	dc	i2'0'	;102
msg	dc	i4'0'	;104
childTicks	dc  i4'0'	;108
p_waitvec	dc  i4'0'	;112
p_slink	dc	i2'0'	;116
p_link	dc	i4'0'	;118
p_rlink	dc	i4'0'	;122
p_prio	dc	i2'0'	;126
procEntSize	gequ  *-CKernData

ProcessQueue	ds 128*31                ; more o' the above
curProcInd	dc i2'0'                 ; basically current process ID
userID	dc i2'0'                 ; kernel userID
mutexNOMO	dc i2'0'                 ; system-wide mutex flag.
timeleft	dc i2'TIMESLICE'	; number of ticks left in timeslice
numProcs	dc i2'1'                 ; number of processes active (inc.Null)
truepid	dc i2'0'                 ; curProcInd/128
shutdown	dc i2'0'                 ; should the null process exit?
gsosDebug	dc i2'0'                 ; GS/OS Debug level
floatingPID	dc i2'2'                 ; next pid to allocate
reschedFlag	dc i2'0'
	END

_asmresched	START
	using KernelStruct

* give process our remaining time plus theirs
	lda >timeleft
	inc a
	sta >timeleft
	cop $7f                  ; force a context switch
	rts
	END

vogue	DATA
	dc c'TIDDLYWINKS000000',h'00'
	END

NullProcess	START
	using KernelStruct
	using sigQueue
	using	deferdata

itself	anop

; check for deferred actions (probably from an interrupt handler to wake
; up a process) and execute them until no more are left.
defer_loop	lda defer_num
	beq	no_defer
	jsl	exec_defer
	bra	defer_loop
no_defer	anop

* if a CDA event is pending across two consecutive runs of the nullprocess,
* GetNextEvent to initiate the CDA.
* Only do the check every four invocations of NullProcess to avoid
* unnecessary overhead

	dec	loopcnt
	bne   noCDA
	lda	#4
	sta	loopcnt

	pha
	pea	$0400
	ph4	#evtRecord
	_EventAvail
	pla
	cmp	#0
	beq	noCDA
	lda   cdaFlag
	cmp	#1
	bcc	justInc
	pha
	pea	$0400
	ph4	#evtRecord
	_GetNextEvent
	pla
	stz	cdaFlag
	bra	noCDA

justInc	inc	a
	sta	cdaFlag

noCDA	lda   sighead
	cmp   sigtail
	beq   nosigs
	jsl   incBusy
	lda   sighead
	asl   a
	tax

	pha
	lda	procs,x
	pha
	lda	sigs,x
	pha
	ph4	#sigerr
	jsl	KERNkill	; this is #pragma toolparms 1
	pla

	lda   sighead
	inc   a
	sta   sighead
	cmp   #32
	bcc   inrange
	stz   sighead
inrange	jsl   decBusy

nosigs	anop
	lda   shutdown
	bne   endGNO   ; kill() will shut down processes before this
	jsl	readyEntry
	cpx	#0       	; if it only found us, loop to avoid
	beq	jumpfuck	; interrupt overhead
	cop   $7f	; otherwise, let that process run
jumpfuck	jmp	itself
endGNO	anop
	rtl
sigerr	dc    i2'0'
loopcnt	dc	i2'4'
cdaFlag	dc	i2'0'
evtRecord	ds  16
	END

sigQueue	DATA
sighead	dc i2'0'
sigtail	dc i2'0'
sigs	ds 64
procs	ds 64
	END

queueSignal	START
	using sigQueue
	pha
	phx
	lda   sigtail
	asl   a
	tax
	lda   1,s
	sta   sigs,x
	lda   3,s
	sta   procs,x

	lda   sigtail
	inc   a
	sta   sigtail
	cmp   #32
	bcc   inrange
	stz   sigtail

inrange	plx
	pla
	rts
	END

	longa off
alarmHB	DATA 
	dc i4'0'
ALtimer	dc i2'6'
	dc h'5AA5'
	END
alarmRoutine	START
	using alarmHB
	using KernelStruct
	using SelTimStruct

	php
	long  ai

	phb
	phk
	plb
; re-activate our heart beat entry
	lda   #6
	sta   ALtimer

; check for SIGALRM timeouts
	ldx   #0
loop	anop
	lda   ProcessState,x
	cmp   #pUnused
	beq   nextProc
	lda   alarmCount,x
	ora   alarmCount+2,x
	beq   nextProc
	lda   alarmCount,x
	sec
	sbc   #1
	sta   alarmCount,x
	lda   alarmCount+2,x
	sbc   #0
	sta   alarmCount+2,x
	ora   alarmCount,x
	bne   nextProc
	phx
	lda   flpid,x
	ldx   #14
	jsr   queueSignal
	plx
nextProc	anop
	txa
	clc
	adc #procEntSize
	tax
	cpx #(procEntSize*NPROC)
	bcc loop

; check for select() timeouts
	ldx	#0
selloop	anop
	lda	SelTimTimeout,x
	ora	SelTimTimeout+2,x
	beq	nextSel
	lda	SelTimTimeout,x
	sec
	sbc	#1
	sta	SelTimTimeout,x
	lda	SelTimTimeout+2,x
	sbc	#0
	sta	SelTimTimeout+2,x
	ora	SelTimTimeout,x
	bne	nextSel
	lda	#1
	sta	SelTimExpired,x
	phx
	lda	SelTimPID,x	; wake up this PID
	pha
	pea	0		; no collision
	jsl	selwakeup
	plx
nextSel	anop
	txa
	clc
	adc	#8
	tax
	cpx	#(8*NPROC)
	bcc	selloop

	plb
	plp
	rtl
	END

* AHHA!
* The remarkably simple new Kernel mutex method
* is totally based off of BusyFlag.  Now things seem to work much
* smoother, and I can accomplish all sorts of stuff now.
* Yea, right, you dumbshit (jb 7/5/93)

incBusy	START
disableps	ENTRY
	jmp   >$E10064
decBusy	ENTRY
enableps	ENTRY
	jmp   >$E10068
	END

newDecBusy	START
	using KernelStruct

	php
	pha
	jsl   oldDecBusy
	lda   >timeleft
	bne   stillmore
	lda   >BusyFlag
	bne   stillmore
	lda   >reschedFlag
	bne   stillmore          ; don't reschedule

* We now allow a context switch when interrupts are off - this
* allows flawless execution of semaphore sleep code, etc.

	lda	3,s                ; check the P reg- is the
	bit	#4                 ; i bit set? if so, do not task
	bne	stillmore	; switch

	lda	>$E100CB
	and	#$FF
	bne	stillmore	; we're in an interrupt handler!

	tsc
	and	#$FF00
	cmp	#$0100
	beq	stillmore

	pla
	plp
	jsl	fakeACop
;	cop $7f                  ; we're out o' time
	rtl
stillmore	pla
	plp
	rtl

	END

ttyStruct	START
pgrp	dc    38i2'0'
	END

* BGCheck MUST MUST MUST be called only if the busy flag is zero
* otherwise, the process will not be suspended before it does the
* I/O procedure.
* Note that the use of this routine in TextTools to generate an error code
* is okay, because the only way for a program to have blocked or ignored
* the signal is to be aware of GNO, and if they do that they should check for
* errors from these texttools calls (which they should check for anyway).
* Input: A = signal number
*        X = TTY device number
* needs to be in the kernel segment

BGCheck	START
	using KernelStruct

retval	equ   1
signum	equ   1
signum2	equ   5
sigptr	equ   1

	phb
	phk
	plb
	phd
	pha                      ; push input parameter
	tsc
	tcd

	txa
	asl   a
	tax
	lda   ttyStruct,x
	beq   nopgrp
	ldx   curProcInd
	cmp   pgrp,x
	beq   nopgrp             ; it's the foreground process
	lda   siginfo+2,x
	pha
	lda   siginfo,x
	pha
	tsc
	tcd
	lda   #$0010
	ldy   signum2
	cpy   #21
	beq   noasl
	asl   a
noasl	ldy   #2
	and   [sigptr],y
	bne   sigblocked
	lda   signum2
	dec   a
	asl   a
	asl   a
	clc
	adc   #8
	tay
	lda   [sigptr],y
	cmp   #1
	bne   notign
	iny2
	lda   [sigptr],y
	cmp   #0
	bne   notign
* the signal is ignored or blocked, so do not send the signal, and
* tell the caller that we did not.
sigblocked	pla     
	pla
	pla
	lda   #-1
	pld
	plb
	rtl
* the signal was not ignored or blocked, so send the signal,
* tell the caller that we did, and return

notign	lda   pgrp,x
	pha
	pei   (signum2)
	jsl   asmkillpg
	cop   $7F
	pla
	pla
	pla
	lda   #1
	pld
	plb
	rtl
* the caller was indeed in the TTY's process group, and so should be left
* quite alone.
nopgrp	pla
	lda   #0
	pld
	plb
	rtl
	END

asmkillpg	START
	using KernelStruct
	subroutine (2:pg,2:signum),0

	ldx   #128               ; don't EVER signal the kernel
sigloop	lda   ProcessState,x
	cmp   #pUnused
	beq   nextproc
	lda   pgrp,x
	cmp   pg
	bne   nextproc

	phx
	lda   flpid,x
	ldx   signum
	jsr   queueSignal
	plx

nextproc	anop
	txa
	clc
	adc   #128
	tax
	cpx   #4096              ; oops!
	bcc   sigloop
	return
	END

extQSignal	START
	using	KernelStruct
space	equ	0
	subroutine (2:s,2:p),space
	lda	p
	ldx	s
	jsr	queueSignal
	return
	END

ttyQSignal	START
	using	KernelStruct
space	equ	0
	subroutine (2:s,2:tty),space

	lda	tty
	asl	a
	tax
	lda   ttyStruct,x
	beq	noSignal	; no process group
	pha
	ldx   #128               ; don't EVER signal the kernel
sigloop	lda   ProcessState,x
	cmp   #pUnused
	beq   nextproc
	lda   pgrp,x
	cmp   1,s
	bne   nextproc

	phx
	lda   >flpid,x
	ldx   s
	jsr   queueSignal
	plx
nextproc	anop
	txa
	clc
	adc   #128
	tax
	cpx   #4096              ; oops!
	bcc   sigloop
	pla
noSignal	return
	END

snooperInfo	DATA KERN2
blockLen	dc	i2'len-blockLen'
nameString	str  '~PROCYON~GNO~SNOOPER'
s_procTable	dc  i4'CKernData'
s_pipeTable	dc  i4'pipeRecord'
refTrack	dc	i4'pipeData'	; lots of dereffing, but hey!
queueSig	dc	i4'extQSignal'
info1	dc	i4'savedCDAInfo'
reftty	dc	i4'ttyStruct'
refpgrp	dc	i4'pgrpInfo'
refRefTab	dc  i4'pipeData'
len	equ	*
	END

* At this point, the process RTL address is on the stack.  Push the P
* register to complete simulating the interrupt environment.

fakeACop	START
	php
	sei
	long	m
	pha
	lda	4,s
	inc	a	; 'fix' rti address
	sta	4,s
	pla
	jmp	>CopVect
	END

readyEntry	Start KERN2
	Using KernelStruct

	ldx	#(1*procEntSize)
loop	lda 	>ProcessState,x
	cmp	#pReady
	beq	found
	cmp	#pNew
	beq	found
	txa
	clc
	adc	#procEntSize
	tax

	cpx	#(procEntSize*NPROC)
	bcc	loop
	ldx	#0
	rtl
found	rtl
	End

sleepbusy	START KERN2
	php
	sei
	short	m
	lda	>$E100FF
	pha
	lda	#0
	sta	>$E100FF
	cop	$7F
	pla
	sta	>$E100FF
	plp
	rtl
	END

	longa	on
	longi	on

exec_defer	START
	using	deferdata
proc	equ	0
val1	equ	4
val2	equ	6

	subroutine (0:foo),8

	phb
	phk
	plb

	php
	sei
	ldy	defer_tail
	tya
	iny
	cpy	#NUM_DEFER
	bne	nowrap
	ldy	#0
nowrap	sty	defer_tail
	dec	defer_num
	asl	a
	asl	a
	tax
	lda	defertab,x
	sta	proc
	lda	defertab+2,x
	sta	proc+2
	lda	defertab+4,x
	sta	val1
	lda	defertab+6,x
	sta	val2
	plp

; pass params in 'C' format
	pei	(val2)
	pei	(val1)
; push return address
	phk
	pea	|comeback-1
; push routine address
	short	m
	lda	proc+2
	pha
	long	m
	lda	proc
	dec	a
	pha
	rtl

comeback	anop
	plb
	return

	END

defer	START
	using deferdata
	subroutine (4:procPtr,2:val1,2:val2),

	phb
	phk
	plb

	php
	sei
	lda	defer_num
	cmp	#NUM_DEFER
	beq	fail

	ldy   defer_head
	tya
	iny
	cpy	#NUM_DEFER
	bne	nowrap
	ldy	#0
nowrap	sty	defer_head
	inc	defer_num
	asl	a
	asl	a
	tax
	lda   procPtr
	sta   defertab,x
	lda	procPtr+2
	sta	defertab+2,x
	lda   val1
	sta	defertab+4,x
	lda	val2
	sta	defertab+6,x
fail1	plp

	plb
	return
fail	anop
	short m
	lda	>$E0C034
	and	#$F0
	pha
	lda	>$E0C034
	inc	a
	and	#$F
	ora	1,s
	sta	>$E0C034
	pla
	long	m
	bra	fail1
	END

deferdata	DATA
defer_head	dc  i2'0'
defer_tail	dc  i2'0'
defer_num	dc  i2'0'
defertab	ds  8*NUM_DEFER
	END

k_sleep	START
	using	KernelStruct

hv	equ	0
last	equ	2
pid128	equ	4

	subroutine (2:pid,2:pri,4:vec),6

	jsl	disableps

	lda	vec	hv = hash_vector(vec);
	and	#$3F
	sta	hv
	lda	pid
	and	#$00FF
	xba
	lsr	a
	sta	pid128
	tax
	lda	vec
	sta	>p_waitvec,x
	lda	vec+2
	sta	>p_waitvec+2,x

	lda	hv
	asl	a
	tax
	lda	>hv_tab,x
	sta	last
	cmp	#0
	bne	addtoend
	lda	pid
	sta	>hv_tab,x
	bra	skip1

addtoend	anop
	and	#$FF
	xba
	lsr	a
	tax
	lda	>p_slink,x
	bne	addtoend

; last proc index is in X

	lda	pid
	sta	>p_slink,x
skip1	lda	#0
	ldx	pid128
	sta	>p_slink,x
	pei	(pid)
	jsl	dosleep
	jsl	sleepbusy
	jsl	enableps
	return
	END

	longa	on
	longi	on
* used to be in signal.c, orig. C source still is
Kreceive	START KERN2
	using	KernelStruct

tmp	equ	0
	subroutine (4:errnoptr),4

	lda	>curProcInd
	tax

	php
	sei
	lda	#2
	sta   >waitdone,x
	lda	>flags,x
	bit	#$0080	; FL_MSGRECVD
	bne	gotmsg
	lda	#3
	sta	>ProcessState,x
	jsl	sleepbusy
gotmsg	lda	>waitdone,x
	cmp	#2
	bne	huhwhat
	lda	>msg,x
	sta	tmp
	lda	>msg+2,x
	sta	tmp+2
	lda	>flags,x
;	and	#$0080.EOR.$FFFF
	and	#$FF7F
	sta	>flags,x
	bra	what2
huhwhat	lda	#$FFFF
	sta	tmp
	sta	tmp+2
what2	plp
	return 4:tmp
	END
