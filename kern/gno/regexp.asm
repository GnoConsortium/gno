*	$Id: regexp.asm,v 1.1 1998/02/02 08:19:42 taubert Exp $
**************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*   Derek Taubert
*
**************************************************************************
*
* RegExp()
*
* Tim's Regular Expression Parser
* Written by Tim Meekins, 6/13/91
*
* Copyright 1991 by Tim Meekins
* This function is hereby donated to the public domain as long as I am
* given credit as the author.
*
* Version 1.0
*    - First release, not very fancy or efficient, but I wrote the entire
*      program in 2 hours and is has to be the world's shortest regular
*      expression parser!
*
*  "Look Ma, no non-deterministic finite state transition table!"
*
**************************************************************************
*
* INPUT:
*    PH4 addr        ;This is the address to a pattern for matching
*    PH4 addr        ;This is the text match against
*    PH4 word        ;This a flag. Bit 16 = 1 if case sensitive.
*    jsl RegExp
*
* OUTPUT:
*    On Exit A=0 if no match, else a match was found.
*
* EXAMPLE:
*   while ((*p != 0) && (RegExp(pattern,*p))) p++;
*
* This example will search sequentially through a list until a match is found.
* Or at least I hope this works, never actually tried this example.
*
* PATTERNS are built the following way:
*
*   pure text is matched directly to the text
*   '*' matchs 0 or more characters
*   '+' matches 1 or more characters
*   '?' matches 0 or 1 characters
*   '[..]' matches one of the characters contained in the brackets. If two
*        characters are separated by '-' then matches if the character is
*        within the range. If the first character in the list is a '^'
*        then a match will occur if NONE of the characters in brackets macth.
*   '\' exactly matches the following character. This lets you match the above
*       characters and '\' itself. Otherwise known as character quoting.
*
**************************************************************************
*
* meekins@cis.ohio-state.edu
* timm@pro-tc.cts.com
*
**************************************************************************

	mcopy m/regexp.mac
	case	on

RegExp	START

result	equ   0
ch	equ   result+2
negflag	equ   ch+2
space	equ   negflag+2

	subroutine (4:pattern,4:text,2:flag),space

	ld2   0,result

;=========================================================================
;
; PHASE 1. Match characters one by one
;
;=========================================================================

phase1	lda   [pattern]
	jsr   ToLower
	cmp   #0
	beq   patt0
	cmp   #'\'
	beq   quote
	cmp   #'+'
	beq   plus
	cmp   #'*'
	beq   star
	cmp   #'?'
	jeq   quest
	cmp   #'['
	jeq   lbrak

phase1a	sta   ch
	lda   [text]
	jsr   ToLower
	cmp   ch
	bne   done
	inc   pattern
	bne   inc01
	inc   pattern+2
inc01	inc   text
	bne   inc02
	inc   text+2
inc02	bra   phase1

patt0	lda   [text]
	and   #$FF
	bne   done
	bra   match

quote	anop                     ;do character quoting
	inc   pattern
	bne   inc03
	inc   pattern+2
inc03	lda   [pattern]
	jsr   ToLower
	cmp   #0
	beq   done
	bra   phase1a


;=========================================================================
;
; PHASE 2. non-deterministic matching
;
;=========================================================================

;
; Match one or more characters
;
plus	anop
	inc   text
	bne   inc04
	inc   text+2
inc04	lda   [text]
	and   #$FF
	beq   done
;
; Match 0 or more characters
;
star	anop
	inc   pattern
	bne   inc05
	inc   pattern+2
inc05	lda   [pattern]
	and   #$FF
	beq   match
starloop	lda   [text]
	and   #$FF
	beq   done
	pei   (pattern+2)
	pei   (pattern)
	pei   (text+2)
	pei   (text)
	pei   (flag)
	jsl   RegExp
	cmp   #0
	bne   match
	inc   text
	bne   starloop
	inc   text+2
	bra   starloop
;
; If a positive match is made, jump to match
; If no match is made, jump to done.
;

match	ld2   1,result
done	return 2:result

;
; Match 0 or 1 characters
;
quest	anop
	inc   pattern
	bne   inc06
	inc   pattern+2
inc06	pei   (pattern+2)
	pei   (pattern)
	pei   (text+2)
	pei   (text)
	pei   (flag)
	jsl   RegExp
	cmp   #0
	bne   match
	inc   text
	bne   inc07
	inc   text+2
inc07	pei   (pattern+2)
	pei   (pattern)
	pei   (text+2)
	pei   (text)
	pei   (flag)
	jsl   RegExp
	cmp   #0
	bne   match
	bra   done
;
; Match one character contained in brackets
;
lbrak	anop
	stz   negflag
	lda   [text]
	jsr   ToLower
	cmp   #0
	beq   done
	sta   ch
	ldy   #1
	lda   [pattern],y
	and   #$FF
	cmp   #'^'
	bne   lbrak3
	inc   negflag

lbrak2	iny
	lda   [pattern],y
	and   #$FF
lbrak3	cmp   #']'
	beq   braknomatch
	iny
	lda   [pattern],y
	dey
	and   #$FF
	cmp   #'-'
	beq   range

	lda   [pattern],y        ;match a single character
	jsr   ToLower
	cmp   #0
	jeq   done
	cmp   ch
	bne   lbrak2

brakmatch	lda   negflag
	beq   brakdone
	jmp   done

braknomatch	lda   negflag
	bne   brakdone2
	jmp   done

brakdone	iny
brakdone2	lda   [pattern],y
	and   #$FF
	jeq   done
	cmp   #']'
	bne   brakdone
	iny
	clc
	tya
	adc   pattern
	sta   pattern
	bne   inc08
	inc   pattern+2
inc08	inc   text
	bne   inc09
	inc   text+2
inc09	jmp   phase1

range	lda   [pattern],y
	iny2
	and   #$FF
	jeq   done
	dec   a
	cmp   ch
	bcc   range2
	lda   [pattern],y
	and   #$FF
	jeq   done
	bra   range3
range2	lda   [pattern],y
	and   #$FF
	jeq   done
	cmp   ch
	bcs   brakmatch
range3	jmp   lbrak2

;=========================================================================
;
; Takes a sixteen bit value, strips to 8 bit and converts to lower case.
;
;=========================================================================

ToLower	anop

	and   #$FF
	ldx   flag
	bmi   lowered
	if2   @a,cc,#'A',lowered
	if2   @a,cs,#'Z'+1,lowered
	add2  @a,#'a'-'A',@a

lowered	rts

	END
