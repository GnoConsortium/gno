*	$Id: ctool.asm,v 1.2 1998/02/22 05:05:40 taubert Exp $

	mcopy m/ctool.mac

	case on

;TDISPOSEHAND	START
;	pha
;	lda	9,s	; we have something on the stack dummy
;	cmp	#$8b98
;	bne	goold
;	lda	11,s
;	cmp	#$00E0
;	bne	goold
;	ldx	#$2C03
;	jsl	$E10000
;always	bra	always
;goold	pla
;OLDDISPHAND	ENTRY
;	jmp	>$000000
;	END

* all processes that end with RTL come here
endproc2	START
	using KernelStruct
	phk
	plb
	ldx   curProcInd
	sta   exitCode,x
	jmp   >endproc
	END

kernTable	START KERN2
	dc i4'(TheEnd-kernTable)/4'

	dc i4'tBootInit-1'             ; 1
	dc i4'tStartUp-1'              ; 2
	dc i4'tShutDown-1'             ; 3
	dc i4'tVersion-1'              ; 4
	dc i4'tReset-1'                ; 5
	dc i4'tStatus-1'               ; 6
	dc i4'NotImpMM-1'              ; 7
	dc i4'NotImpMM-1'              ; 8

	dc i4'TIgetpid-1'              ; 9
	dc i4'TIkill-1'                ; a
	dc i4'TIfork-1'                ; b
	dc i4'NotImpMM-1'              ; c Was exec...
	dc i4'TIswait-1'               ; d
	dc i4'TIssignal-1'             ; e
	dc i4'TIscreate-1'             ; f
	dc i4'TIsdelete-1'             ; 10
	dc i4'TIkvm_open-1'            ; 11
	dc i4'TIkvm_close-1'           ; 12
	dc i4'TIkvm_getproc-1'         ; 13
	dc i4'TIkvm_nextproc-1'        ; 14
	dc i4'TIkvm_setproc-1'         ; 15
	dc i4'TIsignal-1'              ; 16
	dc i4'TIwait-1'                ; 17
	dc i4'TItcnewpgrp-1'           ; 18
	dc i4'TIsettpgrp-1'            ; 19
	dc i4'TItctpgrp-1'             ; 1a
	dc i4'TIsigsetmask-1'          ; 1b
	dc i4'TIsigblock-1'            ; 1c
	dc i4'TIexecve-1'              ; 1d
	dc i4'TIalarm-1'               ; 1e
	dc i4'TIsetdebug-1'            ; 1f
	dc i4'TIsetsystemvector-1'     ; 20
	dc i4'TIsigpause-1'            ; 21
	dc i4'TIdup-1'                 ; 22
	dc i4'TIdup2-1'                ; 23
	dc i4'TIpipe-1'                ; 24
	dc i4'TIgetpgrp-1'             ; 25
	dc i4'TIioctl-1'         ; 26
	dc i4'TIstat-1'	; 27
	dc i4'TIfstat-1'	; 28
	dc i4'TIlstat-1'	; 29
	dc i4'TIgetuid-1'	; 2A
	dc i4'TIgetgid-1'	; 2B
	dc i4'TIgeteuid-1'	; 2C
	dc i4'TIgetegid-1'	; 2D
	dc i4'TIsetuid-1'	; 2E
	dc i4'TIsetgid-1'        ; 2F
	dc i4'TIprocsend-1'	; 30
	dc i4'TIprocreceive-1'	; 31
	dc i4'TIrecvclr-1'	; 32
	dc i4'TIrecvtim-1'	; 33
	dc i4'TIsetpgrp-1'	; 34
	dc i4'TItimes-1'	; 35
	dc i4'TIpcreate-1'	; 36
	dc i4'TIpsend-1'	; 37
	dc i4'TIpreceive-1'	; 38
	dc i4'TIpdelete-1'	; 39
	dc i4'TIpreset-1'	; 3A
	dc i4'TIpbind-1'	; 3B
	dc i4'TIpgetport-1'	; 3C
	dc i4'TIpgetcount-1'	; 3D
	dc i4'TIscount-1'	; 3E
	dc i4'TIfork2-1'	; 3F
	dc i4'TIgetppid-1'	; 40
	dc i4'TISetGNOQuitRec-1'	; 41
	dc i4'TIalarm10-1'	; 42
	dc i4'TIselect-1'	; 43
	dc i4'TIInstallNetDriver-1' ; 44
	dc i4'TIsocket-1'	; 45
	dc i4'TIbind-1'	; 46
	dc i4'TIconnect-1'	; 47
	dc i4'TIlisten-1'	; 48
	dc i4'TIaccept-1'	; 49
	dc i4'TIrecvfrom-1'	; 4A
	dc i4'TIsendto-1'	; 4B
	dc i4'TIrecv-1'	; 4C
	dc i4'TIsend-1'	; 4D
	dc i4'TIgetpeername-1' ; 4E
	dc i4'TIgetsockname-1' ; 4F
	dc i4'TIgetsockopt-1' ; 50
	dc i4'TIsetsockopt-1' ; 51
	dc i4'TIshutdown-1' ; 52
	dc i4'TIsetreuid-1' ; 53
	dc i4'TIsetregid-1' ; 54

TheEnd	anop
	END

QDSTARTUPPATCH	START KERN2
	using KernelStruct

	php
	long  ai

	phb
	phk
	plb
	pha
	phx
	phy

	pha
	_QDStatus	; quickdraw is already started!
	pla
	bne	killProc

	lda	>curProcInd
	tax
	lda	>ttyID,x
	cmp	#3      	; no QDStartup from a terminal!
	bne	killProc

	lda   >flags,x
	ora   #64	; COMPLIANT, NORESTART, QDSTARTUP
	sta   >flags,x

	ply
	plx
	pla
	plb
	plp
	jmp	>DoOldQD

killProc	anop
	short m
	lda	#0
	sta	>$E100FF
	long	m

	ph4	#badBoy
	_WriteCString

	jmp	>endproc

badBoy	dc	c'QuickDraw is already started.'

	END

DoOldQD	START
OLDQDSTARTUP	ENTRY
	jmp	>$000000
	END

* The new Sane Startup only stores the program's chosen SANE DP space
* into the kernel process entry.  The context switcher must now
* copy the sanedp around.

SANESUPATCH	START
	using KernelStruct
sanedp	equ 7+1

	phb
	phk
	plb
	
	lda	sanedp,s
	ldx	curProcInd
	sta	SANEwap,x
	plb
OLDSANESU	ENTRY
	jmp	>$000000
	END

SANESDPATCH	START
	using KernelStruct

	phb
	phk
	plb
	
	lda	#0
	ldx	curProcInd
	sta	SANEwap,x
	plb
	phd
	jmp	bye0
	END

SANESTPATCH	START
	using KernelStruct
sanest	equ	7+1	
	
	phb
	phk
	plb
	ldx	curProcInd
	lda	SANEwap,x
	beq	goway
	lda	#$FFFF
goway	sta	sanest,s
	plb
	phd
	jmp	bye0
	END

OURSYSFAILMGR	START
errorCode	equ   7+3+4
strPtr	equ   7+3

	phb
	phk
	plb
	phd
	tsc
	tcd
	lda   strPtr
	ora   strPtr+2
	bne   yString
	ph4   #defaultStr
	bra   dostr
yString	anop
	pei	(strPtr+2)
	pei	(strPtr)
dostr	_ErrWriteString
	lda   errorCode
	ErrWriteChar #' '
	ErrWriteChar #'$'
	lda   errorCode
	jsr   writeacc
	lda   errorCode          ; we must halt the system
	cmp   #$0305
	beq   dopanic            ; on certain error codes, or
	cmp   #$0308
	beq   dopanic            ; the system will act really goofy
	cmp   #$0681
	beq   dopanic
	cmp   #$0682
	beq   dopanic
	cmp   #$08FF
	beq   dopanic
	ldx   #$0903
	jsl   $E10008
	pea   $0009              ; kill us!
	ldx   #$0A03
	jsl   $E10008            ; goodbye cruel world!
	bra	dopanic
	
defaultStr	str   'System Error ->'

dopanic	short m
	lda   >$E0C029           ; turn off SHR graphics
	and   #$7F
	sta   >$E0C029
	lda   #$1F               ; red on white, set text and bg
	sta   >$E0C022           ; color so error can always be seen
	lda	>$E100FF	; if busy flag is set, allow us to
	bne	dopanic1           ; enter GSBug to see what happened
	sei
dopanic1	bra   dopanic1
	END
	longa on
	longi on

NotImpMM	START KERN2
	sec
	rtl
	END

NULLTOOLFUNC	START KERN2
	clc
	rtl
	END

tVersion	START KERN2
	lda	#$0206
	sta	7,s
	lda	#0
	clc
	rtl
	END

tBootInit	START KERN2
	clc
	RTL
	END

tStartUp	START KERN2
	clc
	RTL
	END

tShutDown	START KERN2
	clc
	RTL
	END

tReset	START KERN2
	clc
	RTL
	END

tStatus	START KERN2
	lda #$8100
	sta 7,s
	lda #0
	clc
	rtl
	END

TIalarm	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNalarm
	END

TIalarm10	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNalarm10
	END

TIdup	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNdup  
	END

TIdup2	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNdup2 
	END

TIgetpid	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNgetpid
	END

TIgetppid	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNgetppid
	END

TIgetpgrp	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNgetpgrp
	END

TIsetpgrp	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNsetpgrp
	END
	
TIgetuid	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNgetuid
	END

TIgetgid	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNgetgid
	END

TIgeteuid	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNgeteuid
	END

TIgetegid	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNgetegid
	END

TIsetuid	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNsetuid
	END

TIsetreuid	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNsetreuid
	END

TIsetgid	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNsetgid
	END

TIsetregid	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNsetregid
	END

TIkill	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNkill
	END

TIfork	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNfork
	END

TIfork2	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNFORK2
	END

*TIexec         START KERN2
*               tsc
*               clc
*               adc #3
*               tcs
*               jmp >KERNexec
*               END

TIexecve	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNexecve
	END

TIswait	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNswait
	END

TIssignal	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNssignal
	END

TIscreate	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNscreate
	END

TIsdelete	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNsdelete
	END

TIkvm_open	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNkvm_open
	END

TIkvm_close	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNkvm_close
	END

TIkvm_getproc	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNkvmgetproc
	END

TIkvm_nextproc	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNkvmnextproc
	END

TIkvm_setproc	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNkvmsetproc
	END

TIpipe	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNpipe
	END

TIsignal	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNsignal
	END

TIsigpause	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNsigpause
	END

TIsetdebug	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNsetdebug
	END

TIsetsystemvector	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNsetsystemvector
	END

TIstat	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNstat
	END

TIfstat	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNfstat
	END

TIlstat	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNlstat
	END

TIwait	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNwait
	END

**********************************************************

TItcnewpgrp	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNtcnewpgrp
	END
TIsettpgrp	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNsettpgrp
	END

TItctpgrp	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNtctpgrp
	END

**********************************************************

TIsigsetmask	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNsigsetmask
	END
TIsigblock	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNsigblock
	END

**********************************************************

TIprocsend	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNsend
	END
TIprocreceive	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNreceive
	END
TIrecvclr	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNrecvclr
	END
TIrecvtim	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNrecvtim
	END

TItimes	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNtimes
	END

TIpcreate	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNPCREATE
	END

TIpsend	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNPSEND
	END

TIpreceive	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNPRECEIVE
	END

TIpdelete	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNPDELETE
	END

TIpreset	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNPRESET
	END

TIpbind	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNPBIND
	END

TIpgetport	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNPGETPORT
	END

TIpgetcount	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNPGETCOUNT
	END

TIscount	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNscount
	END

TISetGNOQuitRec	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNSETGNOQUITREC
	END

TIInstallNetDriver	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNINSTALLNETDRIVER
	END
TIsocket	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNSOCKET
	END
TIbind	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNBIND
	END
TIconnect	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNCONNECT
	END
TIlisten	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNLISTEN
	END
TIaccept	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNACCEPT
	END
TIrecvfrom	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNRECVFROM
	END
TIsendto	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNSENDTO
	END
TIrecv	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNRECV
	END
TIsend	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNSEND
	END
TIgetpeername	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNGETPEERNAME
	END
TIgetsockname	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNGETSOCKNAME
	END
TIgetsockopt	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNGETSOCKOPT
	END
TIsetsockopt	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNSETSOCKOPT
	END
TIshutdown	START KERN2
	tsc
	clc
	adc #3
	tcs
	jmp >KERNSHUTDOWN
	END
	
**********************************************************

addsig	START
	using KernelStruct
space	equ   0

	subroutine (2:s,2:p),space
	lda   p
	asl2  a
	asl2  a
	asl2  a
	asl   a
	tax
	lda   >flpid,x
	ldx   s
	jsr   queueSignal
	return
	END

execveHook	START
	using KernelStruct
	using ctxtstuff
	phk                      ; that this ever worked is amazing
	plb                      ; 'sys' must usually be in another bank
	lda   ctx_S1
	tcs
	short a
	lda   ctx_PC+2
	pha
	long  a
	lda   ctx_PC
	dec   a
	pha                      ; rtl to the fledgling process
	lda   ctx_D
	tcd
	lda   ctx_B
	pha
	ldx   ctx_X
	ldy   ctx_Y
	lda   ctx_A
	jsl   decBusy            ; heh heh, ha!
	plb
	plb
	rtl
	END

ctxtRestore	START
	using KernelStruct
	using ctxtstuff
	phk
	plb

	jsl cSignalHook          ; unblock this signal
	jsl incBusy
	lda   >curProcInd
	tax
	pla
	sta   >waitdone,x        ; restore 'wait/block' status
	pla
	sta ctx_A
	pla
	sta ctx_X
	pla
	sta ctx_Y
	pla
	sta ctx_state           ;S is implicit; use for process state
	pla
	sta ctx_D
	pla
	sta ctx_B
	pla
	sta ctx_P
	pla
	pla
	sta ctx_PC
	pla
	sta ctx_K

; now restore the registers and jml to the old PC, dude
;              short m
;              lda >ctx_K
;              pha
;              long m
;              lda >ctx_PC              ; we'll rtl back to the routine
;              dec a
;              pha

	sei
	lda >curProcInd
	tax
	lda >ctx_state           ; restore the old process state
	sta >ProcessState,x
	lda >ctx_Y
	tay
	lda >ctx_X
	tax
	lda >ctx_D
	tcd
	short	a
	lda >ctx_B
	pha
	plb
	lda >ctx_P
	pha
	long	a
	lda >ctx_A
	jsl   decBusy
;               plp
;               xce
	plp
; note that this rti isn't the one we created, it's the one shoved on
; the stack by an actual VBL or COP instruction.  So we're pulling some major
; dick with this code.

	cop $7f                  ; in case process state changes
	rti                      ; rti back to the routine
	END

ctxtstuff	DATA
ctx_A	dc i2'0'
ctx_X	dc i2'0'
ctx_Y	dc i2'0'
ctx_state	dc i2'0'
ctx_D	dc i2'0'
ctx_B	dc i1'0'
ctx_B1	dc i1'0'
ctx_P	dc i2'0'
ctx_S1	dc i2'0'                 ; argh- obsolete
ctx_PC	dc i2'0'
ctx_K	dc i2'0'
	END

tool_exit	START
	phy
	tsc
	clc
	adc 1,s
	tay
	tsc
	phx
	tax
	phb
	pea $0000
	plb
	plb
	lda |7,x
	sta |7,y
	lda |5,x
	sta |5,y
	lda |3,x
	sta |3,y
	plb
	plx
	tsc
	clc
	adc 1,s
	adc #2
	tcs
	txa
	cmp #1
	rtl
	END

exits	START
bye4	ENTRY
	ldx #0
	pld
	ldy #4
	jmp tool_exit
bye6	ENTRY
	ldx #0
	pld
	ldy #6
	jmp tool_exit
bye0	ENTRY
	ldx #0
	pld
	ldy #0
	jmp tool_exit
bye2	ENTRY
	ldx #0
	pld
	ldy #2
	jmp tool_exit
bye8	ENTRY
	ldx #0
	pld
	ldy #8
	jmp tool_exit
bye10	ENTRY
	ldx #0
	pld
	ldy #10
	jmp tool_exit
bye14	ENTRY
	ldx #0
	pld
	ldy #14
	jmp tool_exit
	END

enableBuf	START
	using InOutData
	php
	phy
	phx
	pha
	lda   >bufState
	beq   nodec
	dec   a
	sta   >bufState
	cmp   #0
	bne   nodec
	IntSource #0             ; make sure Kbd ints are enabled
	short	mx
	sta	>$E0C001	; re-enable 80-STORE
	long	mx
	ph4	#cbuf
	pea	$011A
	jsl	$E100B0
nodec	anop
	pla
	plx
	ply
	plp
	rtl
cbuf	dc	i2'5'
	END

disableBuf	START
	using InOutData

	php
	phy
	phx
	pha

	lda   >bufState
	inc   a
	sta   >bufState
	cmp   #1
	bne   goaway
	lda	#0
	sta	>OutStopped

	IntSource #1             ; turn off our kbd "interrupt"
	ph4	#cbuf
	pea	$011A
	jsl	$E100B0
goaway	anop
	pla
	plx
	ply
	plp
	rtl
cbuf	dc	i2'6'
	END

