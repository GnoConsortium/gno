***********************************************************************
*
* GNO command-line parser
* Version 1.0b
* Written by Tim Meekins
* Copyright (C) 1991-1996 by Procyon, Inc.
*
* This function will take the command-line passed to a utility and parse
* it into an argv,argc structure like those used in C programs. This
* was written NOT as a replacement for a C parser, but for use by assembly
* language programmers writing shell commands.
*
* This function ASSUMES that the ByteWorks Memory Manager has been started
* up and is usable. This will eventually be superceded by the GNO Memory
* Management library.
*
* This function is based on actual GNO/ME shell (gsh) parsing code.
*
* $Id: parsearg.asm,v 1.1 1997/02/28 05:12:47 gdr Exp $
*
**************************************************************************

	keep	parsearg
	mcopy parsearg.mac

dummy	START		; ends up in .root
	END

**************************************************************************
*
* Parse a single command
*
**************************************************************************

~GNO_PARSEARG	START
;
; TOKENS used by the parser
;
T_WORD	equ	1
T_EOF	equ	2

MAXARG	equ	256
;
; What state is the lexical analyzer in
;
NEUTRAL	equ	0	;a neutral state, get anything
INQUOTE	equ	1	;parsing a quoted string
INWORD	equ	2	;parsing a word
SINGQUOTE equ	3	;single quote string
;
; local variables and function parameters
;
ptr	equ	1
ch	equ	ptr+4
state	equ	ch+2
buf	equ	state+2
argv	equ	buf+4
word	equ	argv+4
temp	equ	word+4
argc	equ	temp+4
space	equ	argc+2
argptr	equ	space+3
commandline equ argptr+4
end	equ	commandline+4

; subroutine (4:commandline,4:argptr),space

	tsc
	sec
	sbc	#space-1
	tcs
	phd
	tcd

	add4	commandline,#8,buf

	ph4	#1024
	jsl	~NEW
	sta	word
	stx	word+2

	ph4	#MAXARG*4
	jsl	~NEW
	sta	argv
	stx	argv+2
	ldy	#2
	sta	[argptr]
	txa
	sta	[argptr],y

	stz	argc

loop	jsr	gettoken
	cmp	#T_WORD	
	beq	tok_word
	jmp	tok_eof
;
; Parse a word token
;
tok_word	if2	argc,ne,#MAXARG,word1
	jmp	done
word1	anop
	ldy	#0
lenloop	lda	[word],y
	iny
	and	#$FF
	bne	lenloop
	pea	0
	phy
	jsl	~NEW
	sta	temp
	stx	temp+2
	ora	temp+2
	bne	word2
	jmp	error
word2	lda	argc	;Copy word to argv[argc]
	asl2	a
	tay
	lda	temp
	sta	[argv],y
	lda	temp+2
	iny2
	sta	[argv],y
	ldy	#0
	short	a
copyloop	lda	[word],y
	sta	[temp],y
	iny
	cmp	#0
	bne	copyloop
	long	a

	inc	argc	;increment argument count
	jmp	loop
;
; Parse a command terminator
;
tok_eof	anop

	lda	argc
	asl2	a
	tay
	lda	#0
	sta	[argv],y
	iny2
	sta	[argv],y

	pei	(word+2)
	pei	(word)
	jsl	~DISPOSE

done	ldy	argc

exit	lda	space
	sta	end-3
	lda	space+1
	sta	end-2
	pld
	tsc
	clc
	adc	#end-4
	tcs

	tya
	rtl

error	anop

	ldy	#0
	jmp	exit

;=====================================================================
;
; lexical stage, return the next token
;
;=====================================================================

gettoken	anop

	mv4	word,ptr
;
; start in the neutral state
;
	ld2	NEUTRAL,state
;
; the main loop
;
lexloop	lda	[buf]
	inc	buf
	and2	@a,#$FF,ch
	bne	switch

	if2	state,ne,#INWORD,loop2
	jmp	endword

loop2	lda	#T_EOF
	jmp	lexdone
;
; jump to the current state
;
switch	lda	state
	asl	a
	tax
	jmp	(statetbl,x)
statetbl	dc	a2'case_neutral'
	dc	a2'case_inquote'
	dc	a2'case_inword'
	dc	a2'case_single'
;
; CASE NEUTRAL
;
case_neutral if2 ch,eq,#' ',lexloop	;space
	if2	@a,eq,#9,lexloop	;tab
	if2	@a,ne,#13,neut4	;return
	lda	#T_EOF
	jmp	lexdone
neut4	if2	@a,ne,#0,neut5	;EOF
	lda	#T_EOF
	jmp	lexdone
neut5	if2	@a,ne,#'"',neut7
startquote lda #INQUOTE
	bra	neut10
neut7	if2	@a,ne,#"'",neut8
startsingle lda #SINGQUOTE
	bra	neut10
neut8	if2	@a,ne,#'\',neut9
	lda	[buf]
	and	#$FF
	inc	buf
neut9	sta	[ptr]	;default
	inc	ptr
	lda	#INWORD
neut10	sta	state
	jmp	lexloop
;
; CASE INQUOTE
;
case_inquote if2 ch,ne,#'\',quote2	;is it a quoted character?
	lda	[buf]
	inc	buf
putword	sta	[ptr]
	inc	ptr
	jmp	lexloop
quote2	if2	@a,ne,#'"',putword
	ld2	INWORD,state
	jmp	lexloop
;
; CASE SINGLEQUOTE
;
case_single	anop
	if2	ch,ne,#"'",putword
	ld2	INWORD,state
	jmp	lexloop
;
; CASE INWORD
;
case_inword if2 ch,eq,#000,endword
	if2	@a,eq,#' ',endword
	if2	@a,eq,#009,endword
	if2	@a,eq,#013,endword
	cmp	#'"'
	jeq	startquote
	cmp	#"'"
	jeq	startsingle
	if2	@a,ne,#'\',putword
	lda	[buf]
	inc	buf
	bra	putword
endword	dec	buf
finiword	lda	#0
	sta	[ptr]
	lda	#T_WORD

lexdone	rts

	END

**************************************************************************
*
* Free the argv[] list
*
**************************************************************************

~GNO_FREEARG	START

space	equ	1
argc	equ	space+3
argv	equ	argc+2
end	equ	argv+4

; subroutine (4:argv,2:argc),space

	tsc
	phd
	tcd

	ldy	#0
loop	lda	argc
	beq	done
	dec	argc
	lda	[argv],y
	tax
	iny2
	lda	[argv],y
	iny2
	phy
	pha
	phx
	jsl	~DISPOSE
	ply
	bra	loop

done	pei	(argv+2)
	pei	(argv)
	jsl	~DISPOSE

	lda	space
	sta	end-3
	lda	space+1
	sta	end-2
	pld
	tsc
	clc
	adc	#end-4
	tcs

	rtl

	END
