*************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*
* $Id: invoke.asm,v 1.6 1998/08/03 17:30:21 tribby Exp $
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
*		2:eapp,2:bg,4:cmd,2:jobflag,2:pipein,2:pipeout,
*		2:pipein2,2:pipeout2,4:pipesem)
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

	tsc
	phd
	tcd
;
; Redirect standard input
;
	ora2	sfile,sfile+2,@a	If no name provided,
               beq   execa	 skip it.
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
	dup2  (pipein,#1)
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
RedirectDev    ds    2	Dev num (0=stdin,1=stdout,2=errout)
RedirectApp    ds    2	Append flag (0=delete)
RedirectFile   ds    4	File name (GS/OS input string)

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

invoke      	START

	using	hashdata
	using	vardata
	using	global
	using	pdata

p	equ	0
biflag	equ	p+4
ptr	equ	biflag+2
rtnval	equ	ptr+4	Return pid, -1 (error), or 0 (no fork)
dir	equ	rtnval+2
space	equ	dir+4 

 subroutine (2:argc,4:argv,4:sfile,4:dfile,4:efile,2:app,2:eapp,2:bg,4:cmd,2:jobflag,2:pipein,2:pipeout,2:pipein2,2:pipeout2,4:pipesem),space

	ld2	-1,rtnval
	stz	biflag	;not a built-in

               lda   argc	Get number of arguments.
               bne   chknull	If != 0 continue with processing.

	lda	sfile	If any of the file pointers
	ora	sfile+2	 are != NULL,
	ora	dfile
	ora	dfile+2
	ora	efile
	ora	efile+2
	beq	nulldone

	ldx	#^hehstr	print error message:
	lda	#hehstr 	' specify a command before redirecting.'
	jsr	errputs

nulldone     	jmp   done

;
; Check for null command
;
chknull        ldy   #2	Move command line
               lda   [argv]	 pointer to
               sta   dir	  dir (4 bytes).
               lda   [argv],y
               sta   dir+2	If pointer == NULL
               ora   dir
               beq   nulldone	  all done.
               lda   [dir]	If first character == '\0',
               and   #$FF
               beq   nulldone	  all done.

;
; check for file
;
checkfile	anop

	pei	(dir+2)
	pei	(dir)
	jsl	IsBuiltin	;check builtin first
	cmp	#-1
	jne	trybuiltin

; Command is not listed in the built-in table. See if it was hashed

               pei   (dir+2)
               pei   (dir)
               ph4   hash_table
               ph4   #hash_paths
               jsl   search
               cmp   #0
               bne   changeit
               cpx   #0
               beq   skip

changeit       sta   dir
               stx   dir+2

skip           lock	mutex2
	pei   (dir+2)
               pei   (dir)
	jsr	c2gsstr
               sta   GRecPath
               sta   ptr
               stx   GRecPath+2
               stx   ptr+2

               GetFileInfo GRec
	unlock mutex2
               jcs   notfound

; File type $B5 is a GS/OS Shell application (EXE)
               if2   GRecFileType,eq,#$B5,doExec

; File type $B3 is a GS/OS application (S16)
               if2   @a,eq,#$B3,doExec

	ldx	vardirexec
	bne	ft02
	cmp	#$0F
	jeq	doDir	Target is a directory; change to it.

; File type $B0 is a source code file (SRC)
ft02           if2   @a,ne,#$B0,badfile
; Type $B0, Aux $00000006 is a shell command file (EXEC)
               if2   GRecAuxType,ne,#6,badfile
               if2   GRecAuxType+2,ne,#0,badfile
               jmp   doShell


badfile        ldx	dir+2
	lda	dir
	jsr	errputs
	ldx	#^err1	Print error message:
	lda	#err1	 'Not executable.'
	jsr	errputs
free	pei	(ptr+2)
	pei	(ptr)
	jsl	nullfree
               jmp   done

;
; launch an executable
;
doExec	pei   (ptr+2)
               pei   (ptr)
               jsl   nullfree
	jsr	prefork
	fork 	#invoke0
               jsr	postfork
	jmp	done


invoke0	phk
	plb
;
; make a copy of cmd
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
	ph4	_cmd
	jsr	cstrlen
	inc	a
	ply
	pha
	ldx	#0
	NewHandle (@xa,@y,#$4018,#0),1
	ply
	ldx	#0
	PtrToHand (_cmd,1,@xy)
	ldy	#2
	lda	[1],y
	tax
	lda	[1]
	pld
	ply
	ply
	phx		;_cmd
	pha
;
; make a copy of dir
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
	ph4	_dir
	jsr	cstrlen
	inc	a
	ply
	pha
	ldx	#0
	NewHandle (@xa,@y,#$4018,#0),1
	ply
	ldx	#0
	PtrToHand (_dir,1,@xy)
	ldy	#2
	lda	[1],y
	tax
	lda	[1]
	pld
	ply
	ply
	phx		;_dir
	pha

	jsl	infork
	bcs	invoke1
               case  on
               jsl   _execve	For 2.0.6: call _execve, not execve
               case  off
	rtl
invoke1	pla
	pla
	pla
	pla
	rtl

;
; Next command is a directory name, so change to that directory
;
doDir          lock	cdmutex
	mv4   GRecPath,PRecPath
               SetPrefix PRec
	unlock cdmutex
	stz	rtnval	Return value: no fork done.
	jmp	free

;
; Next command is a shell command file: fork a shell script
;
doShell        inc	biflag	;don't free argv...
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

*	fork	#exec0
               jsr	postfork
	jmp	free

exec0	ph2	_argc	;for argfree
	ph4	_argv

	ph4	_dir	;for shellexec
	ph2	_argc
	ph4	_argv
	jsl	infork
	bcs	exec0c
	signal (#SIGCHLD,#0)
               PushVariablesGS NullPB
	pea	1
               jsl   ShellExec
	jsl	argfree
               PopVariablesGS NullPB
	rtl

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
	cmp	#1                 Either fork or don't fork.
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
forkbuiltin	cop	$7F	Give palloc a chance

	ph2	_argc
	ph4	_argv
	jsl	infork
	bcs	fork0c
               jsl   builtin
	rtl

; Error reported by infork; clean up stack and return to caller
fork0c	pla
	pla
	pla
	rtl


;
; It's a non-forked builtin
;
noforkbuiltin	anop
	pei	(argc)
	pei	(argv+2)
	pei	(argv)
	jsl	builtin
	stz	rtnval	Return value: no fork done.
	bra	done

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

	lda	pipein
	beq	notfound0

	ssignal _semaphore
	sdelete _semaphore
	mv4	pjoblist,p
	ldy	#16	;p_jobid
	lda	[p],y
	getpgrp @a
	eor	#$FFFF
	inc	a
	kill	(@a,#9)
	sigpause #0
notfound0	anop


done	cop	$7F
	lda	biflag
	bne	skipfrarg

	pei	(argc)
	pei	(argv+2)
	pei	(argv)
	jsl	argfree

skipfrarg	pei	(cmd+2)
	pei	(cmd)
	jsl	nullfree

	return 2:rtnval
;
;
; stuff to do just before forking
;
prefork	lock	mutex	
	SetInGlobals (#$FF,#00)
	mv4	sfile,_sfile
	mv4	dfile,_dfile
	mv4	efile,_efile
	mv2	app,_app
	mv2	eapp,_eapp
	mv4	cmd,_cmd
	mv4	dir,_dir
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
	lda	pipesem
	sta	putsem+1
	lda	pipesem+1
	sta	putsem+2
	rts

;
; stuff to do right after forking
;
postfork	sta	rtnval
	lda	pipein
	beq	postfork2
               sta	CloseRef
	Close CloseParm
postfork2	lda	pipeout
	beq	postfork3
               sta	CloseRef
	Close CloseParm
postfork3	lda	rtnval
	cmp	#-1
	bne	postfork4	
	ldx	#^deadstr	Print error message:
	lda	#deadstr	 'Cannot fork (too many processes?)'
	jsr	errputs
	unlock mutex
	jmp	done

postfork4	ldx	jobflag
	dex
	beq	postfork5
	pha
	pei	(bg)
	pei	(cmd+2)
	pei	(cmd)
	lda	pipein
	bne	postfork4a
	jsl	palloc
	bra	postfork5
postfork4a	jsl	pallocpipe
postfork5	lda   >mutex	;DANGER!!!!! Assumes knowledge of
               beq   postfork6	;lock/unlock structure!!!!!!!!
	cop	$7F
               bra   postfork5
postfork6	rts

;
; stuff to do in fork
;
infork	phk
	plb

	lda	_jobflag
	bne	invoke0b

	Open	ttyopen
	jcs	doneinfork

	lda	_pipein
	bne	invoke0a
	tcnewpgrp ttyref
invoke0a	settpgrp ttyref
               lda	_bg	;if in background then reset tty to
	and	#$FF
	beq	invoke0b   	;to the shell process group
	tctpgrp (gshtty,gshpid)

invoke0b	ph4	_sfile
	ph4	_dfile
	ph4	_efile
	ph2	_app
	ph2	_eapp
	ph2	_pipein
	ph2	_pipeout
	ph2	_pipein2
	ph2	_pipeout2
	jsl	redirect
	jcs	doneinfork

	unlock mutex

	lda	_pipein
	bne	invoke0c
	lda	_pipeout
	beq	invoke0d
               screate #0
putsem	sta	>$FFFFFF
	swait @a
	bra	invoke0d
invoke0c	lda	_pipeout
	bne	invoke0d
waitsemy	lda	_semaphore
	bne	goodsemy
	cop	$7F
	bra	waitsemy
goodsemy	ssignal _semaphore
	sdelete _semaphore

invoke0d	anop

	clc
	bra	indone

doneinfork	unlock mutex
	sec
indone	rtl
                  
mutex	key
mutex2	key
cdmutex	key

_argc	dc	i2'0'
_argv	dc	i4'0'
_sfile	dc	i4'0'
_dfile	dc	i4'0'
_efile	dc	i4'0'
_app	dc	i2'0'
_eapp	dc	i2'0'
_cmd	dc	i4'0'
_dir	dc	i4'0'
_pipein	dc	i2'0'
_pipeout	dc	i2'0'
_pipein2	dc	i2'0'
_pipeout2	dc	i2'0'
_bg	dc	i2'0'
_jobflag	dc	i2'0'
_semaphore	dc	i2'0'

str            dc    c'[0]',h'0d00'
err1	dc	c': Not executable.',h'0d00'
err2	dc	c': Command not found.',h'0d00'
hehstr	dc	c'heh heh, next time you''ll need to specify a command '
	dc	c'before redirecting.',h'0d00'
deadstr	dc	c'Cannot fork (too many processes?)',h'0d00' ;try a spoon


; Parameter block for GS/OC call GetFileInfo
GRec           dc    i'4'	pCount (# of parameters)
GRecPath       ds    4	pathname (input; ptr to GS/OS string)
               ds    2	access (access attributes)
GRecFileType   ds    2	fileType (file type attribute)
GRecAuxType    ds    4	auxType (auxiliary type attribute)


PRec           dc    i'2'
PRecNum        dc    i'0'
PRecPath       ds    4

CloseParm	dc	i2'1'
CloseRef	dc	i2'0'

Err	dc	i2'0'

ttyopen	dc	i2'2'
ttyref	dc	i2'0'
	dc	i4'ttyname'
ttyname	gsstr	'.tty'

	END
