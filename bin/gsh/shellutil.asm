**************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*
* $Id: shellutil.asm,v 1.5 1998/08/03 17:30:24 tribby Exp $
*
**************************************************************************
*
* SHELLUTIL.ASM
*   By Tim Meekins
*   Modified by Dave Tribby for GNO 2.0.6
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

cmpdcstr	START

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
               ~NEW
               sta   gstr
               stx   gstr+2
	incad	@xa
	incad	@xa
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
	~NEW
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
	incad	q
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

ok	~DISPOSE

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
* For gsh 2.0: pass in addr of a GS/OS string, and pass back addr of
* allocated GS/OS result buffer (with null word added at end), not c-strings.
*
**************************************************************************

getenv	START
               
len	equ	1
retval	equ	len+2
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

	lock	mutex
;
; Get the variable's length using ReadVariableGS
;			Set up parameter block:
	mv4	var,RVname		Addr of name, from user.
               ld4	TempResultBuf,RVresult	Use temporary result buf.
	ReadVariableGS ReadVar		Get length.
;
; Allocate memory for value string
;
	lda	TempRBlen	Get length of value.
	bne	notnull	Return null if 0.
	sta	retval
	sta	retval+2
	bra	exit

notnull	inc2	a	Add 4 bytes for result buf len words.
	inc2	a
	sta	len	Save result buf len.
	inc	a	Add 1 more for terminating null byte.
	pea	0
	pha
	~NEW		Request the memory.
	sta	RVresult	Store address in ReadVariable
	stx	RVresult+2	 parameter block and
	sta	retval	  direct page pointer.
	stx	retval+2
	ora	retval+2	If address == NULL,
	beq	exit	 return NULL to user.

	lda	len	Store result buffer length
	sta	[retval]	 at beginning of buf.
;
; Read the full value into the allocated memory
;
	ReadVariableGS ReadVar
;
; Add null byte at end of text to make it work as a C string
;
	ldy	TempRBlen	Get length of value,
	iny4		 + 4 (for length words at start)
	lda	#0	Store zero at end of string.
	short	a
	sta	[retval],y
	long	a
;
; All done.
;
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
                          
mutex	key

; Parameter block for shell ReadVariableGS call (p 423 in ORCA/M manual)
ReadVar	anop
	dc	i2'3'	pCount
RVname	ds	4	Pointer to name (passed by user)
RVresult	ds	4	GS/OS Output buffer ptr
RVexpflag	ds	2	export flag

; GS/OS result buffer for getting the full length of the PATH env var.
TempResultBuf	dc	i2'5'	Only five bytes total.
TempRBlen	ds	2	Value's length returned here.
	ds	1	Only 1 byte for value.

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
