**************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*
* $Id: shellutil.asm,v 1.3 1998/06/30 17:25:55 tribby Exp $
*
**************************************************************************
*
* SHELLUTIL.ASM
*   By Tim Meekins
*
* Utility functions used by the shell. Mainly string functions.
*
* Note: text set up for tabs at col 16, 22, 41, 49, 57, 65
*              |     |                  |       |       |       |
*	^	^	^	^	^	^	
**************************************************************************

	mcopy /obj/gno/bin/gsh/shellutil.mac

dummyshellutil	start		; ends up in .root
	end

	setcom 60

;=========================================================================
;
; Convert the accumulator to lower case.
;
;=========================================================================

tolower        START

	cmp	#'A'
	bcc	done
	cmp	#'Z'+1
	bcs	done
	adc	#'a'-'A'
done	rts

               END

;=========================================================================
;
; Convert ':' to '/'
;
;=========================================================================

toslash        START

	cmp	#':'
	bne	done
	lda	#'/'
done	rts

               END

;=========================================================================
;
; Convert a c string to lower case
;
;=========================================================================

lowercstr      START

space          equ   1
p              equ   space+2
end            equ   p+4

               tsc
               phd
               tcd

               short a

               ldy   #-1
loop           iny
	lda   [p],y
               beq   done
	cmp	#'A'
	bcc	loop
	cmp	#'Z'+1
	bcs	loop
	adc	#'a'-'A'
               sta   [p],y
               bra   loop

done           rep	#$21
	longa	on
	lda   space
               sta   end-2
               pld
               tsc
               adc   #end-3
               tcs

               rts

               END

;=========================================================================
;
; Get the length of a c string.
;
;=========================================================================

cstrlen        START

space          equ   1
p              equ   space+2
end            equ   p+4

               tsc
               phd
               tcd

               short a

               ldy   #0
loop           lda   [p],y
               beq   done
               iny
               bra   loop

done           rep	#$21
	longa	on
               lda   space
               sta   end-2
               pld
               tsc
               adc   #end-3
               tcs

               tya

               rts

               END

;=========================================================================
;
; Copy one string to another. Assumes an alloccstr has been performed on
; destination.
;
;=========================================================================

copycstr       START

space          equ   1
q              equ   space+2
p              equ   q+4
end            equ   p+4

               tsc
               phd
               tcd

               short a

               ldy   #0
loop           lda   [p],y
               beq   done
               sta   [q],y
               iny
               bra   loop

done           sta   [q],y

               rep	#$21
	longa	on
               lda   space
               sta   end-2
               pld
               tsc
               adc   #end-3
               tcs

               rts

               END

;=========================================================================
;
; Converts a pascal string to a c string. This allocates memory for the
; new c string.
;
;=========================================================================

p2cstr         START

cstr           equ   1
space          equ   cstr+4
p              equ   space+2
end            equ   p+4

               tsc
               sec
               sbc   #space-1
               tcs
               phd
               tcd

               lda   [p]
               and   #$FF
	inc	a
               pea   0
               pha
               jsl   ~NEW
               sta   cstr
               stx   cstr+2
               lda   [p]
	inc	p
               and   #$FF
               tax

               short a
               ldy   #0
               cpx   #0
               beq   done
loop           lda   [p],y
               sta   [cstr],y
               iny
               dex
               bne   loop

done           lda   #0
               sta   [cstr],y

               ldx   cstr+2
               ldy   cstr

               rep	#$21
	longa	on
               lda   space
               sta   end-2
               pld
               tsc
               adc   #end-3
               tcs

               tya

               rts

               END

;=========================================================================
;
; Converts a c string to a pascal string. Does not allocate memory.
;
;=========================================================================

c2pstr         START

space          equ   1
p              equ   space+2
cstr           equ   p+4
end            equ   cstr+4

               tsc
               phd
               tcd

               short a

               ldy   #0
loop           lda   [cstr],y
               beq   endstr
               iny
               sta   [p],y
               bra   loop
endstr         tya
               sta   [p]

               rep	#$21
	longa	on
               lda   space
               sta   end-2
               pld
               tsc
               adc   #end-3
               tcs

               rts

               END

;=========================================================================
;
; Converts a c string to a pascal string. Allocates memory for p string.
;
;=========================================================================

c2pstr2        START

p	equ	1
space          equ   p+4
cstr           equ   space+2
end            equ   cstr+4

               tsc
               sec
               sbc   #space-1
               tcs
               phd
               tcd

	pei	(cstr+2)
	pei	(cstr)
	jsr	cstrlen
	inc	a
	pea	0
	pha
	jsl	~NEW
	sta	p
	stx	p+2

               short a

               ldy   #0
loop           lda   [cstr],y
               beq   endstr
               iny
               sta   [p],y
               bra   loop
endstr         tya
               sta   [p]

	ldx	p+2
	ldy	p

               rep	#$21
	longa	on
               lda   space
               sta   end-2
               pld
               tsc
               adc   #end-3
               tcs

	tya

               rts

               END

;=========================================================================
;
; Compare two c strings. Return 0 if equal, -1 if less than, +1 greater
;
;=========================================================================

cmpcstr        START

space          equ   1
q              equ   space+2
p              equ   q+4
end            equ   p+4

               tsc
               phd
               tcd

               short a

               ldx   #0
               
               ldy   #0
strloop        lda   [p],y
               beq   strchk
               cmp   [q],y
               bne   notequal
               iny
               bra   strloop

strchk         lda   [q],y
               beq   done

lessthan	dex
	bra	done

notequal	bcc	lessthan
	inx

done           rep	#$21
	longa	on
               lda   space
               sta   end-2
               pld
               tsc
               adc   #end-3
               tcs

               txa
               rts

               END

;=========================================================================
;
; Compare two downshifted c strings. Return 0 if ==, -1 if <, +1 >
;
;=========================================================================

cmpdcstr        START

hold           equ   1
space          equ   hold+2
q              equ   space+2
p              equ   q+4
end            equ   p+4

               tsc
               sec
               sbc   #space-1
               tcs
               phd
               tcd

               ldx   #0
               
               ldy   #0
strloop        lda   [q],y
               and   #$FF
               jsr   tolower
               sta   hold
               lda   [p],y
               and   #$FF
               beq   strchk
               jsr   tolower
               cmp   hold
               bne   notequal
               iny
               bra   strloop

strchk         lda   hold
               beq   done

lessthan	dex
	bra	done

notequal	bcc	lessthan
	inx

done           anop
               lda   space
               sta   end-2
               pld
               tsc
               clc
               adc   #end-3
               tcs

               txa
               rts

               END

;=========================================================================
;
; Convert a c string to a GS/OS string (don't forget to dispose when done)
;
;=========================================================================

c2gsstr        START

len            equ   1
gstr           equ   len+2
space          equ   gstr+4
cstr           equ   space+2
end            equ   cstr+4

               tsc
               sec
               sbc   #space-1
               tcs
               phd
               tcd

               pei   (cstr+2)
               pei   (cstr)
               jsr   cstrlen
               sta   len
	adc	#3
               pea   0
               pha
               jsl   ~NEW
               sta   gstr
               stx   gstr+2
	inc	a
	inc	a
          	pei   (cstr+2)
               pei   (cstr)
               phx
               pha
               jsr   copycstr
               lda   len
               sta   [gstr]

               ldx   gstr+2
               ldy   gstr

               lda   space
               sta   end-2
               pld
               tsc
               adc   #end-3
               tcs

               tya
               rts

               END

;=========================================================================
;
; Takes two strings, concats into a newly created string.
;
;=========================================================================

catcstr        START

new	equ	1
space          equ   new+4
q              equ   space+2
p              equ   q+4
end            equ   p+4

               tsc
               sec
               sbc   #space-1
               tcs
               phd
               tcd

	pei	(p+2)
	pei	(p)
	jsr	cstrlen
	pha
	pei	(q+2)
	pei	(q)
	jsr	cstrlen
	adc	1,s
	inc	a
	inc	a
	plx
	pea	0
	pha
	jsl	~NEW
	sta	new
	stx	new+2

	ldy	#0
copy1	lda	[p],y
	and	#$FF
	beq	copy2
	sta	[new],y
	iny
	bra	copy1
copy2	lda	[q]
	and	#$FF
	sta	[new],y
	beq	done
	iny
	inc	q
	bra	copy2	

done           ldx   new+2
	ldy	new
	lda   space
               sta   end-2
               pld
               tsc
               clc
               adc   #end-3
               tcs

               tya
               rts

               END

;=====================================================================
;
; call ~DISPOSE if pointer is not NULL
;
;=====================================================================

nullfree	START

	lda	6,s
	and	#$FF80
	beq	notbad
	brk	$db
notbad	anop

	lda	4,s
	ora	6,s
	bne	ok
               lda	2,s
	sta	6,s
	lda	1,s
	sta	5,s
	plx
	plx
	rtl

ok	jml	~DISPOSE

	END

;=====================================================================
;
; Print a carriage return and a newline, unless "newline" shell var
;  is set.
;
;=====================================================================

newlineX       START

	using	vardata

	lda	varnewline
	beq	newline
               rts

;=====================================================================
;
; Print a carriage return and a newline
;
;=====================================================================

newline        ENTRY

	lda	#13
	jmp	putchar

	END

**************************************************************************
*
* Quick little routine for reading variables and converting to
* null terminated strings. We could probably just link the Orca/C
* library getenv(), but lets stay Orca-free, after all, that's why this
* is written in assembly! :)
*
* Borrowed from termcap
*
**************************************************************************

getenv	START

newval	equ	1
len	equ	newval+4
namebuf	equ	len+2
retval	equ	namebuf+4
space	equ	retval+4
var	equ	space+3
end	equ	var+4

;	subroutine (4:var),space

	tsc
	sec
	sbc	#space-1
	tcs
	phd
	tcd

	stz	retval
	stz	retval+2

	lock	mutex
;
; get length of variable name
;
	short	a
	ldy	#0
findlen	lda	[var],y
	beq	gotlen
	iny
	bra	findlen
gotlen	long	a
	sty	len
;
; allocate a buffer to put the pascal string
;	
	iny
	pea	0
	phy
	jsl	~NEW
	sta	namebuf
	stx	namebuf+2
	sta	varparm
	stx	varparm+2
	ora	namebuf+2
	jeq	exit
;
; now make a pascal string from the c string
;	
	short	ai
	lda	len
	sta	[namebuf]
	ldy	#0
copyname	lda	[var],y
	beq	copydone
               iny
	sta	[namebuf],y
	bra	copyname
copydone	long	ai
;
; allocate a return buffer
;
	jsl	alloc256
	sta	newval
	stx	newval+2
	sta	varparm+4
	stx	varparm+6
	ora	newval+2
	jeq	exit0
;
; Let's go read the variable
;
	Read_Variable varparm
;
; Was it defined?
;
	lda	[newval]
	and	#$FF
	jeq	exit1	;return a null if not defined
;
; Make a return buffer to return the variable value
;
	inc	a
	pea	0
	pha
	jsl	~NEW
	sta	retval
	stx	retval+2
	ora	retval+2
	jeq	exit1
;
; And copy the resulting value
;
	short	ai
	lda	[newval]
	tay
copyval	cpy	#0
	beq	val1
	lda	[newval],y
	dey
	sta	[retval],y
	bra	copyval
val1	lda	[newval]
	tay
	lda	#0
	sta	[retval],y
	long	ai	
;
; hasta la vista, baby
;
exit1	ldx	newval+2
	lda	newval
	jsl	free256

exit0	pei	(namebuf+2)
	pei	(namebuf)
	jsl	nullfree

exit	unlock mutex
	ldy	retval
	ldx	retval+2
	lda	space+1
	sta	end-2
	lda	space
	sta	end-3
	pld
	tsc
	clc
	adc	#end-4
	tcs
	tya
	
	rtl
                          
varparm	ds	8
mutex	key

	END

rmemcpy	START
	subroutine (2:num,4:src,4:dest),0

	ldy	num
	beq	done
	tya
	dey
	bit	#1
	bne	odd
	dey
	bra	lp1
odd	short	m
	lda	[src],y
	sta	[dest],y
	long	m
	dey
	dey
	bmi	done
lp1	lda	[src],y
	sta	[dest],y
	dey
	dey
               bpl	lp1
done	return
	END
