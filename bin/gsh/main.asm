**************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*
**************************************************************************
*
* MAIN.ASM
*   By Tim Meekins
*
**************************************************************************

	mcopy /obj/gno/bin/gsh/main.mac

dummy	start		; ends up in root
	end

	setcom 60

init	START
	jml	~GNO_COMMAND
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

argloop	dec	argc
	jeq	start
	clc
	lda	argv
               adc	#4
	sta	argv
	ldy	#2
	lda	[argv]
	sta	arg
	lda	[argv],y
	sta	arg+2
	lda	[arg]
	and	#$FF
	cmp	#'-'
	beq	intoption

; parse remaining args as a command to run

	inc	ExecFlag
	inc	FastFlag
	ph4	#1024
	jsl	~NEW
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
cmd1           lda	#' '
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
	bra	start

intoption	ldy	#1
optloop	lda	[arg],y
	and	#$FF
	beq	nextarg
	cmp	#'f'
	beq	optf
	cmp	#'c'
	beq	parsec

showusage	ErrWriteCString #usage
	bra	done

optf	inc	FastFlag

nextopt	iny
	bra	optloop

nextarg	cpy	#1
	beq	showusage
	jmp	argloop

parsec         clc
	lda	argv
	adc	#4
	sta	argv
	dec	argc
	beq	showusage
	inc	CmdFlag
	inc	FastFlag
	mv4	argv,CmdArgV
	mv2	argc,CmdArgC

start	case	on
	jsl	shell
	case	off

done	return

str	dc	h'0d0a0a'
 dc c'Before gsh may be run, the GNO/ME system, or kernel, must be running.'
	dc	h'0d0a0a00'

usage	dc	c'Usage: gsh [-cf] [argument...]',h'0d0a00'

	END
