		case	on

		keep	bprasm

doline		start
		pha
		tsc
		phd
		tcd
		phb
		phk
		plb

counter		equ	1
RTLs		equ	3
dest		equ	6
source		equ	10
cols		equ	14
actual		equ	16
PREVIOUS	equ	18

		pei	actual
		stz	counter
		ldy	#0
loop		anop
		lda	[source],y
		jsr	puthex
		iny
		dec	cols
		dec	actual
		bne	loop
		ldy	counter
loop2		lda	cols
		beq	done
		lda	#$2020
		sta	[dest],y
		iny
		sta	[dest],y
		iny
		iny
		dec	cols
		bra	loop2

done		anop
		pla
		sta	actual
		tya
		clc
		inc	a
		adc	actual
		sta	counter
		ldy	actual
chrloop		dey
		cpy	#$ffff
		beq	endloop
		lda	[source],y
		and	#$ff
		cmp	#$7f
		bcs	noprint
		cmp	#$20
		bcs	printable
noprint		lda	#'.'
printable	sep	#$20
		sta	[source],y
		rep	#$20
		bra	chrloop
endloop		ldy	actual
		dey
		lda	[source],y
		and	#$00ff
		ora	#$0d00
		sta	[source],y

		ldy	counter
		plb
		lda	4
		sta	PREVIOUS-2
		lda	3
		sta	PREVIOUS-3
		pld
		tsc
		clc
		adc	#PREVIOUS-4
		tcs
		tya
		rtl
		end

puthex		private
counter		equ	1
RTLs		equ	3
dest		equ	6
source		equ	10
cols		equ	14
		phy
		ldy	counter
		pha
		lsr	a
		lsr	a
		lsr	a
		lsr	a
		and	#$f
		tax
		lda	hexdigits,x
		sta	[<dest],y
		iny
		pla
		and	#$f
		tax
		lda	hexdigits,x
		and	#$00ff
		ora	#$2000
		sta	[<dest],y
		iny
		iny
		sty	counter
		ply
		rts
		end

hexdigits	privdata
		dc	c'0123456789ABCDEF'
		end
