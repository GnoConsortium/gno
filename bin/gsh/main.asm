**************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*
* $Id: main.asm,v 1.8 1999/01/14 17:44:25 tribby Exp $
*
**************************************************************************
*
* MAIN.ASM
*   By Tim Meekins
*   Modified by Dave Tribby for GNO 2.0.6
*
* Startup portion of shell
* 
* Note: text set up for tabs at col 16, 22, 41, 49, 57, 65
*              |     |                  |       |       |       |
*	^	^	^	^	^	^	
**************************************************************************

	mcopy /obj/gno/bin/gsh/main.mac

	setcom 60

**************************************************************************

; Segment for direct-page and stack

stack    data	STACK		; ends up in main.root
         kind	$12

; Define direct-page/stack and fill it with question marks it can be
; examined for how much is used.

	dc	128c'??'	;  256 bytes
	dc	128c'??'	;  512 bytes total
	dc	128c'????'	; 1024 bytes total
	dc	128c'????????'	; 2048 bytes total

	end

**************************************************************************

init	START

; Call the code to emulate C program startup:
;   store Accumulator in ~USER_ID, X- and Y- registers as ~COMMANDLINE
;   start up memory manager (~MM_INIT)
;   parse commandline (via ~GNO_PARSEARG) and push argc and arvg on stack

	jml	~GNO_COMMAND

; Control continues with the entry point "MAIN". When MAIN returns to
; ~GNO_COMMAND via rtl, it frees argv and argc before doing its own rtl.

	END


MAIN	START
	
	using	global

p	equ	0
arg	equ	p+4
space	equ	arg+4

	subroutine (2:argc,4:argv),space

	kernStatus @a
	bcc	ok

	ErrWriteCString #str
	jmp	done

ok	stz	FastFlag
	stz	CmdFlag
	stz	ExecFlag

; Parse gsh's command-line arguments.

argloop	dec	argc	Decrement argument count.
	jeq	go_start	If none left, ready to start working.
	clc
	lda	argv	Point to next
	adc	#4	 argument pointer.
	sta	argv
	ldy	#2
	lda	[argv]	Set arg to point to
	sta	arg	 the argument text.
	lda	[argv],y
	sta	arg+2
	lda	[arg]	Get first character
	and	#$FF	 of argument.
	cmp	#'-'	If it's a "-",
	beq	intoption	 handle as an option.


; Parse remaining args as a command to run (in ExecCmd)

	inc	ExecFlag
	inc	FastFlag
	ph4	#1024
	~NEW
	sta	ExecCmd
	sta	p
	stx	ExecCmd+2
	stx	p+2

cmd3	ldy	#0
cmd0	lda	[arg],y	
	and	#$ff
	beq	cmd1
	sta	[p],y
	iny
	bra	cmd0
cmd1	lda	#' '
	sta	[p],y
	sec		;inc a
	tya
	adc	p
	sta	p
	dec	argc
	beq	cmd2
	clc
	lda	argv
	adc	#4
	sta	argv
	ldy	#2
	lda	[argv]
	sta	arg
	lda	[argv],y
	sta	arg+2
	bra	cmd3
cmd2	lda	#0
	sta	[p]
go_start	bra	start


; Parse an argument as an option (first character is "-")

intoption	ldy	#1
optloop	lda	[arg],y
	and	#$FF
	beq	nextarg
	cmp	#'f'
	beq	optf
	cmp	#'c'
	beq	parsec

; Option is not recognized.
showusage	ErrWriteCString #usage
	bra	done


; Option = "-f": Skip history, gshrc
optf	inc	FastFlag

nextopt	iny
	bra	optloop

nextarg	cpy	#1
	beq	showusage
	jmp	argloop

; Option = "-c": execute shell commands found in file named by next argument
parsec	clc
	lda	argv
	adc	#4
	sta	argv
	dec	argc
	beq	showusage
	inc	CmdFlag
	inc	FastFlag
	mv4	argv,CmdArgV
	mv2	argc,CmdArgC

;
; When preliminary setup is complete, control transfers to here!
;
start	case	on
	jsl	shell
	case	off

done	return

str	dc	h'0d0a0a'
 dc c'Before gsh may be run, the GNO/ME system, or kernel, must be running.'
	dc	h'0d0a0a00'

usage	dc	c'Usage: gsh [-cf] [argument...]',h'0d0a00'

	END
