**************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*
* $Id: sv.asm,v 1.3 1998/06/30 17:26:02 tribby Exp $
*
**************************************************************************
*
* SV.ASM
*   By Tim Meekins
*
* String Vector routines.
*
* Note: text set up for tabs at col 16, 22, 41, 49, 57, 65
*              |     |                  |       |       |       |
*	^	^	^	^	^	^	
**************************************************************************

	mcopy /obj/gno/bin/gsh/sv.mac
                         
dummysv	start		; ends up in .root
	end

	setcom 60

**************************************************************************
*
* Allocate a string vector array
*
**************************************************************************

sv_alloc	START

ptr	equ	0
space	equ	ptr+4

	subroutine (2:size),space

	lda	size
	inc	a	;for size and count
	inc	a	;at least one null entry
	asl	a
	asl	a
	pea	0
	pha
	jsl	~NEW
	sta	ptr
	stx	ptr+2

	ldy	#2
	lda	size
	sta	[ptr],y
	lda	#0
	sta	[ptr]
	ldy	#4

	ldx	size

init	sta	[ptr],y
	iny
	iny
	sta	[ptr],y
	iny
	iny
	dex
	bpl	init	;not bne so that extra null at end

	add4	ptr,#4,ptr

	return 4:ptr

	END

**************************************************************************
*
* Add a string to the string vector
*
**************************************************************************

sv_add	START

p	equ	0
base	equ	p+4
space	equ	base+4

	subroutine (4:vect,4:string,2:allocflag),space

	sub4	vect,#4,base

	ldy	#2
	lda	[base],y
	cmp	[base]
	beq	exit	;ack, the vector is full!
;
; 1 = allocate memory, 0 = use string as is...
;
	lda	allocflag
	beq	asis
	
	pei	(string+2)
	pei	(string)
	jsr	cstrlen
	inc	a
	pea	0
	pha
	jsl	~NEW
	sta	p
	stx	p+2

	pei	(string+2)
	pei	(string)
	phx
	pha
	jsr	copycstr
               bra	doit

asis	mv4	string,p

doit	lda	[base]
	tax
	asl	a
	asl	a
	tay
	lda	p
	sta	[vect],y
	iny
	iny
	lda	p+2
	sta	[vect],y

	txa	
	inc	a
	sta	[base]

exit	return

	END

**************************************************************************
*
* Dispose a string vector
*
**************************************************************************

sv_dispose	START

p	equ	0
space	equ	p+4

	subroutine (4:vect),space

	sub4	vect,#4,p

loop	lda	[p]
	beq	done
	asl	a
	asl	a
	tay
	lda	[vect],y
	tax
	iny
	iny
	lda	[vect],y
	pha
	phx
	jsl	nullfree
	lda	[p]
	dec	a
	sta	[p]
	bra	loop

done	pei	(p+2)
	pei	(p)
	jsl	nullfree

	return

	END

**************************************************************************
*
* Column print a string vector
*
**************************************************************************

sv_colprint	START

numrow	equ	0
numcol	equ	numrow+2
base	equ	numcol+2
maxlen	equ	base+4
space	equ	maxlen+2

	subroutine (4:sv),space
	
	sub4	sv,#4,base
;
; Find the maximum string length
;
	lda	#1
	sta	maxlen

	ldy	#0
	lda	#0
lenloop	pha
	lda	[sv],y
	tax
	iny
	iny
	lda	[sv],y
	iny
	iny
	phy
	pha
	phx
	jsr	cstrlen
	cmp	maxlen
	bcc	nextlen
	sta	maxlen
nextlen	ply
	pla
	inc	a
	cmp	[base]
	bcc	lenloop
;
; add one for a space
;
	inc	maxlen
;
; calculate the number of columns....this is probably simpler than doing
; a divide..
;
	ldx	#0
	txa
	dex
	clc
colloop	inx
	adc	maxlen
	cmp	#80
	bcc	colloop
	cpx	#6+1
	bcc	okcol
	ldx	#6	;limit of 6 columns
okcol	stx	numcol
;
; recalculate the width
;
	UDivide (#80,@x),(maxlen,@a)
;
; calculate the height
;
	lda	[base]
	UDivide (@a,numcol),(numrow,@x)
	cpx	#0
	beq	foocol
	inc	numrow
foocol	anop
;
; find the index for each column...
;                                  
	lda	#0
	tax
	clc
mkidxloop	sta	offtbl,x
	inx
	adc	numrow
	cmp	[base]
	bcc	mkidxloop
;
; well....I think we can print now (yay!)
;
	ldx	#0
printloop	lda	offtbl,x
	and	#$FF
	cmp	[base]
	bcs	nextprint0
	inc	a
	short	a
	sta	offtbl,x
	long	a
	phx
	dec	a
	asl	a
	asl	a
	tay
	lda	[sv],y
	tax
	iny
	iny
	lda	[sv],y
	pha
	phx
	tax
	lda	1,s
	jsr	puts
	jsr	cstrlen
tabit	cmp	maxlen
	bcs	nextprint
	pha
	lda	#' '
	jsr	putchar
	pla
	inc	a
	bra	tabit
nextprint	plx
	inx
	cpx	numcol
	bcc	printloop
nextprint0	jsr	newline
	ldx	#0
	dec	numrow
	bne	printloop

doneprint	return

offtbl	ds	7

	END

**************************************************************************
*
* Sort a string vector
*
**************************************************************************

sv_sort	START

space	equ	0

	subroutine (4:sv),space

	pei	(sv+2)
	pei	(sv)
	sub4	sv,#4,sv
	pea	0
	lda	[sv]
	dec	a
	pha
	jsl	_qsort

	return

	END

_qsort	PRIVATE

vleft	equ	0
i	equ	vleft+4
last	equ	i+2
ptr2	equ	last+2
ptr1	equ	ptr2+4
idx1	equ	ptr1+4
space	equ	idx1+2

	subroutine (4:sv,2:left,2:right),space
;
; if (left >= right)
;    return;
;
	lda	right	;if one or two elements do nothing
	jmi	exit
	cmp	left
	jcc	exit
;
; swap(v, left, (left + right)/2);
;
	lda	left
	asl	a
	asl	a
	sta	idx1
	tay
	lda	[sv],y
	sta	ptr1
	iny
	iny
	lda	[sv],y
	sta	ptr1+2
	add2	left,right,@a
	lsr	a
	asl	a
	asl	a
	tay
	lda	[sv],y
	sta	ptr2
	lda	ptr1
	sta	[sv],y
	iny
	iny
	lda	[sv],y
	sta	ptr2+2
	lda	ptr1+2
	sta	[sv],y
	ldy	idx1
	lda	ptr2
	sta	[sv],y
	iny
	iny
	lda	ptr2+2
	sta	[sv],y
;
; last = left;
;
	lda	left
	sta	last

	asl	a
	asl	a
	tay	
	lda	[sv],y
	sta	vleft
	iny
	iny
	lda	[sv],y
	sta	vleft+2
;
; for (i=left+1; i <=right; i++)
;
	lda	left
	inc	a
	sta	i
forloop	lda	i
	cmp	right
	beq	okloop
	bcs	endloop
okloop	anop
;
;     if (strcmp(v[i],v[left]) < 0)
;
	asl	a
	asl	a
	tay
	lda	[sv],y
	tax
	iny
	iny
	lda	[sv],y
	pha
	phx
	pei	(vleft+2)
	pei	(vleft)
	jsr	cmpcstr
	bpl	nextloop
;
;        swap (v, ++last, i)
;	
	inc	last
	lda	last
	asl	a
	asl	a
	sta	idx1
	tay
	lda	[sv],y
	sta	ptr1
	iny
	iny
	lda	[sv],y
	sta	ptr1+2
	lda	i
	asl	a
	asl	a
	tay
	lda	[sv],y
	sta	ptr2
	lda	ptr1
	sta	[sv],y
	iny
	iny
	lda	[sv],y
	sta	ptr2+2
	lda	ptr1+2
	sta	[sv],y
	ldy	idx1
	lda	ptr2
	sta	[sv],y
	iny
	iny
	lda	ptr2+2
	sta	[sv],y

nextloop	inc	i
	jmp	forloop
;
; swap(v, left, last)
;                            
endloop	lda	left
	asl	a
	asl	a
	sta	idx1
	tay
	lda	[sv],y
	sta	ptr1
	iny
	iny
	lda	[sv],y
	sta	ptr1+2
	lda	last
	asl	a
	asl	a
	tay
	lda	[sv],y
	sta	ptr2
	lda	ptr1
	sta	[sv],y
	iny
	iny
	lda	[sv],y
	sta	ptr2+2
	lda	ptr1+2
	sta	[sv],y
	ldy	idx1
	lda	ptr2
	sta	[sv],y
	iny
	iny
	lda	ptr2+2
	sta	[sv],y
;
; qsort(v, left, last-1)
;
	pei	(sv+2)
	pei	(sv)
	pei	(left)
	lda	last
	dec	a
	pha
	jsl	_qsort
;
; qsort(v, last+1, right)
;
	pei	(sv+2)
	pei	(sv)
	lda	last
	inc	a
	pha
	pei	(right)
	jsl	_qsort
                     
exit	return

	END
