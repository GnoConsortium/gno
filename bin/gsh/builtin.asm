************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*
**************************************************************************
*
* BUILTIN.ASM
*   By Tim Meekins
*
* Builtin command searching and execution.
*
**************************************************************************

	mcopy /obj/gno/bin/gsh/builtin.mac

dummy	start		; ends up in .root
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
* Find a builtin command
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
	jeq	done

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

done	ldy	val
	lda	space
	sta	end-3
	lda	space+1
	sta	end-2
	pld
	tsc
	clc
	adc	#end-4
	tcs
	tya
	rtl

	END

**************************************************************************
*
* Is it a built-in?
*
**************************************************************************

IsBuiltin	START

	using	BuiltinData

tbl	equ	0
space	equ	tbl+4

	subroutine (4:name),space

	ld4	builtintbl,tbl
builtinloop    ldy   #2
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
foundit	ldy	#8
	lda	[tbl],y
	bra	foundbuiltin

nofile	lda	#-1
foundbuiltin   sta   tbl

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
; command. MUST BE SORTED.
;
builtintbl     dc    a4'aliasname,alias',i2'0'
	dc	a4'bgname,bg',i2'1'
	dc	a4'bindkeyname,bindkey',i2'0'
               dc    a4'cdname,cd',i2'1'
               dc    a4'chdirname,chdir',i2'1'
               dc    a4'clearname,clear',i2'0'
	dc	a4'cmdname,cmdbi',i2'0'
               dc    a4'dfname,df',i2'0'
	dc	a4'dirsname,dirs',i2'0'
               dc    a4'echoname,echo',i2'0'
	dc	a4'editname,edit',i2'1'
               dc    a4'exitname,exit',i2'1'
               dc    a4'exportname,export',i2'1'
	dc	a4'fgname,fg',i2'1'
	dc	a4'hashname,hashbi',i2'0'
               dc    a4'hname,PrintHistory',i2'0'
	dc	a4'jobsname,jobs',i2'1'
               dc    a4'killname,kill',i2'1'
	dc	a4'popdname,popd',i2'1'
               dc    a4'pfxname,prefix',i2'1'
               dc    a4'psname,psbi',i2'0'
	dc	a4'pushdname,pushd',i2'1'
               dc    a4'pwdname,pwd',i2'1'
               dc    a4'rehashname,rehash',i2'1'
               dc    a4'setname,set',i2'0'
	dc	a4'setbugname,setdebug',i2'0'
	dc	a4'setenvname,setenv',i2'0'
	dc	a4'sourcename,source',i2'0'
	dc	a4'stopname,stop',i2'1'
	dc	a4'tsetname,tset',i2'1'
	dc	a4'unaliasname,unalias',i2'1'
	dc	a4'unhashname,unhash',i2'1'
	dc	a4'unsetname,unset',i2'1'
	dc	a4'whichname,which',i2'0'
	dc	i4'0,0'

aliasname      dc    c'alias',h'00'
bgname	dc	c'bg',h'00'
bindkeyname	dc	c'bindkey',h'00'
chdirname      dc    c'chdir',h'00'
cdname         dc    c'cd',h'00'
clearname      dc    c'clear',h'00'
cmdname	dc	c'commands',h'00'
dirsname	dc	c'dirs',h'00'
dfname         dc    c'df',h'00'
echoname	dc	c'echo',h'00'
editname	dc	c'edit',h'00'
exitname	dc	c'exit',h'00'
exportname     dc    c'export',h'00'
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
rehashname     dc    c'rehash',h'00'
setbugname	dc	c'setdebug',h'00'
setname        dc    c'set',h'00'
setenvname	dc	c'setenv',h'00'
sourcename	dc	c'source',h'00'
stopname	dc	c'stop',h'00'
tsetname	dc	c'tset',h'00'
unaliasname    dc    c'unalias',h'00'
unhashname     dc    c'unhash',h'00'
unsetname      dc    c'unset',h'00'
whichname      dc    c'which',h'00'

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

dir	equ	1
space	equ	dir+4
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

	lda	argc
	dec	a
	beq	cdhome
	dec	a
	jeq	normalcd

showusage      lda   [argv]
	tax
	ldy	#2
	lda	[argv],y
	stx	argv
	sta	argv+2
	lda	[argv],y	
	and	#$FF
	beq	cdusage
	ldx	#^Usage2
	lda	#Usage2
	jsr	errputs
	jmp	exit
cdusage	ldx	#^Usage
	lda	#Usage
	jsr	errputs
	jmp	exit
;
; set prefix to home
;
cdhome	jsl	alloc256
	sta	dir
	stx	dir+2
	sta	ReadName
	stx	ReadName+2
	ora	ReadName+2
	bne	madeit
	lda	#$201
	jmp	ohshit
madeit	Read_Variable ReadVar
	lda	[dir]
	and	#$FF
	beq	nohome
	ph4	ReadName
	jsr	p2cstr
	sta	dir
	stx	dir+2
	ph4	@xa
	lda	ReadName
	ldx	ReadName+2
	jsl	free256
	pl4	ReadName
	bra	setprefix

nohome	ldx	dir+2
	lda	dir
	jsl	free256
	jmp	exit
;
; set prefix to specified path
;
normalcd	stz	ReadName
	stz	ReadName+2

	ldy	#4
	lda	[argv],y
	sta	dir
	iny2
	lda	[argv],y
	sta	dir+2 

	lda	[dir]
	and	#$FF
	if2	@a,ne,#'-',setprefix
	jmp	showusage

setprefix      pei   (dir+2)
	pei	(dir)
	jsr	c2gsstr
	sta	PRecPath
	sta	GRecPath
	stx	PRecPath+2
	stx	GRecPath+2

	GetFileInfo GRec
	bcc	ok
ohshit	sta	Err
	Error Err
	bra	done

ok	if2	GRecFT,eq,#$F,ok2
	ldx	dir+2
	lda	dir
	jsr	errputs
	ldx	#^direrr
	lda	#direrr
	jsr	errputs
	bra	done

ok2	SetPrefix PRec
	bcs	ohshit
done	ph4	PRecPath
	jsl	nullfree
	ora2	ReadName,ReadName+2,@a
	beq	whoaboy
	ph4	ReadName
	jsl	nullfree
whoaboy	anop

exit	unlock cdmutex

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

cdmutex	key

PRec	dc	i'2'
PRecNum	dc	i'0'
PRecPath	ds	4

GRec	dc	i'3'
GRecPath	ds	4
GRecAcc	ds	2
GRecFT	ds	2

ReadVar	dc	a4'home'
ReadName	ds	4
home	str	'home'

Err	ds	2

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
nl	equ	val+2
ptr	equ	nl+2
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

	stz	nl
	if2	argc,lt,#2,loop
	ldy	#4
	lda	[argv],y
	sta	ptr
	iny2
	lda	[argv],y
	sta	ptr+2
	ldy	#1
	lda	[ptr]
	and	#$FF
	if2	@a,ne,#'-',loop
	lda	[ptr],y
	and	#$FF
	if2	@a,ne,#'n',showusage
	iny
	lda	[ptr],y
	and	#$FF
	bne	showusage
	inc	nl
	add2	argv,#4,argv
	dec	argc
	bra	loop

showusage	ldx	#^Usage
	lda	#Usage
	jsr	errputs
	jmp	exit

loop	add2	argv,#4,argv
	dec	argc
	jeq	done
	ldy	#2
	lda	[argv],y
	sta	ptr+2
	lda	[argv]
	sta	ptr
putloop	lda	[ptr]
	and	#$FF
	cmp	#0
	jeq	doneput
	cmp	#'\'
	jne	putit
	inc	ptr
	lda	[ptr]
	and	#$FF
	jeq	doneput
	if2	@a,ne,#'b',esc02
	ldx	#1
	jsr	moveleft
	bra	didit
esc02	if2	@a,ne,#'f',esc03
	jsr	clearscrn
	bra	didit
esc03	if2	@a,ne,#'n',esc04
	lda	#13
	bra	putit
esc04	if2	@a,ne,#'r',esc05
	lda	#13
	bra	putit
esc05	if2	@a,ne,#'t',esc06
	lda	#9
	bra	putit
esc06	if2	@a,ne,#'0',putit
	stz	val
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
	inc	ptr
	bra	escloop
putval	lda	val
putit	jsr	putchar
didit	inc	ptr
	jmp	putloop
doneput	lda	#' '
	jsr	putchar
	jmp	loop

done	lda	nl
	bne	exit
	jsr	newline

exit	jsr	flush
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

	lda	argc
	dec	a
	beq	wait

	ldx	#^Usage
	lda	#Usage
	jsr	errputs
	bra	exit

wait	lock	pwdmutex	

	jsl	alloc256
	sta	gpptr
	stx	gpptr+2
	sta	ptr
	stx	ptr+2

	lda	#256
	sta	[ptr]

	GetPrefix gpparm
	bcc	ok

awshit	sta	err
	Error err
	bra	done

ok	ldy	#2
	lda	[ptr],y
	xba
	sta	[ptr],y

	ldx	ptr+2
	lda	ptr
	clc
	adc	#3
	jsr	putp
	jsr	newline

done	ldx	ptr+2
	lda	ptr
	jsl	free256

	unlock pwdmutex

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

pwdmutex	key

gpparm	dc	i'2'
	dc	i'0'
gpptr	ds	4

err	ds	2

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
chkbuiltin     pei   (file+2)
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
tryhash        pei   (file+2)
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
	jsl	alloc256
	sta	ptr
	stx	ptr+2
	sta	gpptr
	stx	gpptr+2
	lda	#256
	sta	[ptr]
	GetPrefix gpparm
	ldy	#2
	lda	[ptr],y
	xba
	sta	[ptr],y

	ldx	ptr+2
	add2	ptr,#3,@a
	jsr	putp
	ldx	file+2
	lda	file
	jsr	puts
	ldx	ptr+2
	lda	ptr
	jsl	free256
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

gpparm	dc	i'2'
	dc	i'0'
gpptr	ds	4

GRec	dc	i'4'
GRecPath	ds	4
	ds	2
GRecFileType   ds    2
GRecAuxType    ds    4

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
space	equ	numstr+4
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

	lda	argc
	dec	a
	beq	showall
	dec	a
	jeq	showone
	dec	a
	jeq	setprefix

	ldx	#^usage
	lda	#usage
	jsr	errputs
	jmp	done

showall	jsl	alloc256
	sta	PRecPath
	stx	PRecPath+2
	sta	dir
	stx	dir+2
	lda	#254
	sta	[dir]
	
	ld2	1,PRecNum
	GetBootVol PRecNum
	jcs	awshit
	ldx	#^bootstr
	lda	#bootstr
	jsr	puts
	ldy	#2
	lda	[dir],y
	xba
	sta	[dir],y
	ldx	dir+2
	lda	dir
	inc2	a
	inc	a
	jsr	putp
	jsr	newline

	stz	PRecNum
allloop	GetPrefix PRec
	jcs	awshit
	ldy	#2
	lda	[dir],y
	beq	nextall
	xba
	sta	[dir],y
	Int2Dec (PRecNum,#pfxstr,#2,#0)
	ldx	#^pfxstr
	lda	#pfxstr
	jsr	puts
	ldx	dir+2
	lda	dir
	inc2	a
	inc	a
	jsr	putp
	jsr	newline

nextall	inc	PRecNum
	if2	PRecNum,cc,#32,allloop
	jmp	finish
		    
showone	ldy	#6
	lda	[argv],y
	sta	numstr+2
	pha
	dey2
	lda	[argv],y
	sta	numstr
	pha
	jsr	cstrlen
	tax

	Dec2Int (numstr,@x,#0),PRecNum

	jsl	alloc256
	sta	PRecPath
	stx	PRecPath+2
	sta	dir
	stx	dir+2
	lda	#254
	sta	[dir]
	GetPrefix PRec
	bcs	awshit
	ldy	#2
	lda	[dir],y
	xba
	sta	[dir],y
	ldx	dir+2
	lda	dir
	inc2	a
	inc	a
	jsr	putp
	jsr	newline
	lda	PRecPath
	ldx	PRecPath+2
	jsl	free256
	
	bra	done

setprefix      ldy   #6
	lda	[argv],y
	sta	numstr+2
	pha
	dey2
	lda	[argv],y
	sta	numstr
	pha
	jsr	cstrlen
	tax

	Dec2Int (numstr,@x,#0),PRecNum

	ldy	#2*4+2
	lda	[argv],y
	pha
	dey2
	lda	[argv],y
	pha
	jsr	c2gsstr
	sta	PRecPath
	sta	GRecPath
	stx	PRecPath+2
	stx	GRecPath+2

	GetFileInfo GRec
	bcc	okay
awshit	ldx	#^errorstr
	lda	#errorstr
	jsr	errputs
	bra	finish

okay	SetPrefix PRec
	bcs	awshit

finish	ph4	PRecPath
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

PRec	dc	i'2'
PRecNum	dc	i'0'
PRecPath	ds	4

GRec	dc	i'2'
GRecPath	ds	4
GRecAcc	ds	2

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
* DF: builtin command
* syntax: df
*
* displays volumes and free space
*
**************************************************************************

df	START

	using	FSTData

space	equ	1
argc	equ	space+3
argv	equ	argc+2
end	equ	argv+4

	tsc
	phd
	tcd

	lock	mutex

	lda	argc
	dec	a
	beq	showall

	ldx	#^Usage
	lda	#Usage
	jsr	errputs
	bra	exit

showall	ldx	#^hdr
	lda	#hdr
	jsr	puts
	ld2	1,DIDevNum
allloop	DInfo	DIParm
	bcs	exit
	jsr	showdev
	inc	DIDevNum
	bra	allloop

exit	unlock mutex
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

showdev        lda	#'.'
	jsr	putchar
	lda	#'d'
	jsr	putchar
	lda	DIdevnum
	cmp	#10
	bcs	dev10
	clc
	adc	#'0'
	jsr	putchar
	lda	#' '
	jsr	putchar
	bra	endnum
dev10          UDivide (DIdevnum,#10),(@a,@x)
	phx
	clc
	adc	#'0'
	jsr	putchar
	pla
	clc
	adc	#'0'
	jsr	putchar
endnum	lda	#' '
	jsr	putchar

	Volume VolParm
	jcc	okdev
	lda	DIid
	cmp	#$20
	bcc	okdid
	lda	#0
okdid	asl2	a
	tay
	ldx	idtbl+2,y
	lda	idtbl,y
	jsr	puts
	short	a
	ldy	devname
	lda	#' '
dev5	cpy	#17
	bcs	dev6
	sta	devname+2,y
	iny
	bra	dev5
dev6	lda	#16
	sta	devname+1
	long	a
	ldx	#^devname+1
               lda	#devname+1
	jsr	putp
	lda	#' '
	jsr	putchar
	
	jmp	newline

okdev	ldy	volname
	short	a
	lda	#' '
dev1	cpy	#17
	bcs	dev2
	sta	volname+2,y
	iny
	bra	dev1
dev2	lda	#16
	sta	volname+1
	ldy	devname
	lda	#' '
dev3	cpy	#17
	bcs	dev4
	sta	devname+2,y
	iny
	bra	dev3
dev4	lda	#16
	sta	devname+1
	long	a

	ldx	#^volname+1
	lda	#volname+1
	jsr	putp
	lda	#' '
	jsr	putchar
	ldx	#^devname+1
	lda	#devname+1
	jsr	putp
	lda	#' '
	jsr	putchar
	Long2Dec (VolFree,#numbuf,#7,#0)
	ldx	#^numbuf
	lda	#numbuf
	jsr	puts
	Long2Dec (VolTot,#numbuf,#7,#0)
	ldx	#^numbuf
	lda	#numbuf
	jsr	puts
;
; [(total - free) * 100] / total
;
	lda	VolFree
	ora	VolFree+2
               beq   put100
	clc			;why clc, need to investigate :)
	lda	VolTot
	sbc	VolFree
	tax
	lda	VolTot+2
	sbc	VolFree+2
	LongMul (@ax,#100),(@ax,@y)
	LongDivide (@xa,VolTot),(@ax,@y)
	Long2Dec (@xa,#capbuf+3,#3,#0)
	ldx	#^capbuf
	lda	#capbuf
	jsr	puts
	bra	putsys

put100	ldx	#^cap100buf
	lda	#cap100buf
	jsr	puts
	             
putsys	lda	VolSysID
	cmp	#$E
	bcc	oksys
	lda	#0
oksys	asl2	a
	tay
	ldx	FSTtable+2,y
	lda	FSTtable,y
	jsr	puts
	jmp	newline

hdr  dc c'.d## Volume           Device           Free    Total   Capacity  System',h'0d'
     dc c'---- ---------------- ---------------- ------- ------- --------  -----------',h'0d00'

Usage	dc	c'Usage: df',h'0d00'
numbuf	dc	c'        ',h'00'
capbuf	dc	c'      %   ',h'00'
cap100buf      dc    c'   100%   ',h'00'

mutex	key

DIParm	dc	i2'8'
DIDevNum	ds	2
	dc	a4'devbuf'
	dc	i2'0'
	dc	i4'0'
	dc	i2'0'
	dc	i2'0'
	dc	i2'0'
DIid	dc	i2'0'

VolParm	dc	i2'5'
	dc	a4'devname'
	dc	a4'volbuf'
VolTot	ds	4
VolFree	ds	4
VolSysID	ds	2

devbuf	dc	i'35'
devname	ds	33
volbuf	dc	i'260'
volname	ds	258

idtbl	dc	a4'id00,id01,id02,id03,id04,id05,id06,id07,id08'
	dc	a4'id09,id0a,id0b,id0c,id0d,id0e,id0f,id10,idff'
	dc	a4'id12,id13,id14,id15,id16,id17,id18,id19,id1a'
	dc	a4'id1b,id1c,id1d,id1e,id1f'
	dc	a4'idff'

idff	dc	c'<unknown>        ',h'00'
id00	dc	c'Apple 5.25 Drive ',h'00'
id01	dc	c'Profile 5MB      ',h'00'
id02	dc	c'Profile 10MB     ',h'00'
id03	dc	c'Apple 3.5 Drive  ',h'00'
id04	dc	c'SCSI             ',h'00'
id05	dc	c'SCSI Hard Drive  ',h'00'
id06           dc	c'SCSI Tape Drive  ',h'00'
id07	dc	c'SCSI CD-ROM      ',h'00'
id08           dc	c'SCSI Printer     ',h'00'
id09           dc	c'Serial Modem     ',h'00'
id0a           dc	c'Console Driver   ',h'00'
id0b	dc	c'Serial Printer   ',h'00'
id0c	dc	c'Serial LaserWrit ',h'00'
id0d	dc	c'AppleTalk LaserW ',h'00'
id0e	dc	c'RAM Disk         ',h'00'
id0f	dc	c'ROM Disk         ',h'00'
id10	dc	c'File Server      ',h'00'
id12	dc	c'Apple Desktop Bu ',h'00'
id13	dc	c'Hard Drive       ',h'00'
id14	dc	c'Floppy Drive     ',h'00'
id15	dc	c'Tape Drive       ',h'00'
id16	dc	c'Character dev dr ',h'00'
id17	dc	c'MFM-encoded      ',h'00'
id18	dc	c'AppleTalk net    ',h'00'
id19           dc	c'Sequential dev   ',h'00'
id1a           dc	c'SCSI Scanner     ',h'00'
id1b          	dc	c'Scanner          ',h'00'
id1c           dc	c'LaserWriter SC   ',h'00'
id1d	dc	c'AppleTalk Main   ',h'00'
id1e	dc	c'AppleTalk fsd    ',h'00'
id1f	dc	c'AppleTalk RPM    ',h'00'

	END

**************************************************************************
*
* FST descriptions
*
**************************************************************************

FSTData	DATA

FSTtable	dc	a4'fst0,fst1,fst2,fst3,fst4,fst5,fst6,fst7,fst8'
	dc	a4'fst0,fsta,fstb,fstc,fstd'

fst0	dc	c'[Unknown]',h'00'
fst1	dc	c'ProDOS',h'00'
fst2	dc	c'DOS 3.3',h'00'
fst3	dc	c'DOS 3.2',h'00'
fst4	dc	c'Pascal',h'00'
fst5	dc	c'MFS',h'00'
fst6	dc	c'HFS',h'00'
fst7	dc	c'Lisa (get real)',h'00'
fst8	dc	c'CP/M',h'00'
fsta	dc	c'MS-DOS',h'00'
fstb	dc	c'High Sierra',h'00'
fstc	dc	c'ISO 9660',h'00'
fstd	dc	c'AppleShare',h'00'

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

	inc	exitamundo

	return

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
	ldx   mode
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
return	return

findflag	inc	arg
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
	dc	h'00'

errstr	dc	c': Unknown flag',h'0d0d00'

nametbl	dc	a4'str01,str02,str03,str04,str05,str06,0'
str01	dc	c'gsostrace',h'00'
str02	dc	c'pathtrace',h'00'
str03	dc	c'gsoserrors',h'00'
str04	dc	c'sigtrace',h'00'
str05	dc	c'systrace',h'00'
str06	dc	c'gsosblocks',h'00'

bittbl	dc	i2'%000001'
	dc	i2'%000010'
	dc	i2'%000100'
	dc	i2'%001000'
	dc	i2'%010000'
	dc	i2'%100000'

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

return	return

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

	ph2	t_size
	jsl	sv_alloc
	sta	sv
	stx	sv+2

	lda	hash_table
	ora	hash_table+2
	beq	exit
	mv4	hash_table,p
	lda	hash_numexe
	beq	doneadd
	ldy	#0
	ldx	t_size
	beq	doneadd
; 
; loop through every hashed file and add it the string vector
;
addloop	lda	[p],y
	sta	q
	iny
	iny
	lda	[p],y
	sta	q+2
	iny
	iny
	ora	q
	beq	skip
	phy
	phx	
	pei	(sv+2)
	pei	(sv)
	pei	(q+2)
	lda	q
	inc	a
	inc	a
	pha
	pea	1
	jsl	sv_add
	plx
	ply
skip	dex
	bne	addloop
doneadd	anop

doneprint	pei	(sv+2)
	pei	(sv)
	jsl	sv_sort
	pei	(sv+2)
	pei	(sv)
	jsl	sv_colprint			

	pei	(sv+2)
	pei	(sv)
	jsl	sv_dispose

exit	return

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

	stz	retval

	dec	argc
	bne	ok

	ldx	#^usage
	lda	#usage
	jsr	errputs
	lda	#1
	sta	retval
	bra	exit

ok	add2	argv,#4,argv

	ldy	#2
	lda	[argv],y
	pha
	lda	[argv]
	pha
	pei	(argc)
	pei	(argv+2)
	pei	(argv)
	pea	0
	jsl	ShellExec
	sta	retval

exit	return 2:retval

usage	dc	c'usage: source file [arguments...]',h'0d00'

	END
                  
**************************************************************************
*
* COMMANDS: builtin command
* syntax: hash
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

doneprint	pei	(sv+2)
	pei	(sv)
	jsl	sv_sort
	pei	(sv+2)
	pei	(sv)
	jsl	sv_colprint			

	pei	(sv+2)
	pei	(sv)
	jsl	sv_dispose

exit	return

	END
