*	$Id: texttool.asm,v 1.1 1998/02/02 08:19:50 taubert Exp $
**************************************************************************
*
* The GNO Multitasking Environment Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*
**************************************************************************
*
* TEXTTOOL.ASM
*   By Tim Meekins
*   and Jawaid Bazyar
* GNO replacement for TextTools toolset
*  Supports redirection, pipes, and loadable drivers
*
* Copyright 1991, Jawaid Bazyar
* 
**************************************************************************

	case	on
	mcopy m/texttool.mac
	copy  global.equates
	copy ../drivers/console.equates

;=========================================================================
;
; Text Tool Data
;
;=========================================================================

IgnoreInfo	DATA
ignSetInDev	dc  i2'0'
ignSetInGlo	dc  i2'0'
ignSetOutDev	dc  i2'0'
ignSetOutGlo	dc  i2'0'
ignSetErrDev	dc  i2'0'
ignSetErrGlo	dc  i2'0'   
	END

TextToolsInfo	DATA
InANDMask	dc    i2'$FF'
InORMask	dc    i2'$80'
OutANDMask	dc    i2'$FF'
OutORMask	dc    i2'$80'
ErrANDMask	dc    i2'$FF'
ErrORMask	dc    i2'$80'
InDeviceType	dc    i2'1'
OutDeviceType	dc    i2'1'
ErrDeviceType	dc    i2'1'
InSlot	dc    i4'3'
OutSlot	dc    i4'3'
ErrSlot	dc    i4'3'
ProcessGroup	dc    i2'0'              ; that's 64 bytes to you and me
stateFlags	dc    i2'0'              ; info on CR/LF removal, plus ?
InVect	dc    i4'0'
OutVect	dc    i4'0'
ErrVect	dc    i4'0'

InCache	dc	i4'0'	; stderr need not be cached
OutCache	dc	i4'0'
	dc    5i2'0'
	ds    64*(NPROC-1)       ; space for other processes
	END

* Table of Installed TextTools Drivers

ReadDDTab	DATA
	dc    i4'0'              ; null device
	dc    i4'sl1Read-1'	; [serial 1]
	dc    i4'sl2Read-1'	; [serial 2]
	dc    i4'0'	; console driver (COUT/KEYIN)
	dc    i4'NullIn-1'	;
	dc    i4'NullIn-1'	;
	dc    i4'NullIn-1'	;
	dc    i4'NullIn-1'	;
	END
WriteDDTab	DATA
	dc    i4'0'              ; null device
	dc    i4'sl1Write-1'	; [serial 1]
	dc    i4'sl2Write-1'	; [serial 2]
	dc    i4'0'              ; console driver (COUT/KEYIN)
	dc    i4'NullOut-1'	;
	dc    i4'NullOut-1'	;
	dc    i4'NullOut-1'	;
	dc    i4'NullOut-1'	;
	END
InitDDTab	DATA
	dc    i4'NullInit-1'     ; null device
	dc    i4'sl1Init-1'	; [serial 1]
	dc    i4'sl2Init-1' 	; [serial 2]
	dc    i4'C80Init-1'      ; console driver (COUT/KEYIN)
	dc    i4'NullInit-1'	;
	dc    i4'NullInit-1'	;
	dc    i4'NullInit-1'	;
	dc    i4'NullInit-1'	;
	END
StatusDDTab	DATA
	dc	i4'NullStatus-1'
	dc    i4'sl1Status-1'	; [serial 1]
	dc	i4'sl2Status-1'
	dc	i4'NullStatus-1'
	dc	i4'NullStatus-1'
	dc	i4'NullStatus-1'
	dc	i4'NullStatus-1'
	dc	i4'NullStatus-1'
	dc	i4'NullStatus-1'

	END
	
NotDone	gequ  1      ; NULL entries are copied from old table

TTtable	DATA
	dc i4'(TheEnd-TTtable)/4'

	dc    i4'NewTextBootInit-1'    ; $010C
	dc    i4'NewTextStartUp-1'     ; $020C
	dc    i4'NewTextShutDown-1'    ; $030C
	dc    i4'NotDone-1'            ; $040C
	dc    i4'NewTextStartUp-1'     ; $050C
	dc    i4'NotDone-1'            ; $060C

	dc    i4'NotDone-1'            ;
	dc    i4'NotDone-1'            ; 

	dc    i4'NewSetInGlob-1'       ; $090C SetInGlobals
	dc    i4'NewSetOutGlob-1'      ; $0A0C SetOutGlobals
	dc    i4'NewSetErrGlob-1'      ; $0B0C SetErrGlobals

	dc    i4'NewGetInGlob-1'       ; $0C0C GetInGlobals
	dc    i4'NewGetOutGlob-1'      ; $0D0C GetOutGlobals
	dc    i4'NewGetErrGlob-1'      ; $0E0C GetErrGlobals

	dc    i4'NewSetInDevice-1'     ; $0F0C SetInputDevice
	dc    i4'NewSetOutDevice-1'    ; $100C SetOutputDevice
	dc    i4'NewSetErrDevice-1'    ; $110C SetErrorDevice

	dc    i4'NewGetInDevice-1'     ; $120C GetInputDevice
	dc    i4'NewGetOutDevice-1'    ; $130C GetOutputDevice
	dc    i4'NewGetErrDevice-1'    ; $140C GetErrorDevice

	dc    i4'NewInitText-1'        ; $150C InitTextDev
	dc    i4'NotDone-1'            ; $160C CtlTextDev
	dc    i4'NewStatusDev-1'       ; $170C StatusTextDev

	dc    i4'NewWriteChar-1'       ; $180C
	dc    i4'NewErrWriteChar-1'    ; $190C
	dc    i4'NewWriteLine-1'       ; $1A0C
	dc    i4'NewErrWriteLine-1'    ; $1B0C
	dc    i4'NewWriteString-1'     ; $1C0C
	dc    i4'NewErrWriteString-1'  ; $1D0C
	dc    i4'NewTextWriteBlock-1'  ; $1E0C
	dc    i4'NewErrWriteBlock-1'   ; $1F0C
	dc    i4'NewWriteCString-1'    ; $200C
	dc    i4'NewErrWriteCString-1' ; $210C
	dc    i4'NewReadChar-1'        ; $220C
	dc    i4'NewTextReadBlock-1'   ; $230C
	dc    i4'NewReadLine-1'        ; $240C
TheEnd	anop
	END


;=========================================================================
;
; Patch the text tool set
;
;=========================================================================

PatchText	START
	using ReadDDTab
	using WriteDDTab

	pushlong #0
	pushword #$0000
	pushword #$000C
	_GetTSPtr
	bcc   ok
	jmp   done

ok	pulllong >oldTextTT

	pushword #$0000          ; system tool set
	pushword #$0C            ; texttools
	pushlong #TTtable        ; thar she blows
	_SetTSPtr

	movelong #KEYIN-1,ReadDDTab+12
	movelong #COUT-1,WriteDDTab+12
	movelong #NullIn-1,ReadDDTab+0
	movelong #NullOut-1,WriteDDTab+0
	ldx   #0                 ; null process
	jsr   InitGlobals

done	anop
	rts
oldTextTT	ENTRY
	dc i4'0'
	END

;=========================================================================
;
; UnPatch the text tool set
;
;=========================================================================

UnPatchText	START

	pushword #$0000
	pushword #$000C
	pushlong >oldTextTT
	_SetTSPtr

	rts
	END

* We want this to do NOTHING...
NewTextBootInit	START
	lda   #0
	clc
	rtl
	END

NewTextStartUp	START
	using KernelStruct
*              phd
*              phb
*              phk
*              plb
*              lda   >truepid
*              xba
*              lsr   a
*              lsr   a
*              tax
*              jsr   InitGlobals
*              plb
	phd
	jmp   bye0
	END

NewTextShutDown	START
	phd
	jmp   bye0
	END

; FORKInitGlob
; KERNfork() jsl's this routine. x is source pid, y is destination pid.
; copies all the TextTools info from parent to child (source to dest)

FORKInitGlob	START
	using TextToolsInfo

	phb
	phk
	plb

	tya
	xba
	lsr   a
	lsr   a
	tay
	phy

	txa
	xba
	lsr   a
	lsr   a
	tax
	lda   #32
	sta   left
cpTInfo	anop
	lda   TextToolsInfo,x
	sta   TextToolsInfo,y
	inx
	inx
	iny
	iny
	dec   left
	bne   cpTInfo
	ply
	lda   #0
	sta   stateFlags,y

	plb
	rtl
left	dc    i2'0'
	END

InitGlobals	START
	using TextToolsInfo

	phx
	pea   $00FF
	pea   $0000
	_SetInGlobals
	pea   $00FF
	pea   $0000
	_SetOutGlobals
	pea   $00FF
	pea   $0000
	_SetErrGlobals

	pea   $0001
	ph4   #3
	_SetInputDevice
	pea   $0001
	ph4   #3
	_SetOutputDevice
	pea   $0001
	ph4   #3
	_SetErrorDevice
	plx

	lda   #0
	sta   stateFlags,x
	rts
	END

NewInitText	START
	using	TextToolsInfo
	using	KernelStruct
	using	InOutData

dev	equ	10

	phd
	lda   >InOutDP
	tcd

	phb
	phk
	plb

	lda   truepid
	xba
	lsr   a
	lsr   a
	tax
	lda	dev,s
	beq	doit
	inx
	inx
	cmp	#1
	beq	doit
	inx	
	inx	
doit	anop
	lda	InDeviceType,x
	cmp	#1
	bne	nodice

	lda   truepid
	xba
	lsr   a
	lsr   a
	tax
	lda	dev,s
	beq	doit2
	inx2
	inx2
	cmp	#1
	beq	doit2
	inx2	
	inx2	
doit2	anop
	lda	InSlot,x
	asl	a
	asl	a
	tax

	push3	#nodice-1
	short	m
	lda	InitDDTab+2,x
	pha
	long	m
	lda	InitDDTab,x
	pha
	rtl	

nodice	plb
	jmp   bye2
	END

NewStatusDev	START
	using	TextToolsInfo
	using	KernelStruct
	using	InOutData

dev	equ	12
req	equ	10

	phd
	lda   >InOutDP
	tcd

	phb
	phk
	plb

	lda   truepid
	xba
	lsr   a
	lsr   a
	tax
	lda	dev,s
	beq	doit
	inx
	inx
	cmp	#1
	beq	doit
	inx	
	inx	
doit	anop
	lda	InDeviceType,x
	cmp	#1
	bne	nodice

	lda   truepid
	xba
	lsr   a
	lsr   a
	tax
	lda	dev,s
	beq	doit2
	inx2
	inx2
	cmp	#1
	beq	doit2
	inx2	
	inx2	
doit2	anop
	lda	req,s
	tay
	lda	InSlot,x
	asl	a
	asl	a
	tax

	push3	#retadr-1
	short	m
	lda	StatusDDTab+2,x
	pha
	long	m
	lda	StatusDDTab,x
	pha
	tya
	rtl	
retadr	ldx	#0
	bcs	nodice
	ldx	#$0C40
nodice	plb
	pld
	ldy	#4
	jmp	tool_exit	
	END

NewGetOutDevice	START
	using TextToolsInfo
	using KernelStruct

	phb
	phk
	plb

	lda   >truepid
	xba
	lsr   a
	lsr   a
	tax

	lda   OutSlot,x
	sta   7+1,s
	lda   OutSlot+2,x
	sta   7+1+2,s
	lda   OutDeviceType,x
	sta   7+1+4,s
	plb
	phd                      ;ack!!
	jmp   bye0

	END

NewGetInDevice	START
	using TextToolsInfo
	using KernelStruct

	phb
	phk
	plb

	lda   >truepid
	xba
	lsr   a
	lsr   a
	tax

	lda   InSlot,x
	sta   7+1,s
	lda   InSlot+2,x
	sta   7+1+2,s
	lda   InDeviceType,x
	sta   7+1+4,s
	plb
	phd                      ;ack!!
	jmp   bye0

	END

NewGetErrDevice	START
	using TextToolsInfo
	using KernelStruct

	phb
	phk
	plb

	lda   >truepid
	xba
	lsr   a
	lsr   a
	tax

	lda   ErrSlot,x
	sta   7+1,s
	lda   ErrSlot+2,x
	sta   7+1+2,s
	lda   ErrDeviceType,x
	sta   7+1+4,s
	plb
	phd                      ;ack!!
	jmp   bye0

	END

* Device numbers
* 0-1: Pascal
* 2: RAM-based
* 3: file redirection
* 4: pipe

NewSetOutDevice	START
	using IgnoreInfo
	using TextToolsInfo
	using KernelStruct

	phb
	phk
	plb
	lda	ignSetOutDev
	beq	noign
	stz	ignSetOutDev
	bra	outta
noign	anop
	
	lda   >truepid
	xba
	lsr   a
	lsr   a
	tax

	lda   7+1,s
	sta   OutSlot,x
	lda   7+1+2,s
	sta   OutSlot+2,x
	lda   7+1+4,s
	sta   OutDeviceType,x
	cmp   #2
	bcc   setPascal
	beq   setRAM
	cmp   #3
	beq   setFile
	lda   #pipeOUT-1
	sta   OutVect,x
	lda   #^pipeOUT-1
	sta   OutVect+2,x
	bra   outta
setFile	anop
	lda   #fileOUT-1
	sta   OutVect,x
	lda   #^fileOUT-1
	sta   OutVect+2,x
	bra   outta
setPascal	lda   OutSlot,x
	asl   a
	asl   a
	tay
	lda   WriteDDTab,y
	sta   OutVect,x
	lda   WriteDDTab+2,y
	sta   OutVect+2,x
	bra   outta
setRAM	lda   OutSlot,x
	clc
	adc   #5
	sta   OutVect,x
	lda   OutSlot+2,x
	adc   #0
	sta   OutVect+2,x
outta	plb
	phd                      ;ack!!
	jmp   bye6

	END

NewSetInDevice	START
	using IgnoreInfo
	using TextToolsInfo
	using KernelStruct

	phb
	phk
	plb

	lda	ignSetInDev
	beq	noign
	stz	ignSetInDev
	bra	outta
noign	anop
	lda   >truepid
	xba
	lsr   a
	lsr   a
	tax

	lda   7+1,s
	sta   InSlot,x
	lda   7+1+2,s
	sta   InSlot+2,x
	lda   7+1+4,s
	sta   InDeviceType,x
	cmp   #2
	bcc   setPascal
	beq   setRAM
	cmp   #3
	beq   setFile
	brk	$00

*              lda   #pipeIN-1
*              sta   InVect,x
*              lda   #^pipeIN-1
*              sta   InVect+2,x
*              bra   outta
setFile	anop
	lda   #fileIN-1
	sta   InVect,x
	lda   #^fileIN-1
	sta   InVect+2,x
	bra   outta
setPascal	lda   InSlot,x
	asl   a
	asl   a
	tay
	lda   ReadDDTab,y
	sta   InVect,x
	lda   ReadDDTab+2,y
	sta   InVect+2,x
	bra   outta
setRAM	lda   InSlot,x
	clc
	adc   #2
	sta   InVect,x
	lda   InSlot+2,x
	adc   #0
	sta   InVect+2,x
outta	plb
	phd                      ;ack!!
	jmp   bye6

	END

NewSetErrDevice	START
	using TextToolsInfo
	using KernelStruct

	phb
	phk
	plb

	lda   >truepid
	xba
	lsr   a
	lsr   a
	tax

	lda   7+1,s
	sta   ErrSlot,x
	lda   7+1+2,s
	sta   ErrSlot+2,x
	lda   7+1+4,s
	sta   ErrDeviceType,x
	cmp   #2
	bcc   setPascal
	beq   setRAM
	lda   #fileERR-1
	sta   ErrVect,x
	lda   #^fileERR-1
	sta   ErrVect+2,x
	bra   outta
setPascal	lda   ErrSlot,x
	asl   a
	asl   a
	tay
	lda   WriteDDTab,y
	sta   ErrVect,x
	lda   WriteDDTab+2,y
	sta   ErrVect+2,x
	bra   outta
setRAM	lda   ErrSlot,x
	clc
	adc   #5
	sta   ErrVect,x
	lda   ErrSlot+2,x
	adc   #0
	sta   ErrVect+2,x

outta	plb
	phd                      ;ack!!
	jmp   bye6

	END

;=========================================================================
;
; _SetInGlobals
;
;=========================================================================

NewSetInGlob	START
	using IgnoreInfo
	using TextToolsInfo
	using KernelStruct

	phb
	phk
	plb

	lda	ignSetInGlo
	beq	noign
	stz	ignSetInGlo
	bra	outta
noign	anop
	lda   >truepid
	xba
	lsr   a
	lsr   a

	tax

	lda   7+1,s
	sta   InORMask,x
	lda   7+1+2,s
	sta   InANDMask,x

outta	plb
	phd                      ;ack!!
	jmp   bye4

	END

NewSetOutGlob	START
	using IgnoreInfo
	using TextToolsInfo
	using KernelStruct

	phb
	phk
	plb

	lda	ignSetOutGlo
	beq	noign
	stz	ignSetOutGlo
	bra	outta
noign	anop
	lda   >truepid
	xba
	lsr   a
	lsr   a
	tax

	lda   7+1,s
	sta   OutORMask,x
	lda   7+1+2,s
	sta   OutANDMask,x
outta	plb
	phd                      ;ack!!
	jmp   bye4

	END

NewSetErrGlob	START
	using TextToolsInfo
	using KernelStruct

	phb       
	phk
	plb

	lda   >truepid
	xba
	lsr   a
	lsr   a
	tax

	lda   7+1,s
	sta   ErrORMask,x
	lda   7+1+2,s
	sta   ErrANDMask,x
	plb
	phd                      ;ack!!
	jmp   bye4

	END

NewGetInGlob	START
	using TextToolsInfo
	using KernelStruct

	phb       
	phk
	plb

	lda   >truepid
	xba
	lsr   a
	lsr   a
	tax

	lda   InORMask,x
	sta   7+1,s
	lda   InANDMask,x
	sta   7+1+2,s
	plb
	phd                      ;ack!!
	jmp   bye0

	END

NewGetOutGlob	START
	using TextToolsInfo
	using KernelStruct

	phb
	phk
	plb

	lda   >truepid
	xba
	lsr   a
	lsr   a
	tax

	lda   OutORMask,x
	sta   7+1,s
	lda   OutANDMask,x
	sta   7+1+2,s
	plb
	phd                      ;ack!!
	jmp   bye0

	END

NewGetErrGlob	START
	using KernelStruct
	using TextToolsInfo

	phb
	phk
	plb

	lda   >truepid
	xba
	lsr   a
	lsr   a
	tax

	lda   ErrORMask,x
	sta   7+1,s
	lda   ErrANDMask,x
	sta   7+1+2,s
	plb
	phd                      ;ack!!
	jmp   bye0

	END

;=========================================================================
;
; _WriteChar
;
;=========================================================================

NewWriteChar	START
	using TextToolsInfo
	using KernelStruct
	using	InOutData

	jsl   decBusy
	phd
	lda   >InOutDP
	tcd

	phb
	phk
	plb

	lda   >truepid
	xba
	lsr   a
	lsr   a
	tax

	lda   7+2+1,s
	and   OutANDMask,x
	ora   OutORMask,x
	jsl   VectOut
	jsl   incBusy
	plb
	jmp bye2
	END

;=========================================================================
;
; _ErrWriteChar
;
;=========================================================================

NewErrWriteChar	START
	using KernelStruct
	using TextToolsInfo
	using	InOutData

	jsl   decBusy
	phd
	lda   >InOutDP
	tcd

	phb
	phk
	plb

	lda   >truepid
	xba
	lsr   a
	lsr   a
	tax

	lda   7+2+1,s
	and   ErrANDMask,x
	ora   ErrORMask,x
	jsl   VectErr
	jsl   incBusy
	plb
	jmp bye2
	END

;=========================================================================
;
; WriteLine
;
;=========================================================================

NewWriteLine	START
	using KernelStruct
	using TextToolsInfo
	using InOutData

index	equ   9
count	equ   7
dp	equ   5
andmask	equ   3
ormask	equ   1
sixrtl	equ   14
strPtr	equ   20

	jsl   decBusy
	phd
	phb
	phk
	plb
	pea   1
	pha
	lda   >InOutDP
	pha
	pha
	pha
	tsc
	tcd
	lda   [strPtr]
	and   #$00FF
	sta   count
	lda   truepid
	xba
	lsr   a
	lsr   a
	tax
	lda   OutANDMask,x
	sta   andmask
	lda   OutORMask,x
	sta   ormask
	lda   count
	beq   done
lp	ldy   index
	lda   [strPtr],y
	and   #$00FF
	and   andmask
	ora   ormask
	phd
	pei   (dp)
	pld
	jsl   VectOut
	pld
	ldy   index
	cpy   count
	beq   done
	inc   index
	bra   lp

done	phd
	pei   (dp)
	pld
	lda   #13
	jsl   VectOut
	lda   #10
	jsl   VectOut
	pld
	jsl   incBusy
	pla
	pla
	pla
	pla
	pla
	plb
	jmp   bye4
	END

;=========================================================================
;
; _ErrWriteLine
;
;=========================================================================

NewErrWriteLine	START
	using KernelStruct
	using TextToolsInfo
	using InOutData

index	equ   9
count	equ   7
dp	equ   5
andmask	equ   3
ormask	equ   1
sixrtl	equ   14
strPtr	equ   20

	jsl   decBusy
	phd
	phb
	phk
	plb
	pea   1
	pha
	lda   >InOutDP
	pha
	pha
	pha
	tsc
	tcd
	lda   [strPtr]
	and   #$00FF
	sta   count
	lda   truepid
	xba
	lsr   a
	lsr   a
	tax
	lda   OutANDMask,x
	sta   andmask
	lda   OutORMask,x
	sta   ormask
	lda   count
	beq   done

lp	ldy   index
	lda   [strPtr],y
	and   #$00FF
	and   andmask
	ora   ormask
	phd
	pei   (dp)
	pld
	jsl   VectErr
	pld
	ldy   index
	cpy   count
	beq   done
	inc   index
	bra   lp

done	phd
	pei   (dp)
	pld
	lda   #13
	jsl   VectErr
	lda   #10
	jsl   VectErr
	pld
	jsl   incBusy
	pla
	pla
	pla
	pla
	pla
	plb

	jmp bye4
	END

;=========================================================================
;
; _WriteString
;
;=========================================================================

NewWriteString	START
	using KernelStruct
	using TextToolsInfo
	using InOutData

index	equ   9
count	equ   7
dp	equ   5
andmask	equ   3
ormask	equ   1
sixrtl	equ   14
strPtr	equ   20

	jsl   decBusy
	phd
	phb
	phk
	plb
	pea   1
	pha
	lda   >InOutDP
	pha
	pha
	pha
	tsc
	tcd
	lda   [strPtr]
	and   #$00FF
	sta   count
	lda   truepid
	xba
	lsr   a
	lsr   a
	tax
	lda   OutANDMask,x
	sta   andmask
	lda   OutORMask,x
	sta   ormask
	lda   count
	beq   done

lp	ldy   index
	lda   [strPtr],y
	and   #$00FF
	and   andmask
	ora   ormask
	phd
	pei   (dp)
	pld
	jsl   VectOut
	pld
	ldy   index
	cpy   count
	beq   done
	inc   index
	bra   lp

done	anop
	jsl   incBusy
	pla
	pla
	pla
	pla
	pla
	plb
	jmp   bye4
	END

;=========================================================================
;
; _ErrWriteString
;
;=========================================================================

NewErrWriteString	START
	using KernelStruct
	using TextToolsInfo
	using InOutData

index	equ   9
count	equ   7
dp	equ   5
andmask	equ   3
ormask	equ   1
sixrtl	equ   14
strPtr	equ   20

	jsl   decBusy
	phd
	phb
	phk
	plb
	pea   1
	pha
	lda   >InOutDP
	pha
	pha
	pha
	tsc
	tcd
	lda   [strPtr]
	and   #$00FF
	sta   count
	lda   truepid
	xba
	lsr   a
	lsr   a
	tax
	lda   OutANDMask,x
	sta   andmask
	lda   OutORMask,x
	sta   ormask
	lda   count
	beq   done

lp	ldy   index
	lda   [strPtr],y
	and   #$00FF
	and   andmask
	ora   ormask
	phd
	pei   (dp)
	pld
	jsl   VectErr
	pld
	ldy   index
	cpy   count
	beq   done
	inc   index
	bra   lp

done	anop
	jsl   incBusy
	pla
	pla
	pla
	pla
	pla
	plb

	jmp   bye4
	END

;=========================================================================
;
; _WriteCString
;
;=========================================================================

NewWriteCString	START
	using KernelStruct
	using TextToolsInfo
	using InOutData

index	equ   9
count	equ   7
dp	equ   5
andmask	equ   3
ormask	equ   1
sixrtl	equ   14
strPtr	equ   20

	jsl   decBusy
	phd
	phb
	phk
	plb
	pea   0
	pha
	lda   >InOutDP
	pha
	pha
	pha
	tsc
	tcd

	lda   truepid
	xba
	lsr   a
	lsr   a
	tax

	lda   OutANDMask,x
	sta   andmask
	lda   OutORMask,x
	sta   ormask

lp	ldy   index
	lda   [strPtr],y
	and   #$00FF
	cmp   #0
	beq   done
	and   andmask
	ora   ormask
	phd
	pei   (dp)
	pld
	jsl   VectOut
	pld
	inc   index
	bra   lp

done	anop
	jsl   incBusy
	pla
	pla
	pla
	pla
	pla
	plb

	jmp bye4
	END

;=========================================================================
;
; _ErrWriteCString
;
;=========================================================================

NewErrWriteCString	START
	using KernelStruct
	using TextToolsInfo
	using InOutData

index	equ   9
count	equ   7
dp	equ   5
andmask	equ   3
ormask	equ   1
sixrtl	equ   14
strPtr	equ   20

	jsl   decBusy
	phd
	phb
	phk
	plb
	pea   0
	pha
	lda   >InOutDP
	pha
	pha
	pha
	tsc
	tcd

	lda   truepid
	xba
	lsr   a
	lsr   a
	tax

	lda   OutANDMask,x
	sta   andmask
	lda   OutORMask,x
	sta   ormask

lp	ldy   index
	lda   [strPtr],y
	and   #$00FF
	cmp   #0
	beq   done
	and   andmask
	ora   ormask
	phd
	pei   (dp)
	pld
	jsl   VectErr
	pld
	inc   index
	bra   lp

done	anop
	jsl   incBusy
	pla
	pla
	pla
	pla
	pla
	plb

	jmp bye4

	END

;=========================================================================
;
; _TextWriteBlock
;
;=========================================================================

NewTextWriteBlock	START
	using KernelStruct
	using TextToolsInfo
	using InOutData

dp	equ   5
andmask	equ   3
ormask	equ   1
sixrtl	equ   10
strPtr	equ   20
index	equ   18
count	equ   16

	jsl   decBusy
	phd                      ; 8,s
	phb                      ; 7,s
	phk
	plb
	lda   >InOutDP
	pha                      ; 5,s (dp)
	pha                      ; 3,s (andmask)
	pha                      ; 1,s (ormask)
	tsc
	tcd

	lda   truepid
	xba
	lsr   a
	lsr   a
	tax
	lda   OutANDMask,x
	sta   andmask
	lda   OutORMask,x
	sta   ormask
	lda   count
	beq   done

lp	ldy   index
	lda   [strPtr],y
	and   #$00FF
	and   andmask
	ora   ormask
	phd
	pei   (dp)
	pld
	jsl   VectOut
	pld
	dec   count
	beq   done
	inc   index
	bra   lp

done	anop
	jsl   incBusy
	pla
	pla
	pla
	plb
	jmp   bye8
	END

;=========================================================================
;
; _ErrWriteBlock
;
;=========================================================================

NewErrWriteBlock	START
	using KernelStruct
	using TextToolsInfo
	using InOutData

dp	equ   5
andmask	equ   3
ormask	equ   1
sixrtl	equ   10
strPtr	equ   20
index	equ   18
count	equ   16

	jsl   decBusy
	phd                      ; 8,s
	phb                      ; 7,s
	phk
	plb
	lda   >InOutDP
	pha                      ; 5,s (dp)
	pha                      ; 3,s (andmask)
	pha                      ; 1,s (ormask)
	tsc
	tcd

	lda   truepid
	xba
	lsr   a
	lsr   a
	tax
	lda   OutANDMask,x
	sta   andmask
	lda   OutORMask,x
	sta   ormask
	lda   count
	beq   done

lp	ldy   index
	lda   [strPtr],y
	and   #$00FF
	and   andmask
	ora   ormask
	phd
	pei   (dp)
	pld
	jsl   VectErr
	pld
	dec   count
	beq   done
	inc   index
	bra   lp

done	anop
	jsl   incBusy
	pla
	pla
	pla
	plb
	jmp   bye8
	END

;=========================================================================
;
; _ReadChar
;
;=========================================================================

NewReadChar	START
	using KernelStruct
	using TextToolsInfo
	using	InOutData

	jsl   decBusy

	phd                      ; where does this go?
	lda   >InOutDP
	tcd

	phb
	phk
	plb

dobgcheck	lda   truepid
	xba
	lsr   a
	lsr   a
	tax
	lda   InDeviceType,x
	cmp   #1
	bne   isfg
	lda   InSlot,x
	tax
	lda   #21
	jsl   BGCheck
	cmp   #0
	beq   isfg
	cmp   #1
	beq   dobgcheck          ; we were suspended, try again

	jsl   incBusy            ; the return code was -1, which means
	plb                      ; to abort this call with an error
	ldx   #$0027             ; code.
	pld
	ldy   #2
	jmp   >tool_exit

isfg	anop

	jsl   VectIn
	pha
	lda   >truepid
	xba
	lsr   a
	lsr   a
	tax
	pla
	and   InANDMask,x
	ora   InORMask,x
	sta   7+2+1+2,s

	lda   7+2+1,s
	and   #%1
	beq   done
	lda   7+2+1+2,s          ;Echo the character
	and   OutANDMask,x
	ora   OutORMask,x
	jsl   VectOut

done	anop
	jsl   incBusy
	plb
	jmp   bye2

	END

;=========================================================================
;
; _TextReadBlock
;
;=========================================================================

NewTextReadBlock	START
	using KernelStruct
	using TextToolsInfo

	jsl   decBusy
	phd
	tsc
	tcd
	phb
	phk
	plb

dobgcheck	lda   truepid
	xba
	lsr   a
	lsr   a
	tax
	lda   InDeviceType,x
	cmp   #1
	bne   isfg
	lda   InSlot,x
	tax
	lda   #21
	jsl   BGCheck
	cmp   #0
	beq   isfg
	cmp   #1
	beq   dobgcheck          ; we were suspended, try again

	jsl   incBusy            ; the return code was -1, which means
*                                       ; to abort this call with an error
	plb
	ldx   #$0027             ; code.
	pld
	ldy   #10
	jmp   >tool_exit

isfg	anop
loop	anop
	jsl   VectIn
	pha
	lda   >truepid
	xba
	lsr   a
	lsr   a
	tax
	lda	1,s
	and   InANDMask,x
	ora   InORMask,x
	ldy   7+2+4
	short a
	sta   [7+2+6],y
	long  a

	lda   7+2+0
	and   #%1
	beq   noecho
	lda   1,s	;Echo the character
	and   OutANDMask,x
	ora   OutORMask,x
	jsl   VectOut

noecho	pla
	inc   7+2+4
	lda   7+2+4
	cmp   7+2+2
	bne   loop

done	anop
	jsl   incBusy
	plb
	jmp   bye10

	END

;=========================================================================
;
; _ReadLine
;
;=========================================================================

NewReadLine	START
	using KernelStruct
	using TextToolsInfo
	using InOutData

cmdlen	equ   1
cmdloc	equ   3
tmp	equ   5
wordspace	equ   7+2+6+10
bufferPtr	equ   7+2+6+6
maxCount	equ   7+2+6+4
eolChar	equ   7+2+6+2
echoFlag	equ   7+2+6

	phd
	pushword #0              ; space for tmp variables
	pushword #0
	pushword #0
	tsc
	tcd

	phb
	phk
	plb
	jsl   decBusy

dobgcheck	lda   truepid
	xba
	lsr   a
	lsr   a
	tax
	lda   InDeviceType,x
	cmp   #1
	bne   isfg
	lda   InSlot,x
	tax
	lda   #21
	jsl   BGCheck
	cmp   #0
	beq   isfg
	cmp   #1
	beq   dobgcheck          ; we were suspended, try again

	jsl   incBusy            ; the return code was -1, which means
	plb
	pla
	pla
	pla                      ; to abort this call with an error
	ldx   #$0027             ; code.
	pld
	ldy   #2
	jmp   >tool_exit

isfg	anop

	stz   <cmdlen
	stz   <cmdloc

cmdloop	anop
	phd
	lda   InOutDP
	tcd
	jsl   VectIn
	pld
	cmp   #0
	bpl   cmd0
	jmp   cmdoa              ;Do open apple
cmd0	and   #$7F
	if2   @a,ne,#$7F,cmd1
	jmp   cmddel
cmd1	if2   @a,eq,eolChar,eol
	if2   @a,cs,#' ',cmdIns  ;Do control char
	jmp   cmdctl
;
; EOL character encountered
;
eol	lda   <cmdlen
	sta   wordspace
	plb
	pla                      ; jb 8/7/91
	pla                      ; remove temp dp space I added to
	pla                      ; make this reentrant
	jsl   incBusy
	jmp   bye10
;
; Insert alphanum character
;
cmdIns	sta   <tmp
	lda   <cmdlen
	cmp   maxCount
	bcc   cmIns0
	WriteChar #7
	jmp   cmdloop
cmIns0	phd
	lda   InOutDP
	tcd
	lda   IODP_gInsertFlag
	pld
	cmp   #0
	beq   cmOver             ;Do insert mode
	short a
	ldy   <cmdlen
cmIns1	anop
	cpy   <cmdloc
	beq   cmIns2
	bcc   cmIns2
	dey
	lda   [bufferPtr],y
	iny
	sta   [bufferPtr],y
	dey
	bra   cmIns1
cmIns2	long  a
	inc   <cmdlen
;
; Place character in string and output
;
cmOver	lda   <tmp
	ldy   <cmdloc             ;Do overstrike mode
	short a
	sta   [bufferPtr],y
	long  a
	iny
	sty   <cmdloc
	ldx   echoFlag
	beq   echo0
	phy
	WriteChar @a
	ply
echo0	cpy   <cmdlen
	bcc   cmdov2 
	beq   cmdov2 
	sty   <cmdlen
;
; Redraw shifted text
;
cmdov2	phd
	lda   InOutDP
	tcd
	lda   IODP_gInsertFlag
	pld
	cmp   #0
	jeq   cmdloop
	ldx   echoFlag
	jeq   cmdloop
	ldx   #0
cmdov3	if2   @y,eq,<cmdlen,cmdov4
	lda   [bufferPtr],y
	iny
	inx
	phx
	phy
	WriteChar @a
	ply
	plx
	bra   cmdov3
cmdov4	anop
cmdov5	dex
	bmi   cmdov6
	phx
	WriteChar #8
	plx
	bra   cmdov5
cmdov6	jmp   cmdloop
;
; Left arrow was pressed
;
cmdctl	if2   @a,ne,#$08,ctl1    ;LEFT-ARROW
	lda   <cmdloc
	bne   ctl0a   
	WriteChar #7
	jmp   cmdloop
ctl0a	dec   a
	sta   <cmdloc
	lda   echoFlag
	beq   echo1
	WriteChar #8
echo1	jmp   cmdloop
;
; Right arrow was pressed
;
ctl1	if2   @a,ne,#$15,ctl2    ;RIGHT-ARROW
	ldy   <cmdloc
	if2   @y,ne,<cmdlen,ctl1a
	WriteChar #7
	jmp   cmdloop
ctl1a	lda   echoFlag
	beq   echo2
	lda   [bufferPtr],y
	WriteChar @a
echo2	inc   <cmdloc
	jmp   cmdloop
;
; Delete character under the cursor
;
ctl2	if2   @a,ne,#$04,ctl3    ;Control-D
delchar	ldy   <cmdloc
	if2   @y,ne,<cmdlen,ctl2a
	jmp   cmdloop
ctl2a	short a
ctl2aa	if2   @y,eq,<cmdlen,ctl2b
	iny
	lda   [bufferPtr],y
	dey
	sta   [bufferPtr],y
	iny
	bra   ctl2aa
ctl2b	lda   #' '
	dey
	sta   [bufferPtr],y
	iny
	sta   [bufferPtr],y
	long  a
	ldy   <cmdloc
	ldx   #0
	lda   echoFlag
	beq   ctl2f
ctl2c	if2   @y,eq,<cmdlen,ctl2e
	bcs   ctl2d
ctl2e	lda   [bufferPtr],y
	iny
	inx
	phx
	phy
	WriteChar @a
	ply
	plx
	bra   ctl2c   
ctl2d	anop
ctl2g	dex
	bmi   ctl2f
	phx
	WriteChar #8
	plx
	bra   ctl2g
ctl2f	dec   <cmdlen
	jmp   cmdloop

ctl3	anop
	jmp   cmdloop
;
; Open Apple - E Insert toggle
;
cmdoa	anop
	and   #$7F
	if2   @a,eq,#'e',cmdoa1
	if2   @a,ne,#'E',cmdoa2
cmdoa1	phd
	lda   InOutDP
	tcd
	eor2  IODP_gInsertFlag,#1,IODP_gInsertFlag
	pld
	jmp   cmdloop
;
; Open Apple - D Delete under cursor
; 
cmdoa2	if2   @a,eq,#'d',cmdoa2a
	if2   @a,ne,#'D',cmdoa3
cmdoa2a	jmp   delchar

cmdoa3	jmp   cmdloop
;
; DELETE was pressed
;
cmddel	anop
	lda   cmdloc
	bne   ctldel2
	WriteChar #7
	jmp   cmdloop
ctldel2	dec   a
	sta   cmdloc
	lda   echoFlag
	beq   echo3
	WriteChar #8
echo3	jmp   delchar

	END

sl1Read	START
	ldx	#$C10E
	bra	dopasc	
sl1Write	ENTRY
	ldx	#$C10F
	bra	dopasc
sl1Init	ENTRY
	ldx	#$C10D
	bra	dopasc
sl1Status	ENTRY
	ldx	#$C110
dopasc	jsl	incBusy
	pea	0
	pea	0
	pea	0
	pea	0
	pha
	short	m
	lda	>0,x
	long	m
	and	#$00FF
	ora	#$C100
	pea	$c1
	pea	$10
	pha
	_FWEntry
	ply
	plx
	pla
	plp
	plp
	long	m
	jsl	decBusy
	rtl
	END

sl2Read	START
	ldx	#$C20E
	bra	dopasc	
sl2Write	ENTRY
	ldx	#$C20F
	bra	dopasc
sl2Init	ENTRY
	ldx	#$C20D
	bra	dopasc
sl2Status	ENTRY
	ldx	#$C210
dopasc	jsl	incBusy
	pea	0
	pea	0
	pea	0
	pea	0
	pha
	short	m
	lda	>0,x
	long	m
	and	#$00FF
	ora	#$C200
	pea	$c2
	pea	$20
	pha
	_FWEntry
	ply
	plx
	pla
	plp
	plp
	long	m
	jsl	decBusy
	rtl
	END
	
NullOut	START KERN2
	rtl
	END

NullIn	START KERN2
	lda #0
	rtl
	END
NullStatus	START KERN2
	lda	#0
	sec
	rtl
	END
NullInit	START KERN2
	rtl
	END

C80Init	START KERN2
	using	InOutData
	phd
	lda	>InOutDP
	tcd
	lda	#12
	jsl	COUT
	pld
	rtl
	END

************************************************************
*
* I/O/E vector entry points
*  all TextTools calls come through these three routines
*
************************************************************

VectIn	START KERN2
	using TextToolsInfo
	using KernelStruct

	phb
	pha
	phx
	pha

	lda   >truepid
	asl   a
	asl   a
	asl   a
	asl   a
	asl   a
	asl   a
	tax
	short m
	lda   >InVect+2,x
	sta	7,s
	long  m
	lda   >InVect,x
	sta	5,s
	pla
	plx
	rtl
	END

VectOut	START KERN2
	using TextToolsInfo
	using KernelStruct

	phb
	pha
	phx
	pha
	lda   >truepid
	asl   a
	asl   a
	asl   a
	asl   a
	asl   a
	asl   a
	tax
	short m
	lda   >OutVect+2,x
	sta	7,s
	long  m
	lda   >OutVect,x
	sta	5,s
	pla
	plx
	rtl
	END

VectErr	START KERN2
	using TextToolsInfo
	using KernelStruct

	phb
	pha
	phx
	pha
	lda   >truepid
	asl   a
	asl   a
	asl   a
	asl   a
	asl   a
	asl   a
	tax
	short m
	lda   >ErrVect+2,x
	sta	7,s
	long  m
	lda   >ErrVect,x
	sta	5,s
	pla
	plx
	rtl
	END

************************************************************
*
* Pipe Driver
*
* Grabs the pipe number (from the TTY struct) and calls
* either loRead or loWrite to get a single byte
*
************************************************************

pipeIN	START KERN2
	using TextToolsInfo

	phy
	phx
	jsr   getTTindex
	lda   >InSlot,x           ; something else needs to set this up
	pha
*              jsl   loRead
	plx
	ply
	rtl
	END

pipeOUT	START KERN2
	using TextToolsInfo

	phy
	phx
	pha
	pha
	jsr   getTTindex
	lda   >OutSlot,x          ; something else needs to set this up
	pha
*              jsl   loWrite
	pla
	plx
	ply
	rtl
	END

************************************************************
*
* Redirection Driver
*
* Note that the redirection drivers only use the old
* entry points- this is because pipes will be handled 
* entirely through our GS/OS intercept
*
************************************************************

fileIN	START KERN2
	using TextToolsInfo
	phb
	phk
	plb
	phx

	pha		; result buffer!
	tsc
	pha		; transfer count
	pha
	pea	0
	pea	1	; request count
	pea	0
	inc	a
	pha		; push buffer address

	jsr   getTTindex
	lda   >InSlot,x          ; something else needs to set this up
	pha		; push the refnum
	pea	4	; number of parameters
	tsc
	inc	a
	pea	0
	pha                      ; address of pBlock
	pea	$2012	; command code
	jsl	StackGSOS
	bcc   leave
	cmp   #$4C
	bne   leave
	lda   #$7F00
	sta	17,s
leave	anop
	tsc
	clc
	adc	#16
	tcs
	pla
	plx
	plb
	rtl

*READPB         dc    i2'5'
*READrefNum     dc    i2'0'
*               dc    i4'buffer'
*               dc    i4'1'
*xferCount      dc    i4'0'
*               dc    i2'1'              ; cache blocks!
*buffer         ds    16                 ; in case we decide to get clever
*char           dc    i2'0'
	END

* This routine temporarily turns off GS/OS debug output, to prevent
* an infinite recursion.  This was done to allow pipes to be accessed
* thru the texttools file interface if desired.  The pipe interface
* sports much better performance, however.

fileOUT	START KERN2
	using TextToolsInfo
	using KernelStruct

	phb
	phk
	plb
	phx
	pha		; data buffer

	tsc
	pha		; transfer count
	pha
	pea	0
	pea	1	; request count
	pea	0
	inc	a
	pha		; push buffer address

	jsr   getTTindex
	lda   >OutSlot,x          ; something else needs to set this up
	pha
;sta   WRITErefNum
	lda   >stateFlags,x
	and   #%00000001         ; 1 = stdout, 2 = stderr
	bne   convMode1          ; last char was a CR
	lda   15,s
	and   #$7F
	cmp   #$0D
	bne   noConv1
	lda   >stateFlags,x
	ora   #%00000001
	sta   >stateFlags,x
	bra   noConv1
convMode1	lda   15,s
	and   #$7F
	cmp   #$0A
	bne   noConv1            ; ignore the LF part of CRLF
	lda   >stateFlags,x
	and   #%1111111111111110
	sta   >stateFlags,x
	pha
	bra   leave1
noConv1	anop
*	lda   15,s
*	sta   buffer
	pea	4	; pCount
	tsx		; put it in X for now

	lda   >gsosDebug
	pha
	lda   #0
	sta   >gsosDebug
	pea	0
	inx
	phx		; push the pBlock ptr
	pea	$2013	; write command code
	jsl	StackGSOS
*Write WRITEPB
	pla
	sta   >gsosDebug
leave1	anop
	tsc
	clc
	adc	#16
	tcs
	pla
	plx
	plb
	rtl

fileERR	ENTRY
	phb
	phk
	plb
	phx
	pha		; data buffer

	tsc
	pha		; transfer count
	pha
	pea	0
	pea	1	; request count
	pea	0
	inc	a
	pha		; push buffer address
	
	jsr   getTTindex
	lda   >ErrSlot,x          ; something else needs to set this up
	pha
*sta   WRITErefNum
	lda   >stateFlags,x
	and   #%00000010         ; 1 = stdout, 2 = stderr
	bne   convMode2          ; last char was a CR
	lda   15,s
	and   #$7F
	cmp   #$0D
	bne   noConv2
	lda   >stateFlags,x
	ora   #%00000010
	sta   >stateFlags,x
	bra   noConv2
convMode2	lda   15,s
	and   #$7F
	cmp   #$0A
	bne   noConv2            ; ignore the LF part of CRLF
	lda   >stateFlags,x
	and   #%1111111111111101
	sta   >stateFlags,x
	pha
	bra   leave2
noConv2	anop
*	lda   1,s
*              sta   buffer

	pea	4	; pCount
	tsx		; put it in X for now
	
	lda   >gsosDebug
	pha
	lda   #0
	sta   >gsosDebug

	pea	0
	inx
	phx
	pea	$2013	; write command code
	jsl	StackGSOS
*Write WRITEPB
	pla
	sta   >gsosDebug
leave2	anop
	tsc
	clc
	adc	#16
	tcs
	pla
	plx
	plb
	rtl

*WRITEPB        dc    i2'5'
*WRITErefNum    dc    i2'0'
*              dc    i4'buffer'
*               dc    i4'1'
*xferCount      dc    i4'0'
*               dc    i2'1'              ; cache blocks!
*buffer         ds    16                 ; in case we decide to get clever
	END

