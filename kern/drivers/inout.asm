*	$Id: inout.asm,v 1.1 1998/02/02 08:17:56 taubert Exp $
**************************************************************************
*
* GNO Multitasking Environment
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*   Derek Taubert
*
**************************************************************************
*
* INOUT.ASM
*   By Jawaid Bazyar & Tim Meekins
*
*   GNO console driver for text display
*
**************************************************************************

*  x  PRINT - print a cstring of text at current cursor
*  x  COUT  - print a character at current cursor & increment cursor
*  x  STORCHAR - stuff a character at current cursor (but don't increment)
*  x  PickChar - get a character from current cursor
*  x  KEYIN - wait for a keystroke with the current cursor (ins/rpl)
*  x  VTAB - calculates position of current line and stores in BASL

	case	on
	mcopy	../drivers/inout.mac
	copy	../gno/inc/tty.inc
	copy	../drivers/console.equates
	copy	../drivers/kern.equates

consoleSem	gequ  $3C                ; output code semaphore

InOutData	DATA
InOutDP	dc 	i2'0'
blockCP	dc 	i2'$FFFF'  	; curProcInd of process blocked on input
;			(or -1 if none)
blinkRate	dc  i2'0'

; ACK! These are hideous- but if they work...
bufState	dc  i2'1'
OutStopped	dc  i2'0'
;ignSetInDev	dc  i2'0'
;ignSetInGlo    dc  i2'0'
;ignSetOutDev   dc  i2'0'
;ignSetOutGlo   dc  i2'0'
;ignSetErrDev	dc  i2'0'
;ignSetErrGlo	dc  i2'0'   

inCOUT	dc  i2'0'
keyMaps	dc  i2'0'
OldFlushVect	dc  i4'0'	
	END

InOutStart	START
	using InOutData
	using ADBData

TempHandle	equ  1

	jsl	PatchDeskManager
	phb
	phk
	plb
	phd
	ph4   #0
	ph4   #IODP_TextDPSize
	lda	contty+t_userid
	ora	#$0100
	pha
	ph2   #%1100000000000101
	ph4   #0
	_NewHandle

	tsc
	tcd
	lda   [TempHandle]
	sta   InOutDP
	tcd
	pla
	pla

;               pea   1
;               jsl   asmSemNew
;               sta   consoleSem

	lda   #$0000             ; >0 = inverse, 0 = normal
	sta   IODP_InvFlag
	stz   IODP_gInsertFlag

	stz   IODP_LeftMar
	stz   IODP_TopMar
	lda   #79
	sta   IODP_RightMar
	lda   #23
	sta   IODP_BotMar
	lda   #1
	sta   IODP_Scroll        ; scroll automatically at end-of-page
	sta   IODP_AutowrapFlag       ; automatically wrap to next line
	stz   IODP_GlobalCoords
	stz   IODP_GotoFlag
	lda	#contty
	sta	IODP_dTermioPtr
	lda	#^contty
	sta	IODP_dTermioPtr+2

	stz   IODP_CH                 ;[-TM 5/29/91]
	stz   IODP_CV
	lda   #0
	short a
	sta   >$57B
	sta   >$25
	long  a

	jsr VTAB

	stz   head
	stz   tail
	SetVector (#$F,#OurADB)
	IntSource #0
	lda   #0
	sta   bufState           ; make sure key buffer is ON

	stz   IODP_CurFlag
	stz   IODP_CurState
	jsr   cursOn
	pushlong #cursorHB
	_SetHeartBeat

	pha
	pea	$2F
	_ReadBParam
	pla
	sta	>blinkRate	

	short	m
	lda	>$E0C035
	and	#%11111110
	sta	>$E0C035
	long	m

	pha
	pha
	pea	$13
	_GetVector

	pl4	OldFlushVect
	pea	$13
	ph4	#NewFlush
	_SetVector

	pld
	plb
	rtl
	END

InOutEnd	START
	using InOutData

	phb
	phk
	plb
	IntSource #1
	pushlong #cursorHB
	_DelHeartBeat
;               phd
;               lda   >InOutDP
;               tcd
;               lda   consoleSem
;               pha
;               jsl   asmSemDispose
;               pld

;               WriteChar #12

	ph4	OldFlushVect
	pea	$13
	_SetVector

	plb
	jsl	UnpatchDeskManager
	rtl

	END

ClipH	START
	lda   IODP_CH
	cmp   #80
	bcc   bye
	lda   #0
	sta   IODP_CH
bye	rts
ClipV	ENTRY
	lda   IODP_CV
	cmp   #24
	bcc   later
	lda   #23
	sta   IODP_CV
later	rts
	END

COUT	START
	jmp	>COUT_CODE
	END
KEYIN	START
	jmp	>KEYIN_CODE
	END

*  Name   > COUT
*  Input  > A = Char to print, x & y don't care
*         > Inp/Out Globals
*  Output > char on screen, cursor position updated, A & X & Y unchanged[!!]

COUT_CODE	START
	using InOutData

	phx
	phd
	pha
	lda	>InOutDP
	tcd
	phb
	phk
	plb
	
stopChk	anop
	lda   >OutStopped
	beq   notStopped
	cop   $7F
	bra   stopChk

notStopped	anop
;               pei   (consoleSem)
;               jsl   asmWait
	jsl   >$E10064		; incBusy
	inc	inCOUT
	lda   IODP_CurFlag
	sta   oldCur             ; save the state!
	jsr   cursOff

	lda   >$57B                ; 80 column ch
	and   #$00FF
	cmp   IODP_CH
	beq   checkCV
	sta   IODP_CH
	jsr   ClipH
checkCV	anop
	lda   >$25
	and   #$00FF
	cmp   IODP_CV
	beq   humph
	sta   IODP_CV
	jsr   ClipV
	jsr   VTAB
humph	anop

	lda   2,s
	and   #$007F      ; strip hi bit
	cmp   #$20
	bcs   notControl
	lda   2,s
	and   #$007F
	jmp   controlChar
notControl	anop
	lda   IODP_GotoFlag
;
; Do a mini state graph for goto xy.
;
	jeq   out
	cmp	#3
	bcs	dosetport

	cmp   #1
	bne   goto2
	inc   IODP_GotoFlag
	lda   2,s
	and   #$7F
	sec
	sbc   #32
	sta   IODP_CH
	jsr   ClipH
	jmp   COUTxit
goto2	stz   IODP_GotoFlag
	lda   2,s
	and   #$7F
	sec
	sbc   #32
	sta   IODP_CV
	jsr   ClipV
	jsr   VTAB
	jmp   COUTxit

* The GotoFlag equals these values during a setport operation:
* 3 = looking for '['
* 4 = LeftMar
* 5 = RightMar
* 6 = TopMar
* 7 = BotMar
* 8 = done

dosetport	cmp   #8
	bcs	donesetport
	cmp	#3
	bne	step2
	lda	2,s
	and	#$7F
	cmp	#'['
	beq	step3
	stz	IODP_GotoFlag	; not found, terminate the setport
	jmp	COUTxit
step2	sec
	sbc	#4
	asl	a
	tax
	lda	2,s
	and	#$7F
	sec
	sbc	#32
	sta	IODP_LeftMar,x
step3	inc	IODP_GotoFlag
	jmp	COUTxit
donesetport	stz   IODP_GotoFlag
* 'clip' the new port dimensions to ensure that they won't confuse the
* code into a crash.  When in doubt, reset the default port parameters.
	lda	IODP_LeftMar
	cmp	IODP_RightMar
	bcc	ok_1
	stz	IODP_LeftMar
	lda	#79
	sta	IODP_RightMar
ok_1	lda	IODP_TopMar
	cmp	IODP_BotMar
	bcc	ok_2
	stz	IODP_TopMar
	lda	#23
	sta	IODP_BotMar
ok_2	lda	IODP_RightMar
	cmp	#80
	bcc	ok_3
	lda	#79
	sta	IODP_RightMar
ok_3	lda	IODP_BotMar
	cmp	#24
	bcc	out
	lda	#23
	sta	IODP_BotMar

* we have a 'printable' character, so find out what's up with the cursor

out	lda IODP_AutowrapFlag         ; do we autowrap
	beq NoWrap
	jsr FSpace
	bra OutPut
NoWrap	lda IODP_CH
	cmp IODP_RightMar
	bcc OutPut
	lda IODP_RightMar
	sta IODP_CH

* we modded the cursor and screen appropriately, so do it!

OutPut	lda   2,s
	and #$007F      ; only the low stuff, please
	bit IODP_InvFlag     ; normal or inverse?
	bpl Norm        ; bit 15 clear means normal text
	bvs NoAnd       ; i.e. don't fix it cause its mousetext
	cmp #$60        ; inverse!
	bcs NoAnd
	cmp #$40        ; special characters?
	bcc NoAnd
	sbc #$40        ; if it's an uppercase
	bra NoAnd
Norm	ora #$80        ; har har har
NoAnd	jsr StorChar
	inc IODP_CH
COUTxit	anop
	short m
	lda IODP_CH
	sta >$57B
	lda IODP_CV
	sta >$25
	long a
	lda   oldCur
	beq   norestore
	jsr   cursOn
	bra   gohome
norestore	anop
	jsr   PickChar
	sta   IODP_CurChar
gohome	anop
;               pei   (consoleSem)
;               jsl   asmSignal
	dec	inCOUT
	jsl   >$E10068		; decBusy
	plb
	pla
	pld
	plx
	rtl
oldCur	dc i2'0'

controlChar	anop
	asl a
	tax
	jmp (controlTable,x)

ctl0D	lda   IODP_LeftMar
	sta   IODP_CH
	jmp   COUTxit

ctl0A	jsr   FSpace2
	jmp   COUTxit

;              cmp   #8                 ; backspace?
ctl08	lda   IODP_CH
	cmp   IODP_LeftMar
	beq   ctl08a
	dec   IODP_CH
	jmp   COUTxit
ctl08a	lda	IODP_RightMar
	sta	IODP_CH
	bra	ctl1F

;              cmp   #21                ; right arrow
ctl15	jsr   FSpace             ; if we're already in never-neverland, move to col0
	inc   IODP_CH                 ; and increment the CH
	jsr   FSpace             ; if we weren't, see if we are now, and fix it too
	jmp   COUTxit

;              cmp   #"_"-64            ; Move up
ctl1F	lda   IODP_CV
	cmp   IODP_TopMar
	beq   ctl4b              ; can't go any further up
	dec   IODP_CV
	jsr   VTAB
;              lda   IODP_RightMar
;              sta   IODP_CH
ctl4b	jmp   COUTxit

;              cmp   #"N"-64            ; normal text mode?
;              cmp   #"X"-64            ; fake normal
ctl0E	anop
	lda   IODP_InvFlag
	and   #%0111111111111111
	sta   IODP_InvFlag
	jmp   COUTxit

ctl18	lda   IODP_InvFlag
	and   #%1011111111111111
	sta   IODP_InvFlag
	jmp   COUTxit

;              cmp   #"O"-64            ; inverse text
ctl0F	lda   IODP_InvFlag
	ora   #$8000
	sta   IODP_InvFlag
	jmp   COUTxit

;              cmp   #"["-64            ; mousetext
ctl1B	lda   IODP_InvFlag
	ora   #%0100000000000000
	sta   IODP_InvFlag
	jmp   COUTxit
;im000000
;i = inverse text?
;m = mouse text?

;              cmp   #"L"-64            ; form feed = clear screen
ctl0C	jsr   HOME
	jmp   COUTxit

;              cmp   #"Y"-64            ; move cursor home
ctl19	stz   IODP_CH
	stz   IODP_CV
	jsr   VTAB
	jmp   COUTxit

;              cmp   #"]"-64            ; clear EOL
ctl1D	jsr   CLREoln
	jmp   COUTxit

;              cmp   #"K"-64            ; clear EOP
ctl0B	jsr   CLREop
	jmp   COUTxit

;              cmp   #"^"-64            ; goto xy
ctl1E	lda   #1
	sta   IODP_GotoFlag
	jmp   COUTxit

;              cmp   #"Z"-64            ; clear line
ctl1A	lda   IODP_CH
	pha
	lda   IODP_LeftMar
	sta   IODP_CH
	jsr   CLREoln
	pla
	sta   IODP_CH
	jmp   COUTxit

;              cmp   #"I"-64            ; tab
ctl09	lda   IODP_CH
	lsr3  a
	inc   a
	asl3  a
	sta   IODP_CH
	jsr   FSpace
	jmp   COUTxit

;              cmp   #"G"-64            ; beep
ctl07	ldx #$40
tone	ldy #$80
	lda $E0C030     ;speaker!
ll	dey
	bne ll
	dex
	bne tone
	jmp   COUTxit

;              cmp   #"F"-64            ; turn off cursor
ctl06	stz   oldCur             ; cursor is already off! (see above)
	jmp   COUTxit

;              cmp   #"E"-64            ; turn on cursor
ctl05	lda   #1
	sta   oldCur             ; make sure it comes back on
	jmp   COUTxit

;              cmp   #"V"-64            ; scroll down one line
ctl16	jsr   ScrollWindowDn
	jmp   COUTxit

;              cmp   #"W"-64            ; scroll up one line
ctl17	jsr   ScrollWindowUp
	jmp   COUTxit

;              cmp   #"\"-64
ctl1C	inc   IODP_CH
	jmp   COUTxit

;	cmp	#'Q'-64
ctl11	ldx	#1
	jsr	InsLine
	jmp	COUTxit
	
;	cmp	#'R'-64
ctl12	ldx	#1
	jsr	DelLine
	jmp	COUTxit

ctlout	anop
	jsr StorChar
	inc IODP_CH
	jmp   COUTxit
setOver	anop
	stz   IODP_gInsertFlag
	jmp   COUTxit
setInsert	anop
	lda   #1
	sta   IODP_gInsertFlag
	jmp   COUTxit
ctlsetport	anop
	lda	#3
	sta	IODP_GotoFlag
	jmp	COUTxit
controlTable	anop
	dc    i2'ctlout'
	dc    i2'setOver'        ; set cursor to overstrike mode
	dc    i2'setInsert'      ; set cursor to insert mode
*	dc    i2'ctlout'
	dc    i2'ctlsetport'
	dc    i2'ctlout'
	dc    i2'ctl05'     ;
	dc    i2'ctl06'     ;
	dc    i2'ctl07'     ;
	dc    i2'ctl08'     ;
	dc    i2'ctl09'     ;
	dc    i2'ctl0A'     ;
	dc    i2'ctl0B'     ;
	dc    i2'ctl0C'     ;
	dc    i2'ctl0D'     ;
	dc    i2'ctl0E'     ;
	dc    i2'ctl0F'     ;
	dc    i2'ctlout'
	dc    i2'ctl11'
	dc    i2'ctl12'
	dc    i2'ctlout'
	dc    i2'ctlout'
	dc    i2'ctl15'     ;
	dc    i2'ctl16'     ;
	dc    i2'ctl17'     ;
	dc    i2'ctl18'     ;
	dc    i2'ctl19'     ;
	dc    i2'ctl1A'     ;
	dc    i2'ctl1B'     ;
	dc    i2'ctl1C'
	dc    i2'ctl1D'     ;
	dc    i2'ctl1E'     ;
	dc    i2'ctl1F'     ;

FSpace	anop
	lda IODP_CH
	cmp IODP_RightMar
	beq Bye
	bcc Bye
	lda IODP_LeftMar
	sta IODP_CH
FSpace2	lda IODP_CV
	cmp IODP_BotMar
	bcs CheckScroll
	inc IODP_CV
	jmp VTAB
CheckScroll	lda IODP_Scroll   ; do they want us to scroll?
	beq Bye           ; nope, har har
	jmp ScrollWindowUp
Bye	rts
	END

cursOff	START
	lda   IODP_CurFlag
	beq   ctl16a             ; already turned off
	lda   IODP_CurState           ; are currently in the 'invert'
	beq   ctl16a             ; phase of cursor blink?
	jsr   normalCurs         ; yeah, so turn it off
ctl16a	stz   IODP_CurFlag
	rts
	END

cursOn	START
	lda   IODP_CurFlag
	bne   ctl17a             ; already turned on
	lda   IODP_CurState           ; are we currently in the 'normal'
	bne   ctl17a             ; phase of cursor blink?
	jsr   invertCurs         ; yeah, so turn it on
ctl17a	lda   #1
	sta   IODP_CurFlag
	rts
	END

*
* Get character under cursor, save it, then invert it
*
invertCurs	START
	phy
ok	jsr   PickChar
	sta   IODP_CurChar
	lda   #"_"+128    ; insert cursor, which do we want?
	ldy   IODP_gInsertFlag ; ins = 1, rpl = 0
	bne   done
	lda   IODP_CurChar
	short a
	eor   #$80
	bmi   done
	cmp   #$60
	bcs   done
	cmp   #$40
	bcc   done
	sbc   #$40
done	long  a
	jsr   StorChar
	lda   #1
	sta   IODP_CurState
	ply
	rts
	END

normalCurs	START
	stz   IODP_CurState
	lda   IODP_CurChar
	jmp   StorChar
	END


*  Name   > KEYIN
*  Input  > x,y,a don't care, keyboard input
*  Output > flashing cursor, A = char typed & X & Y unchanged

KEYIN_CODE	START
	using ADBData
	using InOutData

	phb
	phk
	plb
	phd
	lda	>InOutDP
	tcd

	jsl   >$E10064		; incBusy
	short a
	lda >$57B                ; 80 column ch
	sta IODP_CH
	lda >$25
	cmp IODP_CV
	beq restart
	sta IODP_CV
	long  a
	jsr VTAB
restart	long  a
	lda   >bufState
	beq   gotBuffer
	brl	noBuffer
gotBuffer	anop
	short ai
	lda   tail
	cmp   head
	bne   GotKey
	long  ai
;	lda	>curProcInd
	lda	#1		; this is how port.asm took care of this
	sta	>blockCP
	jsl	contty+t_GetProcInd	; get pointer to process entry
	sta	IODP_procPtr
	stx	IODP_procPtr+2
	ldy	#2		; set it to blocked state
	lda	#pBlocked
	sta	[IODP_procPtr],y
	jsl   >$E10068		; decBusy
	ldy	#84		; index of waitDone field
	lda	#0
	sta	[IODP_procPtr],y	; set to zero, moron

	cop	$7f		; wait for data to come in
	jsl   >$E10064		; incBusy
	ldy	#84
	lda	[IODP_procPtr],y
	beq   restart
	cmp   #1
	beq   restart            ; if waitdone 0|1 restart the call
	lda   #$FFFF             ; waitdone was -1, so
	sta   >blockCP           ; reset this to keep things clean
	lda   #$7E43             ; signal
	bra   doneread
GotKey	anop
	long ai
	lda   #$FFFF
	sta   >blockCP           ; reset this to keep things clean
	ldx   tail
	lda   keybuf,x
	and   #$00FF	; don't clear this here
	sta   tmp1
	short i
	ldx   tail
	lda   modbuf,x
	inx
	stx   tail
	long  i
	xba
	and   #$FF00             ; save only that
	ora   tmp1               ; add key modifiers to character input
doneread	anop
	jsl   >$E10068		; decBusy
	pld
	plb
	rtl
tmp1	dc i2'0'
noBuffer	anop
	jsl   >$E10068		; decBusy
	short a
loop	anop
	lda   >$E0C000
	bpl   loop
	and   #$7F
	sta   >tmp1
	lda   >$E0C025
	sta   >tmp1+1
	sta   >$E0C010
	long  a
	lda   >tmp1
	pld
	plb
	rtl
	END

*  Name   > Delay/Private
*  Input  > none
*  Output > Delay of about 1/2 second,X & Y unchanged, A = ??

*Delay          PRIVATE
*
*               using ADBData
*               using InOutData
*
*              lda   >blinkRate	;Flash rate
*              and   #$FF
*              asl	a
*              asl	a
*              sta	count
*
*elayLoop1     lda   tail
*              cmp   head
*              bne 	DelayOut
*              lda   $E1C019
*              and   #$80
*              beq   DelayLoop1
*
*elayLoop2     lda   tail
*              cmp   head
*              bne 	DelayOut
*              lda   $E1C019
*              and   #$80
*              bne   DelayLoop2
*
*              dec count
*              bne DelayLoop1
*
*elayOut       rts
*
*ount          dc i2'0'
*              END

*
* ADB interrupt handler (only if keyboard)
*
OurADB	START KERN2

	using ADBData
	using InOutData

	php
	phd
	long	m
	lda	>InOutDP
	tcd

	lda   >bufState          ; whether the buffer is on or off
	jne   done2
	short ai
	lda   >$E0C000
	and	#$7f	; turn off that stupid bit
	pha
	jsr   checkIntr
	bcc   notatty            ; c=1 means intr char was found
	sta   >$E0C010
	jmp   done3
notatty	anop		
	lda	>$E0C025	; we'll look at this, first
	pha
* now char/mods are on stack
	lda	>keyMaps
	bit	#%1000	; vt100 arrows on?
	beq	notVT100

	lda	1,s	; check for the control key
	bit	#%0010
	jne	notVT100	; they hit control-something

	lda	2,s
	cmp	#$A
	beq	isArrow
	cmp	#$B
	beq	isArrow
	cmp	#$8
	beq	isArrow
	cmp	#$15
	jne	notVT100	; not one of the arrows
isArrow	anop
	lda	>keyMaps	; is OA mapping set? if so, we
	bit	#%0011	; should not map to vt100 arrows
	beq	notMetaMap	; then map as an OA-ascii
	lda	1,s
	bit	#$80
	bne	doMetaMap

notMetaMap	ldy   #0
	lda	#27
	jsr	add2q
	lda	#'O'
	jsr	add2q
	pla
	pla		; the character
	ldx	#'A'
	cmp	#$B
	beq	doit1
	ldx	#'B'
	cmp	#$A
	beq	doit1
	ldx	#'C'
	cmp	#$15
	beq	doit1
	ldx	#'D'
doit1	txa
	jsr	add2q
	jmp	doUnblock

notVT100	anop
	lda	>keyMaps
	bit	#%0001
	beq	doNormal
	bit	#%0110
	beq	doNormal	; neither bit is set. Goofup!
	bit	#%0100
	bne	doHiBitMap
doMetaMap	anop
	lda	1,s
	bit	#$80	; oa?
	beq	doNormal
	ldy	#0
	lda	#27	; the META character (ESC)
	jsr	add2q	; y is the modifier flag
	pla
	pla
	jsr	add2q
	bra	doUnblock
doHiBitMap	pla
	tay
	bit	#$80
	beq	doNormal1	; it's not set!!!!
	and	#$7F
	tay		; okay now
	pla		; grab the character
	ora	#$80
	jsr	add2q
	bra	doUnblock

doNormal	ply
doNormal1	anop
	pla
	jsr	add2q
; this chunk unblocks any process that was waiting on keyboard input

doUnblock	php
	sta   >$E0C010
	long ai

	lda	>$E100B8	; this is going to GSBug via EM, not
	bit	#1	; a process
	bne	GSBugActive

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

GSBugActive	anop
done	anop
	plp
done2	anop
	pld
	plp
	clc
	rtl
done3	anop
	pla
	pld
	plp
	clc
	rtl

add2q	pha	
	lda   >head
	inc	a
	cmp	>tail	; buffer overflow?
	beq	done2	; yes, ignore this key
	lda	>head
	tax
	pla
	sta   >keybuf,x
	tya
	sta   >modbuf,x
	inx
	txa
	sta   >head
	rts

	longi on
	longa on

	END

* Handles Control-OA-Delete, and flushes the GNO keyboard buffer

NewFlush	START
	using	ADBData
	php
	long	ai
	lda	#0
	sta	>head
	sta	>tail
	plp
	clc
	rtl
	longa	on
	longi	on
	END

checkIntr	START KERN2
	using InOutData

	php
	long  ai
	and   #$7f
	pha
	short	m

	ldy	#sg_flags
	lda	[IODP_dTermioPtr],y
	bit	#RAW	; RAW mode?
	beq	x9	; yep, no character checking
	brl	notty
x9	ldy	#t_quitc
	lda   [IODP_dTermioPtr],y
	cmp	#-1
	beq	x0
	cmp	1,s
	beq   gotQQ
x0	ldy	#t_suspc
	lda   [IODP_dTermioPtr],y
	cmp	#-1
	beq	x1
	cmp	1,s
	beq   gotZ
x1	ldy	#t_intrc
	lda   [IODP_dTermioPtr],y
	cmp	#-1
	beq	x2
	cmp	1,s
	beq   gotC
x2	lda	>OutStopped
	bne	x3
	ldy	#t_stopc
	lda   [IODP_dTermioPtr],y
	cmp	#-1
	beq	x3
	cmp	1,s
	beq   gotS
x3	ldy	#t_startc
	lda	[IODP_dTermioPtr],y
	cmp	#-1
	beq	notty
	cmp   1,s
	beq   gotQ
	bra   notty
gotS	long	m
	pla
	lda   #1
	sta   >OutStopped
	plp
	sec
	rts
gotQ	long	m
	pla
	lda   >OutStopped
	beq   notQ
	lda   #0
	sta   >OutStopped
	plp
	sec
	rts
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
	ph2	>contty+t_devNum	; push our device number
	jsl	contty+t_sendSignal

; flush internal editing buffers on interrupt character
	lda	#0
	sta	>contty+editInd
	sta	>contty+editBegin
	sta	>contty+st_flags

	ph4	#$80027410
	ph4	#0
	lda	>contty+t_devNum
	pha
	jsl	ConIOCTL	; flush the raw input queue

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

ADBData	DATA

head	ds    2
tail	ds    2
keybuf	ds    256
modbuf	ds    256

	END

*  Name   > CalcBankCH/Private
*  Input  > CH = cursor horizontal position
*  Output > Bank number of col in CH stored in BASL+2, y = a = 40col offset
	longa off
CalcBankCH	PRIVATE

	lda   #0                 ;TM 5/18/91
	xba                      ;----------
	lda IODP_CH
	cmp IODP_RightMar
	bcc Calc2
	beq Calc2
	lda IODP_RightMar
Calc2	lsr a
	tay               ; column number
	lda   #0          ; c = 1 if main mem, 0 if auxmem
	rol a
	eor #%00000001    ; not quite as fast as before, but hey...
	sta IODP_BASL+2
	rts

	END
	longa on

*  Name   > STORCHAR
*  Input  > A is Char to poke, x & y don't care
*         > cursor stored in CH,CV,BASL
*         > A must NOT be greater than $00FF
*  Output > A,X,Y unchanged, char in banks $E0 or $E1

StorChar	START

	phy
	short a
	pha
	jsr CalcBankCH
	pla
	sta [IODP_BASL],y
	long a
	ply
	rts

	END

*  Name   > PickChar
*  Input  > x & y don't care
*         > cursor stored in CH,CV,BASL
*  Output > A = char, X,Y unchanged

PickChar	START

	phy
	short a
	jsr CalcBankCH
	lda [IODP_BASL],y
	long a
	ply
	and #$00FF      ; strip garbage off
	rts

	END

*  Name   > VTAB,VTABZ
*  Input  > CV for VTAB; A for VTABZ (both contain line to calculate base for)
*  Output > Base screen address for line, A = BASE.

VTAB	START
	using TextTable

	lda IODP_CV
VTABZ	ENTRY
	phy
	asl a
	tay
	lda TextRowAddr,y
	sta IODP_BASL
	ply
	rts
	END

TextTable	DATA
TextRowAddr	dc i2'$0400'
	dc i2'$0480'
	dc i2'$0500'
	dc i2'$0580'
	dc i2'$0600'
	dc i2'$0680'
	dc i2'$0700'
	dc i2'$0780'
	dc i2'$0428'
	dc i2'$04A8'
	dc i2'$0528'
	dc i2'$05A8'
	dc i2'$0628'
	dc i2'$06A8'
	dc i2'$0728'
	dc i2'$07A8'
	dc i2'$0450'
	dc i2'$04D0'
	dc i2'$0550'
	dc i2'$05D0'
	dc i2'$0650'
	dc i2'$06D0'
	dc i2'$0750'
	dc i2'$07D0'
	END

HOME	START
	using InOutData

	phx
	phy
	lda 	IODP_LeftMar
	bne 	DoFillBox
	lda 	IODP_RightMar
	cmp	#79
	bne	DoFillBox
	lda	IODP_TopMar
	sta	IODP_CV
Loop	jsr	VTAB        ; set the line
	ldx	IODP_BASL
	ldy	#20         ; only 40 per bank, and 2 bytes/pass
	lda	#$A0A0      ; har har
Loop1	anop
	sta   >$000000,x
	sta   >$010000,x
	inx
	inx
	dey
	bne	Loop1

	lda	IODP_CV
	cmp	IODP_BotMar
	bcs	urgh

	inc   a
	sta	IODP_CV
	bra 	Loop

urgh	stz	IODP_CH
	stz	IODP_CV
	ply
	plx
	jmp	VTAB

DoFillBox	lda   IODP_Scroll
	pha
	stz	IODP_Scroll

	lda	#" "
	jsr	EraseBox

	pla
	sta	IODP_Scroll
	lda	IODP_LeftMar
	sta	IODP_CH
	lda	IODP_TopMar
	sta	IODP_CV
	ply
	plx
	jmp	VTAB

	END

*  Name   > PrintXChars
*  Input  > x = number of chars to print
*         > a = character to print
*  Output > x = 0, a = char

PrintXChars	START KERN2
	cpx #0
	beq DonePXC
PrintEm	jsl COUT
	dex
	bne PrintEm
DonePXC	rts
	END

CLREoln	START

	lda IODP_CH
	pha             ;save it to restore later

Loop	lda #" "+128
	jsr StorChar
	inc IODP_CH
	lda IODP_CH
	cmp IODP_RightMar
	bcc Loop
	beq Loop

	pla
	sta IODP_CH
	rts

	END

CLREop	START

	lda IODP_CH
	pha
	lda IODP_CV
	pha

Loop	jsr CLREoln     ; do the end of This line
	inc IODP_CV
	jsr VTAB
	lda IODP_LeftMar
	sta IODP_CH
	lda IODP_CV
	cmp IODP_BotMar
	bcc Loop
	beq Loop

	pla
	sta IODP_CV
	jsr VTAB
	pla
	sta IODP_CH
	rts
	END


CLRSop	START

	lda IODP_CH
	sta curh
	lda IODP_CV
	sta curv

	lda IODP_LeftMar
	sta IODP_CH
FirstLoop	lda #' '+128
	jsr StorChar
	inc IODP_CH
	lda IODP_CH
	cmp curh
	beq FirstLoop
	bcc FirstLoop
	lda #0
	sta IODP_CV
	lda IODP_LeftMar
	sta IODP_CH
Loop	jsr VTAB
	jsr CLREoln     ; do the end of This line
	inc IODP_CV
	jsr VTAB
	lda IODP_LeftMar
	sta IODP_CH
	lda IODP_CV
	cmp curv
	bcc Loop

	lda curh        ; already vtabbed to that line
	sta IODP_CH
	rts
curh	dc i2'0'
curv	dc i2'0'
	END

ScrollWindowUp	START
	using InOutData

*               bra   QuickScroll        ;TM

Okay	lda IODP_LeftMar
	bne SlowScroll
	lda IODP_RightMar
	cmp #79
	beq QuickScroll

SlowScroll	lda IODP_CV
	pha
	lda IODP_CH
	pha
	phx                      ;TM 5/17/91

	lda IODP_TopMar
	sta IODP_CV
	jsr VTAB
	ldx IODP_BASL
	stx DstLine
LineLoop	inc IODP_CV
	jsr VTAB
	ldx IODP_BASL
	stx SrcLine

* copy a line using PickChar and storchar

	lda IODP_LeftMar
	sta IODP_CH

InLineLoop	ldx SrcLine
	stx IODP_BASL

	jsr PickChar
	ldx DstLine
	stx IODP_BASL
	jsr StorChar    ; this should work, but I dunno how fast
	inc IODP_CH
	lda IODP_CH
	cmp IODP_RightMar
	bcc InLineLoop
	beq InLineLoop

	ldx SrcLine
	stx DstLine
	lda IODP_CV
	cmp IODP_BotMar
	bcc LineLoop

	ldx SrcLine
	stx IODP_BASL
	lda IODP_LeftMar
	sta IODP_CH
ClrLineLoop	lda #$A0                 ;this better work
	jsr StorChar
	inc IODP_CH
	lda IODP_CH
	cmp IODP_RightMar
	bcc ClrLineLoop
	beq ClrLineLoop

	plx                      ;TM 5/17/91
	pla
	sta IODP_CH
	pla
	sta IODP_CV
	jmp VTAB

GoAway	rts
SrcLine	dc i2'0'
DstLine	dc i2'0'

* QuickScroll uses self-modifying code to kick ass

QuickScroll	lda IODP_CV
	pha
	lda IODP_CH
	pha
	phx                      ;TM 5/17/91
	phy

	lda IODP_TopMar
	sta IODP_CV
	jsr VTAB
	lda IODP_BASL
	sta CopyTwo0+1
	sta CopyTwo1+1
	ldx IODP_CV
	cpx IODP_BotMar      ; if there's only one line, just clear it
	bne QSNextLine
	tax             ; we wanna clear this line and not scroll
	bra clearline
QSNextLine	inc IODP_CV
	jsr VTAB
	lda IODP_BASL
	sta CopyOne0+1
	sta CopyOne1+1

	ldx #38         ; yeah, oh yeah
CopyOne0	lda >$000000,x
CopyTwo0	sta >$000000,x
CopyOne1	lda >$010000,x
CopyTwo1	sta >$010000,x
	dex
	dex
	bpl CopyOne0

	lda CopyOne0+1
	sta CopyTwo0+1
	sta CopyTwo1+1

	lda IODP_CV
	cmp IODP_BotMar
	bcc QSNextLine

	ldx CopyOne1+1
clearline	lda #$A0A0               ; spaces out the ass
	ldy #10                  ;TM
clrloop	sta >$010000,x
	sta >$000000,x
	inx
	inx
	sta >$010000,x           ;TM
	sta >$000000,x           ;
	inx                      ;
	inx                      ;
	dey
	bne clrloop

	ply
	plx                      ;TM 5/17/91
	pla
	sta IODP_CH
	pla
	sta IODP_CV
	jmp VTAB

	END

ScrollWindowDn	START
	using InOutData

*               bra   QuickScroll        ;TM

Okay	lda IODP_LeftMar
	bne SlowScroll
	lda IODP_RightMar
	cmp #79
	beq QuickScroll

SlowScroll	lda IODP_CV
	pha
	lda IODP_CH
	pha
	phx                      ;TM 5/17/91

	lda IODP_BotMar
	sta IODP_CV
	jsr VTAB
	ldx IODP_BASL
	stx DstLine
LineLoop	dec IODP_CV
	jsr VTAB
	ldx IODP_BASL
	stx SrcLine

* copy a line using PickChar and storchar

	lda IODP_LeftMar
	sta IODP_CH

InLineLoop	ldx SrcLine
	stx IODP_BASL

	jsr PickChar
	ldx DstLine
	stx IODP_BASL
	jsr StorChar    ; this should work, but I dunno how fast
	inc IODP_CH
	lda IODP_CH
	cmp IODP_RightMar
	bcc InLineLoop
	beq InLineLoop

	ldx SrcLine
	stx DstLine
	lda IODP_TopMar
	cmp IODP_CV
	bcc LineLoop

	ldx SrcLine
	stx IODP_BASL
	lda IODP_LeftMar
	sta IODP_CH
ClrLineLoop	lda #$A0                 ;this better work
	jsr StorChar
	inc IODP_CH
	lda IODP_CH
	cmp IODP_RightMar
	bcc ClrLineLoop
	beq ClrLineLoop

	plx                      ;TM 5/17/91
	pla
	sta IODP_CH
	pla
	sta IODP_CV
	jmp VTAB

GoAway	rts
SrcLine	dc i2'0'
DstLine	dc i2'0'

* QuickScroll uses self-modifying code to kick ass
QuickScroll	lda IODP_CV
	pha
	lda IODP_CH
	pha
	phx                      ;TM 5/17/91
	phy

	lda IODP_BotMar
	sta IODP_CV
	jsr VTAB
	lda IODP_BASL
	sta CopyTwo0+1
	sta CopyTwo1+1
	ldx IODP_CV
	cpx IODP_TopMar      ; if there's only one line, just clear it
	bne QSNextLine
	tax             ; we wanna clear this line and not scroll
	bra clearline
QSNextLine	dec IODP_CV
	jsr VTAB
	lda IODP_BASL
	sta CopyOne0+1
	sta CopyOne1+1

	ldx #38         ; yeah, oh yeah
CopyOne0	lda >$000000,x
CopyTwo0	sta >$000000,x
CopyOne1	lda >$010000,x
CopyTwo1	sta >$010000,x
	dex
	dex
	bpl CopyOne0

	lda CopyOne0+1
	sta CopyTwo0+1
	sta CopyTwo1+1

	lda IODP_CV
	cmp IODP_TopMar
	bne QSNextLine

	ldx CopyOne1+1
clearline	lda #$A0A0               ; spaces out the ass
	ldy #10                  ; TM
clrloop	sta >$010000,x
	sta >$000000,x
	inx
	inx
	sta >$010000,x           ;TM
	sta >$000000,x           ;
	inx                      ;
	inx                      ;
	dey
	bne clrloop

	ply
	plx                      ;TM 5/16/91
	pla
	sta IODP_CH
	pla
	sta IODP_CV
	jmp VTAB

	END

* Insert Xreg lines at the current cursor location, using some of the
* text screen attributes

InsLine	START
	stx temp
	lda IODP_TopMar
	pha
	lda IODP_CV
	sta IODP_TopMar
loop	ldx temp
	beq nomoreins
	jsr ScrollWindowDn
	dec temp
	bra loop
nomoreins	pla
	sta IODP_TopMar
	rts
temp	dc i2'0'
	END

DelLine	START
	stx temp
	lda IODP_TopMar
	pha
	lda IODP_CV
	sta IODP_TopMar
loop	ldx temp
	beq nomoredel
	jsr ScrollWindowUp
	dec temp
	bra loop
nomoredel	pla
	sta IODP_TopMar
	rts
temp	dc i2'0'
	END

	longa off
cursorHB	DATA
	dc i4'0'
count	dc i2'30'
	dc h'5AA5'
	END
cursorRout	START
	using InOutData
	using cursorHB
	phb

	phk
	plb

	php
	phd
	long  ai
	lda   >InOutDP
	tcd
	lda	>inCOUT
	bne	rInCout	; if inside COUT, ignore this HB
	lda   >blinkRate	;Flash rate
	and   #$FF
	cmp   #0
	bne   flashing           ; is the rate 0?
	lda   #10
	sta   count
	lda   IODP_CurState           ; basically, force the cursor on
	bne   nocurs             ; it's on, so leave it alone
	lda   #1
	sta   IODP_CurState
	lda   IODP_CurFlag
	beq   nocurs
	jsr   invertCurs
	bra   nocurs
flashing	anop
	asl   a
	tax
	lda   tickTbl-2,x
	sta   count
	lda   IODP_CurState
	eor   #1                 ; flip the cursor state we're
	sta   IODP_CurState
	lda   IODP_CurFlag
	beq   nocurs
	lda   IODP_CurState
	beq   norm
	jsr   invertCurs
	bra   nocurs
norm	jsr   normalCurs
nocurs	pld
	plp
	clc
	plb
	rtl

* don't change the cursor state while we're inside COUT... it's a
* race condition

rInCout	lda   >blinkRate	;Flash rate
	and   #$FF
	cmp   #0
	bne   isflash	; is the rate 0?
	lda   #10
	bra	storeit
isflash	asl   a
	tax
	lda   tickTbl-2,x
storeit	sta   count
	bra	nocurs

tickTbl	dc i2'60'
	dc i2'30'
	dc i2'15'
	dc i2'10'
	END

FlexBeep	START KERN2
	phx
	phy
	pha

	ldx #$40
tone	ldy #$80
	lda $E0C030     ;speaker!
ll	dey
	bne ll
	dex
	bne tone

	pla
	ply
	plx
	rts
	END

* Special support for CDA keyboard input
* Save some state so we can restore it when we leave
ConSaveAllPatch	START
	using cursorHB
	using InOutData

	php
	sei
	long  ai
	phb
	phk
	plb
	pha
	phx
	phy

	lda   blockCP
	sta   tblockCP
	lda   count
	sta   tcount
	lda   #0
	sta   count
	lda   bufState
	sta   tbufState
	inc   a
	sta   bufState

	ply
	plx
	pla
	plb
	plp
	dc    i1'$5C'
CONOLDSAVEALL	ENTRY
	dc    i4'0'

ConRestAllPatch	ENTRY
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

	lda   tblockCP
	sta   blockCP
	lda   tcount
	sta   count
	lda   tbufState
	sta   bufState

	pha	                   ; re-read the blink rate
	pea	$2F                ; in case they changed it
	_ReadBParam
	pla
	sta	>blinkRate	

	pld
	ply
	plx
	pla
	plb
	plp
gooldRest	dc  i1'$5C'
CONOLDRESTALL	ENTRY
	dc  i4'0'

tblockCP	dc  i2'0'
tcount	dc  i2'0'
tbufState	dc  i2'0'
	END

