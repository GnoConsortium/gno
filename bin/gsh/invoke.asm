*************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*
* $Id: invoke.asm,v 1.10 1998/12/31 18:29:13 tribby Exp $
*
**************************************************************************
*
* INVOKE.ASM
*   By Tim Meekins
*   Modified by Dave Tribby for GNO 2.0.6
*
* Command invocation routines.
*
* Note: text set up for tabs at col 16, 22, 41, 49, 57, 65
*              |     |                  |       |       |       |
*	^	^	^	^	^	^	
**************************************************************************
*
* Interfaces defined in this file:
*
*  redirect	subroutine (4:sfile,4:dfile,4:efile,2:app,2:eapp,2:pipein,
*		2:pipeout,2:pipein2,2:pipeout2)
*	returns with carry set/clear to indicate failure/success
*
*  invoke	subroutine (2:argc,4:argv,4:sfile,4:dfile,4:efile,2:app,
*		2:eapp,2:bg,4:cline,2:jobflag,2:pipein,2:pipeout,
*		2:pipein2,2:pipeout2,4:pipesem,4:awaitstatus)
*	return 2:rtnval
*
**************************************************************************

	mcopy /obj/gno/bin/gsh/invoke.mac

dummyinvoke	start		; ends up in .root
	end

	setcom 60

SIGINT	gequ	 2
SIGKILL	gequ	 9
SIGTERM	gequ	15
SIGSTOP	gequ	17
SIGTSTP	gequ	18
SIGCONT	gequ	19
SIGCHLD	gequ	20
SIGTTIN	gequ	21
SIGTTOU	gequ	22

**************************************************************************
*
* Redirect
*
**************************************************************************

redirect	START

space	equ	1
pipeout2	equ	space+3
pipein2	equ	pipeout2+2
pipeout	equ	pipein2+2
pipein	equ	pipeout+2
eapp	equ	pipein+2
app	equ	eapp+2
efile	equ	app+2
dfile	equ	efile+4
sfile	equ	dfile+4
end	equ	sfile+4

;	subroutine (4:sfile,4:dfile,4:efile,2:app,2:eapp,2:pipein,2:pipeout,2:pipein2,2:pipeout2),space

* NOTE: only called from invoke, after fork_mutex is locked

	tsc
	phd
	tcd

;
; Redirect standard input
;
	ora2	sfile,sfile+2,@a	If no name provided,
	beq	execa	 skip it.
	pei	(sfile+2)	Convert c-string
	pei	(sfile)	 filename to
	jsr	c2gsstr	  GS/OS string.
	sta	RedirectFile	Store filename pointer
	stx	RedirectFile+2	 in parameter block.
	stz	RedirectDev	stdin devnum = 0.
	stz	RedirectApp	Cannot append.
	RedirectGS RedirectParm
	php
	ph4	RedirectFile	Free allocated GS/OS string.
	jsl	nullfree
	plp
	bcc	execa	If RedirectGS failed,
	ldx	#^err1	 print error message:
	lda	#err1	  'Error redirecting standard input.'
	jmp	badbye	   and quit.
;
; Redirect standard output
;
execa	ora2	dfile,dfile+2,@a
	beq	execb
	pei	(dfile+2)
	pei	(dfile)
	jsr	c2gsstr
	sta	RedirectFile
	stx	RedirectFile+2
	ld2	1,RedirectDev	stdout devnum = 1
	mv2	app,RedirectApp
	RedirectGS RedirectParm
	php
	ph4	RedirectFile
	jsl	nullfree
	plp
	bcc	execb
	ldx	#^err2	Print error message:
	lda	#err2	 'Error redirecting standard output.'
	jmp	badbye
;	     
; Redirect standard error
;
execb	ora2	efile,efile+2,@a
	beq	execc
	pei	(efile+2)
	pei	(efile)
	jsr	c2gsstr
	sta	RedirectFile
	stx	RedirectFile+2
	ld2	2,RedirectDev
	mv2	eapp,RedirectApp
	RedirectGS RedirectParm
	php
	ph4	RedirectFile
	jsl	nullfree
	plp
	bcc	execc
	ldx	#^err3	Print error message:
	lda	#err3	 'Error redirecting standard error.'
	jmp	badbye
;		     
; Is input piped in?
;
execc	lda	pipein
	beq	execd
	dup2	(pipein,#1)
	mv2	pipein2,CloseRef
	Close CloseParm
	ldx	#0
	lda	pipein
	SetInputDevice (#3,@xa)
;
; Is output piped?
;
execd	lda	pipeout
	beq	exece
	dup2	(pipeout,#2)
	mv2	pipeout2,CloseRef
	Close CloseParm
	ldx	#0
	lda	pipeout
	SetOutputDevice (#3,@xa)
exece	anop

;
; All the file and pipe redirection has been handled. Time to say goodbye.
;
goodbye	ldy	#0
	bra	exit

badbye	jsr	errputs
	cop	$7F	; get out of the way
	ldy	#1

exit	lda	space
	sta	end-3
	lda	space+1
	sta	end-2
	pld
	tsc
	clc
	adc	#end-4
	tcs

	cpy	#1	Clear/set carry for success/failure.

	rtl	  

;
; Parameter block for shell call to redirect I/O (ORCA/M manual p.425)
;
RedirectParm	dc	i2'3'	pCount
RedirectDev	ds	2	Dev num (0=stdin,1=stdout,2=errout)
RedirectApp	ds	2	Append flag (0=delete)
RedirectFile	ds	4	File name (GS/OS input string)

;
; Parameter block for GS/OS call to close a file
;
CloseParm	dc	i'1'	pCount
CloseRef	dc	i'0'	refNum

err1	dc	c'gsh: Error redirecting standard input.',h'0d00'
err2	dc	c'gsh: Error redirecting standard output.',h'0d00'
err3	dc	c'gsh: Error redirecting standard error.',h'0d00'
							 
	END	          

**************************************************************************
*
* Invoke a command (PHASE 0)
*
**************************************************************************

invoke	START

	using	hashdata
	using	vardata
	using	global
	using	pdata

p	equ	0
biflag	equ	p+4
ptr	equ	biflag+2
rtnval	equ	ptr+4	Return pid, -1 (error), or 0 (no fork)
cpath	equ	rtnval+2
hpath	equ	cpath+4
space	equ	hpath+4

 subroutine (2:argc,4:argv,4:sfile,4:dfile,4:efile,2:app,2:eapp,2:bg,4:cline,2:jobflag,2:pipein,2:pipeout,2:pipein2,2:pipeout2,4:pipesem,4:awaitstatus),space

	ld2	-1,rtnval
	stz	biflag	Clear built-in flag
	stz	hpath	 and address from hash table.
	stz	hpath+2

	lda	argc	If number of arguments == 0,
	bne	chknull	 nothing to do. (Shouldn't happen
nulldone	jmp	done	  because invoke checks argc==0).

;
; Check for null command
;
chknull	ldy	#2	Move 1st argument
	lda	[argv]	 pointer to
	sta	cpath	  cpath (4 bytes).
	lda	[argv],y
	sta	cpath+2	If pointer == NULL
	ora	cpath
	beq	nulldone	  all done.
	lda	[cpath]	If first character == '\0',
	and	#$FF
	beq	nulldone	  all done.

;
; Check command: is it builtin, in hash table, etc?
;
	pei	(cpath+2)	IsBuiltin returns
	pei	(cpath)	 0 if forked built-in
	jsl	IsBuiltin	 1 if non-forked built-in
	cmp	#-1	 -1 if not a built-in
	jne	trybuiltin

;
; Command is not listed in the built-in table. See if it was hashed
;
	pei	(cpath+2)
	pei	(cpath)
	ph4	hash_table
	ph4	#hash_paths
	jsl	search
	cmp	#0
	bne	changeit
	cpx	#0
	beq	noentry

changeit	sta	cpath	Use full path from
	stx	cpath+2	 hash table.
	sta	hpath	Save adddress for deallocation.
	stx	hpath+2

;
; Get information about the command's filename
;
noentry	lock	info_mutex

	pei	(cpath+2)
	pei	(cpath)
	jsr	c2gsstr
	sta	GRecPath
	sta	ptr
	stx	GRecPath+2
	stx	ptr+2
	GetFileInfo GRec

	unlock info_mutex
	jcs	notfound	If error getting info, print error.

; Is file type $B5: GS/OS Shell application (EXE)?
	if2	GRecFileType,eq,#$B5,doExec

; Is file type $B3: GS/OS application (S16)?
	if2	@a,eq,#$B3,doExec

	ldx	vardirexec	If $NODIREXEC isn't set, and
	bne	ft02
	cmp	#$0F
	jeq	doDir	 file is a directory: change to it.

; Is file type $B0: source code file (SRC)?
ft02	if2	@a,ne,#$B0,badfile
; Type $B0, Aux $00000006 is a shell command file (EXEC)
	if2	GRecAuxType,ne,#6,badfile
	if2	GRecAuxType+2,ne,#0,badfile
	jmp	doShell

;
; Command file is not an appropriate type
;
badfile	ldx	cpath+2
	lda	cpath
	jsr	errputs
	ldx	#^err1	Print error message:
	lda	#err1	 'Not executable.'
	jsr	errputs
free	pei	(ptr+2)	Free memory used to hold
	pei	(ptr)	 GS/OS string with path.
	jsl	nullfree
	jmp	done

*
* ---------------------------------------------------------------
*
* Launch an executable (EXE or S16 file)

doExec	pei	(ptr+2)
	pei	(ptr)
	jsl	nullfree
	jsr	prefork
	fork	#invoke0
	jsr	postfork
	jmp	done

;
; Forked shell starts here...
;
invoke0	phk		Make sure data bank register
	plb		 is the same as program bank.
;
; Make copies of command line (cline) and path (cpath) for child
;
	pha
	pha
	tsc
	phd
	tcd
	ldx	#0
	tsc	
	FindHandle @xa,1
	ldy	#6
	lda	[1],y	;This is the UserID!
	and	#$F0FF
	pha
	ph4	_cline
	jsr	cstrlen
	inc	a
	ply
	pha
	ldx	#0
	NewHandle (@xa,@y,#$4018,#0),1
	ply
	ldx	#0
	PtrToHand (_cline,1,@xy)
	ldy	#2
	lda	[1],y
	tax
	lda	[1]
	pld
	ply
	ply
	phx		;_cline
	pha
;
	pha
	pha
	tsc
	phd
	tcd
	ldx	#0
	tsc	
	FindHandle @xa,1
	ldy	#6
	lda	[1],y	;This is the UserID!
	and	#$F0FF
	pha
	ph4	_cpath
	jsr	cstrlen
	inc	a
	ply
	pha
	ldx	#0
	NewHandle (@xa,@y,#$4018,#0),1
	ply
	ldx	#0
	PtrToHand (_cpath,1,@xy)
	ldy	#2
	lda	[1],y
	tax
	lda	[1]
	pld
	ply
	ply
	phx		;_cpath
	pha

	jsl	infork
	bcs	invoke1
;
; Call _execve(_cpath,_cline) to replace forked shell with executable file
;
	case	on
	jsl	_execve	For 2.0.6: call _execve, not execve
	case	off
	rtl
;
; Error reported by infork; clean up stack and return to caller
;
invoke1	pla
	pla
	pla
	pla
	rtl

*
* ---------------------------------------------------------------
*
* Next command is a directory name, so change to that directory

doDir	lock	cdmutex
	mv4	GRecPath,PRecPath
	SetPrefix PRec
	unlock cdmutex
	pei	(ptr+2)	Free memory used to hold
	pei	(ptr)	 GS/OS string with path.
	jsl	nullfree
	lda	#0	Completion status = 0.
;
; Rest of cleanup is shared with non-forked builtin
;
	jmp	nfcleanup

*
* ---------------------------------------------------------------
*
* Next command is a shell command file: fork a shell script

doShell	anop
	inc	biflag	;don't free argv...
	jsr	prefork

* int fork2(void *subr, int stack, int prio, char *name, word argc, ...)
	pea	0
	ldy	#2
	lda	[argv],y
	pha
	lda	[argv]
	pha
	pea	0
	pea	1024
	ph4	#exec0
	case	on
	jsl	fork2
	case	off

	jsr	postfork
	jmp	free

;
; Forked shell starts here...
;
exec0	anop
	ph4	_hpath	argfree parameters.
	ph2	_argc
	ph4	_argv

	ph4	_cpath	ShellExec parameters
	ph2	_argc
	ph4	_argv
	jsl	infork
	bcs	exec0c
	signal (#SIGCHLD,#0)
	PushVariablesGS NullPB
	pea	1	jobflag = 1
	jsl	ShellExec
	jsl	argfree
	PopVariablesGS NullPB
	rtl

;
; Error reported by infork; clean up stack and return to caller
;
exec0c	pla
	pla
	pla
	pla
	pla
	pla
	pla
	pla
	rtl

; Null parameter block used for shell calls PushVariables
; (ORCA/M manual p.420) and PopVariablesGS (p. 419)
NullPB	dc	i2'0'	pCount

*
* ---------------------------------------------------------------
*
* File name was found in the built-in table

trybuiltin	inc	biflag	It's a built-in. Which type?
	cmp	#1	Either fork or don't fork.
	beq	noforkbuiltin	

;
; It's a forked builtin
;
	jsr	prefork
	fork	#forkbuiltin
	jsr	postfork
	jmp	done
;
; Control transfers here for a forked built-in command
;
forkbuiltin	anop
	cop	$7F	Give palloc a chance
	ph2	_argc
	ph4	_argv
	jsl	infork
	bcs	fork0c
	jsl	builtin
	rtl

;
; Error reported by infork; clean up stack and return to caller
;
fork0c	pla
	pla
	pla
	rtl

* ---------------------------------------------------------------

;
; It's a non-forked builtin
;
noforkbuiltin	anop
	pei	(argc)
	pei	(argv+2)
	pei	(argv)
	jsl	builtin
	and	#$00FF	Make return status look like result of
	xba		 wait(): high-order byte = status.

nfcleanup	anop
	sta	[awaitstatus]	
	stz	rtnval	Return value (pid) = no fork done.
;
; There might be a process waiting on a pipe
;
	lda	[pipesem]
	sta	_semaphore
	bra	chkpipe

*
* ---------------------------------------------------------------
*
* Command was not found as built-in or as a file

notfound	pei	(ptr+2)
	pei	(ptr)
	jsl	nullfree
	ldy	#2
	lda	[argv],y
	tax	
	lda	[argv]
	jsr	errputs
	ldx	#^err2	Print error message:
	lda	#err2	 'Command not found.'
	jsr	errputs

chkpipe	lda	pipein
	beq	done

; Input being piped into a command that was not found.

	ssignal _semaphore
	sdelete _semaphore

	mv4	pjoblist,p
	ldy	#16	;p_jobid
	lda	[p],y	Get forked process's pid.
	_getpgrp @a	Get forked process's group number.
	eor	#$FFFF
	inc	a
	kill	(@a,#9)	Kill all processes in that group.
	sigpause #0


done	cop	$7F
	lda	biflag	If built-in flag is clear,
	bne	skipfrarg

	pei	(hpath+2)	Free arguments.
	pei	(hpath)
	pei	(argc)
	pei	(argv+2)
	pei	(argv)
	jsl	argfree

skipfrarg	pei	(cline+2)	Free command-line.
	pei	(cline)
	jsl	nullfree

	return 2:rtnval


* ---------------------------------------------------------------
*
* Support routines
*
* ---------------------------------------------------------------

;
; Tasks to do just before forking
;
prefork	lock	fork_mutex	Lock the fork mutual exclusion.
	SetInGlobals (#$FF,#00)
;
; Move essential parameters from stack to mutual-exclusion protected memory
;
	mv4	sfile,_sfile
	mv4	dfile,_dfile
	mv4	efile,_efile
	mv2	app,_app
	mv2	eapp,_eapp
	mv4	cline,_cline
	mv4	cpath,_cpath
	mv4	hpath,_hpath
	mv2	argc,_argc
	mv4	argv,_argv
	mv2	pipein,_pipein
	mv2	pipeout,_pipeout
	mv2	pipein2,_pipein2
	mv2	pipeout2,_pipeout2
	mv2	bg,_bg
	mv2	jobflag,_jobflag
	lda	[pipesem]
	sta	_semaphore

	lda	pipesem	Set address of
	sta	putsem+1	 semaphone in
	lda	pipesem+1	  LDA instruction.
	sta	putsem+2

	rts

* ---------------------------------------------------------------

;
; Tasks the parent process does right after forking
;
postfork	sta	rtnval	Save pid as return value.
	lda	pipein	If pipein != 0,
	beq	postfork2
	sta	CloseRef
	Close CloseParm		close pipein.

postfork2	lda	pipeout	If pipeout != 0,
	beq	postfork3
	sta	CloseRef
	Close CloseParm		close pipeout.

postfork3	lda	rtnval	If return value == -1,
	cmp	#-1
	bne	postfork4	
	ldx	#^deadstr	  Print error message:
	lda	#deadstr	   'Cannot fork (too many processes?)'
	jsr	errputs
	unlock fork_mutex	  Unlock the fork mutual exclusion.
	jmp	postfork6	  Return to caller.

postfork4	ldx	jobflag
	dex
	beq	postfork5	If jobflag == 1,
	pha		
	pei	(bg)
	pei	(cline+2)
	pei	(cline)
	lda	pipein	  if pipein == 0,
	bne	postfork4a
	jsl	palloc		palloc(0,cline)
	bra	postfork5	  else
postfork4a	jsl	pallocpipe		pallocpipe(0,cline)

postfork5	anop
;
; Wait for fork mutual exclusion lock to clear (cleared by infork)
;
	lda	>fork_mutex	;DANGER!!!!! Assumes knowledge of
	beq	postfork6	;lock/unlock structure!!!!!!!!
	cop	$7F
	bra	postfork5

postfork6	rts

* ---------------------------------------------------------------

;
; Startup tasks by forked process
;
infork	phk		Make sure data bank register
	plb		 is the same as program bank.
;
; NOTE: next two lines were added for v2.0.6 in order to prevent background
;       processes from being waited on when they are kicked off from an
;       exec file.  The side effect of having the process become owned by
;       the null process may not be desired. (Perhaps there is a better way!)
;
	lda	_bg	If in background
	bne	optty
	lda	_jobflag	 or jobflag == 0,
	bne	infork0b

optty	Open	ttyopen	Open tty.
	jcs	errinfork

	lda	_pipein
	bne	infork0a	If not in pipeline,
	tcnewpgrp ttyref	 allocate new process group.
infork0a	settpgrp ttyref	Set current process to have proc group.
	lda	_bg	If in background,
	beq	infork0b
	tctpgrp (gshtty,gshpid)	  reset tty to the shell process group.

infork0b	ph4	_sfile	Redirect I/O
	ph4	_dfile
	ph4	_efile
	ph2	_app
	ph2	_eapp
	ph2	_pipein
	ph2	_pipeout
	ph2	_pipein2
	ph2	_pipeout2
	jsl	redirect
	jcs	errinfork

	unlock fork_mutex

;
; Wait on appropriate pipe semaphores
;
	lda	_pipein
	bne	infork0c
	lda	_pipeout
	beq	clean_exit

;
; _pipein == 0  &&  _pipeout != 0
;
	screate #0	Create a semaphore with count=0.
putsem	sta	>$FFFFFF	Store semaphore number in pipesem.
	swait @a	Block on semaphore until ssignal.
	bra	clean_exit

infork0c	lda	_pipeout
	bne	clean_exit
;
; _pipein != 0  &&  _pipeout != 0
;
waitsemy	lda	_semaphore	While _semaphore == 0,
	bne	goodsemy
	cop	$7F		allow other processes to run.
	bra	waitsemy
goodsemy	ssignal _semaphore	Release _semaphore
	sdelete _semaphore	 and delete it.

clean_exit	anop
	clc
	bra	indone
;
; Arrive at "errinfork" if ttyopen or redirect didn't work.
;
errinfork	unlock fork_mutex
;	sec		Note: carry already set

indone	rtl		Return to caller; status in carry.
	   


fork_mutex	key		Mutual exclusion for forking
info_mutex	key		Mutual exclusion for GetFileInfo
cdmutex	key		Mutual exclusion for SetPrefix

;
; Variables protected by fork_mutex (set by parent, used by child process)
;
_argc	dc	i2'0'
_argv	dc	i4'0'
_sfile	dc	i4'0'
_dfile	dc	i4'0'
_efile	dc	i4'0'
_app	dc	i2'0'
_eapp	dc	i2'0'
_cline	dc	i4'0'
_cpath	dc	i4'0'
_hpath	dc	i4'0'
_pipein	dc	i2'0'
_pipeout	dc	i2'0'
_pipein2	dc	i2'0'
_pipeout2	dc	i2'0'
_bg	dc	i2'0'
_jobflag	dc	i2'0'
_semaphore	dc	i2'0'

;
; String constants
;
str	dc	c'[0]',h'0d00'
err1	dc	c': Not executable.',h'0d00'
err2	dc	c': Command not found.',h'0d00'
deadstr	dc	c'Cannot fork (too many processes?)',h'0d00' ;try a spoon


; Parameter block for GS/OS call GetFileInfo
GRec	dc	i'4'	pCount (# of parameters)
GRecPath	ds	4	pathname (input; ptr to GS/OS string)
	ds	2	access (access attributes)
GRecFileType	ds	2	fileType (file type attribute)
GRecAuxType	ds	4	auxType (auxiliary type attribute)


PRec	dc	i'2'
PRecNum	dc	i'0'
PRecPath	ds	4

CloseParm	dc	i2'1'
CloseRef	dc	i2'0'

Err	dc	i2'0'

; Parameter block for opening tty
ttyopen	dc	i2'2'
ttyref	dc	i2'0'
	dc	i4'ttyname'
ttyname	gsstr	'.tty'

	END
