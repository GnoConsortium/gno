*
* Performance modifications to crypt.c routines.
* 19-22 January 1992 by Jawaid Bazyar
* Copyright 1992, Procyon Inc.
*
* $Id: crypta.asm,v 1.2 1998/02/05 16:07:06 gdr-ftp Exp $
*
* Because of the four storage blocks listed below (copyOfData, e, ikey, and
* yb), this doesn't seem to be compatible with the large memory model.
* This should be changed.  These routines should also be placed in the
* libc_gen__ load segment.
*
	case	on

dummy	start			; ends up in .root
	end

	mcopy	crypta.mac

* void __crypt_transpose (struct block *data, struct ordering *t, int n);
__crypt_transpose START
	subroutine (2:n,4:t,4:data),2

	ldy	#62
lp	lda	[data],y
	sta	copyOfData,y
	dey2
	bpl	lp
	lda	#0	; clear out hi-byte of A
	short	m

lp2	ldy	n
	beq	donelp2
	dec	n
	ldy	n
	lda	[t],y
	dec	a
	tay
	lda	copyOfData,y
	ldy	n
	sta	[data],y
	bra 	lp2
donelp2	long  m
	return

copyOfData ds	64
	END

* void __crypt_rotate (struct block *key);
__crypt_rotate	START
data0	equ	0
data28	equ	2
ep	equ	4
	subroutine (4:key),6

	lda	#55
	sta	ep
	short	m
	lda	[key]
	sta	data0
	ldy	#28
	lda	[key],y
	sta	data28
	ldy	#0

lp	cpy	ep
	bcs	donelp
	iny
	lda	[key],y
	dey
	sta	[key],y
	iny
	bra	lp
donelp	lda	data0
	ldy	#27
	sta	[key],y
	lda	data28
	ldy	#55
	sta	[key],y	
               long  m
	return
	END

* void __crypt_f (int i, struct block *key, struct block *a, struct block *x);
__crypt_f START
k	equ	0
p	equ	2
q	equ	4
r	equ	6
xb	equ	8
	subroutine (4:x,4:a,4:key,2:i),10

	ldy	#62
lp1	lda	[a],y
	sta	e,y
	dey2
	bpl	lp1

	ph2	#48
	ph4	__crypt_EP
	ph4	#e
	jsl	transpose

	lda	i
	asl	a
	tay
	lda	__crypt_rots,y
	sta	k
lp2	lda	k
	beq	donelp2
	ph4	key
	jsl	rotate
	dec	k
	bra	lp2

donelp2	anop
	ldy	#62
lp3	lda	[key],y
	sta	ikey,y
	dey2
	bpl	lp3

	ph2	#48
	ph4	#KeyTr2
	ph4	#ikey
	jsl	transpose

	short	m
	ldy	#48
lp4	cpy	#0
	beq	donelp4
	dey
	lda	e,y
	eor	ikey,y
	sta	yb,y		
	bra	lp4

donelp4	anop		
	long	m
	ldy	#0
	stz 	p
	stz	q	
	stz	k

kloop	lda	k
	cmp	#8
	bcc	okay
	jmp	donekloop
	
okay	ldy	p
	lda	yb,y
	iny
	and	#$FF	
	asl	a
	asl	a
	asl	a
	asl	a
	asl	a
	sta	r

	lda	yb,y
	iny
	and	#$FF
	asl	a
	asl	a
	asl	a
	clc
	adc	r
	sta	r

	lda	yb,y
	iny
	and	#$FF
	asl	a
	asl	a
	clc
	adc	r
	sta	r
	
	lda	yb,y
	iny
	and	#$FF
	asl	a
	clc
	adc	r
	sta	r
	
	lda	yb,y
	iny
	and	#$FF
	clc
	adc	r
	sta	r

	lda	yb,y
	iny
	and	#$FF
	asl	a
	asl	a
	asl	a
	asl	a
	clc
	adc	r
	sta	r
	sty	p                  ; store it temporarily

	lda	k
	asl	a
	asl	a
	asl	a
	asl	a
	asl	a
	asl	a	; k * 64
	clc
	adc	r	; + r;
	tay
	lda	__crypt_s_boxes,y
	and	#$FF
	sta	xb

*  *q++ = (xb >> 3) & 1;
	ldy	q
	lda	xb
	lsr	a
	lsr	a
	lsr	a
	and	#1
	short	m
	sta	[x],y
	long	m
	iny

*  *q++ = (xb >> 2) & 1;
	lda	xb
	lsr	a
	lsr	a
	and	#1
	short	m
	sta	[x],y
	long	m
	iny

*  *q++ = (xb >> 1) & 1
	lda	xb
	lsr	a
	and	#1
	short	m
	sta	[x],y
	long	m
	iny

*  *q++ = xb & 1;
	lda	xb
	and	#1
	short	m
	sta	[x],y
	long	m
	iny
	sty	q			
	inc	k
	jmp	kloop
donekloop	anop
	ph2	#32
	ph4	#ptr
	ph4	x
	jsl	transpose
	return

e	ds	64
ikey	ds	64
yb	ds	64
	END
