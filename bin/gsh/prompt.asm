*************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*
* $Id: prompt.asm,v 1.3 1998/06/30 17:25:50 tribby Exp $
*
**************************************************************************
*
* PROMPT.ASM
*   By Tim Meekins
*   Modified by Dave Tribby for GNO 2.0.6
*
* This displays the command-line prompt
*
* Note: text set up for tabs at col 16, 22, 41, 49, 57, 65
*              |     |                  |       |       |       |
*	^	^	^	^	^	^	
**************************************************************************

	mcopy /obj/gno/bin/gsh/prompt.mac

dummyprompt	start		; ends up in .root
	end

	setcom 60

WritePrompt    START

	using HistoryData


prompt	equ	0
hour           equ   prompt+4
minute         equ   hour+2
offset         equ   minute+2
pfx	equ	offset+2
space          equ   pfx+4

year           equ   hour
monday         equ   minute
precmd         equ	prompt

               subroutine (0:dummy),space

	ph4	#precmdstr
	jsl	findalias
	sta	precmd
	stx	precmd+2
	ora	precmd+2
	beq	getvar

	pei	(precmd+2)
	pei	(precmd)
	ph2	#0
	jsl	execute

getvar	Read_Variable promptparm

               php
               sei		;interrupt free environment

               lda   promptbuf
               and   #$FF
               bne   parseprompt

	ldx	#^dfltPrompt
	lda	#dfltPrompt
	jsr	puts

               bra   donemark2

precmdstr	dc	c'precmd',h'00'

parseprompt    anop

	ph4	#promptbuf
	jsr	p2cstr
	phx		;for disposal
	pha
	stx	prompt+2
	sta	prompt

promptloop     lda   [prompt]
	inc	prompt
               and   #$FF
               beq   done
               cmp   #'%'
               beq   special
               cmp   #'!'
               jeq   phist
	cmp	#'\'
	jeq	quoteit
_putchar       jsr   putchar
               bra   promptloop

done           jsr	standend
	jsr	cursoron
	jsl	nullfree
donemark2      plp
	jsr	flush
               return

special        lda   [prompt]
	inc	prompt
               and   #$FF
               beq   done
               cmp   #'%'
               beq   _putchar
               cmp   #'h'
               beq   phist
               cmp   #'!'
               beq   phist
               cmp   #'t'
               beq   ptime
               cmp   #'@'
               beq   ptime
               cmp   #'S'
               jeq   pstandout
               cmp   #'s'
               jeq   pstandend
               cmp   #'U'
               jeq   punderline
               cmp   #'u'
               jeq   punderend
               cmp   #'d'
               jeq   pcwd
               cmp   #'/'
               jeq   pcwd
               cmp   #'c'
               jeq   pcwdend
               cmp   #'C'
               jeq   pcwdend
               cmp   #'.'
               jeq   pcwdend
               cmp   #'n'
               jeq   puser
               cmp   #'W'
               jeq   pdate1
               cmp   #'D'
               jeq   pdate2
	cmp	#'~'
	jeq	ptilde

               jmp   promptloop
;
; Put history number
;
phist          lda   lasthist
               inc   a
               jsr   WriteNum
               jmp   promptloop
;
; Print current time
;
ptime          ReadTimeHex (minute,hour,@a,@a)
               lda   hour
               and   #$FF
               if2   @a,cc,#13,ptime2
               sub2  @a,#12,@a
ptime2         if2	@a,ne,#0,ptime2b
	lda	#12
ptime2b	jsr   WriteNum
	lda	#':'
	jsr	putchar
               lda   minute
               xba
               and   #$FF
               pha
               cmp   #10
               bcs   ptime2a
	lda	#'0'
	jsr	putchar
ptime2a        pla
               jsr   WriteNum
               lda   hour
               and   #$FF
               if2   @a,cs,#12,ptime3
ptime5         lda   #'a'
               bra   ptime4
ptime3         lda   #'p'
ptime4         jsr   putchar
	lda	#'m'
	jmp	_putchar
;
; Set Stand Out
;
pstandout      jsr	standout
	jmp	promptloop
;
; UnSet Stand Out
;
pstandend      jsr	standend
	jmp	promptloop
;
; Set Underline
;
punderline     jsr	underline
	jmp	promptloop
;
; UnSet Underline
;
punderend      jsr	underend
	jmp	promptloop
;                             
; Current working directory
;
pcwd           jsl	alloc1024
	sta	pfx
	stx	pfx+2
               sta	GPpfx
	stx	GPpfx+2
	lda	#1024
	sta	[pfx]

	GetPrefix GPParm
               ldy   #2
	lda	[pfx],y
               clc
	adc	#3
	sta	offset
	ldy	#4
pcwd1          lda   [pfx],y
               and   #$FF
	jsr	toslash
         	phy
	jsr	putchar
               ply
               iny
               cpy   offset
               bcc   pcwd1
               ldx	pfx+2
	lda	pfx
	jsl   free1024
	jmp   promptloop
;
; Current tail of working directory
;
pcwdend        anop
	jsl	alloc1024
	sta	pfx
	stx	pfx+2
               sta	GPpfx
	stx	GPpfx+2
	lda	#1024
	sta	[pfx]

	GetPrefix GPParm
               ldy	#2
	lda	[pfx],y
               clc
	adc	#3
	sta	offset
	tay
pcwdend1       dey
               bmi   pcwdend2
               lda   [pfx],y
               and   #$FF
               cmp   #':'
               bne   pcwdend1
pcwdend2       iny
               cpy   offset
               beq   pcwdend3
               lda   [pfx],y
               and   #$FF
               cmp   #':'
               beq   pcwdend3
               phy
	jsr	putchar
               ply
               bra   pcwdend2
pcwdend3       ldx	pfx+2
	lda	pfx
	jsl	free1024
	jmp   promptloop
;
; Current working directory substituting '~' if necessary
;
ptilde         anop
	jsl	alloc1024
	sta	pfx
	stx	pfx+2
               sta	GPpfx
	stx	GPpfx+2
	lda	#1024
	sta	[pfx]

	GetPrefix GPParm
               ldy	#2
	lda	[pfx],y
               clc
	adc	#4
	tay
               lda   #0
               sta   [pfx],y
	pei	(pfx+2)
	lda	pfx
               clc
	adc	#4
	pha
	jsl	path2tilde
	phx
	pha
	jsr	puts
	jsl	~DISPOSE
               ldx	pfx+2
	lda	pfx
	jsl	free1024
	jmp   promptloop
;                          
; Write user name
;
puser          Read_Variable userparm
	ldx	#^buf2
	lda	#buf2
	jsr	putp
               jmp   promptloop
;
; Write date as mm/dd/yy
;
pdate1         ReadTimeHex (@a,year,monday,@a)
               lda   monday
               and   #$FF00
               xba
               inc   a
               jsr   WriteNum
	lda	#'/'
	jsr	putchar
               lda   monday
               and   #$FF
               inc   a
               jsr   WriteNum
	lda	#'/'
	jsr	putchar
               lda   year
               and   #$FF00
               xba
               jsr   WriteNum
               jmp   promptloop
;
; Write date as yy-mm-dd
;
pdate2         ReadTimeHex (@a,year,monday,@a)
               lda   year
               and   #$FF00
               xba
               jsr   WriteNum
	lda	#'-'
	jsr	putchar
               lda   monday
               and   #$FF00
               xba
               inc   a
               jsr   WriteNum
	lda	#'-'
	jsr	putchar
               lda   monday
               and   #$FF
               inc   a
               jsr   WriteNum
               jmp   promptloop
;
; check for \ quote
;
quoteit	lda   [prompt]
	inc	prompt
               and   #$FF
               jeq   done
	cmp	#'n'
	beq	newline
	cmp	#'r'
	beq	newline
	cmp	#'t'
	beq	tab
	cmp	#'b'
	beq	backspace
	jmp	_putchar
newline	lda	#13
	jmp	_putchar
tab	lda	#9
	jmp	_putchar
backspace	lda	#8
	jmp	_putchar
;                  
; Write a number between 0 and 9,999
; 
WriteNum       cmp   #10
               bcs   write1
               adc   #'0'
	jmp	putchar
write1         cmp   #100
               bcs   write2
               Int2Dec (@a,#num+2,#2,#0)
	ldx	#^num+2
	lda	#num+2
	jmp	puts
write2         cmp   #1000
               bcs   write3
               Int2Dec (@a,#num+1,#3,#0)
	ldx	#^num+1
	lda	#num+1
	jmp	puts
write3         Int2Dec (@a,#num,#4,#0)
	ldx	#^num
	lda	#num
	jmp	puts

GPParm         dc    i2'2'
               dc    i2'0'
GPpfx	dc    a4'0'
promptparm     dc    a4'promptname'
               dc    a4'promptbuf'
promptname     str   'prompt'
dfltPrompt     dc    c'% ',h'00'
num            dc    c'0000',h'00'
promptbuf      ds    256
userparm       dc    a4'user'
               dc    a4'buf2'
user           str   'user'
buf2           ds    256

               END
