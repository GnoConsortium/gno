**************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*
* $Id: sv.asm,v 1.5 1998/08/03 17:30:25 tribby Exp $
*
**************************************************************************
*
* SV.ASM
*   By Tim Meekins
*   Modified by Dave Tribby for GNO 2.0.6
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
	asl	a	Multiply by 4 (bytes/entry).
	asl	a
	pea	0
	pha
	~NEW		Allocate the memory
	sta	ptr	 and save address in
	stx	ptr+2	  direct page variable.

	ldy	#2	Store number of entries
	lda	size	 as the value of the
	sta	[ptr],y	  first entry's high byte
	lda	#0	   and zero as the low byte.
	sta	[ptr]

	ldy	#4	Set Y to index to 2nd entry.
	ldx	size	X = num of entries following.

init	sta	[ptr],y	Set all entries
	iny2		 to 0x00000000.
	sta	[ptr],y
	iny2
	dex
	bpl	init	;not bne so that extra null at end

	add4	ptr,#4,ptr	Set ptr to point at second entry.

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

	sub4	vect,#4,base	base points to entry # 0.

	ldy	#2
	lda	[base],y	If number of entries in table
	cmp	[base]	 == number in use,
	beq	exit	  the vector is full!
;
; 1 = allocate memory, 0 = use string as is...
;
	lda	allocflag	If "allocate memory" flag is set,
	beq	asis
	
	pei	(string+2)		Determine length of
	pei	(string)		 new string.
	jsr	cstrlen
	inc	a
	pea	0
	pha
	~NEW		Allocate memory for it.
	sta	p                          Store address in p/p+1.
	stx	p+2

	pei	(string+2)		Copy the string into
	pei	(string)		 the new memory.
	phx
	pha
	jsr	copycstr
               bra	doit	else

asis	mv4	string,p		Just copy address to p/p+1.

;
; p contains address of string to be added to the vector.
;
doit	lda	[base]
	tax		X = number of entries in use.
	asl	a
	asl	a
	tay		Y = offset to next avail entry.
	lda	p	Set entry to address of added string.
	sta	[vect],y
	iny2
	lda	p+2
	sta	[vect],y

	txa	
	inc	a
	sta	[base]	Bump the number of entries in use.

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

	sub4	vect,#4,p	p points to head of vector

	lda	[p]	Get number of entries in use.
	beq	done	Done if zero.

loop	asl	a
	asl	a
	tay		Y points to last used entry.
	lda	[vect],y
	tax
	iny2
	lda	[vect],y
	pha
	phx
	jsl	nullfree	Free memory used by this entry.
	lda	[p]
	dec	a
	sta	[p]	Number used = number used - 1.
	bne	loop	If more to do, stay in loop.

done	pei	(p+2)	Free the vector itself.
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
	
	sub4	sv,#4,base	base = ptr to entry 0.
;
; Find the maximum string length
;
	lda	#1
	sta	maxlen

	lda	#0	Keep track of entry number in Acc.
lenloop	pha
	asl	a
	asl	a
	tay		Y = offset to entry number.
	lda	[sv],y
	tax
	iny2
	lda	[sv],y
	pha
	phx
	jsr	cstrlen	Get length of entry's string.
	cmp	maxlen	If > maxlen,
	bcc	nextlen
	sta	maxlen	  set maxlen to this length.
nextlen	pla
	inc	a	Bump entry number.
	cmp	[base]	If not done,
	bcc	lenloop	 stay in loop.
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
	inx2
	adc	numrow
	cmp	[base]
	bcc	mkidxloop
;
; well....I think we can print now (yay!)
;
	asl	numcol	Double numcol since it's compared
	ldx	#0	 against X to end the loop.
printloop	lda	offtbl,x
	cmp	[base]
	bcs	nextprint0
	inc	a
	sta	offtbl,x
	phx
	dec	a
	asl	a
	asl	a
	tay
	lda	[sv],y
	tax
	iny2
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
	inx2
	cpx	numcol
	bcc	printloop
nextprint0	jsr	newline
	ldx	#0
	dec	numrow
	bne	printloop

doneprint	return

;
; Offset table: one entry for each column
;
offtbl	ds	14

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
	iny2
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
	iny2
	lda	[sv],y
	sta	ptr2+2
	lda	ptr1+2
	sta	[sv],y
	ldy	idx1
	lda	ptr2
	sta	[sv],y
	iny2
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
	iny2
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
	iny2
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
	iny2
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
	iny2
	lda	[sv],y
	sta	ptr2+2
	lda	ptr1+2
	sta	[sv],y
	ldy	idx1
	lda	ptr2
	sta	[sv],y
	iny2
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
	iny2
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
	iny2
	lda	[sv],y
	sta	ptr2+2
	lda	ptr1+2
	sta	[sv],y
	ldy	idx1
	lda	ptr2
	sta	[sv],y
	iny2
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
