**************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*
* $Id: shell.asm,v 1.3 1998/06/30 17:25:52 tribby Exp $
*
**************************************************************************
*
* SHELL.ASM
*   By Tim Meekins
*   Modified by Dave Tribby for GNO 2.0.6
*
* This is the main routines for the shell.
*
* Note: text set up for tabs at col 16, 22, 41, 49, 57, 65
*              |     |                  |       |       |       |
*	^	^	^	^	^	^	
**************************************************************************
*
* Interfaces defined in this file:
*
* shell	subroutine (0:dummy)
*  NOTE: gnoloop is an entry defined in shell.
*              
* AppendHome	subroutine (4:str)
*	return 4:outPtr
*
* DoLogin	jsr with no parameters
*
* signal2	subroutine (4:fubar)
*
* signal18	subroutine (4:fubar)
*
**************************************************************************

	mcopy /obj/gno/bin/gsh/shell.mac

dummyshell	start		; ends up in .root
	end

	setcom 60

SIGINT	gequ	 2
SIGTSTP	gequ	18
SIGCHLD	gequ	20

cmdbuflen      gequ  1024

**************************************************************************
*
* shell: entry point for acting upon commands
*
**************************************************************************
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

	tsc		Save stack pointer
	sta	cmdcontext	 in cmdcontext
	tdc		  and direct page reg
	sta	cmddp	   ind cmddp.

*               PushVariables 0

	Open	ttyopen	Open tty,
	bcc	settty	 checking for error.
	ErrWriteCString #ttyerr
	jmp	quit

ttyerr	dc	c'gsh: Failed opening tty.',h'0d00'


settty	mv2	ttyref,gshtty
	tcnewpgrp gshtty
	settpgrp gshtty
	getpid
	sta	gshpid

	jsr	InitTerm

	lda	FastFlag	If FastFlag is set,
	bne	fastskip1	 skip copyright message.
               lda	gshpid	; only print the copyright msg
	cmp	#2	; if not using login
	bne	fastskip1
	ldx	#^gnostr
	lda	#gnostr
	jsr	puts
fastskip1	anop

;
; Set up signal handlers
;
	signal (#SIGINT,#signal2)
	signal (#SIGTSTP,#signal18)
	signal (#SIGCHLD,#pchild)
;
; Set entry point for users calling system
;
	setsystemvector #system

;
; Initialize some stuff
;
	jsr	initalias	Set all AliasTable entries to 0.
	jsr	InitDStack	Zero out directory stack.
	jsr	InitVars	Set value of all env var flags.
	lda	FastFlag	If fast startup flag isn't set,
	bne	fastskip2
	jsr	InitHistory	 Init: historyFN->"$HOME/history",
	jsr   ReadHistory	  read in history from disk,
               jsr   DoLogin               and read $HOME/gshrc.
	jsr	newline
fastskip2      anop
	lda	didReadTerm
	bne	didit
	jsr	readterm
didit	jsl   hashpath	;hash $path

;
; Check for command-line arguments -c and -e
;
	lda	CmdFlag
	beq	cmdskip

;
; The -c flag is set: execute remaining arguments as a command file and exit.
;
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

;
; The -e flag is set: execute remaining arguments as a command and exit.
;
	ph4	ExecCmd
	ph2	#0
	jsl	Execute
               jmp	done1

execskip       anop

****************************************************************
*
* Main loop for reading and executing commands
*
****************************************************************

	stz	lastabort

gnoloop        entry

;
; Set the fundamental registers.
;
	phk		Copy Program Bank register
	plb		 into Data Bank register.
               lda   cmdcontext	Set Stack Pointer and
               tcs		 Direct Page register
	lda	cmddp	  to values saved when
	tcd		   entering shell.

               jsl   WritePrompt	Print prompt.
               jsr   GetCmdLine	Get response.
               bcs   done
	jsr	newline

               lda   cmdlen	Check for empty string.
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
	ldx	#^stopstr	Print message:
	lda	#stopstr	 "There are stopped jobs"
	jsr	puts
	jsr	newlineX
	bra	gnoloop	Continue getting commands.

donekiller	jsl	jobkiller
done2	lda	FastFlag
	bne	fastskip5
	jsr   SaveHistory
fastskip5	jsr   dispose_hash

quit	PopVariablesGS NullPB
	Quit  QuitParm

QuitParm       dc    i'0'

; Null parameter block used for shell calls PushVariables
; (ORCA/M manual p.420) and PopVariablesGS (p. 419)
NullPB	dc	i2'0'	pCount

gnostr         dc    h'0d',c'GNO/Shell 2.0.6',h'0d'
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

DoLogin        START

	ph4	#gshrcName
               jsl   AppendHome
	phx		Save pointer to GS/OS input
	pha		 string for later.

	clc		Adjust the pointer
	adc	#2	 to skip the length word
	bcc	no_ovf	  so it's a C string.
	inx
no_ovf	phx
	pha

*   ShellExec	subroutine (4:path,2:argc,4:argv,2:jobflag)
;			(ptr to $HOME/gshrc is on stack)
	lda	#0
	pha		argc = 0
	pha		argv = NULL
	pha
	pha		jobflag = 0
	jsl	ShellExec

; Dispose $HOME/gshrc string
	pla		Get address of
	plx		 GS/OS input string.
	sec		Subtract two bytes to get
	sbc	#2	 addr of original output buffer.
	bcs	no_undf
	dex
no_undf	jsl	free256
	rts

gshrcName	dc	c'/gshrc',h'00'

               END


* Append a C string to the value of the $HOME variable.  If $HOME is
* not set, then it appends the C string to the string '@/'.  Returns
* a pointer to a GC string.

AppendHome	START
outPtr	equ	0
len	equ	4
	subroutine (4:str),6

	jsl	alloc256	Allocate memory for
               stx	outPtr1+2	 GS/OS output buffer
	sta	outPtr1	  that will hold the
               stx	outPtr+2	   value of $HOME and
	sta	outPtr	    the final result.

	pei	(str+2)	
	pei	(str)
	jsr	cstrlen
	sta	len

               lda	#255	Max len is 255 (leave room
	sta	[outPtr]	 for C string terminator).

	ReadVariableGS rvbl	ReadVariable $HOME
               bcs	doAtSign	If error, use @/

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

;
; $HOME is null string or not defined. Use @
;
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

noSep	lda	#':'	No separator found; use ":".

foundSep	sta	[str]	Store separator at end of string.
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

	clc		Add 2 bytes to address of
	lda	outPtr	 GS/OS output buffer to
	adc	#2	  get address if GS/OS
	bcc	no_ovf	   input string.
	inc	outPtr+2
	clc
no_ovf	sta	outPtr

               lda	[outPtr]	Adjust string length
	clc		 to include appended
	adc	len	  string (parameter).
	sta	[outPtr]

	return 4:outPtr

atSign	dc	c'@',i1'0'

; Parameter block for Shell call ReadVariable (p 423 in ORCA/M reference)
rvbl           dc	i2'3'	pCount
	dc	a4'in'	address of variable's name
outPtr1	dc	a4'0'	pointer to result buffer
	dc	i2'0'	value of 'Export' flag (returned)

in	dosin	'HOME'

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
