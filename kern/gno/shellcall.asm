*	$Id: shellcall.asm,v 1.1 1998/02/02 08:19:48 taubert Exp $
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
* SHELLCALL.ASM
*   By Tim Meekins
*
* This file contains routines for patching GS/OS to intercept Shell
* calls and do special things with certain GS/OS calls.
*
* Phase 1 Modifications to Orca 2.0 compat: (11/18/91)
*   removal of old c-style calls, and fixing all routines to work
*   directly from pBlock rather than a copy.
* Phase 2: (11/19-20/91)
*   Addition of most pCount checking, and addition of routines
*   that don't take strings as parameters
* Phase 3: (11/20/91)
*   String manipulation changes/additions
* Phase 4: (11/21/91)
*   The variable pCount calls (FastFile, etc) and calls that differ
*   greatly from Orca 1.x.
**************************************************************************

	case	on
	copy  global.equates
	mcopy m/shellcall.mac

shellLoc	gequ  12

;
; Table of commands
;
ShellCallTbl	DATA
	dc    a2'_Get_L_Info'    ;$101 - GetLInfo
	dc    a2'_Set_L_Info'    ;$102 - SetLInfo
	dc    a2'_GetLang'	;$103
	dc    a2'_SetLang'	;$104
	dc    a2'_Error'         ;* $105 - Error
	dc    a2'_Set'           ;* $106 - Set
	dc    a2'_Version'       ;* $107 - Version
	dc    a2'_Read_Indexed'  ;* $108 - ReadIndexed
	dc    a2'_Init_Wildcard' ;$109 - InitWildcard
	dc    a2'_Next_Wildcard' ;$10A - NextWildcard
	dc    a2'_Read_Variable' ;* $10B - ReadVariable
	dc    a2'NotImp'         ;$10C
	dc    a2'_Execute'       ;* $10D - Execute
	dc    a2'_FastFile'      ;$10E - FastFile
	dc    a2'_Direction'     ;* $10F - Direction
	dc    a2'_Redirect'      ;* $110 - Redirect
	dc    a2'NotImp'         ;$111
	dc    a2'NotImp'         ;$112
	dc    a2'_Stop'          ;* $113 - Stop
	dc    a2'_ExpandDevices' ;* $114 - ExpandDevices
	dc    a2'_UnsetVariable' ;* $115 - UnsetVariable
	dc    a2'_Export'        ;* $116 - Export
	dc    a2'_PopVariables'  ;* $117 - PopVariables
	dc    a2'_PushVariables' ;* $118 - PushVariables
	dc    a2'_SetStopFlag'   ;* $119 - SetStopFlag
	dc    a2'_ConsoleOut'    ;* $11A - ConsoleOut
	dc    a2'NotImp'         ;$11B
	dc    a2'NotImp'         ;$11C
	dc    a2'NotImp'         ;$11D
	dc	a2'_KeyPress'	;$11E
	dc	a2'_ReadKey'	;$11F
	END

;=========================================================================
;
; ($101) GetLInfo
;
;=========================================================================

_Get_L_Info	START
_Get_L_Info	name

	ph2   cmdNum
	ph4   pBlock
	jsl   cGetLInfo
	jmp   GSOSReturn
	END

;=========================================================================
;
; ($102) SetLInfo
;
;=========================================================================

_Set_L_Info	START
_Set_L_Info	name

	ph2   cmdNum
	ph4   pBlock
	jsl   cSetLInfo
	jmp   GSOSReturn
	END

;=========================================================================
;
; ($103) GetLang
;
;=========================================================================

_GetLang	START
_GetLang	name
PBLang	equ	0

	lda	curLangNum
	ldy	#PBLang
	sta	[pBlock],y
	lda	#0
	jmp	GSOSReturn

;=========================================================================
;
; ($104) SetLang
;
;=========================================================================

_SetLang	ENTRY

	ldy	#PBLang
	lda	[pBlock],y
	sta	curLangNum
	lda	#0
	jmp	GSOSReturn
curLangNum	dc  i2'1'	
	END


;=========================================================================
;
; ($105) Error
;
;=========================================================================

_Error	START
_Error	name

	lda	cmdNum
	and	#$40
	beq	orca_1
	lda	pCount
	cmp	#1
	beq	orca_1
	lda	#4
	jmp	GSOSReturn

orca_1	lda   [pBlock]
	jsl   printError

	lda   #0
	jmp   GSOSReturn

	END

;=========================================================================
;
; ($106) Set
;
;=========================================================================

_Set	START
_Set	name
PBexport	equ   8

varC	equ   shellLoc+0
valC	equ   shellLoc+4

	lda   cmdNum
	and   #$40               ;If bit $40 then c strings
	beq   orca_1

	lda   pCount
	cmp   #3
	beq   orca_2
	lda   #4
	jmp   GSOSReturn

orca_2	anop
	ldy   #2                 ;$
	lda   [pBlock],y         ;$
	pha                      ;$
	lda   [pBlock]           ;$
	pha                      ;$
	jsl   gs2cstr
	phx
	pha
	sta   varC
	stx   varC+2

	ldy   #6                 ;$
	lda   [pBlock],y         ;$
	pha                      ;$
	ldy   #4                 ;$
	lda   [pBlock],y         ;$
	pha                      ;$

	jsl   gs2cstr
	phx
	pha
	sta   valC
	stx   valC+2
	jsl   setvar

	pei   (varC+2)           ; set the export flag according
	pei   (varC)
	ldy   #PBexport          ; to the new parm in Orca 2.0
	lda   [pBlock],y
	pha
	jsl   exportvar
	bra   orca_com

orca_1	ldy   #2                 ;$
	lda   [pBlock],y         ;$
	pha                      ;$
	lda   [pBlock]           ;$
	pha                      ;$
	jsl   p2cstr
	phx
	pha
	sta   varC
	stx   varC+2

	ldy   #6                 ;$
	lda   [pBlock],y         ;$
	pha                      ;$
	ldy   #4                 ;$
	lda   [pBlock],y         ;$
	pha                      ;$

	jsl   p2cstr
	phx
	pha
	sta   valC
	stx   valC+2

	jsl   setvar

orca_com	pei   (varC+2)
	pei   (varC)
	jsl   ~NDISPOSE
	pei   (valC+2)
	pei   (valC)
	jsl   ~NDISPOSE

	lda   #0
	jmp   GSOSReturn

	END

;=========================================================================
;
; ($107) Version
;
;=========================================================================

_Version	START
_Version	name

	lda   cmdNum
	and   #$40
	beq	orca_1
	lda	pCount
	cmp	#1
	beq	orca_1
	lda	#4
	jmp	GSOSReturn

orca_1	ldy   #2
	lda   #$2020
	sta   [pBlock],y
	lda   #$3032             ;version (3031 = 1.0) (3032 = 2.0)
	sta   [pBlock]
	lda   #0
	jmp   GSOSReturn

	END

;=========================================================================
;
; ($108) Read_Indexed
;
;=========================================================================

_Read_Indexed	START
_Read_Indexed	name

var_name	equ   0
value	equ   4
index	equ   8
PBexport	equ   $A

var	equ   shellLoc+0
vn_tmp	equ   shellLoc+4

	ldy   #index             ;$
	lda   [pBlock],y         ;$
	pha                      ;$
	jsl   indexvar
	stx   var+2
	sta   var

	lda   cmdNum
	and   #$40
	beq   orca_1
	lda   pCount
	cmp   #4
	beq   orca_2
	lda   #4
	jmp   GSOSReturn
orca_2	anop
	ldy   #2
	lda   var
	ora   var+2
	bne   orca21
	jmp   nomore
orca21	anop
	ldy   #12                ; copy the export flag
	lda   [var],y            ; to the parameter block
	ldy   #PBexport
	sta   [pBlock],y

	ldy   #4+2
	lda   [var],y
	pha
	dey2
	lda   [var],y
	pha
	ldy   #var_name+2        ;$
	lda   [pBlock],y         ;$
	pha                      ;$
	lda   [pBlock]           ;$
	pha                      ;$
	jsl   copyc2res
	ldy   #8+2
	lda   [var],y
	pha
	dey2
	lda   [var],y
	pha
	ldy   #value+2           ;$
	lda   [pBlock],y         ;$
	pha                      ;$
	ldy   #value             ;$
	lda   [pBlock],y         ;$
	pha                      ;$
	jsl   copyc2res
	lda   #0
	jmp   GSOSReturn

orca_1	anop
	ldy   #0
	lda   var
	ora   var+2
	beq   nomore

	ldy   #4+2
	lda   [var],y
	pha
	dey2
	lda   [var],y
	pha
	ldy   #var_name+2        ;$
	lda   [pBlock],y         ;$
	pha                      ;$
	lda   [pBlock]           ;$
	pha                      ;$
	jsl   c2pstr
	ldy   #8+2
	lda   [var],y
	pha
	dey2
	lda   [var],y
	pha
	ldy   #value+2           ;$
	lda   [pBlock],y         ;$
	pha                      ;$
	ldy   #value             ;$
	lda   [pBlock],y         ;$
	pha                      ;$
	jsl   c2pstr
	lda   #0
	jmp   GSOSReturn

nomore	phy
	ldy   #var_name+2        ;$
	lda   [pBlock],y         ;$
	sta   vn_tmp+2           ;$
	lda   [pBlock]           ;$
	sta   vn_tmp             ;$
	ldy   #value+2
	lda   [pBlock],y
	sta   var+2
	ldy   #value
	lda   [pBlock],y
	sta   var
	ply
	lda   #0
	sta   [vn_tmp],y
	sta   [var],y
	jmp   GSOSReturn

	END

;=========================================================================
;
; ($109) Init_Wildcard
;
;=========================================================================

_Init_Wildcard	START
_Init_Wildcard	name
	using globals
	using KernelStruct

wildcard	equ   0
iw_flags	equ   4                  ;I'm ignoring the flags (no prompting)

tbl	equ   shellLoc  
ptr	equ   shellLoc+4
w_file	equ   shellLoc+8
w_dirpath	equ   shellLoc+12
w_pfx	equ	shellLoc+16

;               DebugStr #debug

	lda   cmdNum
	and   #$40
	beq   orca_1
	lda   pCount
	cmp   #2
	beq   orca_2
	lda   #4
	jmp   GSOSReturn
orca_2	anop
	ldy   #wildcard+2
	lda   [pBlock],y
	sta   w_file+2
	lda   [pBlock]
	sta   w_file
	lda   [w_file]
	inc   a
	inc   a                  ; add two to get length of ds
	pea   0
	pha                      ; make a copy of the gs string
	jsl   ~NEW
	pei   (w_file+2)
	pei   (w_file)
	stx   w_file+2
	sta   w_file
	phx
	pha
	jsl   copygsstr
	bra   common
orca_1	anop
	ldy   #wildcard+2
	lda   [pBlock],y
	sta   w_file+2
	pha                      ; push addr of p string for
	lda   [pBlock]
	sta   w_file
	pha                      ; use later (by copy)
	pea   0
	lda   [w_file]           ; get length
	and   #$ff
	inc   a
	inc   a                  ; add two to get length of gs string
	pha
	jsl   ~NEW
	stx   w_file+2
	sta   w_file
	phx
	pha
	jsl   copyp2gs           ; convert to a GS string

common	anop
	stz   WC_PathBuf
	stz   WC_Patt
	lda   [w_file]
;               and   #$FF
	jeq   done
;
; Find last separator and separate the file pattern from the path
;
	tay
	iny
strippat	lda   [w_file],y
	and   #$FF
	cmp   #':'
	beq   gotpat
	cmp   #'/'
	beq   gotpat
	dey
	cpy   #1
	bne   strippat
;
; No path, so grab the current prefix
;
; Copy the pointer to prefix 0 straight out of the process structure

	phy
	ldx	curProcInd
	lda	prefixh,x	
	sta	w_pfx
	lda	prefixh+2,x
	sta	w_pfx+2
	ldy	#4
	lda	[w_pfx],y
	sta	WC_PathPtr
	sta	w_dirpath
	ldy	#6
	lda	[w_pfx],y
	sta	WC_PathPtr+2
	sta	w_dirpath+2
	ply
	bra   gotpat2
;
; Copy the path and pattern
;
gotpat	ld4   WC_PathBuf,(WC_PathPtr,w_dirpath)
gotpat2	sty   tmp
	ldx   #0
	iny
	lda   [w_file]
	beq   copiedpat
	inc   a                  ; +1, because y is postindexed
	sta   chk+1
	short m
copypat	anop
	lda   [w_file],y
	cmp   #'='
	bne   putpat
	lda   #'*'
putpat	sta   WC_Patt,x
	inx
	iny
chk	cpy   #0
	bcc   copypat
	beq   copypat
copiedpat	lda   #0
	sta   WC_Patt,x
	ldx   tmp
	dex                      ; argh! this is nasty!
	beq   copiedpath
	ldy   #2
copypath	lda   [w_file],y
	sta   WC_PathBuf,y
	iny
	dex
	bne   copypath
copiedpath	dey
	dey
	sty   WC_PathBuf
	long  ai

;
; Expand the device
;
;               lda   tmp                ;skip this 
;               beq   fixGSOS
;               ExpandDevices ExpParm
;               jcs   err

;fixGSOS        lda   WC_PathBuf
;               xba
;               sta   WC_PathBuf

	Open  WC_OpenParms
	jcs   err
	mv2   WC_OpenRef,(GDERef,CloseRef)
	stz   GDEbase
	stz   GDEdisp
	ld2   6,GDEParm
	GetDirEntry GDEParm
	jcs   err
	pea   0
	lda   GDEentry
	inc   a
	asl2  a
	pha
	jsl   ~NEW
	sta   tbl
	stx   tbl+2

	stz   tbloff             ; initialize offset to zero
	ldx   GDEentry
	bne   itsAlright
	jmp   nofiles            ; there are no files in this directory!

itsAlright	ldy   #0
	tya
clrtbl	sta   [tbl],y
	iny2
	sta   [tbl],y
	iny2
	dex
	bne   clrtbl

	ld2   5,GDEParm
	ld2   1,(GDEbase,GDEdisp)
searchloop	GetDirEntry GDEParm
	jcs   GDEerr
	ldy   buf
	lda   #0
	sta   buf+2,y
;               WriteCString #buf+2
;               WriteChar #13
;               WriteChar #10
;               WriteCString #WC_Patt
;               WriteChar #13
;               WriteChar #10
	ph4   #WC_Patt
	ph4   #buf+2
	ph2   #0	; case insensitive
	jsl   RegExp
	cmp   #0
	beq   searchloop
;               WriteChar #10

* allocate space for the file/pathname.  If bit 0 of flags is set, only
* use the filename, otherwise include the whole pathname.

	ldy   #iw_flags
	lda   [pBlock],y
	and   #1
	bne   fnameonly
	pea   0
	lda   buf
	inc   a
	inc   a
	clc
	adc   [w_dirpath]        ; pointer to directory entry
	pha
	jsl   ~NEW
	sta   ptr
	stx   ptr+2
	ph4   WC_PathPtr
	ph4   ptr
	jsl   copygsstr
	ph4   #buf
	ph4   ptr
	jsl   gsstrcat
	bra   commonptr

fnameonly	pea   0
	lda   buf
	inc   a
	inc   a
	pha

	jsl   ~NEW
	sta   ptr
	stx   ptr+2
	ph4   #buf
	ph4   ptr
	jsl   copygsstr

commonptr	ldy   tbloff
	lda   ptr
	sta   [tbl],y
	iny2
	lda   ptr+2
	sta   [tbl],y
	iny2
	sty   tbloff

	jmp   searchloop

GDEerr	cmp   #$61               ;End of directory?
	bne   err
nofiles	ldy   tbloff
	lda   #0
	sta   [tbl],y
	iny2
	sta   [tbl],y
	Close CloseParm
;
; do we need to delete the old table
;
	lda   truepid
	asl2  a
	tax
	lda   WCtbl,x
	ora   WCtbl+2,x
	beq   settbl
;
; delete old table
;
	lda   WCtbl,x
	sta   ptr
	lda   WCtbl+2,x
	sta   ptr+2
	ldy   #0
clrit	lda   [ptr],y
	tax
	iny2
	lda   [ptr],y
	iny2
	cmp   #0
	bne   disp
	cpx   #0
	beq   nodisp
disp	phy
	pha
	phx
	jsl   ~NDISPOSE
	ply
	bra   clrit
nodisp	pei   (ptr+2)
	pei   (ptr)
	jsl   ~NDISPOSE

settbl	lda   truepid
	asl   a
	tax
	asl   a
	tay
	lda   #0
	sta   WCidx,x
	lda   tbl
	sta   WCtbl,y
	lda   tbl+2
	sta   WCtbl+2,y

done	lda   #0
err	pha
	pei   (w_file+2)
	pei   (w_file)
	jsl   ~NDISPOSE
	pla
	jmp   GSOSReturn

tmp	ds    2

tbloff	ds    2

CloseParm	dc    i'1'
CloseRef	ds    2

GDEParm	dc    i'6'
GDERef	ds    2
	ds    2
GDEbase	dc    i'1'
GDEdisp	dc    i'1'
	dc    a4'GDBuf'
GDEentry	ds    2

GDBuf	dc    i'67'
buf	ds    65

WC_PathBuf	ds    129
WC_Patt	ds    40


WC_OpenParms	dc    i'2'
WC_OpenRef	ds    2
WC_PathPtr	ds    4

;debug          str   'init_wilcard'

	END

globals	DATA

WCidx	dc    32i2'0'
WCtbl	dc    32i4'0'

	END

;=========================================================================
;
; ($10A) Next_Wildcard
;
;=========================================================================

_Next_Wildcard	START
_Next_Wildcard	name
	using globals
	using KernelStruct

PBnextfile	equ   0

tbl	equ   shellLoc
ptr	equ   shellLoc+4
nextfile	equ   shellLoc+8

	ldy   #PBnextfile+2      ;$
	lda   [pBlock],y         ;$
	sta   nextfile+2         ;$
	lda   [pBlock]           ;$
	sta   nextfile           ;$
	lda   truepid
	asl   a
	tay
	asl   a
	tax
	lda   WCtbl,x
	sta   tbl
	lda   WCtbl+2,x
	sta   tbl+2
	ora	tbl
	bne	alloced
	jmp	erruer	
alloced	tyx
	lda   WCidx,x
	tay
	lda   [tbl],y
	iny2
	ora   [tbl],y
	bne   stillmore
	jmp   erruer
stillmore	anop
	iny2
	tya
	sta   WCidx,x
	dey2
	dey2
	lda   [tbl],y
	sta   ptr
	iny2
	lda   [tbl],y
	sta   ptr+2

* insert nextfile/nextfileGS check here
	lda   cmdNum
	and   #$40
	beq   orca_1
	lda   pCount
	cmp   #13
	bcs   badpcount
	cmp   #0
	bne   orca_2
badpcount	lda   #4
	jmp   GSOSReturn
orca_2	anop
	ph4   ptr
	ph4   nextfile
	jsl   copygs2res
	lda   ptr
	ldy   #PBnextfile
	sta   [pBlock],y
	lda   ptr+2
	ldy   #PBnextfile+2
	sta   [pBlock],y
	lda   pCount
	cmp   #1
	bne   doGFI
	lda   #0
	bra   doneNW
doGFI	lda   pBlock
	sec
	sbc   #2
	tax
	lda   pBlock+2
	sbc   #0
	pha
	phx
	pea   $2006              ; GetFileInfoGS
	jsl   $E100B0            ; GS/OS stack entry point
doneNW	pha
	lda   nextfile
	ldy   #PBnextfile
	sta   [pBlock],y
	lda   nextfile+2
	ldy   #PBnextfile+2
	sta   [pBlock],y
	pla
	jmp   GSOSReturn

* Copy the GSString to a PString in the form required by Next_Wildcard
* (the old P16 version), converting :'s to /'s along the way

orca_1	lda   [ptr]              ; copy a GS string
	sta   [nextfile]         ; to the P-string output buffer
	tay
	iny
	short a
copy	lda   [ptr],y
	dey
	cmp	#':'
	bne	nocolon
	lda	#'/'
nocolon	sta   [nextfile],y
	cpy   #1
	bne   copy
	long  a
	lda   #0
	jmp   GSOSReturn

erruer	anop
	lda   cmdNum
	and   #$40
	beq   orca_1a
	ldy   #2
	lda   #0
	sta   [nextfile],y
	jmp   GSOSReturn
orca_1a	lda   #0
	short m
	sta   [nextfile]
	long  m
	jmp   GSOSReturn

	END

;=========================================================================
;
; ($10B) Read_Variable
;
;=========================================================================

_Read_Variable	START
_Read_Variable	name
PBvar_name	equ   0
PBvalue	equ   4
PBexport	equ   8

var_name	equ   shellLoc+0
value	equ   shellLoc+4
var	equ   shellLoc+8

	lda   cmdNum
	and   #$40               ;If bit $40 then c strings
	beq   orca_1
	lda   pCount
	cmp   #3
	beq   orca_2
	lda   #4
	jmp   GSOSReturn

orca_2	anop
	ldy   #PBvar_name+2      ;$
	lda   [pBlock],y         ;$
	pha                      ;$
	lda   [pBlock]           ;$
	pha                      ;$
	jsl   gs2cstr
	phx
	pha
	phx
	pha
	jsl   readvarrec
	sta   var
	stx   var+2
	jsl   ~NDISPOSE           ; get rid of the temp C string

	ldy   #PBvalue+2         ;$
	lda   [pBlock],y         ;$
	sta   value+2            ;$
	ldy   #PBvalue           ;$
	lda   [pBlock],y         ;$
	sta   value              ;$

	lda   var
	ora   var+2
	beq   novar2

	ldy   #8
	lda   [var],y
	sta   var_name
	ldy   #10
	lda   [var],y
	sta   var_name+2

	pei   (var_name+2)
	pei   (var_name)
	ph4   value
	jsl   copyc2res
	ldy   #12
	lda   [var],y
	ldy   #PBexport
	sta   [pBlock],y
	lda   #0
	jmp   GSOSReturn
novar2	anop
	ldy   #2
	lda   #0
	sta   [value],y
	jmp   GSOSReturn

orca_1	anop
;               pei   (var_name+2)
;               pei   (var_name)
	ldy   #PBvalue+2         ;$
	lda   [pBlock],y         ;$
	sta   value+2            ;$
	ldy   #PBvalue           ;$
	lda   [pBlock],y         ;$
	sta   value              ;$

	ldy   #PBvar_name+2      ;$
	lda   [pBlock],y         ;$
	pha                      ;$
	lda   [pBlock]           ;$
	pha                      ;$
	jsl   p2cstr
	phx
	pha
	phx
	pha
	jsl   readvar
	sta   var_name
	stx   var_name+2
	jsl   ~NDISPOSE
	lda   var_name
	ora   var_name+2
	beq   novar

	pei   (var_name+2)
	pei   (var_name)
	pei   (value+2)
	pei   (value)
	jsl   c2pstr

	lda   #0
	jmp   GSOSReturn

novar	anop
	lda   #0
	sta   [value]
	jmp   GSOSReturn

	END

;=========================================================================
;
; ($10D) Execute
;
;=========================================================================

_Execute	START
_Execute	name
	using KernelStruct

	lda	cmdNum
	and	#$40
	beq	orca_1
	lda	pCount
	cmp	#2
	beq	orca_1
	lda	#4
	jmp	GSOSReturn

orca_1	jsl   decBusy
;               jsr   decMutex

	ldx   curProcInd
	lda   executeHook,x
	ora   executeHook+2,x
	bne   validHook
	lda   #26
	jsl   incBusy
;               jsr   incMutex
	jmp   GSOSReturn
validHook	anop
	lda   [pBlock]
	bne   nonewvar
	PushVariables 0
nonewvar	ldy   #4
	lda   [pBlock],y
	pha
	dey2
	lda   [pBlock],y
	pha
	push3 #returnadr-1
	ldx   curProcInd
	lda   executeHook+2,x
	short m
	pha
	long  m
	lda   executeHook,x
	dec   a
	pha
	rtl                      ; jump to the routine
returnadr	anop
	pha
	lda   [pBlock]
	bne   nokillvar
	PopVariables 0
nokillvar	anop
	jsl   incBusy
	pla
	jmp   GSOSReturn

	END

;=========================================================================
;
; ($10E) FastFile
;
;=========================================================================
; fastfile.c does all the work

_FastFile	START
_FastFile	name

	ph2   pCount
	lda   cmdNum
	and   #$40
	pha                      ; osFlag (0 = P16, !0 = GSOS)
	ph4   pBlock             ; pointer to pBlock
	jsl   fastEntry          ; har!
	jmp   GSOSReturn
	END

;=========================================================================
;
; ($10F) Direction
;
;=========================================================================

_Direction	START
_Direction	name
	using KernelStruct
	using TextToolsInfo

	lda	cmdNum
	and	#$40
	beq   orca_1
	lda	pCount
	cmp	#2
	beq	orca_1
	lda	#4
	jmp	GSOSReturn

* redirection mapping
* 0,1 = none   0
*  unless xxSlot != kernelStruct.ttyID, which is 1 (.printer)
* 2   = none   0  (no Orca-based redirection)
* 3   = disk   2
* 4   = pipe   2
* any others are 1 (generalized communication)

orca_1	lda   [pBlock]
	cmp   #3
	bcc   noerr
	lda   #$53
	jmp   GSOSReturn        ; Parameter out of range
noerr	anop
	lda   truepid
	asl   a
	asl   a
	asl   a
	asl   a
	asl   a
	asl   a
	pha
	lda   [pBlock]
	asl   a
	clc
	adc   1,s
	tax
	lda   InDeviceType,x
	cmp   #2
	bcc   Rconsole
	beq   Rram
	cmp   #3
	beq   Rdisk
	cmp   #4
	beq   Rdisk
Rcomm	ldy   #2
	lda   #1
	sta   [pBlock],y
	pla
	lda   #0
	jmp   GSOSReturn
Rram	ldy   #2
	lda   #0
	sta   [pBlock],y
	pla
	lda   #0
	jmp   GSOSReturn
Rdisk	ldy   #2
	lda   #2
	sta   [pBlock],y
	pla
	lda   #0
	jmp   GSOSReturn
Rconsole	lda   [pBlock]
	asl   a
	asl   a
	clc
	adc   1,s
	tax
	lda   InSlot,x
	ldx   curProcInd
	cmp   ttyID,x
	beq   Rram               ; no redirection
	bra   Rcomm
	END

;=========================================================================
;
; ($110) Redirect
;
;=========================================================================

_Redirect	START
_Redirect	name

	using KernelStruct
	using TextToolsInfo
PBdevice	equ   0
PBappend	equ   2
PBfile	equ   4

file	equ   shellLoc+0

	lda	cmdNum
	and	#$40
	beq	orca_1
	lda	pCount
	cmp	#3
	beq	orca_2
	lda	#4
	jmp	GSOSReturn
orca_2	ldy	#PBfile+2
	lda	[pBlock],y
	sta	CRpathname+2
	ldy	#PBfile
	lda	[pBlock],y
	sta	CRpathname
	mv4	CRpathname,(OPENpathname,destPathname)
	bra	redirectCom

orca_1	ldy   #PBfile+2          ;$
	lda   [pBlock],y         ;$
	sta   file+2             ;$
	ldy   #PBfile            ;$
	lda   [pBlock],y         ;$
	sta   file               ;$

	lda   [file]
	and   #$00ff
	cmp   #65
	bcc   strokay
	lda   #$0053	; oops- decimal error #s are boo-boo
	jmp   GSOSReturn
strokay	ph4   file               ; convert thingy to a GS string
	ph4   #gspath+1
	jsl   copypstr
	lda   gspath+1
	and   #$00FF
	sta   gspath
	ld4	gspath,(CRpathname,OPENpathname,destPathname)
redirectCom	anop
;               lda   device
	lda   [pBlock]           ;$
	beq   doOpen
;               lda   append
	ldy   #PBappend
	lda   [pBlock],y         ;$
	bne   doOpen
doCreate	anop
	Create CRparm
	bcc   doOpen
	cmp   #$47               ; duplicate pathname?
	beq   delFile
	cmp   #$58
	beq   doOpen             ; not a block device- Char device
	jmp   GSOSReturn
delFile	Destroy destParm
	bcc   doCreate
	jmp   GSOSReturn
doOpen	anop
	ldx   #1                 ;$
;               lda   device
	lda   [pBlock]           ;$
	beq   storeAccess
	ldx   #2                 ;$
storeAccess	stx   OPENreqAccess      ;$
	lda	[pBlock]	; get 'device' parameter
	inc	a                  ; I/O channel before we open
	sta	CLOSEparm+2
	Close CLOSEparm          ; the new one.
	bcs	openError1
	Open  OPENParm
	bcc   okay
openError1	jmp   GSOSReturn        ; error if >> on nonexistent file
okay	anop
;               lda   append             ; move mark to eof?
	ldy   #PBappend          ;$ move mark to eof?
	lda   [pBlock],y         ;$
	beq   noAppend
	lda   OPENrefNum
	sta   EOFref
	sta   MARKref
	GetEOF EOFpb
	bcs   nonOkey
	movelong EOFeof,MARKdisp
	SetMark MARKpb
	bcs   nonOkey
	bra   noAppend
EOFpb	dc    i2'2'
EOFref	dc    i2'0'
EOFeof	dc    i4'0'
MARKpb	dc    i2'3'
MARKref	dc    i2'0'
MARKbase	dc    i2'0'
MARKdisp	dc    i4'0'

noAppend	anop
	pea   $3                 ; file redirection
	pea   $0
	ph2   OPENrefNum
	lda   #$0F
	clc
;               adc   device
	adc   [pBlock]           ;$
	xba
	ora   #$0C
	tax
	jsl   $E10000            ; make the tool call
	lda   #0
nonOkey	jmp   GSOSReturn

gspath	ds    66
CRparm	dc    i2'4'
CRpathname	dc    i4'gspath'
	dc    i2'$C3'
	dc    i2'$04'
	dc    i4'0'
destParm	dc    i2'1'
destPathname	dc    i4'gspath'
OPENParm	dc    i2'3'
OPENrefNum	dc    i2'0'
OPENpathname	dc    i4'gspath'
OPENreqAccess	dc    i2'0'
CLOSEparm	dc    i2'1'
	dc	i2'0'
	END

;=========================================================================
;
; ($113) Stop
;
;=========================================================================


_Stop	START
_Stop	name
	using KernelStruct

	lda	cmdNum
	and	#$40
	beq	orca_1
	lda	pCount
	cmp	#1
	beq	orca_1
	lda	#4
	jmp	GSOSReturn

orca_1	lda   >truepid
	asl   a
	tax
	lda   StopFlags,x
	sta   [pBlock]
	jmp   GSOSReturn
StopFlags	ENTRY
	dc 32i2'0'
	END

;=========================================================================
;
; ($114) ExpandDevices
;
;=========================================================================

_ExpandDevices	START
_ExpandDevices	name
PBpathname	equ   0
PBoutname	equ   4

pathname	equ   shellLoc+0

	lda	cmdNum
	and	#$40
	beq	orca_1
	ldy	#PBpathname+2
	lda	[pBlock],y
	sta	inputPath+2
	lda	[pBlock]
	sta   inputPath
	ldy	#PBoutname+2
	lda	[pBlock],y
	sta	outputPath+2
	ldy	#PBoutname
	lda	[pBlock],y
	sta	outputPath
	ExpandPath epParm
	jmp	GSOSReturn

orca_1	anop
	ldy   #PBpathname+2      ;$
	lda   [pBlock],y         ;$
	sta   pathname+2         ;$
	lda   [pBlock]           ;$
	sta   pathname           ;$

	lda   [pathname]
	and   #$00FF
	tay
	sta   inputbuf
	beq   nocopy1
	short m
loop1	lda   [pathname],y
	sta   inputbuf+1,y
	dey
	bne   loop1
nocopy1	long  m
	ld4	inputbuf,inputPath
	ld4	outputbuf,outputPath

	ExpandPath epParm
	bcs   error

	lda   outputbuf+2
	and   #$00ff
	tay
	sta   [pathname]
	beq   nocopy2
	short a
loop2	lda   outputbuf+3,y
	sta   [pathname],y
	dey
	bne   loop2
nocopy2	long  a
	lda   #0
error	anop
	jmp   GSOSReturn


epParm	anop
	dc i2'3'
inputPath	dc i4'inputbuf'
outputPath	dc i4'outputbuf'
ed_flags	dc i2'0'
inputbuf	ds 66
outputbuf	dc i2'68'
	ds 66
	END

;=========================================================================
;
; ($115) UnsetVariable
;
;=========================================================================

_UnsetVariable	START
_UnsetVariable	name

PBname	equ   0
name	equ   shellLoc+0

	lda   cmdNum
	and   #$40
	beq   orca_1

orca_2	anop
	ldy   #PBname+2          ;$
	lda   [pBlock],y         ;$
	pha                      ;$
	lda   [pBlock]           ;$
	pha                      ;$
	jsl   gs2cstr
	sta   name
	stx   name+2
	bra   orca_com

orca_1	anop
	ldy   #PBname+2          ;$
	lda   [pBlock],y         ;$
	pha                      ;$
	lda   [pBlock]           ;$
	pha                      ;$

	jsl   p2cstr
	sta   name
	stx   name+2
orca_com	anop
	pei   (name+2)
	pei   (name)
	jsl   unsetvar
	pei   (name+2)
	pei   (name)
	jsl   ~NDISPOSE
	lda   #0
	jmp   GSOSReturn

	END

;=========================================================================
;
; ($116) Export
;
;=========================================================================

_Export	START
_Export	name

PBname	equ   0
PBflags	equ   4

name	equ   shellLoc+0

	lda   cmdNum
	and   #$40               ;If bit $40 then c strings
	beq   orca_1
	lda	pCount
	cmp	#2
	beq	orca_2
	lda	#4
	jmp	GSOSReturn

orca_2	anop
	ldy   #PBname+2          ;$
	lda   [pBlock],y         ;$
	pha                      ;$
	lda   [pBlock]           ;$
	pha                      ;$
	jsl   gs2cstr
	sta   name
	stx   name+2
	bra   orca_com
orca_1	anop
;               pei   (name+2)
;               pei   (name)
	ldy   #PBname+2          ;$
	lda   [pBlock],y         ;$
	pha                      ;$
	lda   [pBlock]           ;$
	pha                      ;$
	jsl   p2cstr
	sta   name
	stx   name+2
orca_com	anop
	pei   (name+2)
	pei   (name)
;               pei   (flags)
	ldy   #PBflags           ;$
	lda   [pBlock],y         ;$
	pha                      ;$
	jsl   exportvar
	pei   (name+2)
	pei   (name)
	jsl   ~NDISPOSE
	lda   #0
	jmp   GSOSReturn

	END

;=========================================================================
;
; ($117) PopVariables
;
;=========================================================================

_PopVariables	START
_PopVariables	name
	using KernelStruct

	lda   cmdNum
	and   #$40
	beq	orca_1
	lda	pCount
	cmp	#0
	beq	orca_1
	lda	#4
	jmp	GSOSReturn

orca_1	lda	truepid
	jsl   popvartbl

	lda   #0
	jmp   GSOSReturn

	END

;=========================================================================
;
; ($118) PushVariables
;
;=========================================================================

_PushVariables	START
_PushVariables	name

	lda   cmdNum
	and   #$40
	beq	orca_1
	lda	pCount
	cmp	#0
	beq	orca_1
	lda	#4
	jmp	GSOSReturn

orca_1	jsl   initvar

	lda   #0
	jmp   GSOSReturn

	END

;=========================================================================
;
; ($119) SetStopFlag
;
;=========================================================================

_SetStopFlag	START
_SetStopFlag	name
	using KernelStruct
	
	lda	cmdNum
	and	#$40
	beq	orca_1
	lda	pCount
	cmp	#1
	beq	orca_1
	lda	#4
	jmp	GSOSReturn

orca_1	lda   >truepid
	asl   a
	tax
	lda   [pBlock]
	sta   StopFlags,x
	jmp   GSOSReturn
	END

;=========================================================================
;
; ($11A) ConsoleOut
;
;=========================================================================
; does nothing if process is in the background (1/21/93 jb)

_ConsoleOut	START
_ConsoleOut	name
	using	KernelStruct

char	equ   0

	lda   cmdNum
	and   #$40
	beq	orca_1
	lda	pCount
	cmp	#1
	beq	orca_1
	lda	#4
	jmp	GSOSReturn
	
orca_1	anop
	jsr	isBGProcess
	bcs	goaway

* Instead of calling the console's COUT, call down to the character device
* dispatcher with a write request.

	jsl   decBusy
	pea	1
	ph4	pBlock
	ldx	curProcInd
	lda	ttyID,x
;	dec	a                  don't need this since this is the actual
	pha
	jsl	ttwrite	value
	jsl	incBusy

goaway	lda   #0
	jmp   GSOSReturn

	END

;=========================================================================
;
; ($11E) KeyPress
;  New ORCA/Shell (2.0.1) call to see if input is pending, and if so
;  what the input is.
;  This routine is the compromise Mike W. and I worked out regarding
;  directly accessing the keyboard from his compilers.
;
;=========================================================================
; Due to the fact that there's no way to 'unget' a character from
; a tty and that the only way to get the data this call wants is to
; read it via ioctl(FIONREAD) & read (which removes the char, making
; a subsequent _KeyRead work improperly), we have to peek at the
; console's input buffer directly to get our data.  This is really icky,
; but we have sufficient checks to prevent the routine from snagging
; a foreground process' input.

_KeyPress	START
_KeyPress	name
	using	ADBData
	using KernelStruct

key	equ	0	key & mod. reg of keypress. Not valid
modifiers	equ  2	if available == 0
available	equ  4	== 1 if key avail, == 0 if not

	lda   cmdNum
	and   #$40
	beq	orca_1
	lda	pCount
	cmp	#3
	beq	orca_1
	lda	#4
	jmp	GSOSReturn
	
orca_1	anop
	ldx	curProcInd
	lda	ttyID,x
	cmp	#3	; is controlling tty the console?
	bne	goaway	; leave w/o doing anything

	jsr	isBGProcess
	bcs	goaway	; ignore progs in the bg

	php
	sei		; temporarily shut off interrupts
	lda	>head	; if head == tail then there
	cmp	>tail	;  is no data in the input buffer
	beq	noKeyWaiting

	lda   >tail	; get the character & modifiers
	tax                      ; from the buffer, indexed by
	lda   keybuf,x
	and   #$00FF	; tail.
	ldy	#key	; store them in the parameter
	sta   [pBlock],y	; block after masking unneeded
	lda	modbuf,x           ; portions out
	and	#$00FF
	jsl	ConvKMR2EM
	ldy	#modifiers
	sta	[pBlock],y
	lda	#1
	ldy	#available
	sta	[pBlock],y
	
	plp	                   ; restore interrupts
	lda   #0
	jmp   GSOSReturn

noKeyWaiting	plp
goaway	lda	#0
	ldy	#available
	sta	[pBlock],y
	lda	#0
	jmp	GSOSReturn

	END

;=========================================================================
;
; ($11F) ReadKey
;  New ORCA/Shell (2.0.1) call to wait for an input character and
;  return the key and modifiers in Event Manager format.
;
;=========================================================================
; ReadKey does a number of checks to maintain sanity.  Currently, if the
; caller is in the BG no action occurs.  If the process' controlling TTY
; is not the console, we do a ReadChar to get pause keys, at least (we
; can't get modifiers from a serial port!).  If the tty is the console,
; we make a direct (fast) call to KEYIN in inout.asm

_ReadKey	START
_ReadKey	name
	using	KernelStruct

key	equ	0	key & mod. reg of keypress. Not valid
modifiers	equ  2	if available == 0

	lda   cmdNum
	and   #$40
	beq	orca_1
	lda	pCount
	cmp	#2	; check the pCount
	beq	orca_1
	lda	#4
	jmp	GSOSReturn
	
orca_1	anop
	jsr	isBGProcess
	bcs	isbg

	ldx	curProcInd
	lda	ttyID,x
*	cmp	#3
*	beq	isConsole	; it's the console
	
	pha	                   ; do something sensible for non-console
	pea	0
	ldx	#$220C	; TextTools/ReadChar
	jsl	$E10000
	pla
	and	#$00FF
	bra	setKey	

*isConsole	anop
;	phd
;               lda   InOutDP
;               tcd
*               jsl   KEYIN
;               pld
setKey	pha
	and	#$00FF

	ldy	#key
	sta	[pBlock],y
	pla
	xba
	and	#$00FF
	jsl	ConvKMR2EM
	ldy	#modifiers
	sta	[pBlock],y

	lda   #0
	jmp   GSOSReturn

* We eventually might want to go ahead and suspend the background
* process for trying to read; this could be useful if some dork
* ever writes a program that uses these routines for input (God help
* us).

isbg	lda	#0
	ldy	#key
	sta	[pBlock],y
	ldy	#modifiers
	sta	[pBlock],y

* Convert the keyboard modifier reg. format to the Event Manager modifiers
* format.

ConvKMR2EM	ENTRY
	tay		; store the old value
	ldx	#0	; initial modifier value
	bit	#%10000000	; open apple?
	beq	x1
	txa
	ora	#%0000000100000000
	tax
x1	tya
	bit	#%01000000
	beq	x2
	txa
	ora	#%0000100000000000
	tax
x2	tya
	bit	#%00010000
	beq	x3
	txa
	ora	#%0010000000000000
	tax
x3	tya
	bit	#%00000100
	beq	x4
	txa
	ora	#%0000010000000000
	tax
x4	tya
	bit	#%00000010
	beq	x5
	txa
	ora	#%0001000000000000
	tax
x5	tya
	bit	#%00000001
	beq	xdone
	txa
	ora	#%0000001000000000
	tax
xdone	txa
	rtl

isBGProcess	ENTRY
	ldx	curProcInd
	lda	ttyID,x
	asl   a
	tax
	lda   ttyStruct,x
	beq   nopgrp
	ldx   curProcInd
	cmp   pgrp,x
	beq   nopgrp             ; it's the foreground process
	sec
	rts
nopgrp	clc
	rts	
	END
