*************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*
* $Id: prompt.asm,v 1.5 1998/08/03 17:30:23 tribby Exp $
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
promptgsbuf	equ	prompt+4
usergsbuf	equ	promptgsbuf+4
hour	equ	usergsbuf+4
minute	equ	hour+2
offset	equ	minute+2
pfx	equ	offset+2
space	equ	pfx+4

year	equ	hour
monday	equ	minute
precmd	equ	prompt

               subroutine (0:dummy),space

	ph4	#precmdstr	If "precmd" alias is defined,
	jsl	findalias
	sta	precmd
	stx	precmd+2
	ora	precmd+2
	beq	getvar

	pei	(precmd+2)	  execute it before parsing prompt.
	pei	(precmd)
	ph2	#0
	jsl	execute

;
; Get value of $PROMPT environment variable
;
getvar	ph4	#promptname
	jsl	getenv

               php		Turn off interrupts: mutex
               sei		 won't do what we want!

	sta	promptgsbuf	Save pointer to GS/OS result buffer.
	stx	promptgsbuf+2	If there is no memory to hold it,
	ora	promptgsbuf+2	 it's undefined, or it has a
	bne	parseprompt	  length of 0,
	ldx	#^dfltPrompt
	lda	#dfltPrompt
	jsr	puts		print the default prompt
               bra   donemark2		  and exit.

;
; Prompt string begins in result buffer after the two length words
;
parseprompt    anop
	clc
	lda	promptgsbuf
	adc	#4
	sta	prompt
	lda	promptgsbuf+2
	adc	#0
	sta	prompt+2

promptloop     lda   [prompt]
	incad	prompt
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

donemark2	anop
	plp		Restore interrupts.

	pei	(promptgsbuf+2)	Free $PROMPT value buffer
	pei	(promptgsbuf)
	jsl	nullfree

	jsr	flush
	return
;
; Previous character was a "%". Handle special character flags
;
special        lda   [prompt]	Get character following "%".
	incad	prompt
               and   #$FF
               beq   done	If end of string, all done.
               cmp   #'%'	If another "%",
               beq   _putchar		print the "%"
               cmp   #'h'	If 'h'
               beq   phist
               cmp   #'!'	 or '!',
               beq   phist		print history number.
               cmp   #'t'	If 't'
               beq   ptime
               cmp   #'@'	 or '@',
               beq   ptime		print time (am/pm format)
               cmp   #'S'	If 'S',
               jeq   pstandout		turn inverse mode on.
               cmp   #'s'	If 's',
               jeq   pstandend		turn inverse mode off.
               cmp   #'U'	If 'U',
               jeq   punderline		turn underline mode on.
               cmp   #'u'	If 'u',
               jeq   punderend		turn underline mode off
               cmp   #'d'	If 'd'
               jeq   pcwd
               cmp   #'/'	 or '/',
               jeq   pcwd		print working directory.
               cmp   #'c'	If 'c'
               jeq   pcwdend
               cmp   #'C'	 or 'C'
               jeq   pcwdend	
               cmp   #'.'	  or '.',
               jeq   pcwdend		print final part of wrk dir.
               cmp   #'n'	If 'n',
               jeq   puser		print $USER
               cmp   #'W'	If 'W',
               jeq   pdate1		print date (mm/dd/yy)
               cmp   #'D'	If 'D',
               jeq   pdate2		print date (yy-mm-dd)
	cmp	#'~'	If '~',
	jeq	ptilde		print wrk dir with ~ subs.
               jmp   promptloop	If none of these characters, ignore it.

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
; Set Stand Out (turn on inverse mode)
;
pstandout      jsr	standout
	jmp	promptloop
;
; UnSet Stand Out (turn off inverse mode)
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
pcwd	anop
	pea	0
               jsl	getpfxstr	Get value of prefix 0.
	sta	pfx
	stx	pfx+2

	ora	pfx+2	If NULL pointer returned,
	jeq	promptloop	 an error was reported.

	ldy	#4	Text starts at byte 4.
pcwd1          lda   [pfx],y	Get next
	and   #$FF	 character.
	beq	freepfx	Done when at end of string.
	jsr	toslash	Convert to slash.
	phy		Hold index on stack
	jsr	putchar	 while printing character.
	ply
	iny		Bump index
	bra	pcwd1	 and stay in loop.

freepfx	ph4	pfx	Free the current directory buffer.
	jsl	nullfree
	jmp   promptloop

;
; Tail of current working directory
;
pcwdend        anop
	pea	0
               jsl	getpfxstr	Get value of prefix 0.
	sta	pfx
	stx	pfx+2

	ora	pfx+2	If NULL pointer returned,
	jeq	promptloop	 an error was reported.

               ldy	#2	Get string's length word
	lda	[pfx],y	 from bytes 2 & 3.
               clc		Add 3 to get offset
	adc	#3	 from beginning of buffer.
	sta	offset
	tay
pcwdend1       dey		If we've backed up to the beginning,
               bmi   pcwdend2	 we can't go any further!
               lda   [pfx],y	Get next character.
               and   #$FF	
               cmp   #':'               If it's not ':',
               bne   pcwdend1	 keep searching backward.
pcwdend2       iny
               cpy   offset
               jeq   freepfx	Free the current directory buffer.
               lda   [pfx],y
               and   #$FF
               cmp   #':'
               jeq   freepfx	Free the current directory buffer.
               phy
	jsr	putchar
               ply
               bra   pcwdend2

;
; Current working directory substituting '~' if necessary
;
ptilde         anop
	pea	0
               jsl	getpfxstr	Get value of prefix 0.
	sta	pfx
	stx	pfx+2

	ora	pfx+2	If NULL pointer returned,
	jeq	promptloop	 an error was reported.
	lda	pfx	Otherwise, restore low-order address.

               clc		Add 4 to start of buffer
	adc	#4	 so it can be treated like
	bcc	pushad	  a c-string.
	inx
pushad	phx
	pha
	jsl	path2tilde	Convert $HOME to "~"
	phx		Push addr onto stack
	pha		  for nullfree.
	jsr	puts	Print tilde string.
	jsl	nullfree	Free the converted string.

	jmp	freepfx	Free the current directory buffer.

;                          
; Write user name
;
puser          ph4	#username	Get value of $USER
	jsl	getenv
	sta	usergsbuf	If buffer wasn't allocated
	stx	usergsbuf+2
	ora	usergsbuf+2
	beq	goploop	 ignore it.

	clc
	lda	usergsbuf	Text begins after
	adc	#4	 four bytes of
	bcc   printit	  length words.
	inx
printit	jsr	puts

	pei	(usergsbuf+2)	Free $USER value buffer
	pei	(usergsbuf)
	jsl	nullfree

goploop	jmp	promptloop

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
	incad	prompt
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


; Name of alias to execute before prompt
precmdstr	dc	c'precmd',h'00'

; Names of environment variables
promptname     gsstr	'prompt'
username	gsstr	'user'

; Default prompt
dfltPrompt     dc    c'% ',h'00'

num            dc    c'0000',h'00'

               END
