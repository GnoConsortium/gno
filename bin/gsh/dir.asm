**************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*
* $Id: dir.asm,v 1.7 1998/10/26 17:04:50 tribby Exp $
*
**************************************************************************
*
* DIR.ASM
*   By Tim Meekins
*   Modified by Dave Tribby for GNO 2.0.6
*
* Directory stack management
*
* Note: text set up for tabs at col 16, 22, 41, 49, 57, 65
*              |     |                  |       |       |       |
*	^	^	^	^	^	^	
**************************************************************************
*
* Interfaces defined in this file:
*
* InitDStack	
*
* dirs	built-in command
*
* pushd	built-in command
*
* popd	built-in command
*
* path2tilde	
*
* getpfxstr	
*
**************************************************************************

	mcopy /obj/gno/bin/gsh/dir.mac

dummydir	start		; ends up in .root
	end

	setcom 60

MAXD	gequ	50


**************************************************************************
*
* Initialize directory stack
*
**************************************************************************

InitDStack	START

	using	DirData

	stz	tods
	stz	dirstack
	stz	dirstack+2

	rts

	END		   

**************************************************************************
*
* DIRS: builtin command
* syntax: dirs [-l]
*
* display the directory stack
*
**************************************************************************

dirs	START

	using	DirData

arg	equ	0
space	equ	arg+4

	subroutine (4:argv,2:argc),space

	lda	argc
	dec	a
	beq	showshort
	dec	a
	bne	using

	ldy	#4
	lda	[argv],y
	sta	arg
	ldy	#6
	lda	[argv],y
	sta	arg+2
	lda	[arg]
	and	#$FF
	cmp	#'-'
	bne	using
	ldy	#1
	lda	[arg],y
	cmp	#'l'
	beq	showlong

using	ldx	#^usingstr
	lda	#usingstr
	jsr	errputs
	bra	exit

showlong	jsl	dotods	Set top of stack to current directory.
	pea	0
	jsl	showdir
	bra	exit

showshort	jsl	dotods	Set top of stack to current directory.
	pea	1
	jsl	showdir

exit	return 2:#0

usingstr	dc	c'usage: dirs [-l]',h'0d00'

	END

**************************************************************************
*
* PUSHD: builtin command
* syntax: pushd [+n | dir]
*
* change directory and push
*
**************************************************************************

pushd	START

	using	DirData
	using	vardata

count	equ	0
p	equ	count+2
arg	equ	p+4
space	equ	arg+4

	subroutine (4:argv,2:argc),space

	lda	argc	Get number of arguments.
	dec	a	If no parameters,
	beq	xchange	 exchange top two dirs on stack.
	dec	a	If > 1 parameter,
	bne	usage	 print usage string.
	
	ldy	#4	Move parameter pointer from
	lda	[argv],y	 argv array to direct
	sta	arg	  page variable "arg".
	ldy	#6
	lda	[argv],y
	sta	arg+2

	lda	[arg]	If first character
	and	#$FF	 of parameter is
	cmp	#'+'	  "+",
	beq	rotate		do the rotate;
	jmp	godir		else push the directory.

;
; More than one parameter provided; print usage string and exit.
;
usage	ldx	#^usagestr
	lda	#usagestr
	jmp	prerrmsg

;
; Either no parameter or "+" provided; exchange directory stack entries.
;
xchange	lda	tods	Get index to top of dir stack.
	bne	xgoodie	If 0,
	ldx	#^err1		print error message
	lda	#err1		"No other directory"
	jmp	prerrmsg

xgoodie	jsl	dotods	Set top of stack to current directory.
	lda	tods
	dec	a
	asl	a
	asl	a
	tax		X = offset to tos-1.
	lda	tods
	asl	a
	asl	a
	tay		Y = offset to tos.
	lda	dirstack,x	Swap tos-1 and tos.
	pha
	lda	dirstack,y
	sta	dirstack,x
	pla
	sta	dirstack,y
	lda	dirstack+2,x
	pha
	lda	dirstack+2,y
	sta	dirstack+2,x
	pla
	sta	dirstack+2,y
	jmp	gototop	chdir to the new tos.

;
; Parameter = +n; do the rotate
;
rotate	add4	arg,#1,p
	pei	(p+2)
	pei	(p)
	jsr	cstrlen
	tax
	Dec2Int (p,@x,#0),@a	Convert parameter to decimal
	sta	count	 and store in count.
	cmp	#0	If parameter is 0
	beq	badnum	 or negative,
	bmi	badnum	  report "invalid number".
	cmp	tods	If count >= tos,
	beq	rotloop
	bcc	rotloop

roterr	ldx	#^err2	   Print error message:
	lda	#err2	    Directory stack not that deep
	jmp	prerrmsg

badnum	ldx	#^errbadnum	Print error message:
	lda	#errbadnum	  Invalid number
	jmp	prerrmsg

;
; Loop to rotate entries in directory stack
;
rotloop	lda	tods
	dec	a
	asl	a
	asl	a
	tay
	lda	dirstack+6,y
	pha
	lda	dirstack+4,y
	pha
rotloop2	lda	dirstack,y
	sta	dirstack+4,y
	lda	dirstack+2,y
	sta	dirstack+6,y
	cpy	#0
	beq	nextrot
	dey
	dey
	dey
	dey
	bra	rotloop2
nextrot	pla
	sta	dirstack
	pla
	sta	dirstack+2
	dec	count
	bne	rotloop
;
; chdir to the top-of-stack directory
;
gototop	lda	tods
	asl	a
	asl	a
	tay
	lda	dirstack+2,y
	pha
	lda	dirstack,y
	pha
	jsl	gotodir
	bra	done	All done.


;
; Parameter = directory name; add it to the stack
;
godir	anop
	lda	tods
	cmp	#MAXD-1	If index >= maximum,
	bcc	stackok
	ldx	#^errfull		print error message.
	lda	#errfull
prerrmsg	jsr	errputs
	bra	exit
stackok	anop
	jsl	dotods	Set top of stack to current directory.
	pei	(arg+2)
	pei	(arg)
	jsl	gotodir	chdir to the parameter directory.
	bne	exit

	inc	tods	Bump the top of stack pointer.
	lda	tods
	asl	a
	asl	a
	tay
	lda	#0
	sta	dirstack,y
	sta	dirstack+2,y
	jsl	dotods	Set top of stack to current directory.

done	lda	varpushdsil	If $PUSHDSILENT not defined,
	bne	exit
	pea	1
	jsl	showdir	  show the directory stack.

exit	return 2:#0

usagestr	dc	c'usage: pushd [+n | dir]',h'0d00'
err1	dc	c'pushd: No other directory',h'0d00'
err2	dc	c'pushd: Directory stack not that deep',h'0d00'
errfull	dc	c'pushd: Directory stack full',h'0d00'
errbadnum	dc	c'pushd: Invalid number',h'0d00'


	END

**************************************************************************
*
* POPD: builtin command
* syntax: popd [+n]
*
* pop a directory from stack and cd to it
*
**************************************************************************

popd	START

	using	DirData
	using	vardata

count	equ	0
arg	equ	count+2
space	equ	arg+4

	subroutine (4:argv,2:argc),space

	lda	argc
	dec	a
	jeq	noarg
	dec	a
	bne	using
	ldy	#4
	lda	[argv],y
	sta	arg
	ldy	#6
	lda	[argv],y
	sta	arg+2
	lda	[arg]
	and	#$FF
	cmp	#'+'
	beq	plus

using	ldx	#^usingstr
	lda	#usingstr
	jsr	errputs
	jmp	exit

plus	add4	arg,#1,arg
	pei	(arg+2)
	pei	(arg)
	jsr	cstrlen
	tax
	Dec2Int (arg,@x,#0),@a
	sta	count
	cmp	#0
	beq	noarg
	lda	tods
	beq	pluserr
	lda	count
	cmp	tods
	beq	doplus
	bcc	doplus

pluserr	ldx	#^err2
	lda	#err2
	jsr	errputs
	bra	exit

doplus	jsl	dotods	Set top of stack to current directory.
	sub2	tods,count,@a
	asl	a
	asl	a
	tax
	phx
	lda	dirstack+2,x
	pha
	lda	dirstack,x
	pha
	jsl	nullfree
	plx
plusloop	lda	dirstack+4,x
	sta	dirstack,x
	lda	dirstack+6,x
	sta	dirstack+2,x
	inx4
	dec	count
	bne	plusloop
	dec	tods

	bra	gototop

noarg	lda	tods
	bne	noarg0

	ldx	#^err1
	lda	#err1
	jsr	errputs
	bra	exit

noarg0	lda	tods
	asl	a
	asl	a
	tay
	lda	dirstack+2,y
	pha
	lda	dirstack,y
	pha
	jsl	nullfree
	dec	tods

gototop	lda	tods
	asl	a
	asl	a
	tay
	lda	dirstack+2,y
	pha
	lda	dirstack,y
	pha
	jsl	gotodir

	lda	varpushdsil
	bne	exit

	pea	1
	jsl	showdir

exit	return 2:#0

usingstr	dc	c'Usage: popd [+n]',h'0d00'
err1	dc	c'popd: Directory stack empty',h'0d00'
err2	dc	c'popd: Directory stack not that deep',h'0d00'

	END

**************************************************************************
*
* Set prefix 0 to the passed c string
*
**************************************************************************

gotodir	PRIVATE

retval	equ	0
space	equ	retval+2

	subroutine (4:dir),space

	stz	retval

	pei	(dir+2)
	pei	(dir)
	jsr	c2gsstr
	sta	PRecPath
	sta	GRecPath
	stx	PRecPath+2
	stx	GRecPath+2

	lock	mutex

	GetFileInfo GRec
	bcc	ok
ohshit	sta	ErrError
	ErrorGS Err
	inc	retval
	bra	done

ok	if2	GRecFT,eq,#$F,ok2
	ldx	dir+2
	lda	dir
	jsr	errputs
	ldx	#^direrr
	lda	#direrr
	jsr	errputs
	inc	retval
	bra	done

ok2	SetPrefix PRec
	bcs	ohshit

done	ph4	PRecPath
	jsl	nullfree

	unlock mutex

	return 2:retval

mutex	key

; Parameter block for GS/OS SetPrefix call
PRec	dc	i'2'	pCount
PRecNum	dc	i'0'	prefixNum (0 = current directory)
PRecPath	ds	4	Pointer to input prefix path

; Parameter block for GS/OS GetFileInfo call
GRec	dc	i'3'	pCount
GRecPath	ds	4	Pointer to input pathname
GRecAcc	ds	2	access (result)
GRecFT	ds	2	fileType (result)

; Parameter block for shell ErrorGS call (p 393 in ORCA/M manual)
Err	dc	i2'1'	pCount
ErrError	ds	2	Error number

dirErr	dc	c': Not a directory',h'0d00'
		       
	END
	
**************************************************************************
*
* Display the directory stack
*
**************************************************************************

showdir	PRIVATE

	using	DirData

idx	equ	0
space	equ	idx+2

	subroutine (2:flag),space

	lda	tods	Get directory stack index.
	asl	a	Multiply by four to
	asl	a	 get byte offset (idx).
	sta	idx

loop	lda	flag	If parameter == 1,
	beq	long

	ldy	idx
	lda	dirstack+2,y		print entry
	pha
	lda	dirstack,y
	pha
	jsl	path2tilde		 but first substitute "~"
	phx			  for home directory.
	pha
	jsr	puts
	jsl	nullfree
	bra	next

long	ldy	idx	else,
	lda	dirstack+2,y		print full entry.
	tax
	lda	dirstack,y
	jsr	puts

next	lda	#' '	Print a space.
	jsr	putchar

	lda	idx	If idx != 0,
	beq	done
	sub2	idx,#4,idx		idx = idx -4
	bra	loop		handle next entry.

done	jsr	newline

	return

	END

**************************************************************************
*
* Set the top of the stack to the current directory
*
**************************************************************************

dotods	PRIVATE

	using	DIRDATA

p	equ	0
idx	equ	p+4
space	equ	idx+2

	subroutine (0:dummy),space

	lda	tods	Get index number.
	asl	a	Multiply index by four
	asl	a	 to get byte offset.
	sta	idx	Store in idx
	tay		 and Y-register.
	lda	dirstack,y	If there is an address
	ora	dirstack+2,y	 in this position,
	beq	setit
	
	lda	dirstack+2,y
	pha
	lda	dirstack,y
	pha
	jsl	nullfree		free it.

setit	lock	mutex

	pea	0
	jsl	getpfxstr	Get value of prefix 0.
	sta	p
	stx	p+2

	ora	p+2	If NULL pointer returned,
	beq	done	 an error was reported.

	ldy	#2	If length of returned
	lda	[p],y	 GS/OS string is 0,
	bne	ok	  an error was reported.
	
	ph4	p	Free the buffer.
	jsl	nullfree

	bra	done

;
; Move text in GS/OS result buffer to beginning of buffer
; (overwritting the two length words).
;
ok	clc		Source is result
	lda	p	 buffer plus
	adc	#4	  four bytes.
	tay
	lda	p+2
	adc	#0
	pha
	phy
	pei	(p+2)	Destination is first
	pei	(p)	 byte of buffer.
	jsr	copycstr

	ldy	idx	Store address of string
	lda	p	 in current position
	sta	dirstack,y	  of directory stack.
	lda	p+2
	sta	dirstack+2,y

done	unlock mutex

	return
		
mutex	key

	END

**************************************************************************
*
* Directory stack data
*
**************************************************************************

DirData	DATA

dirstack	ds	MAXD*4
tods	dc	i'0'

	END

**************************************************************************
*
* Replace $HOME with a '~' in string
*
**************************************************************************

path2tilde	START
	
ptr	equ	0
newpath	equ	ptr+4
home	equ	newpath+4
space	equ	home+4

	subroutine (4:path),space

	pei	(path+2)	Get length of
	pei	(path)	 path string
	jsr	cstrlen	  parameter.
	inc2	a	Add 2, and allocate
	pea	0	 memory for result string.
	pha
	~NEW
	sta	newpath
	stx	newpath+2
	sta	ptr
	stx	ptr+2

	ph4	#homename	Get $HOME environment variable.
	jsl	getenv
	sta	home
	stx	home+2
	ora	home+2	If buffer wasn't allocated
	jeq	notfound2	  cannot search for $HOME.

	ldy	#2	Get result length word.
	lda	[home],y
	beq	notfound2	If 0, just copy the rest.
	tax		Use X to count down HOME chars.
	ldy	#0	path index is based from 0.
checkhome	lda	[path],y
	and	#$FF	Isolate character in parameter,
	beq	notfound2	 checking for end of string,
	jsr	tolower	  converting to lower-case
	jsr	toslash	   and changing ":" to "/".
	pha		Hold on stack for comparison.
	iny4		$home has 4 bytes of length info
	lda	[home],y	 that need to be indexed over.
	dey2		Take back 3 of the offset,
	dey		 nudging Y ahead by 1.
	and	#$FF	Isolate $home character,
	jsr	tolower	 converting to lower-case
	jsr	toslash	  and changing ":" to "/".
	cmp	1,s	If the parameter character !=,
	bne	notfound	 there is no match.
	pla		Pop the parameter character off stack.
	dex		Decrement $home length counter.
	bne	checkhome	If more, stay in loop.

;
; First part of parameter matched $HOME
;
	cmp	#'/'	This char = "/"?
	beq	found	 yes -- it's a match.
	lda	[path],y	If the following character
	and	#$FF	 is zero (end of string),
	beq	found
	jsr	toslash	  '/', or ':', we have a match.
	cmp	#'/'
	bne	notfound2
found	lda	#'~'	Store '~' as first character
	sta	[ptr]	 in result buffer, and bump
	incad	ptr	  result pointer.
	bra	copyrest

;
; First part of parameter does not match $HOME
;     
notfound	pla		Get rid of comparison value on stack.
notfound2	ldy	#0	Not found: copy from beginning.

;
; Copy remainder of parameter (Y-reg marks start) to destination string
;
copyrest	short	a
copyloop	lda	[path],y
	beq	endcopy
	cmp	#':'
	bne	copyput
	lda	#'/'
copyput	sta	[ptr]
	long	a
	incad	ptr
	short	a
	iny
	bra	copyloop
endcopy	sta	[ptr]
	long	a
	dec	ptr	If final character
	lda	[ptr]	 was "/",
	cmp	#'/'
	bne	skipshorten
	lda	#0		obliterate it.
	sta	[ptr]

skipshorten	pei	(home+2)	Free memory allocated
	pei	(home)	 for the value of $HOME.
	jsl	nullfree

	return 4:newpath

homename	gsstr	'home'	Env variable name

	END


**************************************************************************
*
* Return pointer to current directory (\0) GS/OS string in a/x
*
**************************************************************************

getpfxstr	START
	
p	equ	0
space	equ	p+4

	subroutine (2:pnum),space

	lock	mutex

; Use dummy GS/OS result buf to get length of pathname
;
	lda	pnum	Put prefix num into
	cmp	#$FFFF	If it's $FFFF,
	beq	doboot	 use GetBootVol, not GetPrefix.
	
	sta	gpnum	Store prefix num in parameter block.
	ld4	TempResultBuf,gppath
	GetPrefix gpparm
	bra	chklen

doboot	ld4	TempResultBuf,gbpath
	GetBootVol gbparm

chklen	lda	TempRBlen	Use that length
	clc		 plus five (for
	adc	#5	  len words and terminator)
	pea	0	   to allocate memory
	pha		    that holds the string.
	~NEW
	sta	p	Store result in
	stx	p+2	 direct page pointer.

	ora	p+2	If memory was not available,
	bne	memok
	lda	#0201	  report memory error and
	bra	rpterr	   return NULL to user.


memok	lda	TempRBlen	Store result buf
	inc2	a	 length at start
	inc2	a	  of buffer.
	sta	[p]

	tay		Store a null byte
	short	a	 at the end of the
	lda	#0	  string so it can
	sta	[p],y	   be used as a c-string.
	long	a
;
; Get the prefix string into the newly allocated buffer.
;
	lda	pnum	Prefix number tells
	cmp	#$FFFF	 whether to use
	beq	doboot2	  GetPrefix or GetBootVol.
	
	mv4	p,gppath
	GetPrefix gpparm
	bcs	rpterr
	bra	done

doboot2	mv4	p,gbpath
	GetBootVol gbparm
	bcc	done	If there was an error,

rpterr	sta	errError		Save the value
	ErrorGS err		 and report it.
	ldy	#2		Set length of returned
	lda	#0		 string to 0 so caller
	sta	[p],y		  can detect error condition.

done	unlock mutex

;
; Return pointer to caller. (Caller has responsibility to deallocate.)
;
	return 4:p
		
mutex	key

;
; Parameter block for GetPrefix GS/OS call
;
gpparm	dc	i2'2'	pCount
gpnum	dc	i2'0'	prefixNum (from parameter)
gppath	dc	i4'0'	prefix returned to this GS/OS buffer.

;
; Parameter block for GetBootVol GS/OS call
;
gbparm	dc	i2'1'	pCount
gbpath	dc	i4'0'	prefix returned to this GS/OS buffer.

;
; GS/OS result buffer for getting the full length of the prefix string
;
TempResultBuf	dc	i2'5'	Only five bytes total.
TempRBlen	ds	2	String's length returned here.
	ds	1	Only 1 byte for value.

;
; Parameter block for shell ErrorGS call (p 393 in ORCA/M manual)
;
err	dc	i2'1'	pCount
errError	ds	2	Error number

	END
