**************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*
* $Id: expand.asm,v 1.5 1998/08/03 17:30:28 tribby Exp $
*
**************************************************************************
*
* EXPAND.ASM
*   By Tim Meekins
*   Modified by Dave Tribby for GNO 2.0.6
*
* Command line expansion routines for wildcards and env vars
*
* Note: text set up for tabs at col 16, 22, 41, 49, 57, 65
*              |     |                  |       |       |       |
*	^	^	^	^	^	^	
**************************************************************************

	mcopy /obj/gno/bin/gsh/expand.mac

dummyexpand	start		; ends up in .root
	end

	setcom 60

**************************************************************************
*
* glob the command line and expand filename wildcard characters
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

; Allocate a buffer, copy the command line into it, and return.
               jsl	alloc1024
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
doglob         jsl	alloc1024	Create an output buffer.
	sta	buf
	stx	buf+2
	sta	ptr	ptr points to next
	stx	ptr+2	 position in buf.

	jsl	alloc1024	Create a word buffer.
	sta	wordbuf
	stx	wordbuf+2

;
; Find the beginning of the next word
;
findword	jsr   g_getbyte	Get character from command line.
               jeq   alldone
               if2   @a,eq,#' ',passthru
               if2   @a,eq,#009,passthru
               if2   @a,eq,#013,passthru
               if2   @a,eq,#010,passthru
	if2	@a,eq,#';',passthru
	if2	@a,eq,#'&',passthru
	if2	@a,eq,#'|',passthru
	if2	@a,eq,#'>',passthru
	if2	@a,eq,#'<',passthru

; It's not a simple pass-through character. See what needs to happen.

               stz   shallweglob
               ldy   #0
               bra   grabbingword

; For pass-through characters, just copy to output buffer.

passthru	jsr   g_putbyte
               bra   findword


;
; single out the next word [y is initialized above]
;
grabword       jsr   g_getbyte
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

; Default action (also completion of some of the other special cases)
grabnext       sta   [wordbuf],y	Save character in word buffer
               iny		 and bump its index.
               bra   grabword	Get next character of word.

; "[", "]", "*", "?"
grabglob       ldx   #1	Set "shallweglob"
               stx   shallweglob	 flag.
               bra   grabnext	Store char in word buf & get next.

; "\"
grabslash      sta   [wordbuf],y	Save "\" in word buffer
               iny		 and bump its index.
               jsr   g_getbyte	Get next character in cmd line.
               beq   procword	If null byte, word is terminated.
               bra   grabnext	Store char in word buf & get next.

; '"'
grabsingle     sta   [wordbuf],y	Save char in word buffer
               iny		 and bump its index.
               jsr   g_getbyte	Get next character in cmd line.
               beq   procword	If null byte, word is terminated.
               if2   @a,eq,#"'",grabnext If "'", store and grab next char.
               bra   grabsingle	Save new char and stay in this loop.

; "'"
grabdouble     sta   [wordbuf],y	Save char in word buffer
               iny		 and bump its index.
               jsr   g_getbyte	Get next character in cmd line.
               beq   procword	If null byte, word is terminated.
               if2   @a,eq,#'"',grabnext If '"', store and grab next char.
               bra   grabdouble	Save new char and stay in this loop.


;
; The complete word is in the buffer. Time to process it.
;
procword       dec   cmd	Decrement cmd line pointer.
               lda   #0	Terminate word buffer with
               sta   [wordbuf],y	 a null byte.

;
; Shall we glob? Shall we scream? What happened, to our postwar dream?
;
      	lda   [wordbuf]
	and   #$FF
               if2   @a,eq,#'-',skipdeglob	;This allows '-?' option.
               lda   shallweglob
               bne   globword
;
; we didn't glob this word, so copy the word buffer to the output buffer
;
skipdeglob     ldy   #0
flushloop      lda   [wordbuf],y
               and   #$FF
               beq   doneflush
               jsr   g_putbyte
               iny
               bra   flushloop
doneflush      jmp   findword

;
; Hello, boys and goils, velcome to Tim's Magik Shoppe
;
; Ok, here's the plan:
;  1. We give InitWildcardGS a PATHNAME.
;  2. NextWildcardGS returns a FILENAME.
;  3. We need to expand to the command-line the full pathname.
;  4. Therefore, we must put aside the prefix, and cat each file returned
;     from NextWildcardGS to the saved prefix, but, we must still pass
;     the entire path to InitWildcardGS.
;  5. This solves our problem with quoting. Expand the quotes before
;     passing along to InitWildcardGS, BUT the saved prefix we saved to cat
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
globword	stz	filesep	
	jsl	alloc1024	Alloc buffer for wildcard pattern.
	sta	exppath
	stx	exppath+2

	incad	@xa	Leave room for length word
	incad	@xa
	sta	eptr
	stx	eptr+2
	sta	sepptr
	stx	sepptr+2
                     
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
               incad	eptr
               bra   exploop
expsep         sty   filesep
               sta   [eptr]
               incad	eptr
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
               incad	eptr
               bra   expsingle
expdouble      lda   [wordbuf],y
               iny
               and   #$FF
               beq   endexp
               if2   @a,eq,#'"',exploop
               sta   [eptr]
               incad	eptr
               bra   expdouble
;
; We really didn't mean to expand the filename, so, copy it back again..
;
endexp         ldy   filesep
copyback       lda   [wordbuf],y
               iny
               and   #$FF
               sta   [sepptr]
               incad	sepptr
               cmp   #0
               bne   copyback
;
; Calculate length by subtracting sepptr from starting address
;
               sub2  sepptr,exppath,@a
               dec2  a                  Don't count length word or \0
	dec	a
               sta   [exppath]
;
; We now have enough to call InitWildCardGS!!!
; [ let's mutex the rest so we don't have to fix InitWC and NextWC ;-) ]
;
	lock	glob_mutex
;
; Call shell routine InitWildcardGS to initialize the filename pattern
;
	stz	count
	mv4	exppath,initWCpath
	InitWildcardGS initWCparm

	ph4	#65	Allocate memory for
	~NEW	 	expanded name.
	sta	gname	Store in direct
	stx	gname+2	 page pointer
	sta	nWCname	  and in NextWildcardGS
	stx	nWCname+2	   parameter block.
	lda	#65	Store maximum length at
	sta	[gname]	 beginning of result buf.
;
; Call shell routine NextWildcardGS to get the next name that matches.
;
WCloop         NextWildcardGS nWCparm
	ldy	#2
	lda	[gname],y
	beq	nomore
;
; Keep count of how many paths are expanded
;
	inc	count

;
; Copy the original path (up to file separator) to output buffer
;
	ldy	#0
outtahere	if2	@y,eq,filesep,globout
	lda	[wordbuf],y
	jsr	g_putspecial
	iny
	bra	outtahere
;
; Copy the expanded filename to output buffer
;
globout	ldy	#2	Get returned length
	lda	[gname],y	 from GS/OS buffer.
	tax
	ldy	#4
globoutta	lda	[gname],y	Copy next character
	jsr	g_putspecial	 into buffer.
	iny
	dex
	bne	globoutta
;
; Place blank as separator after name and see if more are expanded.
;
               lda   #' '
               jsr   g_putbyte
               bra   WCloop

;
; All of the names (if any) from this pattern have been expanded.
;
nomore         anop

	unlock glob_mutex
;
; Deallocate path buffers (we should probably alloc once, not each word... )
;
               pei   (gname+2)
               pei   (gname)
               jsl   nullfree
               ldx   exppath+2
               lda   exppath
               jsl   free1024

	lda	count	If somehing was expanded,
               jne   findword	 go find the next word.

; Nothing was expanded from the wildcard.  If we wanted to act like
; ksh, we could pass the original text by doing a "jmp skipdeglob".
; Since passing a bad filename can mess up some programs, we will
; print an error message for this name and continue processing others.

	ldx	#^nomatch
	lda	#nomatch
	jsr	errputs

	ldx	wordbuf+2
	lda	wordbuf
	jsr	errputs

	ldx	#^ignored
	lda	#ignored
	jsr	errputs

               jmp   findword	Go find the next word.

;
; Goodbye, cruel world, I'm leaving you today, Goodbye, goodbye.
;
alldone        jsr   g_putbyte
	ldx   wordbuf+2
               lda   wordbuf
               jsl   free1024

bye            return 4:buf


;
; Subroutine of glob: get a byte from the original command-line
;
g_getbyte	lda   [cmd]
               incad	cmd
               and   #$FF
               rts

;
; Subroutine of glob: put special characters. Same as g_putbyte, but
; if it is a special shell character then quote it.
;
g_putspecial	and   #$7F
	if2	@a,eq,#' ',special
	if2	@a,eq,#'.',special
	if2	@a,eq,#013,special
	if2	@a,eq,#009,special
	if2	@a,eq,#';',special
	if2	@a,eq,#'&',special
	if2	@a,eq,#'<',special
	if2	@a,eq,#'>',special
	if2	@a,eq,#'|',special
	bra	g_putbyte
special	pha
	lda	#'\'
	jsr	g_putbyte
	pla		; fall through to g_putbyte...
;
; Subroutine of glob: store a byte into the new command-line
;
g_putbyte	short	a
	sta	[ptr]
	long	a
	incad	ptr
	rts


glob_mutex	key

;
; Parameter block for shell InitWildcardGS call (p 414 in ORCA/M manual)
;
InitWCParm	dc	i2'2'	pCount
InitWCPath	ds	4	Path name, with wildcard
	dc	i2'%00000001'	Flags (this bit not documented!!!)

;
; Parameter block for shell NextWildcardGS call (p 414 in ORCA/M manual)
;
nWCparm	dc	i2'1'	pCount
nWCname	ds	4	Pointer to returned path name

nomatch	dc	c'No match: ',h'00'
ignored	dc	c' ignored',h'0d00'

               END

**************************************************************************
*
* Expand $variables and tildes not in single quotes
*
**************************************************************************

expandvars     START

; Maximum number of characters in an environment variable or $<
MAXVAL	equ	512


ptr            equ   1
buf            equ   ptr+4
dflag          equ   buf+4
space          equ   dflag+2
cmd            equ   space+3
end            equ   cmd+4

;               subroutine (4:cmd),space

               tsc
               sec
               sbc   #space-1
               tcs
               phd
               tcd

	stz	dflag	Delimiter flag = FALSE.

	jsl	alloc1024
               sta   buf
               sta   ptr
               stx   buf+2
               stx   ptr+2

loop           jsr   e_getbyte
               jeq   done
               if2   @a,eq,#"'",quote
               if2   @a,eq,#'$',expand
               if2   @a,eq,#'~',tilde
               if2   @a,eq,#'\',slasher
               jsr   e_putbyte
               bra   loop

slasher        jsr   e_putbyte
               jsr   e_getbyte
               jsr   e_putbyte
               bra   loop

quote          jsr   e_putbyte
               jsr   e_getbyte
               jeq   done
               if2   @a,ne,#"'",quote
               jsr   e_putbyte
               bra   loop

;
; Tilde expansion: use the contents of $home, but make sure the
; path delimiter(s) match what the user wants (either "/" or ":")
;
tilde          anop
	lock	exp_mutex

               lda   #"oh"	Strangely enough,
               sta   name	 this spells "home"!
               lda   #"em"
               sta   name+2
	sta	dflag	Delimiter flag = TRUE.
               ldx   #4
	jmp	getval

;
; Expand an environment variable since a '$' was encountered.
;
expand         anop
	lock	exp_mutex

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
inname         jsr   e_getbyte
	cpx	#255	Only the first 255 characters
	beq	nameloop	 are significant.
               sta   name,x
               inx
               bra   nameloop

;
; expand in braces {}
;
braceexpand    jsr   e_getbyte
	ldx	#0
braceloop      lda   [cmd]
	and	#$FF
	beq	getval
	jsr	e_getbyte
	if2	@a,eq,#'}',getval
	cpx	#255	Only the first 255 characters
	beq	braceloop	 are significant.
	sta	name,x
	inx
	bra	braceloop

;
; get text from standard input
;
stdinexpand    jsr   e_getbyte
	ReadLine (#value,#MAXVAL,#13,#0),@a
	sta	valueln
	bra	chklen

;
; Get a value for this variable
;
getval         stx   nameln	Save length of name.
               ReadVariableGS ReadVarPB	Read its value.

	lda	valueln	Get value length.
	cmp	#MAXVAL+1	If > maximum allowed length,
	bcs	expanded	 we didn't get anything.
	cmp	#0
chklen	anop
	beq	expanded           If 0, nothing to do
	tax		Save length in X-reg.
	lda	dflag	If delimiter flag isn't set,
	beq	storeval	 go store the variable value

; Check to see if delimiters in the variable need to be switched
	lda	valueln	Set up length in
	tax		  X-reg and get
	lda	[cmd]	    next command line
	and	#$FF	      character.
	cmp	#"/"	If it's a slash, see if
	beq	chkvarslash	 variable needs to convert to slash.
	cmp	#":"	If it's not a colon,
	bne	storeval	 no need to convert.
	lda	value	Get first character of value.
	and	#$FF
	cmp	#"/"	If it's not a slash,
	bne	storeval	 no need to convert.

; Convert variable from "/" to ":" delimiter
	short m
chk_s	lda	value-1,x
	cmp	#"/"
	bne	bump_s
	lda	#":"
	sta	value-1,x
bump_s	dex
	bpl	chk_s
	long	m
	bra	storeval

chkvarslash	anop
	lda	value	Get first character of value.
	and	#$FF
	cmp	#":"	If it's not a colon,
	bne	storeval	 no need to convert.

; Convert variable from ":" to "/" delimiter
	short m
chk_c	lda	value-1,x
	cmp	#":"
	bne	bump_c
	lda	#"/"
	sta	value-1,x
bump_c	dex
	bpl	chk_c
	long	m

;
; Store the variable's value in the out buffer
;
storeval	anop
	lda	valueln	Get length.
	tay		Save length in Y-reg.
	ldx   #0	Use X-reg in index value.
putval         lda   value,x
               jsr   e_putbyte
               inx
	dey	
               bne   putval

expanded       unlock exp_mutex
	stz	dflag	Delimiter flag = FALSE.
               jmp   loop

done           jsr   e_putbyte

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

e_getbyte	lda	[cmd]
	incad	cmd
	and	#$FF
	rts

e_putbyte	short	a
	sta	[ptr]
	long	a
	incad	ptr
               rts

exp_mutex	key


; GS/OS string to hold variable name
namestr	anop
nameln	ds	2	Length of name
name           ds    256	Room for 255 chars + 0.


; GS/OS result buffer to hold up to MAXVAL bytes of value
valueresult	anop
	dc	i2'MAXVAL+4'	Length of result buffer
valueln	ds	2	Length of value
value          ds    MAXVAL	Room for MAXVAL chars


; Parameter block for shell ReadVariableGS calls
ReadVarPB	anop
	dc	i2'3'	pCount
RVname	dc	a4'namestr'	Name (pointer to GS/OS string)
RVvalue	dc	a4'valueresult'	Value (ptr to result buf or string)
RVexport	ds	2	Export flag

               END
