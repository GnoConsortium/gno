**************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*
* $Id: shell.asm,v 1.2 1998/04/24 15:38:37 gdr-ftp Exp $
*
**************************************************************************
*
* SHELL.ASM
*   By Tim Meekins
*
* This is the main routines for the shell.
*
**************************************************************************

	mcopy /obj/gno/bin/gsh/shell.mac

dummy	start		; ends up in .root
	end

	setcom 60

SIGINT	gequ	 2
SIGTSTP	gequ	18
SIGCHLD	gequ	20

cmdbuflen      gequ  1024

               case  on
shell          start
               case  off

               using global
	using	pdata
	using	HistoryData
               using	termdata

p	equ	0
space	equ	p+4

	subroutine (0:dummy),space

               tsc
               sta   cmdcontext
	tdc
	sta	cmddp

*               PushVariables 0

	Open	ttyopen
	bcc	settty
	ErrWriteCString #ttyerr
	jmp	quit
ttyerr	dc	c'gsh: Failed opening tty.',h'0d00'

settty	mv2	ttyref,gshtty
	tcnewpgrp gshtty
	settpgrp gshtty
	getpid
	sta	gshpid

	jsr	InitTerm

	lda	FastFlag
	bne	fastskip1
               lda	gshpid	; only print the copyright msg
	cmp	#2	; if not using login
	bne	fastskip1
	ldx	#^gnostr
	lda	#gnostr
	jsr	puts
fastskip1	anop

	signal (#SIGINT,#signal2)
	signal (#SIGTSTP,#signal18)
	signal (#SIGCHLD,#pchild)
	setsystemvector #system
;
; Initialize some stuff
;
	lda	FastFlag
	bne	fastskip2
               jsr	InitHistory
	jsr   ReadHistory	;Read in history from disk
fastskip2	jsr	initalias	;initialize alias
	jsr	InitDStack	;initialize directory stack
	lda	FastFlag
	bne	fastskip3
               jsr   DoLogin            ;Read gshrc
	jsr	newline
fastskip3      anop
	lda	didReadTerm
	bne	didit
	jsr	readterm
didit	jsl   hashpath	;hash $path

	lda	CmdFlag
	beq	cmdskip

	mv4	CmdArgV,p
	ldy	#2
	lda	[p],y
	pha
	lda	[p]
	pha
	ph2	CmdArgc
	pei	(p+2)
	pei	(p)
	pea	0
	jsl	ShellExec
	jmp	done1

cmdskip	lda	ExecFlag
	beq	execskip

	ph4	ExecCmd
	ph2	#0
	jsl	Execute
               jmp	done1

execskip       anop

	stz	lastabort

gnoloop        entry

	phk
	plb

               lda   cmdcontext	;dare you to make a mistake
               tcs
	lda	cmddp
	tcd

               jsl   WritePrompt
               jsr   GetCmdLine
               bcs   done
	jsr	newline
               lda   cmdlen
               beq   gnoloop
	jsr	cursoron
	jsr	newlineX
	jsr	flush
               ph4   #cmdline
	ph2	#0
               jsl   execute
	lda	exitamundo
	bne	done1
               jsr   newlineX
	stz	lastabort
               bra   gnoloop
;
; shut down gsh
;
done           jsr	newline
	jsr	newlineX
done1	ora2  pjoblist,pjoblist+2,@a
	beq	done2
               lda	lastabort
	bne	donekiller
	inc	lastabort
	stz	exitamundo
	ldx	#^stopstr
	lda	#stopstr
	jsr	puts
	jsr	newlineX
	bra	gnoloop
donekiller	jsl	jobkiller
done2	lda	FastFlag
	bne	fastskip5
	jsr   SaveHistory
fastskip5	jsr   dispose_hash

quit	PopVariables 0
	Quit  QuitParm
QuitParm       dc    i'0'

gnostr         dc    h'0d',c'GNO/Shell 2.0.4',h'0d'
               dc    c'Copyright 1991-1993, Procyon, Inc. & Tim Meekins. '
               dc    c'ALL RIGHTS RESERVED',h'0d'
               dc    h'0d00'
stopstr	dc	c'gsh: There are stopped jobs.',h'0d00'

ttyopen	dc	i2'2'
ttyref	dc	i2'0'
	dc	i4'ttyname'
ttyname	gsstr	'.tty'

exitstr	dc	c'000000',h'0d00'

lastabort	ds	2

               END                       

;=========================================================================
;
; Interpret the login file (gshrc).
; If $HOME is set, we presume the gshrc file is there.  If not,
; or if an error occurs getting the $HOME variable, we use
; @:gshrc.
;
;=========================================================================


* Appends a C string to the value of the $HOME variable.  If $HOME is
* not set, then it appends the C string to the string '@/'.  Returns
* a pointer to a GC string.  Call DisposeHome/AX to deallocate the
* string.

DisposeHome	START
	dec	a
	dec	a
	jsl	free256
	rtl
	END

AppendHome	START
outPtr	equ	0
len	equ	4
	subroutine (4:str),6

	jsl	alloc256
               stx	outPtr1+2
	sta	outPtr1
               stx	outPtr+2
	sta	outPtr

	pei	(str+2)
	pei	(str)
	jsr	cstrlen
	sta	len

               lda	#256
	sta	[outPtr]

	GSOS 	$014B,rvbl	;ReadVariable
               bcs	doAtSign

	ldy	#2
	lda	[outPtr],y
	beq	doAtSign	; $HOME not defined?
      	clc
	adc	#4	; turn into a cstring
	tay
	short	m
	lda	#0
	sta	[outPtr],y
	long	m
	bra	doAppend

doAtSign	lda	atSign
               ldy	#4
	sta	[outPtr],y
	lda	#1
	ldy   #2
	sta	[outPtr],y

doAppend       anop
	ldy	#0
	short	m
lp	lda	[outPtr],y
               beq	noSep
	cmp	#':'
	beq	foundSep
	cmp	#'/'
	beq	foundSep
               iny
	bra	lp
noSep	lda	#':'

foundSep	sta	[str]
	long	m

	pei	(str+2)
	pei	(str)
	pei	(outPtr+2)
	lda	outPtr
	clc
	adc	#4
	pha
	case	on
	jsl	strcat
	case	off

	inc	outPtr
	inc	outPtr
               lda	[outPtr]
	clc
	adc	len
	sta	[outPtr]           ; adjust GS/OS string length
	return 4:outPtr

atSign	dc	c'@',i1'0'
rvbl           dc	i2'3'
	dc	a4'in'
outPtr1	dc	a4'0'
	dc	i2'0'	; value of 'Export' flag

in	dosin	'HOME'

	END

DoLogin        START

	ph4	#gshrcName
               jsl   AppendHome
	phx		; saved pointer for later
	pha
	phx
	inc	a
	inc	a	; adjust to the C string
	pha

	lda	#0
	pha
	pha
	pha
	pea	0
	jsl	shellexec

	pla
	plx
	jsl	DisposeHome
	rts

gshrcName	dc	c'/gshrc',h'00'

               END

;=========================================================================
;
; GLOBAL data
;
;=========================================================================

global         DATA

ID             ds    2
GSOSDP         ds    2
cmdloc         ds    2
cmdlen         ds    2
cmdline        ds    cmdbuflen
buffer         ds    256
wordlen	ds	2
wordpbuf	ds	1
wordbuf	ds	256
nummatch	ds	2
matchbuf	ds	512*4
cmdcontext     ds    2
cmddp	ds	2
gshtty	ds	2
gshpid         ds	2
exitamundo     dc    i'0'               ;!=0 if exit
signalled	dc	i'0'

FastFlag	dc	i'0'
CmdFlag	dc	i'0'
CmdArgV	ds	4
CmdArgC	ds	2
ExecFlag	dc	i'0'
ExecCmd	ds	4

               END

;=========================================================================
;
; SIGINT handler when typed at command-line
;
;=========================================================================

signal2	START

	using	global

	subroutine (4:fubar),0
	WriteCString #msg
	inc	signalled
;	ld2	$80,$E0C000
	return

msg	dc	c'^C',h'0d0a00'

	END

;=========================================================================
;
; SIGTSTP handler when typed at command-line
;
;=========================================================================

signal18	START

	using	global

	subroutine (4:fubar),0
	WriteCString #msg
	inc	signalled
;	ld2	$80,$E0C000
	return           

msg	dc	c'^Z',h'0d0a00'

	END
