************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*
* $Id: builtin.asm,v 1.7 1998/10/26 17:04:49 tribby Exp $
*
**************************************************************************
*
* BUILTIN.ASM
*   By Tim Meekins
*   Modified by Dave Tribby for GNO 2.0.6
*
* Builtin command searching and execution.
*
* Note: text set up for tabs at col 16, 22, 41, 49, 57, 65
*              |     |                  |       |       |       |
*	^	^	^	^	^	^	
**************************************************************************
*
* Interfaces defined in this file:
*
*   builtin	subroutine (2:argc,4:argv)
*	Returns completion status in Accumulator
*		
*   IsBuiltin	subroutine (4:name)
*	return 2:tbl
*
* Remainder are interfaces to builtin commands with interface
*	subroutine (4:argv,2:argc)
*	returns status in accumulator
*   cd	(chdir is entry as an alternate name)
*   clear
*   echo
*   pwd
*   which
*   prefix
*   rehash	(unhash is entry as an alternate name)
*   exit
*   setdebug
*   psbi	(command name is "ps")
*   hashbi	(command name is "hash")
*   source
*   cmdbi	(command name is "commands")
*
**************************************************************************

	mcopy /obj/gno/bin/gsh/builtin.mac

dummybuiltin	start		; ends up in .root
	end

	setcom 60

p_next	gequ	0	;next in global proclist
p_friends	gequ	p_next+4	;next in job list
p_flags	gequ	p_friends+4	;various job status flags
p_reason	gequ	p_flags+2	;reason for entering this state
p_index	gequ	p_reason+2	;job index
p_pid	gequ	p_index+2	;process id
p_jobid	gequ	p_pid+2	;process id of job leader
p_command	gequ	p_jobid+2	;command (how job invoked)
p_space	gequ	p_command+4	;space for structure

**************************************************************************
*
* Find and execute a builtin command
*
**************************************************************************

builtin	START

	using BuiltinData

val	equ	1
file	equ	val+2
tbl	equ	file+4
space	equ	tbl+4
argv	equ	space+3
argc	equ	argv+4
end	equ	argc+2

;	 subroutine (2:argc,4:argv),space

	tsc
	sec
	sbc	#space-1
	tcs
	phd
	tcd

	ldy	#2
	lda	[argv]
	sta	file
	lda	[argv],y
	sta	file+2
	ld4	builtintbl,tbl
	ld2	-1,val
	lda	argc
	beq	done

loop	ldy	#2
	lda	[tbl]
	ora	[tbl],y
	beq	done
	lda	[tbl],y
	pha
	lda	[tbl]
	pha
	pei	(file+2)
	pei	(file)
	jsr	cmpcstr
	beq	foundit
	bpl	done
	add2	tbl,#10,tbl
	bra	loop

foundit	ldy	#4
	lda	[tbl],y
	sta	ourproc+1
	iny
	lda	[tbl],y
	sta	ourproc+2

	pei	(argv+2)
	pei	(argv)
	pei	(argc)
ourproc	jsl	>$FFFFFF	;might want to mutex this!!!!!!
	sta	val

	pei	(argc)
	pei	(argv+2)
	pei	(argv)
	jsl	argfree

done	ldy	val	Y-reg = return value.

	lda	space
	sta	end-3
	lda	space+1
	sta	end-2
	pld
	tsc
	clc
	adc	#end-4
	tcs

	tya		Accumulator = return value.

	rtl

	END

**************************************************************************
*
* Is it a built-in?
*
* Return value is: -1 if not a built-in, 0 if forked built-in,
*	    1 if non-forked built-in.
*
**************************************************************************

IsBuiltin	START

	using	BuiltinData

tbl	equ	0
space	equ	tbl+4

	subroutine (4:name),space

	ld4	builtintbl,tbl
builtinloop	ldy	#2
	lda	[tbl]
	ora	[tbl],y
	beq	nofile
	lda	[tbl],y
	pha
	lda	[tbl]
	pha
	pei	(name+2)
	pei	(name)
	jsr	cmpcstr
	beq	foundit
	bpl	nofile
	add2	tbl,#10,tbl
	bra	builtinloop
foundit	ldy	#8	Get the fork/nofork flag
	lda	[tbl],y	 and use it as return value.
	bra	foundbuiltin

nofile	lda	#-1	Set not-found return value.
foundbuiltin	sta	tbl

	return 2:tbl

	END

**************************************************************************
*
* Builtin data
*
**************************************************************************

BuiltinData	DATA
;
; First address is a pointer to the name, the second is a pointer to the
; command. Third value is fork flag (0 to fork, 1 for no fork).
; TABLE MUST BE SORTED BY COMMAND NAME.
;
builtintbl	dc	a4'aliasname,alias',i2'0'
	dc	a4'bgname,bg',i2'1'
	dc	a4'bindkeyname,bindkey',i2'0'
	dc	a4'cdname,cd',i2'1'
	dc	a4'chdirname,chdir',i2'1'
	dc	a4'clearname,clear',i2'1'		Changed to unforked
	dc	a4'cmdname,cmdbi',i2'0'
	dc	a4'dirsname,dirs',i2'0'
	dc	a4'echoname,echo',i2'0'
	dc	a4'editname,edit',i2'1'
	dc	a4'exitname,exit',i2'1'
	dc	a4'exportname,export',i2'1'
	dc	a4'fgname,fg',i2'1'
	dc	a4'hashname,hashbi',i2'0'
	dc	a4'hname,PrintHistory',i2'0'
	dc	a4'jobsname,jobs',i2'1'
	dc	a4'killname,kill',i2'1'
	dc	a4'popdname,popd',i2'1'
	dc	a4'pfxname,prefix',i2'1'
	dc	a4'psname,psbi',i2'0'
	dc	a4'pushdname,pushd',i2'1'
	dc	a4'pwdname,pwd',i2'1'
	dc	a4'rehashname,rehash',i2'1'
	dc	a4'setname,set',i2'0'
	dc	a4'setbugname,setdebug',i2'0'
	dc	a4'setenvname,setenv',i2'0'
	dc	a4'sourcename,source',i2'1' 	Changed to unforked
	dc	a4'stopname,stop',i2'1'
	dc	a4'tsetname,tset',i2'1'
	dc	a4'unaliasname,unalias',i2'1'
	dc	a4'unhashname,unhash',i2'1'
	dc	a4'unsetname,unset',i2'1'
	dc	a4'whichname,which',i2'0'
	dc	i4'0,0'

aliasname	dc	c'alias',h'00'
bgname	dc	c'bg',h'00'
bindkeyname	dc	c'bindkey',h'00'
chdirname	dc	c'chdir',h'00'
cdname	dc	c'cd',h'00'
clearname	dc	c'clear',h'00'
cmdname	dc	c'commands',h'00'
dirsname	dc	c'dirs',h'00'
echoname	dc	c'echo',h'00'
editname	dc	c'edit',h'00'
exitname	dc	c'exit',h'00'
exportname	dc	c'export',h'00'
fgname	dc	c'fg',h'00'
hashname	dc	c'hash',h'00'
hname	dc	c'history',h'00'
jobsname	dc	c'jobs',h'00'
killname	dc	c'kill',h'00'
pfxname	dc	c'prefix',h'00'
popdname	dc	c'popd',h'00'
psname	dc	c'ps',h'00'
pushdname	dc	c'pushd',h'00'
pwdname	dc	c'pwd',h'00'
rehashname	dc	c'rehash',h'00'
setbugname	dc	c'setdebug',h'00'
setname	dc	c'set',h'00'
setenvname	dc	c'setenv',h'00'
sourcename	dc	c'source',h'00'
stopname	dc	c'stop',h'00'
tsetname	dc	c'tset',h'00'
unaliasname	dc	c'unalias',h'00'
unhashname	dc	c'unhash',h'00'
unsetname	dc	c'unset',h'00'
whichname	dc	c'which',h'00'

	END

**************************************************************************
*
* CD: builtin command
* syntax: cd [pathname]
*
* Changes the current prefix to pathname. If no pathname then set to $HOME
*
**************************************************************************

cd	START
chdir	ENTRY

dpg	equ	1	Direct page pointer.
buf	equ	dpg+4	Buffer address to be freed.
status	equ	buf+4	Status returned from command.
space	equ	status+2
argc	equ	space+3
argv	equ	argc+2
end	equ	argv+4

;	 subroutine (4:argv,2:argc),space

	tsc
	sec
	sbc	#space-1
	tcs
	phd
	tcd

	lock	cdmutex

	stz	buf	Clear the pointer to
	stz	buf+2	 allocated buffer.

	stz	status	Assume good status.

	lda	argc	Number of parameters
	dec	a	 determines type of cd...
	beq	cdhome	  either to $HOME
	dec	a	   or to the directory
	jeq	paramcd	    on the command line.

;
; Illegal parameters: print usage string
;
showusage	inc	status	Return status = 1.
	lda	[argv]
	tax
	ldy	#2
	lda	[argv],y
	stx	argv
	sta	argv+2
	lda	[argv],y	
	and	#$FF
	beq	cdusage
	ldx	#^Usage2	Print chdir usage
	lda	#Usage2
	jsr	errputs
	jmp	exit
cdusage	ldx	#^Usage	Print cd usage
	lda	#Usage
	jsr	errputs
	jmp	exit

;
; Set prefix to $home
;
cdhome	anop
	ph4	#home	Get value of $HOME
	jsl	getenv
	sta	buf	If GS/OS result buffer
	stx	buf+2	 wasn't allocated,
	ora	buf+2
	jeq	exit	   there's no more to do.

	clc		Calculate address
	lda	buf	 of GS/OS input string
	adc	#2	  (2 bytes from start of
	sta	PRecPath	   result buffer).
	sta	GRecPath
	lda	buf+2
	adc	#0
	sta	PRecPath+2
	sta	GRecPath+2

	bra	getinfo

;
; Set prefix to path specified on command line
;
paramcd	anop

	ldy	#4
	lda	[argv],y
	sta	dpg
	iny2
	lda	[argv],y
	sta	dpg+2 

	lda	[dpg]
	and	#$FF
	if2	@a,ne,#'-',setprefix
	jmp	showusage

setprefix	pei	(dpg+2)
	pei	(dpg)
	jsr	c2gsstr
	sta	PRecPath
	sta	GRecPath
	sta	buf
	stx	PRecPath+2
	stx	GRecPath+2
	stx	buf+2

;
; Get file information to determine whether target is a valid directory
;
getinfo	GetFileInfo GRec
	bcc	ok
ohshit	sta	ErrError
	ErrorGS Err
	bra	done

ok	if2	GRecFT,eq,#$F,ok2
	ldx	dpg+2
	lda	dpg
	jsr	errputs
	ldx	#^direrr
	lda	#direrr
	jsr	errputs
	bra	done

;
; Everything looks OK. Set prefix 0 to the indicated value
;
ok2	SetPrefix PRec
	bcs	ohshit

;
; Deallocate buffer (if necessary), unlock mutex, cleanup stack, and leave
;
done	ph4	buf
	jsl	nullfree

exit	unlock cdmutex

	ldy	status	Put return status in Y-reg

	lda	space
	sta	end-3
	lda	space+1
	sta	end-2
	pld
	tsc
	clc
	adc	#end-4
	tcs

	tay		Put return status in Accumulator.

	rtl

cdmutex	key		Mutual exclusion key

; Parameter block for GS/OS SetPrefix call
PRec	dc	i'2'	pCount
	dc	i'0'	prefixNum (0 = current directory)
PRecPath	ds	4	Pointer to input prefix path

; Parameter block for GS/OS GetFileInfo call
GRec	dc	i'3'	pCount
GRecPath	ds	4	Pointer to input pathname
GRecAcc	ds	2	access (result)
GRecFT	ds	2	fileType (result)

home	gsstr	'home'	Env variable name

; Parameter block for shell ErrorGS call (p 393 in ORCA/M manual)
Err	dc	i2'1'	pCount
ErrError	ds	2	Error number

Usage	dc	c'Usage: cd [pathname]',h'0d00'
Usage2	dc	c'Usage: chdir [pathname]',h'0d00'
dirErr	dc	c': Not a directory',h'0d00'

	END

**************************************************************************
*
* CLEAR: builtin command
* syntax: clear
*
* clears the screen
*
**************************************************************************

clear	START

space	equ	1
argc	equ	space+3
argv	equ	argc+2
end	equ	argv+4

;	 subroutine (4:argv,2:argc),space

	tsc
	phd
	tcd

	lda	argc
	dec	a
	beq	clearit

	ldx	#^Usage
	lda	#Usage
	jsr	errputs
	bra	exit

clearit	jsr	clearscrn
	jsr	flush

exit	lda	space
	sta	end-3
	lda	space+1
	sta	end-2
	pld
	tsc
	clc
	adc	#end-4
	tcs

	lda	#0

	rtl	  

Usage	dc	c'Usage: clear',h'0d00'

	END

**************************************************************************
*
* ECHO: builtin command
* syntax: echo [-n] [text][...]
*
* Echo displays to stdout what is on the command (except -n).
*
* If '-n' specified then don't print newline.
*
**************************************************************************

echo	START

val	equ	1
nl	equ	val+2	flag: was -n option set?
ptr	equ	nl+2
space	equ	ptr+4
argc	equ	space+3
argv	equ	argc+2
end	equ	argv+4

;	 subroutine (4:argv,2:argc),space

* Add space on stack for local variables

	tsc
	sec
	sbc	#space-1
	tcs
	phd
	tcd

	stz	nl	Clear the -n flag.
	dec	argc	Decrement argument counter.
	jeq	done	Done if no more arguments.

	ldy	#4
	lda	[argv],y	Set ptr to
	sta	ptr	 point to the
	iny2		  text of the
	lda	[argv],y	   first
	sta	ptr+2	    argument.
	ldy	#1	
	lda	[ptr]	Get first
	and	#$FF	  character.
	if2	@a,ne,#'-',loop	If != '-', handle as regular param.


; First argument begins with "-"; only legal value is -n

	lda	[ptr],y	Get second
	and	#$FF	  character.
	if2	@a,eq,#'n',gotn	If != 'n', it's a bad one.

showusage	ldx	#^Usage	Incorrect parameter usage:
	lda	#Usage	 display the usage string.
	jsr	errputs
	jmp	exit

gotn	iny
	lda	[ptr],y	Get third
	and	#$FF	  character.
	bne	showusage	If != 0, it's a bad one.
	inc	nl	Set the -n flag.
	add2	argv,#4,argv	Bump argument pointer.
	dec	argc	Decrement argument counter.
	jeq	done	Done if no more arguments.

;
; Beginning of main processing loop of echo parameters.
;
loop	add2	argv,#4,argv	Bump argument pointer.
	ldy	#2
	lda	[argv],y	Set ptr to argv (next argument)
	sta	ptr+2
	lda	[argv]
	sta	ptr
putloop	lda	[ptr]	Get first
	and	#$FF	 character.
	cmp	#0	If 0,
	jeq	doneput	  done with this argument.
	cmp	#'\'	If != "\"
	jne	putit	  go save in print buffer.
	incad	ptr	Escape character found; point
	lda	[ptr]	 to the next
	and	#$FF	  character.
	beq	doneput	If 0, done with this argument.
	if2	@a,ne,#'b',esc02	Check for escape codes: "b"
	ldx	#1
	jsr	moveleft			moveleft
	bra	didit
esc02	if2	@a,ne,#'f',esc03		"f"
	jsr	clearscrn			clearscreen
	bra	didit
esc03	if2	@a,ne,#'n',esc04		"n"
	lda	#13			print newline
	bra	putit
esc04	if2	@a,ne,#'r',esc05		"r"
	lda	#13			print newline
	bra	putit
esc05	if2	@a,ne,#'t',esc06		"t"
	lda	#9			print tab
	bra	putit
esc06	if2	@a,ne,#'0',putit		"0"
	stz	val			decode numeric value
	ldy	#1
escloop	lda	[ptr],y
	and	#$FF
	beq	putval
	if2	@a,cc,#'0',putval
	if2	@a,cs,#'9'+1,putval
	sub2	@a,#'0',@a
	pha
	lda	val
	asl2	a
	adc	val
	asl	a
	adc	1,s
	sta	val
	pla
	incad	ptr
	bra	escloop

putval	lda	val	Get numeric escape code.

putit	jsr	putchar	Save character in accumulator.
didit	incad	ptr	Point to next char in arg
	jmp	putloop	 and go process it.

doneput	dec	argc	Decrement argument counter.
	beq	done	Done if no more arguments.
	bmi	done	 (or if there were no arguments!)
	lda	#' '	Add a blank
	jsr	putchar	 between arguments.
	jmp	loop	Get next argument.

done	lda	nl	If "-n" flag isn't set,
	bne	exit
	jsr	newline	  add a newline.

exit	jsr	flush	Print the buffer.


* Clear parameters from stack and return from subroutine.

	lda	space
	sta	end-3
	lda	space+1
	sta	end-2
	pld
	tsc
	clc
	adc	#end-4
	tcs

	lda	#0

	rtl	  

Usage	dc	c'Usage: echo [-n] [strings...]',h'0d00'

	END

**************************************************************************
*
* PWD: builtin command
* syntax: pwd
*
* print the working directory.
*
**************************************************************************

pwd	START

ptr	equ	1
space	equ	ptr+4
argc	equ	space+3
argv	equ	argc+2
end	equ	argv+4

;	 subroutine (4:argv,2:argc),space

	tsc
	sec
	sbc	#space-1
	tcs
	phd
	tcd

	dec	argc	If an argument was provided,
	beq	wait

	ldx	#^Usage	  print the usage string.
	lda	#Usage
	jsr	errputs
	bra	exit

wait	lock	pwdmutex	

	pea	0
	jsl	getpfxstr	Get value of prefix 0.
	sta	ptr
	stx	ptr+2

	ora	ptr+2	If NULL pointer returned,
	beq	done	 an error was reported.

	ldy	#2	If length of returned
	lda	[ptr],y	 GS/OS string is 0,
	beq	freebuf	  an error was reported.

	lda	ptr	X/A = address of
	clc		 text (four bytes
	adc	#4	  beyond start).
	bcc	doputs
	inx
doputs	jsr	puts	Print the c-string
	jsr	newline	 and add a newline.

freebuf	ph4	ptr	Free the buffer.
	jsl	nullfree

done	unlock pwdmutex

exit	lda	space	Deallocate stack space
	sta	end-3	 and return to the caller.
	lda	space+1
	sta	end-2
	pld
	tsc
	clc
	adc	#end-4
	tcs

	lda	#0	Return status always 0.

	rtl	  

pwdmutex	key

Usage	dc	c'Usage: pwd',h'0d00'

	END

**************************************************************************
*
* WHICH: builtin command
* syntax: which [command ...]
*
* displays the location of command
*
**************************************************************************

which	START

	using hashdata

ptr	equ	1
file	equ	ptr+4
space	equ	file+4
argc	equ	space+3
argv	equ	argc+2
end	equ	argv+4

;	 subroutine (4:argv,2:argc),space

	tsc
	sec
	sbc	#space-1
	tcs
	phd
	tcd
;
; display usage if no arguments given
;
	if2	argc,ge,#2,loop
	ldx	#^whicherr
	lda	#whicherr
	jsr	errputs
	jmp  exit
;
; loop through each argument
;
loop	add2	argv,#4,argv
	dec	argc
	jeq	exit

	lda	[argv]
	sta	file
	ldy	#2
	lda	[argv],y
	sta	file+2

;
; see if it's an alias
;
	pei	(file+2)
	pei	(file)
	jsl	findalias
	sta	ptr
	stx	ptr+2
	ora	ptr+2
	beq	chkbuiltin
	ldx	#^aliasstr
	lda	#aliasstr
	jsr	puts
	ldx	ptr+2
	lda	ptr
	jsr	puts
	jmp	nextarg
;
; was it a built-in?
;
chkbuiltin	pei	(file+2)
	pei	(file)
	jsl	IsBuiltin	
	cmp	#-1
	beq	tryhash
foundbuiltin	ldx	#^builtstr
	lda	#builtstr
	jsr	puts
	jmp	nextarg
;
; See if it was hashed
;
tryhash	pei	(file+2)
	pei	(file)
	ph4	hash_table
	ph4	#hash_paths
	jsl	search
	cmp	#0
	bne	foundhash
	cpx	#0
	beq	thispfx
;
; It was hashed, so say so.
;
foundhash	jsr	puts
	jmp	nextarg
;
; It must be in the current prefix, so check it out.
;
thispfx	lock	pwdmutex
;
; check for existence of file
;
	pei	(file+2)
	pei	(file)
	jsr	c2gsstr
	sta	GRecPath
	stx	GRecPath+2
	sta	ptr
	stx	ptr+2

	GetFileInfo GRec
	bcs	nofile
;
; we found the file, so display the cwd
;
showcwd	pei	(ptr+2)
	pei	(ptr)
	jsl	nullfree

	pea	0
	jsl	getpfxstr	Get value of prefix 0.
	sta	ptr
	stx	ptr+2

	ora	ptr+2	If NULL pointer returned,
	beq	donecwd	 an error was reported.

	ldy	#2	If length of returned
	lda	[ptr],y	 GS/OS string is 0,
	beq	freebuf	  an error was reported.

	lda	ptr	X/A = address of
	clc		 text (four bytes
	adc	#4	  beyond start).
	bcc	doputs
	inx
doputs	jsr	puts	Print the directory name.

	ldx	file+2	Print the file name.
	lda	file
	jsr	puts

freebuf	ph4	ptr	Free the buffer.
	jsl	nullfree
	stz	ptr
	stz	ptr+2
	bra	donecwd
;
; No such command
;
nofile	ldx	#^cantdoit
	lda	#cantdoit
	jsr	puts
donecwd	unlock pwdmutex
	pei	(ptr+2)
	pei	(ptr)
	jsl	nullfree
;
; Go try the next file.
;
nextarg	jsr	newline
	jmp	loop

exit	lda	space
	sta	end-3
	lda	space+1
	sta	end-2
	pld
	tsc
	clc
	adc	#end-4
	tcs

	lda	#0

	rtl	  

whicherr	dc	c'Usage: which [file ...]',h'0d00'
builtstr	dc	c'Shell Built-in Command',h'00'
cantdoit	dc	c'Command Not Found',h'00'
aliasstr	dc	c'Aliased as ',h'00'

pwdmutex	key

GRec	dc	i'4'
GRecPath	ds	4
	ds	2
GRecFileType	ds	2
GRecAuxType	ds	4

	END

**************************************************************************
*
* PREFIX: builtin command
* syntax: prefix [num [prefix]]
*
* sets prefix number num to prefix
*
**************************************************************************

prefix	START

dir	equ	1
numstr	equ	dir+4
pfxnum	equ	numstr+4
space	equ	pfxnum+2
argc	equ	space+3
argv	equ	argc+2
end	equ	argv+4

;	 subroutine (4:argv,2:argc),space

	tsc
	sec
	sbc	#space-1
	tcs
	phd
	tcd

	lock	mutex

	lda	argc	Get number of arguments.
	dec	a
	beq	showall	If no parameters, show all prefixes.
	dec	a
	jeq	showone	If one, show one.
	dec	a
	jeq	setprefix	If two, set a prefix.

	ldx	#^usage
	lda	#usage
	jsr	errputs
	jmp	done
;
; No parameters provided: show all the prefixes
;
showall	anop
	lda	#$FFFF
	sta	pfxnum	First prefix # will be 0.

	pha		Get the boot volume string.
	jsl	getpfxstr
	sta	dir
	stx	dir+2

	ora	dir+2	If NULL pointer returned,
	beq	bumppfx	 an error was reported.

	ldx	#^bootstr
	lda	#bootstr
	jsr	puts
	ldx	dir+2
	lda	dir	X/A = address of
	clc		 text (four bytes
	adc	#4	  beyond start).
	bcc	doputs
	inx
doputs	jsr	puts	Print the directory name
	jsr	newline	 and a newline.
	bra	nextall	Jump into the all loop.

allloop	lda	pfxnum
	pha
	jsl	getpfxstr
	sta	dir
	stx	dir+2

	ora	dir+2	If NULL pointer returned,
	beq	bumppfx	 an error was reported.

	ldy	#2	Get length word.
	lda	[dir],y
	beq	nextall	If zero, do the next prefix.
	Int2Dec (pfxnum,#pfxstr,#2,#0)
	ldx	#^pfxstr
	lda	#pfxstr
	jsr	puts
	ldx	dir+2
	lda	dir	X/A = address of
	clc		 text (four bytes
	adc	#4	  beyond start).
	bcc	doputs2
	inx
doputs2	jsr	puts	Print the directory name
	jsr	newline

nextall	ph4	dir	Free the GS/OS result buffer
	jsl	nullfree	 allocated for pathname.
bumppfx	inc	pfxnum	Bump the prefix number.
	if2	pfxnum,cc,#32,allloop
	jmp	finish

;
; One parameter provided: show a single prefix
;    
showone	ldy	#1*4+2	Put pointer to
	lda	[argv],y	 first command
	sta	numstr+2	  argument in
	pha		   numstr, and
	dey2		    also on stack
	lda	[argv],y	     as parameter.
	sta	numstr
	pha
	jsr	cstrlen	Get length of argument.
	tax

	Dec2Int (numstr,@x,#0),@a 	Convert to integer.
	cmp	#32	If prefix num >= 32,
	bcc	getpfx
	jsr	newline		just print blank line.
	jmp	done

getpfx	pha		Get that prefix value.
	jsl	getpfxstr
	sta	dir
	stx	dir+2

	ora	dir+2	If NULL pointer returned,
	jeq	done	 an error was reported.

	Int2Dec (pfxnum,#pfxstr,#2,#0)

	ldy	#2	Get length word.
	lda	[dir],y
	beq	donewline	If zero, just print newline.
	ldx	dir+2
	lda	dir	X/A = address of
	clc		 text (four bytes
	adc	#4	  beyond start).
	bcc	doputs3
	inx
doputs3	jsr	puts	Print the directory name
donewline	jsr	newline
	ph4	dir	Free the GS/OS result buffer
	jsl	nullfree	 allocated for pathname.
	jmp	done

;
; Two parameters provided: set a prefix
;
setprefix	ldy	#1*4+2	Put pointer to
	lda	[argv],y	 first command
	sta	numstr+2	  argument (prefix
	pha		   num) in numstr, and
	dey2		    also on stack
	lda	[argv],y	     as parameter.
	sta	numstr
	pha
	jsr	cstrlen	Get length of argument.
	tax

	Dec2Int (numstr,@x,#0),PRecNum	Convert to integer string.

	lda	PRecNum
	cmp	#32	If prefix num >= 32,
	bcs	done		nothing to do.

	ldy	#2*4+2	Put pointer to
	lda	[argv],y	 second command
	pha		  argument (value)
	dey2		   on stack as parameter.
	lda	[argv],y
	pha
	jsr	c2gsstr	Convert to GS string.

	sta	GRecPath	Store in GetFileInfo
	stx	GRecPath+2	 parameter block and
	sta	PRecPath	  SetPrefix p.b.
	stx	PRecPath+2

;
; Get file information to determine whether target is a valid directory
;
	GetFileInfo GRec
	bcc	ok
	sta	ErrError
	ErrorGS Err
	bra	done

ok	if2	GRecFT,eq,#$F,ok2	If filetype != $F,
	ldx	#^direrr		print error message
	lda	#direrr		 'Not a directory'
	jsr	errputs
	bra	done

ok2	SetPrefix PRec	Set the prefix.
	bcc	finish	If error flag set,
	ldx	#^errorstr		print error message
	lda	#errorstr		 'could not set prefix,
	jsr	errputs		   pathname may not exist.'

finish	ph4	PRecPath	Free the name string buffer.
	jsl	nullfree


done	unlock mutex

	lda	space
	sta	end-3
	lda	space+1
	sta	end-2
	pld
	tsc
	clc
	adc	#end-4
	tcs

	lda	#0

	rtl	  

mutex	key

errorstr	dc	c'prefix: could not set prefix, pathname may not exist.'
	dc	h'0d00'
usage	dc	c'Usage: prefix prefixnum prefixname',h'0d00'
bootstr	dc	c' *: ',h'00'
pfxstr	dc	c'00: ',h'00'
dirErr	dc	c'prefix: Not a directory',h'0d00'
		  
;
; Parameter block for GS/OS GetFileInfo call
;
GRec	dc	i'3'	pCount
GRecPath	ds	4	Pointer to input pathname
GRecAcc	ds	2	access (result)
GRecFT	ds	2	fileType (result)

;
; Parameter buffer for SetPrefix GS/OS call
;
PRec	dc	i'2'	pCount
PRecNum	dc	i'0'	prefix number
PRecPath	ds	4	pointer to GS/OS string with value

;
; Parameter block for shell ErrorGS call (p 393 in ORCA/M manual)
;
Err	dc	i2'1'	pCount
ErrError	ds	2	Error number

	END

**************************************************************************
*
* REHASH: builtin command
* syntax: rehash
*
* rehashes the path
*
**************************************************************************
*
* UNHASH: builtin command
* syntax: unhash
*
* turns off command hashing
*
**************************************************************************

rehash	START
unhash	ENTRY

space	equ	1
argc	equ	space+3
argv	equ	argc+2
end	equ	argv+4

	tsc
	phd
	tcd

	lda	argc
	dec	a
	beq	doit

	ldx	#^Usage
	lda	#Usage	
	jsr	errputs
	ldy	#2
	lda	[argv],y
	tax
	lda	[argv]
	jsr	errputs
	lda	#13
	jsr	errputchar
	bra	exit

doit	jsr	dispose_hash	;remove old table
	lda	[argv]
	tax
	ldy	#2
	lda	[argv],y
	sta	argv+2
	stx	argv
	lda	[argv]
	and	#$FF
	jsr	tolower
	if2	@a,eq,#'u',exit	;if 'rehash' do the hashing.
	jsl	hashpath	;hash the path

exit	lda	space
	sta	end-3
	lda	space+1
	sta	end-2
	pld
	tsc
	clc
	adc	#end-4
	tcs

	lda	#0

	rtl	  

Usage	dc	c'Usage: ',h'00'

	END


**************************************************************************
*
* EXIT: builtin command
* syntax: exit
*
* exit gsh
*
**************************************************************************

exit	START

	using	global

space	equ	0

	subroutine (4:argv,2:argc),space

	inc	exit_requested

	return 2:#0

	END

**************************************************************************
*
* SETDEBUG: builtin command
* syntax: setdebug (val | [+|-]flag)
*
* sets debugging in kernel
*
**************************************************************************

setdebug	START

arg	equ	0
newdebug	equ	arg+4
mode	equ	newdebug+2
space	equ	mode+2

	subroutine (4:argv,2:argc),space

	lda	argc
	dec	a
	bne	ok
showusage	ldx	#^usage
	lda	#usage
	jsr	errputs
	jmp	return

ok	stz	mode
	mv2	globaldebug,newdebug
	dec	argc
	add2	argv,#4,argv
loop	lda	[argv]
	sta	arg
	ldy	#2
	lda	[argv],y
	sta	arg+2
	lda	[arg]
	and	#$FF
	if2	@a,eq,#'-',turnoff
	if2	@a,eq,#'+',turnon
	ldx	mode
	bne	showusage
	ldx	argc
	dex	
	bne	showusage
	if2	@a,cc,#'0',showusage
	if2	@a,cs,#'9'+1,showusage
	pei	(arg+2)
	pei	(arg)
	jsr	cstrlen
	tax
	Dec2Int (arg,@x,#0),@a
	sta	newdebug
	jmp	done

turnoff	jsr	findflag
	eor	#$FFFF
	and	newdebug
	bra	turnnext
turnon	jsr	findflag
	ora	newdebug
turnnext	sta	newdebug
	ld2	1,mode
	add2	argv,#4,argv
	dec	argc
	beq	done
	jmp	loop

done	setdebug newdebug
	mv2	newdebug,globaldebug
return	return 2:#0

findflag	incad	arg
	ldy	#0
findloop	phy
	lda	nametbl,y
	ora	nametbl+2,y
	beq	nofind
	lda	nametbl+2,y
	pha
	lda	nametbl,y
	pha
	pei	(arg+2)
	pei	(arg)
	jsr	cmpcstr
	beq	foundit
	pla
	add2	@a,#4,@y
	bra	findloop

foundit	pla
	lsr	a
	tax
	lda	bittbl,x
	rts

nofind	pla
	ldx	arg+2
	lda	arg
	jsr	errputs
	ldx	#^errstr
	lda	#errstr
	jsr	errputs
	lda	#-1
	pla		;rts address
	jmp	showusage

usage	dc	c'Usage: setdebug (value | [+|-]flag ... )',h'0d0d'
	dc	c'Flags: gsostrace  - Trace GS/OS calls',h'0d'
	dc	c'       gsosblocks - Trace GS/OS parameter blocks',h'0d'
	dc	c'       gsoserrors - Trace GS/OS errors',h'0d'
	dc	c'       pathtrace  - Trace GS/OS pathnames',h'0d'
	dc	c'       sigtrace   - Trace signals',h'0d'
	dc	c'       systrace   - Trace system calls',h'0d'
* >> Next line is temporary
	dc	c'       breakpoint - Coded brk instructions',h'0d'
	dc	h'00'

errstr	dc	c': Unknown flag',h'0d0d00'

nametbl	dc	a4'str01,str02,str03,str04,str05,str06,str07,0'
str01	dc	c'gsostrace',h'00'
str02	dc	c'pathtrace',h'00'
str03	dc	c'gsoserrors',h'00'
str04	dc	c'sigtrace',h'00'
str05	dc	c'systrace',h'00'
str06	dc	c'gsosblocks',h'00'
* >> Next line is temporary; Also: remove str07 in nametbl
str07	dc	c'breakpoint',h'00'

bittbl	dc	i2'%000001'
	dc	i2'%000010'
	dc	i2'%000100'
	dc	i2'%001000'
	dc	i2'%010000'
	dc	i2'%100000'
* >> Next line is temporary
	dc	i2'%10000000'

* >> Next line is temporary
check4debug	ENTRY

globaldebug	dc	i2'0'

	END

**************************************************************************
*
* PS: builtin command
* syntax: ps
*
* display process status
*
**************************************************************************

psbi	START

	using	pdata

myuid	equ	0
t	equ	myuid+2
ps2	equ	t+4
pr2	equ	ps2+4
pr	equ	pr2+4
ps	equ	pr+4
space	equ	ps+4

	subroutine (4:argv,2:argc),space

	lda	argc
	dec	a
	beq	ok
showusage	ldx	#^usage
	lda	#usage
	jsr	errputs
	jmp	return

ok	getuid
	sta	myuid
	kvm_open             
	sta	ps
	stx	ps+2
	ora2	@a,ps+2,@a
	bne	ok2
	ldx	#^kvmerrstr
	lda	#kvmerrstr
	jsr	errputs
	jmp	done

ok2	ldx	#^header
	lda	#header
	jsr	puts

loop	kvmnextproc ps
	sta	pr
	stx	pr+2
	ora2	@a,pr+2,@a
	jeq	done

	ldy	#94	;ps->p_uid
	lda	[pr],y
	cmp	myuid
	jne	skip

	ldy	#2
	lda	[ps],y	;ps->pid
	Int2Dec (@a,#pidstr,#4,#0)
	ldx	#^pidstr
	lda	#pidstr
	jsr	puts

	ldy	#2	
	lda	[pr],y	;pr->processState
	cmp	#9
	bcc	okstate
	lda	#9
okstate	asl	a
	asl	a
	asl	a
	tax
	ldy	#8
putstate	lda	statetbl,x
	phx
	phy
	jsr	putchar
	ply
	plx
	inx
	dey
	bne	putstate

	ldy	#6
	lda	[pr],y	;pr->ttyID
	beq	ttnul
	cmp	#3
	bne	ttnum
	lda	#'c'
	jsr	putchar
	lda	#'o'
	bra	showuser
ttnul	lda	#'n'
	jsr	putchar
	lda	#'u'
	bra	showuser
ttnum	ldy	#0
ttnumlup	cmp	#10
	bcc	ttnum0
	sec
	sbc	#10
	iny
	bra	ttnumlup
ttnum0	pha
	tya
	clc
	adc	#'0'
	jsr	putchar
	pla
	clc
	adc	#'0'
showuser	jsr	putchar

	ldy	#4
	lda	[pr],y	;pr->userID
	Int2Hex (@a,#userstr+1,#4)
	ldx	#^userstr
	lda	#userstr
	jsr	puts

	ldy	#94
	lda	[pr],y	;pr->puid
	Int2Hex (@a,#puidstr,#4)
	ldx	#^puidstr
	lda	#puidstr
	jsr	puts

	ldy	#50            
	lda	[pr],y	;pr->ticks
	tax
	iny2
	lda	[pr],y
	LongDivide (@ax,#60),(@ax,@yy)
	LongDivide (@xa,#60),(t,@ax)
	Long2Dec (@xa,#timestr+4,#2,#0)
	Long2Dec (t,#timestr,#3,#0)
	lda	timestr+4
	and	#$FF
	cmp	#' '
	bne	time0
	short	a
	lda	#'0'
	sta	timestr+4
	long	a
time0	ldx	#^timestr
	lda	#timestr
	jsr	puts

	ldy	#34
	lda	[pr],y	;pr->args
	tax
	iny2
	lda	[pr],y
	jne	goodcmd
	cpx	#0
	jne	goodcmd

	lda	pjoblist	;check for name in job list
	ldx	pjoblist+2

jobloop	sta	pr2
	stx	pr2+2

	ora	pr2+2
	beq	childof
	ldy	#p_pid
	lda	[pr2],y
	ldy	#2
	cmp	[ps],y	;ps->pid
	bne	loop2

	ldy	#p_command+2
	lda	[pr2],y
	tax
	ldy	#p_command
	lda	[pr2],y
	jsr	puts
	bra	next

loop2	ldy	#p_next+2
	lda	[pr2],y
	tax
	ldy	#p_next
	lda	[pr2],y
	bra	jobloop

childof	ldx	#^forkstr
	lda	#forkstr
	jsr	puts
	kvm_open
	sta	ps2
	stx	ps2+2
	lda	[pr]
	tay
	kvmgetproc (ps2,@y)
	sta	pr2
	stx	pr2+2
	kvm_close ps2
	ldy	#34
	lda	[pr2],y	;pr2->args
	tax
	iny2
	lda	[pr2],y
	bne	goodcmd
	cpx	#0
	bne	goodcmd

	ldx	#^forkstr2
	lda	#forkstr2
	jsr	puts
	bra	next              

goodcmd	tay
	txa
	tyx
	clc
	adc	#8
	jsr	puts

next	jsr	newline
skip	jmp	loop

done	kvm_close ps

return	return 2:#0

usage	dc	c'Usage: ps',h'0d00'
kvmerrstr	dc	c'ps: error in kvm_open()',h'0d00'
header	dc	c'  ID  STATE   TT MMID  UID   TIME COMMAND',h'0d00'
pidstr	dc	c'0000  ',h'00'
userstr	dc	c' 0000 ',h'00'
puidstr	dc	c'0000 ',h'00'
timestr	dc	c'000:00 ',h'00'
forkstr	dc	c'forked child of ',h'00'
forkstr2	dc	c'an unknown process',h'00'

test1	dc	c'getuid = $'
test1a	dc	c'0000',h'0d00'

statetbl	dc	c'defunct '
	dc	c'running '
	dc	c'ready   '
	dc	c'blocked '
	dc	c'ready   '
	dc	c'suspend '
	dc	c'waiting '
	dc	c'waiting '
	dc	c'paused  '
	dc	c'unknown '
		           
	END

**************************************************************************
*
* HASH: builtin command
* syntax: hash
*
* display hashed commands
*
**************************************************************************

hashbi	START

	using	hashdata

sv	equ	0
q	equ	sv+4
p	equ	q+4
space	equ	p+4

	subroutine (4:argv,2:argc),space

	ph2	t_size	Get size of hash table.
	jsl	sv_alloc	Allocate a string vector array.
	sta	sv
	stx	sv+2

	lda	hash_table	If no hash table
	ora	hash_table+2	 has been allocated,
	beq	exit	  exit.

	mv4	hash_table,p	Move address to dir pg variable.
	lda	hash_numexe	Get the number of executable files.
	beq	doneadd	Done if 0.
; 
; loop through every hashed file and add it the string vector
;
	ldy	#0	Y is index into the next entry.
	ldx	t_size	X is the number of entries left.
	beq	doneadd
addloop	lda	[p],y	Get next hash table entry.
	sta	q	
	iny
	iny
	lda	[p],y
	sta	q+2
	iny
	iny
	ora	q	If this entry isn't used,
	beq	skip	 skip to the next one.

	phy		Hold the Y and X regs on stack.
	phx	
	pei	(sv+2)	Insert string in table entry
	pei	(sv)	 into the string vector.
	clc
	lda	q
	adc	#2	(Note: tn_name in hash.asm == 2)
	tax
	lda	q+2
	adc	#0
	pha
	phx
	pea	1	(allocflag: 1 = allocate memory)
	jsl	sv_add
	plx		Restore X and Y regs from stack.
	ply
skip	dex
	bne	addloop
;
; Files have all been added to the string vector
;
doneadd	anop

	pei	(sv+2)
	pei	(sv)
	jsl	sv_sort	Sort the string vector.

	pei	(sv+2)
	pei	(sv)
	jsl	sv_colprint	Print the string vector in columns.

	pei	(sv+2)
	pei	(sv)
	jsl	sv_dispose	Dispose of the string vector memory.

exit	return 2:#0

	END

**************************************************************************
*
* SOURCE: builtin command
* syntax: source file [arguments...]
*
* executes a shell file w/o pushing environment
*
**************************************************************************

source	START

retval	equ	0
space	equ	retval+2

	subroutine (4:argv,2:argc),space

	dec	argc	If no filename was provided,
	bne	ok

	ldx	#^usage	  Print usage string.
	lda	#usage
	jsr	errputs
	lda	#1	  Return error status.
	sta	retval
	bra	exit

ok	stz	retval

	add2	argv,#4,argv

*   ShellExec	subroutine (4:path,2:argc,4:argv,2:jobflag)

	ldy	#2	path is filename argument
	lda	[argv],y
	pha
	lda	[argv]
	pha
	pei	(argc)	reuse argc
	pei	(argv+2)	reuse argv
	pei	(argv)
	pea	0	jobflag = 0
	jsl	ShellExec
	sta	retval

exit	return 2:retval

usage	dc	c'usage: source file [arguments...]',h'0d00'

	END
	   
**************************************************************************
*
* COMMANDS: builtin command
* syntax: commands
*
* display builtin commands
*
**************************************************************************

cmdbi	START

	using	BuiltinData

sv	equ	0
space	equ	sv+4

	subroutine (4:argv,2:argc),space

	ph2	#50
	jsl	sv_alloc

	sta	sv
	stx	sv+2

	ldx	#0
; 
; loop through every hashed file and add it the string vector
;
addloop	lda	builtintbl,x
	ora	builtintbl+2,x
	beq	doneadd
	phx	
	pei	(sv+2)
	pei	(sv)
	lda	builtintbl+2,x
	pha
	lda	builtintbl,x
	pha
	pea	1
	jsl	sv_add
	pla
	clc
	adc	#10
	tax
	bra	addloop
doneadd	anop

	pei	(sv+2)
	pei	(sv)
	jsl	sv_sort
	pei	(sv+2)
	pei	(sv)
	jsl	sv_colprint			

	pei	(sv+2)
	pei	(sv)
	jsl	sv_dispose

exit	return 2:#0

	END
