***********************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*
**************************************************************************
*
* EXPAND.ASM
*   By Tim Meekins
*
* Command line expansion routines.
*
**************************************************************************

               mcopy m/expand.mac
               keep  o/expand.mac

**************************************************************************
*
* glob the command line
*
**************************************************************************

glob           START

	using	vardata

count          equ   0
gname          equ   count+2
sepptr         equ   gname+4
eptr           equ   sepptr+4
exppath        equ   eptr+4
filesep        equ   exppath+4
shallweglob    equ   filesep+2
wordbuf        equ   shallweglob+2
ptr            equ   wordbuf+4
buf            equ   ptr+4
space          equ   buf+4

               subroutine (4:cmd),space
;
; Check for noglob variable and exit if it's set to something.
;
	lda	varnoglob
               beq   doglob
               jsl	alloc1024	;create a tmp output buffer buffer
               sta   buf
               stx   buf+2
               pei   (cmd+2)
               pei   (cmd)
               pei   (buf+2)
               pei   (buf)
               jsr   copycstr
               jmp   bye
;
; noglob isn't set, so now we can actually start.
;
doglob         jsl	alloc1024	;create an output buffer buffer
               sta   buf
               sta   ptr
               stx   buf+2
               stx   ptr+2
               jsl	alloc1024	;create a word buffer
               sta   wordbuf
               stx   wordbuf+2
;
; strip some white space
;
skipit         jsr   getbyte
               jeq   alldone
               if2   @a,eq,#' ',whitestuff
               if2   @a,eq,#009,whitestuff
               if2   @a,eq,#013,whitestuff
               if2   @a,eq,#010,whitestuff
	if2	@a,eq,#';',whitestuff
	if2	@a,eq,#'&',whitestuff
	if2	@a,eq,#'|',whitestuff
	if2	@a,eq,#'>',whitestuff
	if2	@a,eq,#'<',whitestuff
               stz   shallweglob
               ldy   #0
               bra   grabbingword
whitestuff     jsr   putbyte
               bra   skipit
;
; single out the next word [y is initialized above]
;
grabword       jsr   getbyte
grabbingword   if2   @a,eq,#"'",grabsingle
               if2   @a,eq,#'"',grabdouble
               if2   @a,eq,#'\',grabslash
               if2   @a,eq,#' ',procword
               if2   @a,eq,#009,procword
               if2   @a,eq,#013,procword
               if2   @a,eq,#010,procword
               if2   @a,eq,#000,procword
	if2	@a,eq,#';',procword
	if2	@a,eq,#'&',procword
	if2	@a,eq,#'|',procword
	if2	@a,eq,#'>',procword
	if2	@a,eq,#'<',procword
               if2   @a,eq,#'[',grabglob  
               if2   @a,eq,#']',grabglob
               if2   @a,eq,#'*',grabglob
               if2   @a,eq,#'?',grabglob
grabnext       sta   [wordbuf],y
               iny
               bra   grabword
grabglob       ldx   #1
               stx   shallweglob
               bra   grabnext
grabslash      sta   [wordbuf],y
               iny
               jsr   getbyte
               beq   procword
               bra   grabnext
grabsingle     sta   [wordbuf],y
               iny
               jsr   getbyte
               beq   procword
               if2   @a,eq,#"'",grabnext
               bra   grabsingle
grabdouble     sta   [wordbuf],y
               iny
               jsr   getbyte
               beq   procword
               if2   @a,eq,#'"',grabnext
               bra   grabdouble
;
; we've grabbed the next word, now process the word
;
procword       dec   cmd
               lda   #0
               sta   [wordbuf],y
;
; Shall we glob? Shall we scream? What happened, to our postwar dream?
;
      	lda   [wordbuf]
	and   #$FF
               if2   @a,eq,#'-',skipdeglob	;This allows '-?' option.
               lda   shallweglob
               bne   globword
;
; we didn't glob this word, so flush the word buffer
;
skipdeglob     ldy   #0
flushloop      lda   [wordbuf],y
               and   #$FF
               beq   doneflush
               jsr   putbyte
               iny
               bra   flushloop
doneflush      jmp   skipit
;
; Hello, boys and goils, velcome to Tim's Magik Shoppe
;
; Ok, here's the plan:
;  1. We give _InitWildcard a PATHNAME.
;  2. _NextWildcard returns a FILENAME.
;  3. We need to expand to the command-line the full pathname.
;  4. Therefore, we must put aside the prefix, and cat each file returned
;     from _NextWildcard to the saved prefix, but, we must still pass
;     the entire path to _InitWildcard.
;  5. This solves our problem with quoting. Expand the quotes before
;     passing along to _InitWildcard, BUT the saved prefix we saved to cat
;     to will still have the quotes in it, so that the tokenizer can deal
;     with it. Whew!
;
; Well, here goes nuthin'....  [Ya know, and I'm reading Levy's book 
; 'Hackers' right now...]
;
;
;
; Expand out the quoted stuff, and keep an eye out for that ubiquitous last
; filename separator... then we can isolate him!
;
globword       stz   filesep
	jsl	alloc1024
               sta   eptr
               stx   eptr+2
               sta   exppath
               stx   exppath+2

               inc   eptr               ;leave room for pascal length
               mv4   eptr,sepptr

               ldy   #0
exploop        lda   [wordbuf],y
               and   #$FF
               beq   endexp
               iny
               if2   @a,eq,#'\',expslash
               if2   @a,eq,#"'",expsingle
               if2   @a,eq,#'"',expdouble
               if2   @a,eq,#'/',expsep
               if2   @a,eq,#':',expsep
expput         sta   [eptr]
               inc   eptr
               bra   exploop
expsep         sty   filesep
               sta   [eptr]
               inc   eptr
               mv4   eptr,sepptr
               bra   exploop
expslash       lda   [wordbuf],y
               iny
               and   #$FF
               beq   endexp
               bra   expput
expsingle      lda   [wordbuf],y
               iny
               and   #$FF
               beq   endexp
               if2   @a,eq,#"'",exploop
               sta   [eptr]
               inc   eptr
               bra   expsingle
expdouble      lda   [wordbuf],y
               iny
               and   #$FF
               beq   endexp
               if2   @a,eq,#'"',exploop
               sta   [eptr]
               inc   eptr
               bra   expdouble
;
; We really didn't mean to expand the filename, so, copy it back again..
;
endexp         ldy   filesep
copyback       lda   [wordbuf],y
               iny
               and   #$FF
               sta   [sepptr]
               inc   sepptr
               cmp   #0
               bne   copyback
;
; save the length, heh, heh, 16-bit sub will do it!
;
               sub2  sepptr,exppath,@a
               dec2  a                  ;don't count length byte or \0!
               short a
               sta   [exppath]
               long  a
;
; We now have enough to call _InitWildCard!!!
; [ let's mutex the rest so we don't have to fix _InitWC and _NextWC ;-) ]
;
wait2          lda   mutex
               beq   wait2a
               cop   $7F
               bra   wait2
wait2a         inc   mutex
;
; start 'em up
;
               stz   count
               mv4   exppath,initWCparm
               Init_Wildcard initWCparm
;
; hey, we better get some memory for these new files...
;
               ph4   #65    
               jsl   ~NEW
               sta   gname
               stx   gname+2
               sta   nWCparm
               stx   nWCparm+2
;
; start the expansion dudes!
;
WCloop         Next_Wildcard nWCparm
               lda   [gname]
               and   #$FF
               beq   nomore
               inc   count
;
; get that owiginal path outta here!
;
               ldy   #0
outtahere      if2   @y,eq,filesep,globout
               lda   [wordbuf],y
               jsr   putspecial
               iny
               bra   outtahere
;
; now get that newly globbed file outta here
;
globout        lda   [gname]
               and   #$FF
               tax
               ldy   #1
globoutta      lda   [gname],y
               jsr   putspecial
               iny
               dex
               bne   globoutta
;
; well well well, one down, how many to go?
;
               lda   #' '
               jsr   putbyte
               bra   WCloop
;
; no more left, whatta we gonna do now!
;
nomore         anop
;
; no match
;
               lda   count
               bne   yesmore
	ldx	#^nomatch
	lda	#nomatch
	jsr	puts
	lda	#0
	sta	[buf]
yesmore        anop
;
; throw em away (we should probably alloc once, not each word... )
;
               pei   (gname+2)
               pei   (gname)
               jsl   nullfree
               ldx   exppath+2
               lda   exppath
               jsl   free1024
               dec   mutex
	lda	count
	beq	alldone2
               jmp   skipit
;
; Goodbye, cruel world, I'm leaving you today, Goodbye, goodbye.
;
alldone        jsr   putbyte
alldone2	ldx   wordbuf+2
               lda   wordbuf
               jsl   free1024

bye            return 4:buf
;
; get a byte from the original command-line
;
getbyte        lda   [cmd]
               inc   cmd
               and   #$FF
               rts
;
; put special characters. Same as putbyte, but if it is a special
; shell character then quote it.
;
putspecial     and   #$7F
	if2	@a,eq,#' ',special
	if2	@a,eq,#'.',special
	if2	@a,eq,#013,special
	if2	@a,eq,#009,special
	if2	@a,eq,#';',special
	if2	@a,eq,#'&',special
	if2	@a,eq,#'<',special
	if2	@a,eq,#'>',special
	if2	@a,eq,#'|',special
	bra	putbyte
special	pha
	lda	#'\'
	jsr	putbyte
	pla
;
; store a byte into the new command-line
;
putbyte        short a
               sta   [ptr]
               long  a
               inc   ptr
               rts

mutex          dc    i'0'

InitWCParm     ds    4
               dc    i2'%00000001'
nWCparm        ds    4

nomatch        dc    c'No match.',h'0d00'

               END

**************************************************************************
*
* Expand variables not in single quotes
*
* * Add error checking if out buf gets too big (> 1024)
* * Get rid of fixed buffers
*
**************************************************************************

expandvars     START

ptr            equ   1
;ptr            equ   0
buf            equ   ptr+4
space          equ   buf+4
cmd            equ   space+3
end            equ   cmd+4

;               subroutine (4:cmd),space

               tsc
               sec
               sbc   #space-1
               tcs
               phd
               tcd

	jsl	alloc1024
               sta   buf
               sta   ptr
               stx   buf+2
               stx   ptr+2

loop           jsr   getbyte
               jeq   done
               if2   @a,eq,#"'",quote
               if2   @a,eq,#'$',expand
               if2   @a,eq,#'~',tilde
               if2   @a,eq,#'\',slasher
               jsr   putbyte
               bra   loop

slasher        jsr   putbyte
               jsr   getbyte
               jsr   putbyte
               bra   loop

quote          jsr   putbyte
               jsr   getbyte
               jeq   done
               if2   @a,ne,#"'",quote
               jsr   putbyte
               bra   loop

tilde          anop
wait2          lda   mutex
               beq   wait2a
               cop   $7F
               bra   wait2
wait2a         inc   mutex

               short a
               lda   #'h'
               sta   name
               lda   #'o'
               sta   name+1
               lda   #'m'
               sta   name+2
               lda   #'e'
               sta   name+3
               long  a
               ldx   #4
               jmp   getval
;
; expand the variable since a '$' was encountered.
;
expand         anop
wait1          lda   mutex
               beq   wait1a
               cop   $7F
               bra   wait1
wait1a         inc   mutex

               lda   #0
               sta   name
	lda	[cmd]
	and	#$FF
	if2	@a,eq,#'{',braceexpand
	if2	@a,eq,#'<',stdinexpand
               ldx   #0
nameloop       lda   [cmd]
               and   #$FF
               beq   getval
               if2   @a,cc,#'0',getval
               if2   @a,cc,#'9'+1,inname
               if2   @a,cc,#'A',getval
               if2   @a,cc,#'Z'+1,inname
               if2   @a,eq,#'_',inname
               if2   @a,cc,#'a',getval
               if2   @a,cc,#'z'+1,inname
               bra   getval
inname         jsr   getbyte
               sta   name,x
               inx
               bra   nameloop
;
; expand in braces {}
;
braceexpand    jsr   getbyte
	ldx	#0
braceloop      lda   [cmd]
	and	#$FF
	beq	getval
	jsr	getbyte
	if2	@a,eq,#'}',getval
	sta	name,x
	inx
	bra	braceloop
;
; get text from standard input
;
stdinexpand    jsr   getbyte
	ReadLine (#value+1,#255,#13,#1),@a
	bra	storeval2
;
; get a value for this variable
;
getval         lda   #0
               sta   name,x
	ph4	#name
	jsr	c2pstr2
	phx
	pha
	sta	parm
	stx	parm+2
               Read_Variable parm
	jsl	nullfree
;
; store the variable value in the out buffer
;
storeval       lda	value
storeval2	and	#$FF
	beq	expanded
	tay
	ldx   #0
putval         lda   value+1,x
               jsr   putbyte
               inx
	dey	
               bne   putval

expanded       dec   mutex
               jmp   loop

done           jsr   putbyte

               ldx   buf+2
               ldy   buf

               lda   space
               sta   end-3
               lda   space+1
               sta   end-2
               pld
               tsc
               clc
               adc   #end-4
               tcs

               tya
               rtl

getbyte        lda   [cmd]
               inc   cmd
               and   #$FF
               rts

putbyte        short a
               sta   [ptr]
               long  a
               inc   ptr
               rts

mutex          dc    i'0'

parm           dc    a4'name'
               dc    a4'value'

name           ds    256
value          ds    256

               END
