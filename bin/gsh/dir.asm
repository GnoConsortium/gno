**************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*
**************************************************************************
*
* DIR.ASM
*   By Tim Meekins
*
* Directory stack management
*
**************************************************************************

               keep  o/dir
               mcopy m/dir.mac

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

showlong	jsl	dotods
	pea	0
	jsl	showdir
	bra	exit

showshort	jsl	dotods
	pea	1
	jsl	showdir

exit	return

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

	lda	argc
	dec	a
	beq	xchange
	dec	a
	bne	usage
	
	ldy	#4
	lda	[argv],y
	sta	arg
	ldy	#6
	lda	[argv],y
	sta	arg+2
	lda	[arg]
	and	#$FF
	cmp	#'+'
	beq	rotate
	jmp	godir

usage	ldx	#^usagestr
	lda	#usagestr
	jsr	errputs
	jmp	exit

xchange	lda	tods	
	bne	xgoodie
	ldx	#^err1
	lda	#err1
	jsr	errputs
	jmp	exit
xgoodie	jsl	dotods
	lda	tods
	dec	a
	asl	a
	asl	a
	tax
	lda	tods
	asl	a
	asl	a
	tay
	lda	dirstack,x
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
	jmp	gototop
                               
rotate	add4	arg,#1,p
	pei	(p+2)
	pei	(p)
	jsr	cstrlen
	tax
	Dec2Int (p,@x,#0),@a
	sta	count
	cmp	#0
	beq	godir
	lda	tods
	beq	roterr
	lda	count
	cmp	tods
	beq	rotloop
	bcc	rotloop

roterr	ldx	#^err2
	lda	#err2
	jsr	errputs
	jmp	exit

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
	bra	gototop

godir	jsl	dotods
               pei	(arg+2)
	pei	(arg)
	jsl	gotodir
	bne	exit

	inc	tods
	lda	tods
	asl	a
	asl	a
	tay
	lda	#0
	sta	dirstack,y
	sta	dirstack+2,y
	jsl	dotods
	bra	done

gototop	lda	tods
	asl	a
	asl	a
	tay
	lda	dirstack+2,y
	pha
	lda	dirstack,y
	pha
	jsl	gotodir

done	lda	varpushdsil
	bne	exit
	pea	1
	jsl	showdir

exit	return

usagestr	dc	c'usage: pushd [+n | dir]',h'0d00'
err1	dc	c'pushd: No other directory',h'0d00'
err2	dc	c'pushd: Directory stack not that deep',h'0d00'

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

doplus	jsl	dotods
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

exit	return    

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

      	pei   (dir+2)
	pei	(dir)
	jsr	c2gsstr
	sta	PRecPath
	sta	GRecPath
	stx	PRecPath+2
	stx	GRecPath+2

	lock	mutex

	GetFileInfo GRec
	bcc	ok
ohshit	sta	Err
	Error Err
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

PRec	dc	i'2'
PRecNum	dc	i'0'
PRecPath	ds	4

GRec	dc	i'3'
GRecPath	ds	4
GRecAcc	ds	2
GRecFT	ds	2

Err	ds	2
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

	lda	tods
	asl	a
	asl	a
	sta	idx

loop	lda	flag
	beq	long
	ldy	idx
	lda	dirstack+2,y
	pha
	lda	dirstack,y
	pha
	jsl	path2tilde
	phx
	pha
	jsr	puts
	jsl	nullfree
               bra	next
long	ldy	idx
	lda	dirstack+2,y
	tax
	lda	dirstack,y
	jsr	puts
next	lda	#' '
	jsr	putchar
	lda	idx
	beq	done
	sub2	idx,#4,idx
	bra	loop

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

	lda	tods
	asl	a
	asl	a
	sta	idx
	tay
	lda	dirstack,y
	ora	dirstack+2,y
	beq	setit
	
	lda	dirstack+2,y
	pha
	lda	dirstack,y
	pha
	jsl	nullfree

setit	lock	mutex
	jsl	alloc256
	sta	gppath
	stx	gppath+2
	sta	p
	stx	P+2
	lda	#254
	sta	[p]
	GetPrefix gpparm
	ldy	#2
	lda	[p],y
	xba
	sta	[p],y
	add4	p,#3,p
	pei	(p+2)
	pei	(p)
	jsr	p2cstr
	sta	p
	stx	p+2
	ldx	gppath+2
	lda	gppath
	jsl	free256

	unlock mutex

	ldy	idx
	lda	p
	sta	dirstack,y
	lda	p+2
	sta	dirstack+2,y

	return
		
mutex	key

gpparm	dc	i2'2'
	dc	i2'0'
gppath	dc	i4'0'

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

	pei	(path+2)
	pei	(path)
	jsr	cstrlen
	inc2	a
	pea	0
	pha
	jsl	~NEW
	sta	newpath
	stx	newpath+2
	sta	ptr
	stx	ptr+2

	jsl	alloc256
	sta	home
	stx	home+2
	sta	varparm+4
	stx	varparm+6
	Read_Variable varparm

	ldy	#0
	lda	[home]
	and	#$FF
	beq	copyrest
	tax
checkhome      lda   [path],y
	and	#$FF
	beq	notfound2
	jsr	tolower
	jsr	toslash
	pha
	iny	
	lda	[home],y
	and	#$FF
	jsr	tolower
	jsr	toslash
	cmp	1,s
	bne	notfound
	pla
	dex
               bne   checkhome
	cmp	#'/'
	beq	found
      	lda   [path],y
	and	#$FF
	beq	found
	jsr	toslash
	cmp	#'/'
	bne	notfound2
     
found	lda	#'~'
	sta	[ptr]
	inc	ptr
               bra   copyrest

notfound	pla
notfound2      ldy   #0
copyrest	short	a
copyloop	lda	[path],y
	beq	endcopy
	cmp	#':'
	bne	copyput
	lda	#'/'
copyput      	sta   [ptr]
	long	a
	inc	ptr
	short	a
	iny
	bra	copyloop
endcopy	sta	[ptr]
	long	a
	dec	ptr
	lda	[ptr]
	cmp	#'/'
	bne	skipshorten
	lda	#0
	sta	[ptr]

skipshorten	ldx	home+2
	lda	home
	jsl	free256

	return 4:newpath

varparm	dc	i4'homename'
	ds	4

homename	str	'home'

	END
