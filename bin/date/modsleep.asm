* Modified sleep routine based on sleep in libgno

* It was necesary to disassemble this routine and reassemble it so that
* date.asm can use its private variables. It needs to do this to make
* date abort when it is in the middle of a sleep()

* Phillip Vandry, May 1993

		case	on

sleep		start
		tsc     
		sec     
		sbc    #$0014
		tcs     
		phd     
		tcd     
		ldx    $07
		phx     
		tsx     
		stx    $07
		pea	sleephandler|-16
		pea	sleephandler
		pea	14
		jsl	signal
		tay
		lda	$07
		tcs     
		pla     
		sta    $07
		tya     
		phx     
		pha     
		lda    $01,s
		sta    $0d
		lda    $03,s
		sta    $0f
		pla     
		pla     
		ldx    $07
		phx     
		tsx     
		stx    $07
		pea    $0000
		pea    $2000
		jsl    sigblock
		tay     
		lda    $07
		tcs     
		pla     
		sta    $07
		tya     
		phx     
		pha     
		lda    $01,s
		sta    $11
		lda    $03,s
		sta    $13
		pla     
		pla     
		lda    #$0000
		sta    ringring
		ldx    $07
		phx     
		tsx     
		stx    $07
		lda    $18
		ldx    #$0000
		phx     
		pha     
		jsl	alarm
		tay     
		lda    $07
		tcs     
		pla     
		sta    $07
		tya     
checkrr		anop
		lda	ringring
		tax     
		beq	theeor
		lda    #$0001
theeor		eor    #$0001
		bne	nojump
		brl	afterbrl
nojump		ldx    $07
		phx     
		tsx     
		stx    $07
		pei    $13
		pei    $11
		pla     
		and    #$dfff
		pha     
		jsl	sigpause
		tay     
		lda    $07
		tcs     
		pla     
		sta    $07
		tya     
		brl	checkrr
afterbrl	ldx    $07
		phx     
		tsx     
		stx    $07
		ldx    $0f
		lda    $0d
		phx     
		pha     
		lda    #$000e
		pha     
		jsl	signal
		tay     
		lda    $07
		tcs     
		pla     
		sta    $07
		tya     
		ldx    $07
		phx     
		tsx     
		stx    $07
		ldx    $13
		lda    $11
		phx     
		pha     
		jsl	sigsetmask
		tay     
		lda    $07
		tcs     
		pla     
		sta    $07
		tya     
		lda    $16
		sta    $18
		lda    $15
		sta    $17
		ldy    $05
		pld     
		tsc     
		clc     
		adc    #$0016
		tcs     
		tya     
		rtl     
		end

sleephandler	start
		tsc     
		sec     
		sbc    #$0006
		tcs     
		phd     
		tcd     
		lda    #$0001
		sta    ringring
		lda    $08
		sta    $0c
		lda    $07
		sta    $0b
		pld     
		tsc     
		clc     
		adc    #$000a
		tcs     
		rtl     
		end

ringring	data
		ds	2
		end

		case	off
