*
* Modified ORCA/C setjmp() and longjmp() routines to save and restore sigmask
*
* Original code from orcalib 2.0.1 - Copyright Byte Works, Inc.
* changes by Derek Taubert
* June 14, 1995
*
* $Id: setjmp.asm,v 1.1 1997/02/28 05:12:45 gdr Exp $
*

	case on

dummy	start			; ends up in .root
	end

         LONGA ON
         LONGI ON
setjmp   START
         TSC   
         PHD   
         TCD   
         CLC   
         ADC   #$0004
         STA   [$04]	; stack pointer
         LDY   #$0002
         LDA   $01,S
         STA   [$04],Y	; direct page register
         LDY   #$0004
         LDA   $00
         STA   [$04],Y	; LSB program counter
         INY   
         INY   
         LDA   $02
         STA   [$04],Y	; MSB program counter
; start sigmask changes
	pea	$0000
	pea	$0000
	jsl	sigblock
	ldy	#$0008
	sta	[$04],Y	; LSB sigmask
	iny
	iny
	txa
	sta	[$04],Y	; MSB sigmask
; end sigmask changes
         PLD   
         PHB   
         PLX   
         PLY   
         PLA   
         PLA   
         PHY   
         PHX   
         PLB   
         LDA   #$0000
         RTL   
         END

         LONGA ON
         LONGI ON
longjmp  START
         TSC   
         TCD   
         PHB   
         PHK   
         PLB   
; start sigmask changes
	ldy	#$000A
	lda	[$04],Y	; MSB sigmask
	pha
	dey
	dey
	lda	[$04],Y	; LSB sigmask
	pha
	jsl	sigsetmask

         LDY   #$0006
lstk     LDA   [$04],Y
         STA   sp,Y
         DEY   
         DEY   
         BPL   lstk

         LDX   $08
         BNE   argok
         INX   
; end sigmask changes
argok    PLB   
         LDA   >sp
         TCS   
         LDA   >pc+$00000002
         STA   $02,S
         LDA   >pc
         STA   $00,S
         LDA   >dp
         TCD   
         TXA   
         RTL   
sp       DC    H'AD'
         DC    H'00'
dp       DC    H'00'
         DC    H'00'
pc       DC    H'19'
         DC    H'00'
         DC    H'00'
         DC    H'04'
         END

         LONGA ON
         LONGI ON
_setjmp  START
         TSC   
         PHD   
         TCD   
         CLC   
         ADC   #$0004
         STA   [$04]
         LDY   #$0002
         LDA   $01,S
         STA   [$04],Y
         LDY   #$0004
         LDA   $00
         STA   [$04],Y
         INY   
         INY   
         LDA   $02
         STA   [$04],Y
         PLD   
         PHB   
         PLX   
         PLY   
         PLA   
         PLA   
         PHY   
         PHX   
         PLB   
         LDA   #$0000
         RTL   
         END

         LONGA ON
         LONGI ON
_longjmp START
         TSC   
         TCD   
         PHB   
         PHK   
         PLB   
         LDX   $08
         BNE   argok
         INX   
argok    LDY   #$0006
lstk     LDA   [$04],Y
         STA   sp,Y
         DEY   
         DEY   
         BPL   lstk
         PLB   
         LDA   >sp
         TCS   
         LDA   >pc+$00000002
         STA   $02,S
         LDA   >pc
         STA   $00,S
         LDA   >dp
         TCD   
         TXA   
         RTL   
sp       DC    H'AD'
         DC    H'00'
dp       DC    H'00'
         DC    H'00'
pc       DC    H'19'
         DC    H'00'
         DC    H'00'
         DC    H'04'
         END
