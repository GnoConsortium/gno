**************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*
* $Id: shell.asm,v 1.7 1998/10/26 17:04:51 tribby Exp $
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
* Dogshrc	jsr with no parameters
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

cmdbuflen	gequ	1024

**************************************************************************
*
* shell: entry point for acting upon commands
*
**************************************************************************
	case	on
shell	start
	case	off

	using global
	using	pdata
	using	HistoryData
	using	termdata
	using hashdata
               
p	equ	0	General pointer
cflag	equ	p+4	Flag: set when path converted
space	equ	cflag+2

	subroutine (0:dummy),space

	tsc		Save stack pointer
	sta	cmdcontext	 in cmdcontext
	tdc		  and direct page reg
	sta	cmddp	   in cmddp.

	PushVariablesGS NullPB	Save environment variables.

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
	lda	gshpid	Only print the copyright msg
	cmp	#2	 if not using login
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
;
; Check for login shell (argv[0] starts with '-')
;
	mv4	~COMMANDLINE,p	Copy commandline addr to dir page.
	ldy	#8	Skip over shell identifier
	lda	[p],y	 to get first char of
	and	#$FF	  command name.
	cmp	#'-'	If not '-',
	bne	nologin_init	 skip over login initialization.

; Change ":" to " " in $PATH if it doesn't start with ":" or contain a " "

	ph4	#pathname	Get $PATH environment variable string
	jsl	getenv
	sta	p	Save address of allocated buffer.
	stx	p+2	 in direct page variable
	ora	p+2	If null,
	beq	nopathconv	 no need to convert

	stz	cflag	Initialize conversion flag = false.
	ldy	#4	Index over GS/OS length fields.
	short	m	Use 8-bit mode.
	lda	[p],y
	beq	endpathconv	If null string, or
	cmp	#':'	 if first character is ':',
	beq	endpathconv	   don't do the conversion.

bumpit	iny
	lda	[p],y
	beq	endpathconv	If at end of string, all done.
	cmp	#' '	If $PATH contains a space,
	bne	chkcolon
	stz	cflag	 abort the conversion.
	bra	endpathconv
chkcolon	cmp	#':'	If next character is ':',
	bne	bumpit
	lda	#' '	  change it to ' ', and
	sta	[p],y
	sta	cflag	  set conversion flag = true.
	bra	bumpit

endpathconv	long	m	Back to 16-bit mode.
	lda	cflag	If there was no conversion,
	beq	freepath	 skip the setting of $PATH.

	clc		Address of $PATH as a GS/OS
	lda	p	 output string is 2 bytes
	adc	#2	  beyond the input string
	sta	SetValue	   starting address.
	lda	p+2
	adc	#0
	sta	SetValue+2
	SetGS	SetPB	Set $PATH to the converted value.

freepath	ph4	p	Free the $PATH C string.
	jsl	nullfree

nopathconv	anop

; Read and execute /etc/glogin
	ph4	#etcglogin	path = "/etc/glogin"
	lda	#0
	pha		argc = 0
	pha		argv = NULL
	pha
	pha		jobflag = 0
	jsl	ShellExec
	
; Read and execute $HOME/glogin
	jsr	Doglogin
	
;
; Initialization that is not specific to login shells
;
nologin_init	anop
	lda	FastFlag	If fast startup flag isn't set,
	bne	fastskip2
	jsr	InitHistory	 Init: historyFN->"$HOME/history",
	jsr	ReadHistory	  read in history from disk,
	jsr	Dogshrc	   and read $HOME/gshrc.
	jsr	newline
fastskip2	anop
	lda	didReadTerm
	bne	didit
	jsr	readterm
didit	anop
	jsl	hashpath	Hash $PATH.
	ld2	1,hash_print	Set hash print flag.

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
	ph2	CmdArgc	argc
	pei	(p+2)	argv
	pei	(p)
	pea	0	jobflag = 0
	jsl	ShellExec
	jmp	done1

cmdskip	lda	ExecFlag
	beq	execskip

;
; The -e flag is set: execute remaining arguments as a command and exit.
;
	ph4	ExecCmd	cmdline
	ph2	#0	jobflag = 0
	jsl	execute
	jmp	done1

;
; Parameter block for shell SetGS calls (p 427 in ORCA/M manual)
;
SetPB	dc	i2'3'	pCount
	dc	i4'pathname'	Name  (pointer to GS/OS string)
SetValue	ds	4	Value (pointer to GS/OS string)
	dc	i2'1'	Export flag

pathname	gsstr	'path'	GS/OS string


etcglogin	dc	c'/etc/glogin',h'00'

execskip	anop

****************************************************************
*
* Main loop for reading and executing commands
*
****************************************************************

	stz	lastabort

gnoloop	entry

;
; Set the fundamental registers.
;
	phk		Copy Program Bank register
	plb		 into Data Bank register.
	lda	cmdcontext	Set Stack Pointer and
	tcs		 Direct Page register
	lda	cmddp	  to values saved when
	tcd		   entering shell.

	jsl	WritePrompt	Print prompt.
	jsr	GetCmdLine	Get response.
	bcs	done
	jsr	newline

	lda	cmdlen	Check for empty string.
	beq	gnoloop
	jsr	cursoron	
	jsr	newlineX
	jsr	flush

	ph4	#cmdline	execute(cmdline,0)
	ph2	#0	jobflag = 0
	jsl	execute

	lda	exit_requested
	bne	done1
	jsr	newlineX
	stz	lastabort
	bra	gnoloop

;
; shut down gsh
;
done	jsr	newline
	jsr	newlineX
done1	ora2	pjoblist,pjoblist+2,@a
	beq	done2
	lda	lastabort
	bne	donekiller
	inc	lastabort
	stz	exit_requested
	ldx	#^stopstr	Print message:
	lda	#stopstr	 "There are stopped jobs"
	jsr	puts
	jsr	newlineX
	bra	gnoloop	Continue getting commands.

donekiller	jsl	jobkiller
done2	lda	FastFlag
	bne	fastskip5
	jsl	SaveHistory
fastskip5	jsr	dispose_hash

quit	PopVariablesGS NullPB
	Quit	QuitParm

QuitParm	dc	i'0'

; Null parameter block used for shell calls PushVariables
; (ORCA/M manual p.420) and PopVariablesGS (p. 419)
NullPB	dc	i2'0'	pCount

gnostr	dc	h'0d',c'GNO/Shell 2.0.6',h'0d'
	dc	c'Copyright 1991-1993, Procyon, Inc. & Tim Meekins. '
	dc	c'ALL RIGHTS RESERVED',h'0d'
	dc	h'0d00'
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
; If $HOME is set, we presume the gshrc file is there.	If not,
; or if an error occurs getting the $HOME variable, we use
; @:gshrc.
;
;=========================================================================

Dogshrc	START

	ph4	#gshrcName
	bra	doit

; Alternate entry point: execute $HOME/glogin

Doglogin	ENTRY
	ph4	#gloginName

doit	jsl	AppendHome
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
no_undf	phx
	pha
	jsl	nullfree
	rts

gshrcName	dc	c'/gshrc',h'00'
gloginName	dc	c'/glogin',h'00'

	END


;=========================================================================
;
; Append a C string to the value of the $HOME variable.	 If $HOME is
; not set, then it appends the C string to the string '@/'.  Returns
; a pointer to a GS/OS input string.
;
;=========================================================================

AppendHome	START

outPtr	equ	0	Pointer into allocated memory
str_len	equ	outPtr+4	Length of string parameter
buf_len	equ	str_len+2	Size of GS/OS buffer
space	equ	buf_len+2

	subroutine (4:str),space

	lock	mutex
;
; Get the variable's length using ReadVariableGS
;
	ld4	TempResultBuf,RVresult	Use temporary result buf.
	ReadVariableGS ReadVar		Get length.

;
; Allocate memory for value string
;
	pei	(str+2)	Get length of
	pei	(str)	 string to be
	jsr	cstrlen	  appended.
	sta	str_len

	lda	TempRBlen	Get length of value.
	bne	notnull	If 0,
	inc	a	 increment because "@" will be used.
notnull	inc2	a	Add 4 bytes for result buf len words.
	inc2	a
	clc		Add length of string parameter.
	adc	str_len
	sta	buf_len	Save result buf length.
	inc	a	Add 1 more for terminating null byte.
	pea	0
	pha
	~NEW		Request the memory.
	sta	RVresult	Store address in ReadVariable
	stx	RVresult+2	 parameter block and
	sta	outPtr	  direct page pointer.
	stx	outPtr+2
	ora	outPtr+2	If address == NULL,
	beq	exit	 return NULL to user.

	lda	buf_len	Store result buffer length
	sta	[outPtr]	 at beginning of buf.
;
; Read the full value into the allocated memory
;
	ReadVariableGS ReadVar	ReadVariable $HOME
	bcs	doAtSign	If error, use @/

	ldy	#2	Get length of
	lda	[outPtr],y	 GS/OS string.
	beq	doAtSign	If $HOME not defined, use "@".
	clc		
	adc	#4	Turn into a c-string
	tay		 by storing a 0 byte
	short	m	  after the last $HOME
	lda	#0	   character.
	sta	[outPtr],y
	long	m
	bra	doAppend

;
; $HOME is null string or not defined. Use @
;
doAtSign	lda	atSign
	ldy	#4
	sta	[outPtr],y
	lda	#1	Set GS/OS buffer
	ldy	#2	 string length word
	sta	[outPtr],y	  to 1.

doAppend	anop
	ldy	#4	Start index beyond length words.
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

foundSep	sta	[str]	Store sep at start of appended string.
	long	m

	pei	(str+2)
	pei	(str)
	ldx	outPtr+2
	lda	outPtr
	clc
	adc	#4
	bcc	pushptr
	inx
pushptr	phx
	pha
	case	on
	jsl	strcat
	case	off

	clc		Add 2 bytes to address of
	lda	outPtr	 GS/OS output buffer to
	adc	#2	  get address of GS/OS
	bcc	no_ovf	   input string.
	inc	outPtr+2
no_ovf	sta	outPtr

	lda	[outPtr]	Adjust string length
	clc		 to include appended
	adc	str_len	  string (parameter).
	sta	[outPtr]
;
; NOTE: The returned value points to a GS/OS string, two bytes offset
;       from the allocated memory for a GS/OS result buffer. When the
;       memory is deallocated, the address must be adjusted back.
;
exit	unlock mutex
	return 4:outPtr

mutex	key

atSign	dc	c'@',i1'0'

;
; Parameter block for Shell call ReadVariable (p 423 in ORCA/M reference)
;
ReadVar	anop
	dc	i2'3'	pCount
	dc	a4'home'	address of variable's name
RVresult	ds	4	GS/OS Output buffer ptr
	ds	2	export flag (returned)
;
; GS/OS result buffer for getting the full length of the HOME env var.
;
TempResultBuf	dc	i2'5'	Only five bytes total.
TempRBlen	ds	2	Value's length returned here.
	ds	1	Only 1 byte for value.
;
; "HOME" as a GS/OS string
;
home	gsstr	'HOME'

	END


;=========================================================================
;
; GLOBAL data
;
;=========================================================================

global	DATA

ID	ds	2
GSOSDP	ds	2
cmdloc	ds	2
cmdlen	ds	2
cmdline	ds	cmdbuflen
buffer	ds	256
wordlen	ds	2
wordbuf	ds	256
nummatch	ds	2
matchbuf	ds	512*4
cmdcontext	ds	2
cmddp	ds	2
gshtty	ds	2
gshpid	ds	2
exit_requested	dc	i'0'	;!=0 if exit
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
