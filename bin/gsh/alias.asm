**************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*
* $Id: alias.asm,v 1.9 1999/02/08 17:26:49 tribby Exp $
*
**************************************************************************
*
* ALIAS.ASM
*   By Tim Meekins
*   Modified by Dave Tribby for GNO 2.0.6
*
* Note: text set up for tabs at col 16, 22, 41, 49, 57, 65
*              |     |                  |       |       |       |
*	^	^	^	^	^	^	
**************************************************************************
*
* Interfaces defined in this file:
*
* alias	subroutine (4:argv,2:argc)
*	Returns with status=0 in Accumulator
*
* unalias	subroutine (4:argv,2:argc)
*	return 2:status
*			 
* initalias	jsr/rts with no parameters
*
* expandalias	subroutine (4:cmd)
*	return 4:buf
*
* addalias	subroutine (4:aliasname,4:aliasval)
*	return
*
* removealias	subroutine (4:aliasname)
*	return
*		                  
* findalias	subroutine (4:aliasname),space
*	return 4:value
*
* startalias	jsl/rtl with no parameters
*
* nextalias	subroutine (4:p)
*	return 2:hashvalz
*
*
**************************************************************************

	mcopy	/obj/gno/bin/gsh/alias.mac

dummyalias	start		; ends up in root
	end

	setcom 60

VTABSIZE	gequ	17

**************************************************************************
*
* ALIAS: builtin command
* syntax: alias [name [def]]
*
* set aliases
*
**************************************************************************

alias	START

	using	AliasData

arg	equ	1
space	equ	arg+4
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

	lock	AliasMutex
	lda	argc	How many arguments were provided?
	dec	a
	beq	showall	None -- show all alias names.
	dec	a
	beq	showone	One -- show a single name.
	jmp	setalias	More -- set an alias.

;
; Show all aliases
;
showall	jsl	startalias
showloop	jsl	nextalias
	sta	arg
	stx	arg+2
	ora	arg+2
	beq	noshow
	ldy	#6
	lda	[arg],y
	tax
	ldy	#4
	lda	[arg],y
	jsr	puts
	lda	#':'
	jsr	putchar
	lda	#' '
	jsr	putchar
	ldy	#10
	lda	[arg],y
	tax
	ldy	#8
	lda	[arg],y
	jsr	puts
	jsr	newline
	bra	showloop	
    
noshow	jmp	exit

;
; Show a single alias
;
showone	ldy	#4+2
	lda	[argv],y
	tax
	pha		;for findalias
	ldy	#4	
	lda	[argv],y
	pha
	jsr	puts	Print name.
	lda	#':'	Print ": ".
	jsr	putchar
	lda	#' '
	jsr	putchar
	jsl	findalias
	sta	arg
	stx	arg+2
	ora	arg+2
	beq	notthere
	lda	arg
	jsr	puts	Print alias value.
	jsr	newline	Print newline.
	jmp	exit	All done.

notthere	ldx	#^noalias	Print message:
	lda	#noalias	 'Alias not defined'
	jsr	puts
	jmp	exit

;
; Set an alias name
;
setalias	ldy	#4+2	;put alias name on stack
	lda	[argv],y
	pha
	ldy	#4
	lda	[argv],y
	pha

	ph4	#2
	~NEW
	sta	arg
	stx	arg+2
	lda	#0
	sta	[arg]

	add2	argv,#8,argv

	dec2	argc

buildalias	lda	argc
	beq	setit

	pei	(arg+2)
	pei	(arg)
	pei	(arg+2)
	pei	(arg)
	ldy	#2
	lda	[argv],y
	pha
	lda	[argv]
	pha
	jsr	catcstr
	stx	arg+2
	sta	arg
	jsl	nullfree

	pei	(arg+2)
	pei	(arg)
	pei	(arg+2)
	pei	(arg)
	ph4	#spacestr
	jsr	catcstr
	stx	arg+2
	sta	arg
	jsl	nullfree

	dec	argc
	add2	argv,#4,argv
	bra	buildalias

setit	pei	(arg+2)
	pei	(arg)
	jsl	addalias
	pei	(arg+2)
	pei	(arg)
	jsl	nullfree

exit	unlock AliasMutex
	lda	space
	sta	end-3
	lda	space+1
	sta	end-2
	pld
	tsc
	clc
	adc	#end-4
	tcs

	lda	#0	Return status always 0.

	rtl

noalias	dc	c'Alias not defined',h'0d00'
spacestr	dc	c' ',h'00'

	END

**************************************************************************
*
* UNALIAS: builtin command
* syntax: unalias [var ...]
*
* removes each alias listed
*
**************************************************************************

unalias	START

	using	AliasData

status	equ	0
space	equ	status+2

	subroutine (4:argv,2:argc),space

	lock	AliasMutex

	stz	status

	lda	argc
	dec	a
	bne	loop

	ldx	#^Usage
	lda	#USage
	jsr	errputs
	inc	status	Return status = 1.
	bra	done

loop	add2	argv,#4,argv
	dec	argc
	beq	done

	ldy	#2
	lda	[argv],y
	pha
	lda	[argv]
	pha
	jsl	removealias

	bra	loop

done	unlock AliasMutex
	return 2:status

Usage	dc	c'Usage: unalias name ...',h'0d00'

	END

;=========================================================================
;
; Init alias table
;
;=========================================================================

initalias	START

	using	AliasData

; Set all entries in AliasTable to 0

	lda	#0
	ldy	#VTABSIZE
	tax
yahaha	sta	AliasTable,x
	inx2
	sta	AliasTable,x
	inx2
	dey	
	bne	yahaha

	rts

	END		 

;=========================================================================
;
; Expand alias
;
;=========================================================================

expandalias	START
	
outbuf	equ	0
sub	equ	outbuf+4
word	equ	sub+4
buf	equ	word+4
bufend	equ	buf+4
inquote	equ	bufend+2
bsflag	equ	inquote+2
space	equ	bsflag+2
	
	subroutine (4:cmd),space

	ph4	maxline_size	Allocate result buffer.
	~NEW
	stx	buf+2
	sta	buf
	stx	outbuf+2	Initialize "next
	sta	outbuf	 output char" pointer.
	clc		Calculate end of buffer-1;
	adc	maxline_size	 note: it's allocated to be in one
	dec	a	  bank, so only need low-order word.
	sta	bufend

	jsl	allocmaxline	Allocate buffer for word that
	stx	word+2	 might be an alias.
	sta	word

	lda	#0
	sta	[buf]	In case we're called with empty string.
	sta	[word]
;
; Eat leading space and tabs (just in case expanding a variable added them!)
;
	bra	eatleader
bump_cmd	inc	cmd
eatleader	lda	[cmd]
	and	#$FF
	jeq	stringend
	cmp	#' '
	beq	bump_cmd
	cmp	#9
	beq	bump_cmd
;
; Parse the leading word
;
	short	a
	ldy	#0
	bra	makeword1	First time, already checked 0, ' ', 9
makeword	lda	[cmd],y
	if2	@a,eq,#0,gotword
	if2	@a,eq,#' ',gotword
	if2	@a,eq,#9,gotword
makeword1	if2	@a,eq,#';',gotword
	if2	@a,eq,#'&',gotword
	if2	@a,eq,#'|',gotword
	if2	@a,eq,#'>',gotword
	if2	@a,eq,#'<',gotword
	if2	@a,eq,#13,gotword
	if2	@a,eq,#10,gotword
	sta	[word],y
	iny
	bra	makeword
;
; We have a word. See if it's an alias.
;
gotword	lda	#0
	sta	[word],y	
	long	a
	cpy	#0	Check for 0 length.
	beq	copyrest
	phy
	pei	(word+2)
	pei	(word)
	jsl	findalias
	sta	sub
	stx	sub+2
	ply
	ora	sub+2
	beq	copyrest
;
; Yes, this is an alias. Copy it into the output buffer.
;
	add2	@y,cmd,cmd	Add length to cmd pointer.

	pei	(sub+2)	Make sure that
	pei	(sub)	 substituted string
	jsr	cstrlen	  will fit into buffer.
	sec
	adc	outbuf
	jge	overflow
	cmp	bufend
	jge	overflow

	ldy	#0
	short	a
	lda	[sub]
putalias	sta	[outbuf]
	inc	outbuf
	iny
	lda	[sub],y
	bne	putalias
	long	a
;
; That alias is expanded. Copy until we reach the next command.
;
copyrest	stz	inquote	Clear the "in quotes" flag
	stz	bsflag	 and "backslashed" flag.

next	anop
	lda	outbuf	Check for output overflow.
	cmp	bufend
	jcs	overflow
	lda	[cmd]	Transfer the character.
	and	#$00FF
	cmp	#13	If carriage-return,
	bne	go8bits
	lda	#0	 treat like end-of-string.
go8bits	short	a
	sta	[outbuf]
	long	a
	inc	cmd	Bump pointers.
	inc	outbuf

	if2	@a,eq,#0,done
;
; If that was a backslashed character, don't check for special chars.
;
	ldx	bsflag
	beq	testq
	stz	bsflag
	bra	next

testq	if2	@a,eq,#"'",singquoter
	if2	@a,eq,#'"',doubquoter
;
; Remaining characters aren't special if we are in a quoted string
;
	ldx	inquote
	bne	next
	if2	@a,eq,#';',nextalias
	if2	@a,eq,#'&',nextalias
	if2	@a,eq,#'|',nextalias
	if2	@a,ne,#'\',next
;
; "\" found
;
backstabber	sta	bsflag
	bra	next

;
; "'" found
;
singquoter	bit	inquote	Check "in quotes" flag.
	bvs	next	In double quotes. Keep looking.
	lda	inquote	Toggle single quote
	eor	#$8000
	sta	inquote
	bra	next
;
; '"' found
;
doubquoter	bit	inquote	Check "in quotes" flag.
	bmi	next	In single quotes. Keep looking.
	lda	inquote	Toggle single quote
	eor	#$4000
	sta	inquote
	bra	next

;
; ";", "|", or "&" found: it's another command
;
nextalias	jmp	eatleader

;
; Terminate string and exit
;
stringend	short	a
	sta	[outbuf]
	long	a
;
; All done: clean up and return to caller
;
done	ldx	word+2
	lda	word
	jsl	freemaxline

	return 4:buf


;
; Report overflow error
;
overflow	anop
	pei	(buf+2)	Free the output buffer.
	pei	(buf)
	jsl	nullfree
	stz	buf
	stz	buf+2

	ldx	#^ovferr	Report overflow error.
	lda	#ovferr
	jsr	errputs

	bra	done

ovferr	dc	c'gsh: Alias overflowed line limit',h'0d00'

	END

;=========================================================================
;
; Add alias to table
;
;=========================================================================

addalias	START
	
	using	AliasData

tmp	equ	0
ptr	equ	tmp+4
hashval	equ	ptr+4
space	equ	hashval+4

	subroutine (4:aliasname,4:aliasval),space

	pei	(aliasname+2)
	pei	(aliasname)
	jsl	hashalias
	sta	hashval
	
	tax	
	lda	AliasTable,x
	sta	ptr
	lda	AliasTable+2,x
	sta	ptr+2

search	lda	ptr
	ora	ptr+2
	beq	notfound
	ldy	#4
	lda	[ptr],y
	tax
	ldy	#4+2
	lda	[ptr],y
	pha
	phx
	pei	(aliasname+2)
	pei	(aliasname)
	jsr	cmpcstr
	jeq	replace
	ldy	#2
	lda	[ptr]
	tax
	lda	[ptr],y
	sta	ptr+2
	stx	ptr
	bra	search

replace	ldy	#8+2
	lda	[ptr],y
	pha	
	ldy	#8
	lda	[ptr],y
	pha
	jsl	nullfree
	pei	(aliasval+2)
	pei	(aliasval)
	jsr	cstrlen
	inc	a
	pea	0
	pha
	~NEW
	sta	tmp
	stx	tmp+2
	ldy	#8
	sta	[ptr],y
	ldy	#8+2
	txa
	sta	[ptr],y
	pei	(aliasval+2)
	pei	(aliasval)
	pei	(tmp+2)
	pei	(tmp)
	jsr	copycstr
	jmp	done

notfound	ph4	#4*3
	~NEW
	sta	ptr
	stx	ptr+2
	ldy	#2
	ldx	hashval
	lda	AliasTable,x
	sta	[ptr]
	lda	AliasTable+2,x
	sta	[ptr],y
	pei	(aliasname+2)
	pei	(aliasname)
	jsr	cstrlen
	inc	a
	pea	0
	pha
	~NEW
	sta	tmp
	stx	tmp+2
	ldy	#4
	sta	[ptr],y
	ldy	#4+2
	txa
	sta	[ptr],y
	pei	(aliasname+2)
	pei	(aliasname)
	pei	(tmp+2)
	pei	(tmp)
	jsr	copycstr
	pei	(aliasval+2)
	pei	(aliasval)
	jsr	cstrlen
	inc	a
	pea	0
	pha
	~NEW
	sta	tmp
	stx	tmp+2
	ldy	#8
	sta	[ptr],y
	ldy	#8+2
	txa
	sta	[ptr],y
	pei	(aliasval+2)
	pei	(aliasval)
	pei	(tmp+2)
	pei	(tmp)
	jsr	copycstr
	ldx	hashval
	lda	ptr
	sta	AliasTable,x
	lda	ptr+2
	sta	AliasTable+2,x
		        
done	return

	END

;=========================================================================
;
; Remove an alias
;
;=========================================================================

removealias	START

	using	AliasData

oldptr	equ	0
ptr	equ	oldptr+4
space	equ	ptr+4

	subroutine (4:aliasname),space

	pei	(aliasname+2)
	pei	(aliasname)
	jsl	hashalias
	tax
	lda	AliasTable,x
	sta	ptr
	lda	AliasTable+2,x
	sta	ptr+2
	lda	#^Aliastable
	sta	oldptr+2
	clc
	txa
	adc	#AliasTable
	sta	oldptr

searchloop	ora2	ptr,ptr+2,@a
	beq	done

	ldy	#4+2
	lda	[ptr],y
	pha
	ldy	#4
	lda	[ptr],y
	pha
	pei	(aliasname+2)
	pei	(aliasname)
	jsr	cmpcstr
	beq	foundit
	mv4	ptr,oldptr
	ldy	#2
	lda	[ptr],y
	tax
	lda	[ptr]
	sta	ptr
	stx	ptr+2
	bra	searchloop

foundit	ldy	#2
	lda	[ptr],y
	sta	[oldptr],y
	lda	[ptr]
	sta	[oldptr]
	ldy	#4+2
	lda	[ptr],y
	pha
	ldy	#4
	lda	[ptr],y
	pha
	jsl	nullfree
	ldy	#8+2
	lda	[ptr],y
	pha
	ldy	#8
	lda	[ptr],y
	pha
	jsl	nullfree
	pei	(ptr+2)
	pei	(ptr)
	jsl	nullfree
		        
done	return

	END

;=========================================================================
;
; Find an alias
;
;=========================================================================

findalias	START

	using	AliasData

ptr	equ	0
value	equ	ptr+4
space	equ	value+4

	subroutine (4:aliasname),space

	stz	value
	stz	value+2

	pei	(aliasname+2)
	pei	(aliasname)
	jsl	hashalias
	tax
	lda	AliasTable,x
	sta	ptr
	lda	AliasTable+2,x
	sta	ptr+2

searchloop	ora2	ptr,ptr+2,@a
	beq	done

	ldy	#4+2
	lda	[ptr],y
	pha
	ldy	#4
	lda	[ptr],y
	pha
	pei	(aliasname+2)
	pei	(aliasname)
	jsr	cmpcstr
	beq	foundit
	ldy	#2
	lda	[ptr],y
	tax
	lda	[ptr]
	sta	ptr
	stx	ptr+2
	bra	searchloop

foundit	ldy	#8
	lda	[ptr],y
	sta	value
	ldy	#8+2
	lda	[ptr],y
	sta	value+2

done	return 4:value

	END

;=========================================================================
;
; Start alias
;
;=========================================================================

startalias	START

	using	AliasData

	stz	AliasNum
	mv4	AliasTable,AliasPtr
	rtl

	END

;=========================================================================
;
; Next alias
;
;=========================================================================

nextalias	START

	using	AliasData

value	equ	0
space	equ	value+4

	subroutine (0:fubar),space

	stz	value
	stz	value+2
puke	if2	AliasNum,cs,#VTABSIZE,done

	ora2	AliasPtr,AliasPtr+2,@a
	bne	flush
	inc	AliasNum
	lda	AliasNum
	asl2	a
	tax
	lda	AliasTable,x
	sta	AliasPtr
	lda	AliasTable+2,x
	sta	AliasPtr+2
	bra	puke

flush	mv4	AliasPtr,value
	ldy	#2
	lda	[value]
	sta	AliasPtr
	lda	[value],y
	sta	AliasPtr+2	

done	return 4:value

	END

;=========================================================================
;
; Hash an alias
;
;=========================================================================

hashalias	PRIVATE

hashval	equ	0
space	equ	hashval+2

	subroutine (4:p),space
		               
	lda	#11
	sta	hashval

	ldy	#0
loop	asl	hashval
	lda	[p],y
	and	#$FF
	beq	done
	clc
	adc	hashval
	sta	hashval
	iny
	bra	loop
done	UDivide (hashval,#VTABSIZE),(@a,@a)

	asl2	a	;Make it an index.
	sta	hashval

	return 2:hashval

	END

;=========================================================================
;
; Alias data
;
;=========================================================================

AliasData	DATA

AliasNum	dc	i2'0'
AliasPtr	dc	i4'0'
AliasMutex	key

AliasTable	ds	VTABSIZE*4

	END	               
