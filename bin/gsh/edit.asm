**************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*
* $Id: edit.asm,v 1.10 1998/12/31 18:29:12 tribby Exp $
*
**************************************************************************
*
* EDIT.ASM
*   By Tim Meekins
*   Modified by Dave Tribby for GNO 2.0.6
*
* The GNO/Shell command-line editor
*
* Note: text set up for tabs at col 16, 22, 41, 49, 57, 65
*              |     |                  |       |       |       |
*	^	^	^	^	^	^	
**************************************************************************

	mcopy /obj/gno/bin/gsh/edit.mac

dummyedit	start		; ends up in .root
	end

	setcom 60

RAW	gequ	$20
CRMOD	gequ	$10
ECHO	gequ	$08
CBREAK	gequ	$02
TANDEM	gequ	$01

OAMAP	gequ	0001		/* map OA-key to some sequence */
OA2META	gequ	0002		/* map OA-key to meta-key */
OA2HIBIT	gequ	0004		/* map OA-key to key|0x80 */
VT100ARROW	gequ	0008		/* map arrows to vt100 arrows */

TIOCGETP	gequ	$40067408
TIOCSETP	gequ	$80067409
TIOCGETK	gequ	$40027414
TIOCSETK	gequ	$80027413
TIOCGLTC	gequ	$40067474
TIOCSLTC	gequ	$80067475

cmdbuflen	gequ	1024

; editor key commands

undefined_char	gequ	0	;<- DO NOT CHANGE THIS DEFINITION
raw_char	gequ	1	;<- DO NOT CHANGE THIS DEFINITION
map_char	gequ	2
backward_char	gequ	3
forward_char	gequ	4
up_history	gequ	5
down_history	gequ	6
beginning_of_line gequ 7
end_of_line	gequ	8
complete_word	gequ	9
newline_char	gequ	10
clear_screen	gequ	11
redisplay	gequ	12
kill_whole_line gequ 13
lead_in	gequ	14
backward_delete_char gequ 15
backward_word	gequ	16
forward_word	gequ	17	
list_choices	gequ	18
kill_end_of_line gequ 19
toggle_cursor	gequ	20
delete_char	gequ	21

WORDGS_SIZE	gequ	256	Size of buffer for word search

**************************************************************************
*
* get a command line from the user
*
**************************************************************************

GetCmdLine	START

	using global
	using HistoryData
	using	pdata
	using	keybinddata
	using	termdata
	using	vardata

	stz	signalled
	stz	cmdlen
	stz	cmdloc
	stz	currenthist
	stz	currenthist+2

	ioctl	(#1,#TIOCGETP,#oldsgtty)
	ioctl	(#1,#TIOCGETP,#newsgtty)
	lda	#CBREAK+CRMOD
	sta	sg_flags
	ioctl	(#1,#TIOCSETP,#newsgtty)

	ioctl (#1,#TIOCGETK,#oldttyk)
	ioctl (#1,#TIOCSETK,#newttyk)

	ioctl (#1,#TIOCGLTC,#oldltc)
	ioctl (#1,#TIOCGLTC,#newltc)
	short	m
	lda	#-1
	sta	newltc+1
	long	m
	ioctl (#1,#TIOCSLTC,#newltc)

cmdloop	lda	#keybindtab
	sta	0
	lda	#^keybindtab
	sta	2
nextchar	jsr	cursoron
	jsr	flush
	jsr	getchar
	sta	4
	ldx	signalled	If signal was received,
	beq	nextchar2
	jsr	cmdsig	 acknowledge it
	bra	cmdloop	  and continue reading.

nextchar2	jsr	cursoroff
	lda	4	Get results of getchar.
	cmp	#-1	EOF?
	beq	eof
	cmp	#4	CTL-D? (treated same as EOF)
	beq	eof
	cmp	#$FF00	Error?
	bcc	findcmd
	and	#$00FF
	sta	ErrError
	ErrorGS Err	  yes--print error code.
	bra	reterr
;
; Parameter block for shell ErrorGS call (p 393 in ORCA/M manual)
;
Err	dc	i2'1'	pCount
ErrError	ds	2	Error number

eof	ldx	cmdlen
	bne	findcmd
	lda	varignore
	bne	findcmd
reterr	jsr	cursoron
	ioctl	(#1,#TIOCSETP,#oldsgtty)
	ioctl (#1,#TIOCSETK,#oldttyk)
	ioctl (#1,#TIOCSLTC,#oldltc)
	sec
	rts

findcmd	asl	a
	sta	addidx+1
	asl	a
addidx	adc	#0
	tay
	lda	[0],y	;get the type of key this is
	asl	a
	tax
	jsr	(keytab,x)
	jmp	cmdloop

keytab	dc	i'beep'	;undefined-char
	dc	i'cmdraw'	;raw-char
	dc	i'cmdloop'	;map-char
	dc	i'cmdleft'	;backward-char
	dc	i'cmdright'	;forward-char
	dc	i'PrevHistory'	;up-history
	dc	i'NextHistory'	;down-history
	dc	i'cmdbegin'	;beginning-of-line
	dc	i'cmdend'	;end-of-line
	dc	i'dotab'	;complete-word
	dc	i'cmdnewline'	;newline
	dc	i'cmdclearscrn'	;clear-screen
	dc	i'cmdredraw'	;redisplay
	dc	i'cmdclrline'	;kill-whole-line
	dc	i'cmdleadin'	;lead-in
	dc	i'cmdbackdel'	;backward-delete-char
	dc	i'cmdleftword'	;backward-word
	dc	i'cmdrightword'	;forward-word
	dc	i'cmdmatch'	;list-choices
	dc	i'cmdclreol'	;kill-end-of-line
	dc	i'cmdcursor'	;toggle-cursor
	dc	i'cmddelchar'	;delete-char

;-------------------------------------------------------------------------
;
; it's multiple character command
;
;-------------------------------------------------------------------------

cmdleadin	pla		;kill return address
	iny
	iny
	lda	[0],y
	tax
	iny
	iny
	lda	[0],y
	sta	0+2
	stx	0
	jmp	nextchar

;-------------------------------------------------------------------------		          
;
; Insert or overwrite an alphanum character
;
;-------------------------------------------------------------------------

cmdraw	lda	cmdlen
	cmp	#cmdbuflen
	bcc	cmIns0
	jmp	beep

cmIns0	lda	insertflag
	beq	cmOver	;Do overstrike mode
	short a
	ldy	cmdlen
cmIns1	cpy	cmdloc
	beq	cmins2
	bcc	cmIns2
	lda	cmdline-1,y
	sta	cmdline,y
	dey
	bra	cmIns1
cmIns2	long	a
	inc	cmdlen
;
; Place character in string and output
;
cmOver	lda	4
	ldy	cmdloc	;Do overstrike mode
	short a
	sta	cmdline,y
	long	a
	iny
	sty	cmdloc
	jsr	putchar
	ldy	cmdloc
	cpy	cmdlen
	bcc	cmdov2 
	beq	cmdov2 
	sty	cmdlen
;
; Redraw shifted text
;
cmdov2	lda	insertflag
	cmp	#0
	bne	cmdov2a
	rts

cmdov2a	ldx	#0
cmdov3	if2	@y,eq,cmdlen,cmdov4
	lda	cmdline,y
	iny
	inx
	phx
	phy
	jsr	putchar
	ply
	plx
	bra	cmdov3
cmdov4	jmp	moveleft

;-------------------------------------------------------------------------
;
; If the shell is interrupted during an edit
;
;-------------------------------------------------------------------------

cmdsig	stz	signalled
	jmp	cmdredraw

;-------------------------------------------------------------------------
;
; end of command...newline
;
;-------------------------------------------------------------------------

cmdnewline	pla		;pull off return address
	sec
	lda	cmdlen
	sbc	cmdlen
	tax
	jsr	moveright
	ldx	cmdlen	;strip trailing space
	beq	retdone
fix	dex
	lda	cmdline,x
	and	#$FF
	if2	@a,eq,#' ',fix
fix0	inx
	stx	cmdlen
	stz	cmdline,x	;terminate string
	txy
	beq	retdone
	ph4	#cmdline
	jsl	InsertHistory
retdone	ioctl	(#1,#TIOCSETP,#oldsgtty)
	ioctl (#1,#TIOCSETK,#oldttyk)
	ioctl (#1,#TIOCSLTC,#oldltc)
	clc
	rts

;-------------------------------------------------------------------------
;
; moved character to the left
;
;-------------------------------------------------------------------------

cmdleft	lda	cmdloc
	bne	ctl0a
	jmp	beep
ctl0a	dec	a
	sta	cmdloc
	ldx	#1
	jmp	moveleft

;-------------------------------------------------------------------------
;
; move character to the right
;
;-------------------------------------------------------------------------

cmdright	ldy	cmdloc
	if2	@y,ne,cmdlen,ctl1a
	jmp	beep
ctl1a	lda	cmdline,y
	jsr	putchar
	inc	cmdloc
	rts

;-------------------------------------------------------------------------
;
; show matching files
;
;-------------------------------------------------------------------------

cmdmatch	lda	cmdlen	
	beq	dontdomatch
	jsr	domatcher
	jmp	cmdredraw
dontdomatch	rts

;-------------------------------------------------------------------------
;
; Clear entire input line 
;
;-------------------------------------------------------------------------

cmdclrline	ldx	cmdloc
	jsr	moveleft
	stz	cmdloc	;fall through to cmdclreol

;-------------------------------------------------------------------------
;
; Clear to end of line
;
;-------------------------------------------------------------------------

cmdclreol	lda	cdcap
	ora	cdcap+2
	beq	ctl4a0
	tputs (cdcap,#1,#outc)
	bra	ctl4g

ctl4a0	sub2	cmdlen,cmdloc,@a
	inc	a
	tax
	tay
	phx
ctl4a	phy
	lda	#' '
	jsr	putchar
	ply
	dey
	bne	ctl4a
	plx
	jsr	moveleft

ctl4g	lda	cmdloc
	sta	cmdlen
	rts

;-------------------------------------------------------------------------
;
; redraw command-line
;
;-------------------------------------------------------------------------

cmdredraw	jsr	newline
redraw2	jsl	WritePrompt
	ldx	cmdlen
	stz	cmdline,x
	ldx	#^cmdline
	lda	#cmdline
	jsr	puts
	sec
	lda	cmdlen
	sbc	cmdloc
	tax
	jmp	moveleft

;-------------------------------------------------------------------------
;
; clear screen
;
;-------------------------------------------------------------------------

cmdclearscrn	jsr	clearscrn
	bra	redraw2

;-------------------------------------------------------------------------
;
; Insert toggle
;
;-------------------------------------------------------------------------

cmdcursor	eor2	insertflag,#1,insertflag
	rts

;-------------------------------------------------------------------------
;			    
; delete character to left
;
;-------------------------------------------------------------------------

cmdbackdel	lda	cmdloc
	bne	ctldel2
	jmp	beep
ctldel2	dec	a
	sta	cmdloc
	ldx	#1
	jsr	moveleft	;fall through to cmddelchar

;-------------------------------------------------------------------------
;
; Delete character under cursor
;
;-------------------------------------------------------------------------

cmddelchar	ldy	cmdloc
	if2	@y,ne,cmdlen,cmdoa2a
	rts
cmdoa2a	short a
cmdoa2aa	if2	@y,eq,cmdlen,cmdoa2b
	lda	cmdline+1,y
	sta	cmdline,y
	iny
	bra	cmdoa2aa
cmdoa2b	lda	#' '
	sta	cmdline-1,y
	sta	cmdline,y
	long	a
	ldy	cmdloc
	ldx	#0
cmdoa2c	if2	@y,eq,cmdlen,cmdoa2e
	bcs	cmdoa2d
cmdoa2e	lda	cmdline,y
	iny
	inx
	phx
	phy
	jsr	putchar
	ply
	plx
	bra	cmdoa2c   

cmdoa2d	jsr	moveleft
	dec	cmdlen
	rts

;-------------------------------------------------------------------------
;
; Jump to beginning of line
;
;-------------------------------------------------------------------------

cmdbegin	ldx	cmdloc
	stz	cmdloc
	jmp	moveleft

;-------------------------------------------------------------------------
;
; Jump to end of line
;
;-------------------------------------------------------------------------

cmdend	if2	cmdloc,eq,cmdlen,cmdoa4a
	ldx	#1
	jsr	moveright
	inc	cmdloc
	bra	cmdend
cmdoa4a	rts

;-------------------------------------------------------------------------
;
; Left one word
;
;-------------------------------------------------------------------------

cmdleftword	lda	cmdloc
	bne	cmdoa5a   
	jsr	beep
cmdoa5z	rts
cmdoa5a	dec	a
	sta	cmdloc
cmdoa5b	ldx	#1
	jsr	moveleft
	ldy	cmdloc
	beq	cmdoa5z
	lda	cmdline,y
	and	#$FF
	cmp	#' '
	bne	cmdoa5c
	dec	cmdloc
	bra	cmdoa5b
cmdoa5c	ldy	cmdloc
	beq	cmdoa5z
	lda	cmdline-1,y
	and	#$FF
	cmp	#' '
	beq	cmdoa5z
	dec	cmdloc
	ldx	#1
	jsr	moveleft
	bra	cmdoa5c

;-------------------------------------------------------------------------
;
; Move one word to the right
;
;-------------------------------------------------------------------------

cmdrightword	if2	cmdloc,ne,cmdlen,cmdoa6a
	jsr	beep
cmdoa6z	rts
cmdoa6a	inc	a
	sta	cmdloc
cmdoa6b	ldx	#1
	jsr	moveright
	ldy	cmdloc
	if2	@y,eq,cmdlen,cmdoa6z
	lda	cmdline,y
	and	#$FF
	cmp	#' '
	beq	cmdoa6c
	inc	cmdloc
	bra	cmdoa6b
cmdoa6c	ldy	cmdloc
	if2	@y,eq,cmdlen,cmdoa6z
	lda	cmdline,y
	and	#$FF
	cmp	#' '
	bne	cmdoa6z
	inc	cmdloc
	ldx	#1
	jsr	moveright
	bra	cmdoa6c

oldsgtty	dc	i1'0'
	dc	i1'0'
	dc	i1'0'
	dc	i1'0'
	dc	i2'0'

newsgtty	dc	i1'0'
	dc	i1'0'
	dc	i1'0'
	dc	i1'0'
sg_flags	dc	i2'0'

oldttyk	dc	i2'0'
newttyk	dc	i2'OAMAP+OA2META+VT100ARROW'

oldltc	ds	6
newltc	ds	6

	END	     

;=========================================================================
;
; ^D file matcher
;
;=========================================================================

domatcher	START

	using	global

	jsr	wordmatch

	lda	nummatch
	bne	m1
	jmp	beep
;
; move to end of command-line
;
m1	sec
	lda	cmdlen
	sbc	cmdloc
	tax
	jsr	moveright
	jsr	newline
;
; start displaying each match
;
	lda	#0
m4	pha
	asl	a
	asl	a
	tay
	lda	matchbuf,y
	ldx	matchbuf+2,y
	jsr	puts
	lda	#' '
	jsr	putchar
	pla
	inc	a
	cmp	nummatch
	bne	m4	

	jsr	newline
	jmp	clearword

	END

;=========================================================================
;
; command line expansion
;
;=========================================================================

dotab	START

	using	global

p	equ	0

	jsr	wordmatch
	lda	nummatch
	bne	t1
meepmeep	jmp	beep
;
; only one! trivial case!
;
t1	dec	a
	bne	t2
t1b	mv4	matchbuf,p
	ldy	wordlen
t1a	lda	[p],y
	and	#$FF
	jeq	completed
	sta	oldchar
	phy
	jsr	insertcmd
	ply
	iny
	bra	t1a
;
; one at a time
;
t2	jsl	dofignore
	lda	nummatch
	beq	meepmeep
	dec	a
	beq	t1b
	mv4	matchbuf,p
	lda	char
	sta	oldchar
	ldy	wordlen
	lda	[p],y
	and	#$FF
	sta	char2
	jsr	tolower
	sta	char
	lda	#0
t3	pha	 
	asl	a
	asl	a
	tay
	lda	matchbuf,y
	sta	p
	lda	matchbuf+2,y
	sta	p+2
	ldy	wordlen
	lda	[p],y
	and	#$FF
	jsr	tolower
	cmp	char
	bne	honk
	pla
	inc	a
	cmp	nummatch
	bne	t3
	lda	char2
	beq	completed
	jsr	insertcmd
	inc	wordlen
	bra	t2

honk	pla
	jsr	beep
	bra	donetab	

completed	lda	cmdloc
	cmp	cmdlen
	bne	donetab
	lda	oldchar
	and	#$FF
	cmp	#':'
	beq	donetab
	cmp	#'/'
	beq	donetab
	lda	#' '
	jsr	insertcmd
donetab	jmp	clearword

char	ds	2
char2	ds	2
oldchar	ds	2

	END

;=========================================================================
;
; remove any entries matching $fignore
;
;=========================================================================

dofignore	START
	
	using	global

var	equ	0
word	equ	var+4
wordnum	equ	word+4
wordlen	equ	wordnum+2
varpos	equ	wordlen+2
newpos	equ	varpos+2
wordpos	equ	newpos+2
gsbuf	equ	wordpos+2
space	equ	gsbuf+4

	subroutine (0:dummy),space

	ph4	#fignore
	jsl	getenv

	sta	gsbuf	If buffer wasn't allocated
	stx	gsbuf+2
	ora	gsbuf+2
	jeq	done	  return to caller.

	clc
	lda	gsbuf	Text begins after
	adc	#4	 four bytes of
	bcc	storevar	  length words.
	inx
storevar	sta	var	Store pointer to
	stx	var+2	 c-string in var.
	phx		Shift
	pha		 to
	jsr	lowercstr	  lower case.

	lda	#0
	sta	wordnum
bigloop	asl	a
	asl	a
	tax
	lda	matchbuf,x
	sta	word
	tay
	lda	matchbuf+2,x
	sta	word+2
	pha
	phy
	jsr	cstrlen
	sta	wordlen
	stz	newpos
figmatch	lda	newpos
	sta	varpos
	ldy	varpos
eatspace	lda	[var],y
	and	#$FF
	beq	bignext
	cmp	#' '
	bne	yummy
	iny
	bra	eatspace

yummy	sty	varpos
	ldx	#0
eatstuff	lda	[var],y
	and	#$FF
	beq	gotstuff
	cmp	#' '
	beq	gotstuff
	inx
	iny
	bra	eatstuff

gotstuff	sty	newpos
	cpx	wordlen
	beq	hgf
	bcs	figmatch
hgf	phx
	sec
	lda	wordlen
	sbc	1,s
	sta	wordpos
	plx
chk	ldy	wordpos
	lda	[word],y
	and	#$FF
	beq	deleteit
	ldy	varpos
	jsr	tolower
	eor	[var],y
	and	#$FF
	bne	figmatch
	inc	varpos
	inc	wordpos
	bra	chk	

deleteit	lda	wordnum
	jsr	removeword
bignext	inc	wordnum
	lda	wordnum
	cmp	nummatch
	bcc	bigloop

	pei	(gsbuf+2)	Free memory allocated
	pei	(gsbuf)	 for the value of $FIGNORE.
	jsl	nullfree

done	anop

	return

fignore	gsstr	'fignore'	Env variable name

	END

;=========================================================================
;
; insert a character into the command-line
;
;=========================================================================

insertcmd	START
	
	using	global

	sta	tmp

	short a
	ldy	cmdlen
cmIns1	cpy	cmdloc
	beq	cmins2
	bcc	cmIns2
	lda	cmdline-1,y
	sta	cmdline,y
	dey
	bra	cmIns1
cmIns2	long	a
	inc	cmdlen
;
; Place character in string and output
;
cmOver	lda	tmp
	ldy	cmdloc	;Do overstrike mode
	short a
	sta	cmdline,y
	long	a
	iny
	sty	cmdloc
	jsr	putchar
	ldy	cmdloc
	cpy	cmdlen
	bcc	cmdov2 
	beq	cmdov2 
	sty	cmdlen
;
; Redraw shifted text
;
cmdov2	ldx	#0
cmdov3	if2	@y,eq,cmdlen,cmdov5
	lda	cmdline,y
	iny
	inx
	phx
	phy
	jsr	putchar
	ply
	plx
	bra	cmdov3
cmdov5	jmp	moveleft

tmp	ds	2

	END

;=========================================================================
;
; remove an entry from the word matcher table
;
;=========================================================================

removeword	START

	using	global

	pha
	asl	a
	asl	a
	pha
	tax
	lda	matchbuf+2,x
	pha
	lda	matchbuf,x
	pha
	jsl	nullfree
	plx
	ply
loop	cpy	nummatch
	beq	done
	lda	matchbuf+4,x
	sta	matchbuf,x
	lda	matchbuf+6,x
	sta	matchbuf+2,x
	inx
	inx
	inx
	inx
	iny
	bra	loop

done	dec	nummatch
	rts

	END
	
;=========================================================================
;
; clear the word matcher table
;
;=========================================================================

clearword	START

	using	global

	lda	#0
loop	pha
	asl	a
	asl	a
	tay
	lda	matchbuf,y
	ldx	matchbuf+2,y
	phx
	pha
	jsl	nullfree
	pla
	inc	a
	cmp	nummatch
	bne	loop

	stz	nummatch	

	rts

	END

;=========================================================================
;
; word matcher for command-line expansion
;
;=========================================================================

wordmatch	START
	
	using	global
	using	hashdata
	using BuiltInData

	lda	#'/'	Default separator is "/".
	sta	sepstyle

	ldx	#0	Keep track of left moves in X-reg.
	ldy	cmdloc	Y-reg = position on line.
	beq	atstart	If 0, can't go any further.
	lda	cmdline,y	If current
	and	#$FF	 character is
	cmp	#' '	  space, and
	bne	findstart2	   the one before
	lda	cmdline-1,y	    it is also a
	and	#$FF	     space,
	cmp	#' '
	bne	findstart
	jmp	beep		beep at the user.
;
; Move backwards to find start of word to expand
;
findstart	inx		Count # of times we've moved left.
	dey		Decrement the buffer index.
	beq	atstart	Done if all the way to start.
findstart2	lda	cmdline-1,y	Get the previous
	and	#$FF	 character.
	cmp	#';'	Compare against shell
	beq	atstart	 characters that
	cmp	#'|'	  indicate a word break.
	beq	atstart
	cmp	#'&'
	beq	atstart
	cmp	#'>'
	beq	atstart
	cmp	#'<'
	beq	atstart
	cmp	#' '
	bne	findstart	Stay in loop until one is found.
;
; Y-reg points to start, X-reg contains number of positions to cursor.
; Search forward to isolate the current word.
;
atstart	sty	startpos
	stx	dir+1	Ugh! Self-modifying code!!
	ldx	#0

isolate	cpy	cmdlen	If at end of line,
	beq	gotiso	 cannot go any further.
	lda	cmdline,y	Isolate the next character.
	and	#$FF	Compare against shell
	cmp	#' '	 characters that
	beq	gotiso	  indicate a word break.
	cmp	#';'
	beq	gotiso
	cmp	#'|'
	beq	gotiso
	cmp	#'&'
	beq	gotiso
	cmp	#'>'
	beq	gotiso
	cmp	#'<'
	beq	gotiso
	sta	wordgs_text,x	Accumulate chars in comparison buffer.
	iny		Bump character index.
	inx		Bump number of chars in word.
	cmp	#WORDGS_SIZE-2	If we haven't filled the buffer,
	bcc	isolate	 keep accumulating characters.
	jmp	beep	Otherwise, beep at the user.
;
; We've isolated the word in wordgs_text. X-reg = # of characters
; and Y-reg is the column number of the final character.
;
gotiso	lda	#0	Terminate string with
	sta	wordgs_text,x	 null byte.
	stx	wordlen	Save length and
	sty	cmdloc	 position on input line.
	txa
	sec		Subtract the number of
dir	sbc	#0-0	 character to original cursor.
	beq	nomove
	tax
	jsr	moveright	Move cursor to end of word.
nomove	anop

;
; Look for a match: either a command (if word is in first position),
; a filename in the current directory, or a variable name.
;
	stz	nummatch
;
; see if it's a command or an argument, hopefully this is nice and
; accurate...it should serve its purpose well at least.
;
	lda	#1
	sta	cmdflag	Assume it's a command (flag=1).
	ldy	startpos	If starting position
	beq	gotflag	 is 0
	dey		  or 1,
	beq	gotflag	   it's a command.

flagskip	lda	cmdline,y	Keep fetching previous
	and	#$FF	 characters until
	cmp	#' '	  a non-space is
	bne	chkflag	   found, or we run
	dey		    back to the beginning.
	bpl	flagskip
	bra	gotflag

;
; We've got the first non-space character before the word in question.
; If it's a meta- character indicating the end of a command, this must
; be the beginning of a new command. Otherwise it's an argument.
;
chkflag	cmp	#';'
	beq	gotflag
	cmp	#'|'
	beq	gotflag
; NOTE: & can be a command terminator, but also part of >& or >>&
	cmp	#'&'
	bne	isarg	Not "&"; must be an argument.
	cpy	#0	If at start of line (unusual for &!)
	beq	gotflag	 it must be a command.
	lda	cmdline-1,y	Get character in front of "&".
	cmp	#'&>'	Anything except '>'
	bne	gotflag	 means we've got a command.

isarg	stz	cmdflag	Argument: cmdflag = 0.

;
; Now that we've established whether we're looking for a command or an
; argument, see if we're looking for a file name or a variable name
;
gotflag	anop
	lda	wordgs_text
	and	#$FF
	cmp	#'$'
	jne	filem

;
; The first character is '$': match for variables ONLY
;
	ld2	1,idxIndex
varloop	ReadIndexedGS idxParm	Get next variable name.
	lda	NameLen
	beq	vardone
	cmp	wordlen	If shorter than word, skip
	jcc	nextvar
;
; Scan this variable name to see if it matches wordgs_text
;
	ldx	#1
varscan	lda	wordgs_text,x
	and	#$FF
	beq	goodvar	Matches (up to current length)
	jsr	tolower
	eor	NameText-1,x
	and	#$FF
	jne	nextvar	Name doesn't match word.
	inx
	bra	varscan
;
; We have a match between the variable name and word being typed
;
goodvar	stz	sepstyle	Don't use ":" or "/" separator.

gv02a	anop
	lda	nummatch	Get the number of matches.
	asl	a	Multiply by 4 (size of address entry)
	asl	a
	pha		Hold offset on stack.
	lda	NameLen	Get length of name,
	inc	a	 and add 3.
	inc	a
	inc	a
	pea	0
	pha
	~NEW		Request memory to hold name.
	sta	0
	stx	0+2
	ply		Get matchbuf offset from stack.
	sta	matchbuf,y	Store allocated memory's
	txa		 address in matchbuf array.
	sta	matchbuf+2,y
	inc	nummatch

	lda	#'$'	First character = "$".
	sta	[0]
	ldx	NameLen	Get length of name in X-Reg.
	ldy	#1	Use Y as offset into strings.
gv01	lda	NameText-1,y	Get next byte of name.
	sta	[0],y	Store in allocated memory.
	iny		Bump the index.
	dex		Decrement the count
	bne	gv01	Keep copying until done.
	lda	sepstyle	Store separator style at end.
	sta	[0],y
;
; Done with this variable. Check the next one.
;
nextvar	inc	idxIndex
	jmp	varloop

vardone	rts		Return from wordmatch

;
; Match by file name wildcard; start by moving
; wordgs_text + trailing "*" to a GS/OS string
;
filem	lda	#1
	sta	iwFlags

	lda	wordlen
	inc	a
	sta	wordgsbuf
	ldy	wordlen
	lda	#'*'
	sta	wordgs_text,y

	ldx	#0
	short	a
	dey
findsep	lda	wordgs_text,y
	cmp	#':'
	beq	gotsep
	cmp	#'/'
	beq	gotsep
	cmp	#'['
	beq	gotglob
	cmp	#'?'
	beq	gotglob
	cmp	#'='
	beq	gotglob
	cmp	#'"'
	beq	gotglob
	cmp	#"'"
	beq	gotglob
	cmp	#'*'
	bne	nextsep
	cpy	#0	;allow boot prefix */
	bne	gotglob
	lda	wordgs_text+1
	cmp	sepstyle
	bne	gotglob
	bra	nextsep
gotglob	long	a
	jmp	beep
	longa	off
gotsep	sta	sepstyle
	inx
nextsep	dey
	bpl	findsep
	long	a
	cpx	#0
	beq	initit
	dec	iwFlags

initit	InitWildcardGS iwparm

filematch	anop
	NextWildcardGS nwparm
	ldy	NameLen	Get length of name.
	jeq	filemdone	If 0, there's no more to search.
	cpy	wordlen
	beq	filematch

	lda	#0	Store null byte at end of name,
	sta	NameText,y	  so it will act like a c-string.

	lda	nummatch	Get the number of matches.
	asl	a	Multiply by 4 (size of address entry)
	asl	a
	pha		Hold offset on stack.
	iny		Get length of name + 1
	pea	0
	phy
	~NEW		Request memory to hold name.
	ply		Get matchbuf offset from stack.
	sta	matchbuf,y	Store allocated memory's
	txa		 address in matchbuf array.
	sta	matchbuf+2,y

	ph4	#NameText	Copy string from
	lda	matchbuf+2,y	 name buffer into
	pha		  matchbuf table entry.
	lda	matchbuf,y
	pha
	jsr	copycstr

	lda	cmdflag	Are we looking for a command?
	beq	fm01
	lda	nwType	Yes: Get the file's type
	cmp	#$0F	== $0F?
	beq	isdir		it's a directory
	cmp	#$B5	== $B5?
	beq	notdir		it's EXE (shell executable)
	cmp	#$B3	== $B3?
	beq	notdir		it's S16 (GS/OS application)
	cmp	#$B0	== $B0?
	bne	filematch		it's SRC (shell source code)
	lda	nwAux		Get file's aux type
	cmp	#6		== $00000006?
	bne	filematch	
	lda	nwAux+2
	bne	filematch			No; try next wildcard.
	bra	notdir			It's shell cmd file.
;
; Not looking for a command
;
fm01	lda	nwType
	cmp	#$0F
	bne	notdir
;
; File type is $0F: it's a directory
;
isdir	lda	nummatch
	asl	a
	asl	a
	tax
	phx
	lda	matchbuf,x
	sta	0
	lda	matchbuf+2,x
	sta	2
	lda	NameLen	Allocate NameLen+2 bytes of memory
	inc	a
	inc	a
	pea	0
	pha
	~NEW
	ply
	sta	8
	stx	10
	pei	(2)
	pei	(0)
	phx
	pha
	sta	matchbuf,y
	txa
	sta	matchbuf+2,y
	jsr	copycstr
	pei	(2)	Free entry that was
	pei	(0)	 allocated in non-directory
	jsl	nullfree	  section of code.
	ldy	NameLen
	lda	sepstyle
	sta	[8],y

notdir	anop
	inc	nummatch	Bump the match count.
	jmp	filematch	Try next wildcard.

filemdone	anop
	lda	cmdflag	;if it's not a command, we're done
	jeq	done
;
; let's now look at the hashed files
;
p	equ	0	Direct page
q	equ	4	 locations.

	ldy	wordlen	;remove '*' from above
	lda	#0
	sta	wordgs_text,y

	lda	hash_table
	ora	hash_table+2
	beq	eq_endhash
	mv4	hash_table,p
	lda	hash_numexe
	jeq	endhash
	ldy	#0
	ldx	t_size
eq_endhash	beq	endhash
; 
; loop through every hashed file and add it the string vector
;
hashloop	lda	[p],y
	sta	q
	iny
	iny
	lda	[p],y
	sta	q+2
	iny
	iny
	ora	q
	beq	nexthash
	incad	q
	incad	q
	phy
	phx	
	pei	(q+2)
	pei	(q)
	jsr	cstrlen
	cmp	wordlen
	bcc	nexthash0
	tax
	ldy	#0
hl	lda	wordgs_text,y
	and	#$FF
	beq	hl0
	jsr	tolower
	eor	[q],y
	and	#$FF
	bne	nexthash0
	iny
	bra	hl
hl0	inx
	lda	nummatch
	asl	a
	asl	a
	pha
	pea	0
	phx
	~NEW
	ply
	pei	(q+2)
	pei	(q)
	phx
	pha
	sta	matchbuf,y
	txa
	sta	matchbuf+2,y
	inc	nummatch
	jsr	copycstr
nexthash0	plx
	ply
nexthash	dex
	bne	hashloop

endhash	anop
;
; add built-ins to the list
;
	ld4	builtintbl,p
bilup	lda	[p]
	ldy	#2
	ora	[p],y
	beq	bidone
	lda	[p]
	sta	q
	lda	[p],y
	sta	q+2
	pei	(q+2)
	pei	(q)
	jsr	cstrlen
	cmp	wordlen
	bcc	binext
	tax
	ldy	#0
bl	lda	wordgs_text,y
	and	#$FF
	beq	bl0
	eor	[q],y
	and	#$FF
	bne	binext
	iny
	bra	bl
bl0	inx
	lda	nummatch
	asl	a
	asl	a
	pha
	pea	0
	phx
	~NEW
	ply
	pei	(q+2)
	pei	(q)
	phx
	pha
	sta	matchbuf,y
	txa
	sta	matchbuf+2,y
	inc	nummatch
	jsr	copycstr
binext	add2	p,#10,p
	bra	bilup
bidone	anop

done	rts		Return from wordmatch.


startpos	ds	2
cmdflag	ds	2

;
; GS/OS string holding match word + wildcard "*"
;
wordgsbuf	ds	2
wordgs_text	ds	WORDGS_SIZE

;
; Parameter block for shell InitWildcardGS call (p 414 in ORCA/M manual)
;
iwparm	dc	i2'2'	pCount
	dc	i4'wordgsbuf'	pathname with wildcard
iwFlags	dc	i2'1'	flags

;
; Parameter block for shell NextWildcardGS call (p 414 in ORCA/M manual)
;
nwparm	dc	i2'4'	pCount
	dc	i4'NameBuf'	pathName
	dc	i2'0'	access
nwType	dc	i2'0'	fileType
nwAux	dc	i4'0'	auxType

;
; Parameter block for shell ReadIndexedGS call (p 421 in ORCA/M manual)
;
idxParm	anop
	dc	i2'4'	pCount
	dc	i4'NameBuf'	Name (pointer to GS/OS result buf)
	dc	i4'ResultBuf'	Value (pointer to GS/OS result buf)
idxIndex	ds	2	Index number
	ds	2	Export flag
;
; GS/OS result buffer to hold name: maximum size = 255 bytes
;
NameBuf	dc	i2'259'	Total length of result buf.
NameLen	ds	2	Name's length returned here.
NameText	ds	255	Text goes here.
	ds	2	Room for terminating null bytes.
;
; GS/OS result buffer for testing whether a variable is defined.
; It doesn't have enough room for > 1 byte to be returned, but we
; only need to get the length of the value.
;
ResultBuf	dc	i2'5'	Only five bytes total.
	ds	2	Value's length returned here.
	ds	1	Only 1 byte for value.

sepstyle	ds	2

	END

**************************************************************************
*
* key bindings data
*
**************************************************************************

keybinddata	DATA

keybindtab	dc	i2'undefined_char',i4'0'		;^@
	dc	i2'beginning_of_line',i4'0' 	;^A
	dc	i2'backward_char',i4'0'		;^B
	dc	i2'undefined_char',i4'0'		;^C
	dc	i2'list_choices',i4'0'		;^D
	dc	i2'end_of_line',i4'0'		;^E
	dc	i2'forward_char',i4'0'		;^F
	dc	i2'undefined_char',i4'0'		;^G
	dc	i2'backward_delete_char',i4'0'	;^H
	dc	i2'complete_word',i4'0'		;^I
	dc	i2'newline_char',i4'0'		;^J
	dc	i2'undefined_char',i4'0'		;^K
	dc	i2'clear_screen',i4'0'		;^L
	dc	i2'newline_char',i4'0'		;^M
	dc	i2'down_history',i4'0'		;^N
	dc	i2'undefined_char',i4'0'		;^O
	dc	i2'up_history',i4'0'		;^P
	dc	i2'undefined_char',i4'0'		;^Q
	dc	i2'redisplay',i4'0' 		;^R
	dc	i2'undefined_char',i4'0'		;^S
	dc	i2'undefined_char',i4'0'		;^T
	dc	i2'kill_whole_line',i4'0'		;^U
	dc	i2'undefined_char',i4'0'		;^V
	dc	i2'undefined_char',i4'0'		;^W
	dc	i2'kill_whole_line',i4'0'		;^X
	dc	i2'kill_end_of_line',i4'0'		;^Y
	dc	i2'undefined_char',i4'0'		;^Z
	dc	i2'lead_in',i4'defescmap'		;^[
	dc	i2'undefined_char',i4'0'		;^\
	dc	i2'undefined_char',i4'0'		;^]
	dc	i2'undefined_char',i4'0'		;^^
	dc	i2'undefined_char',i4'0'		;^_
	dc	95i2'raw_char,0,0'			;' ' .. '~'
	dc	i2'backward_delete_char',i4'0'	;^? (DEL)

defescmap	dc	4i2'undefined_char,0,0'		;^@ .. ^C
	dc	i2'list_choices',i4'0'		;^D
	dc	3i2'undefined_char,0,0'		;^E .. ^G
	dc	i2'backward_word',i4'0'		;^H
	dc	i2'complete_word',i4'0'		;^I
	dc	2i2'undefined_char,0,0'		;^J, ^K
	dc	i2'clear_screen',i4'0'		;^L
	dc	i2'undefined_char,0,0'		;^M
	dc	i2'undefined_char,0,0'		;^N
	dc	i2'undefined_char,0,0'		;^O
	dc	i2'undefined_char,0,0'		;^P
	dc	i2'undefined_char,0,0'		;^Q
	dc	i2'undefined_char,0,0'		;^R
	dc	i2'undefined_char,0,0'		;^S
	dc	i2'undefined_char,0,0'		;^T
	dc	i2'forward_word,0,0'		;^U
	dc	i2'undefined_char,0,0'		;^W
	dc	i2'undefined_char,0,0'		;^X
	dc	i2'undefined_char,0,0'		;^X
	dc	i2'undefined_char,0,0'		;^Y
	dc	i2'undefined_char,0,0'		;^Z
	dc	i2'complete_word',i4'0'		;^[
	dc	16i2'undefined_char,0,0'		;^\ .. +
	dc	i2'beginning_of_line',i4'0' 	; ,
	dc	i2'undefined_char,0,0'		; 
	dc	i2'end_of_line',i4'0'		; .
	dc	19i2'undefined_char,0,0'		; .+1 .. A
	dc	i2'backward_word',i4'0'		;B
	dc	3i2'undefined_char,0,0'		;C ... E
	dc	i2'forward_word',i4'0'		;F
	dc	8i2'undefined_char,0,0'		;G ... N
	dc	i2'lead_in',i4'vt100key'		;O
	dc	18i2'undefined_char,0,0'		;P ... a
	dc	i2'backward_word',i4'0'		;b
	dc	2i2'undefined_char,0,0'		;c ... d
	dc	i2'toggle_cursor',i4'0'		;e
	dc	i2'forward_word',i4'0'		;f
	dc	25i2'undefined_char,0,0'		;g ... ^?

vt100key	dc	65i2'undefined_char,0,0'		;^@ ... @
	dc	i2'up_history',i4'0'		;A
	dc	i2'down_history',i4'0'		;B
	dc	i2'forward_char',i4'0'		;C
	dc	i2'backward_char',i4'0'		;D
	dc	59i2'undefined_char,0,0'		;E ... ^?

	END

**************************************************************************
*
* bind a key to a function
*
**************************************************************************

bindkeyfunc	START
	
	using	keybinddata

p	equ	0
tbl	equ	p+4
len	equ	tbl+4
space	equ	len+2

	subroutine (4:keystr,2:function),space

	lda	keystr
	ora	keystr+2
	jeq	done

	pei	(keystr+2)
	pei	(keystr)
	jsr	cstrlen
	sta	len
	ld4	keybindtab,tbl

loop	lda	len
	jeq	done
	dec	a
	jeq	putit	;last char in string

	lda	[keystr]
	and	#$FF
	asl	a
	sta	addb+1
	asl	a
	clc
addb	adc	#0
	tay
	lda	[tbl],y
	cmp	#lead_in
	beq	next
	phy
	ph4	#128*6
	~NEW
	sta	p
	stx	p+2
	ldy	#128*6-2
	lda	#0
zap	sta	[0],y
	dey
	dey
	bpl	zap
	ply
	lda	#lead_in
	sta	[tbl],y
	iny
	iny
	lda	p
	sta	[tbl],y
	iny
	iny
	lda	p+2
	sta	[tbl],y
	mv4	p,tbl
	dec	len
	add4	keystr,#1,keystr
	bra	loop

next	iny
	iny
	lda	[tbl],y
	tax
	iny
	iny
	lda	[tbl],y
	sta	tbl+2
	stx	tbl
	dec	len
	add4	keystr,#1,keystr
	jmp	loop

putit	lda	[keystr]
	and	#$FF
	asl	a
	sta	adda+1
	asl	a
	clc
adda	adc	#0
	tay
	lda	function
	sta	[tbl],y
	iny
	iny
	lda	#0
	sta	[tbl],y
	iny
	iny
	sta	[tbl],y

done	return

	END

**************************************************************************
*
* BINDKEY: builtin command
* syntax: bindkey [-l] function string
*
* bind a keystring to an editor function.
*
**************************************************************************

bindkey	START
	
str	equ	0
func	equ	str+4
arg	equ	func+2
status	equ	arg+4
space	equ	status+2

	subroutine (4:argv,2:argc),space

	stz	status

	lda	argc
	dec	a
	bne	ok
showusage	ldx	#^usage
	lda	#usage
	jsr	errputs
	inc	status
	bra	goexit

ok	dec	argc
	add2	argv,#4,argv
	lda	[argv]
	sta	arg
	ldy	#2
	lda	[argv],y
	sta	arg+2
	lda	[arg]
	and	#$FF
	cmp	#'-'
	bne	startbind
	ldy	#1
	lda	[arg],y
	cmp	#'l'
	beq	list
	bra	showusage

list	ldx	#^liststr	
	lda	#liststr
	jsr	puts
	bra	goexit

startbind	lda	argc
	dec	a
	jeq	showusage
	dec	a
	jne	showusage

	ldy	#0
findloop	phy
	lda	nametbl,y
	ora	nametbl+2,y
	beq	nofind
	lda	nametbl+2,y
	pha
	lda	nametbl,y
	pha
	pei	(arg+2)
	pei	(arg)
	jsr	cmpcstr
	beq	foundit
	pla
	add2	@a,#4,@y
	bra	findloop

nofind	pla
	ldx	arg+2
	lda	arg
	jsr	errputs
	ldx	#^errstr
	lda	#errstr
	jsr	errputs
	inc	status
goexit	bra	exit

foundit	pla
	lsr	a
	tax
	lda	functbl,x
	sta	func

	add2	argv,#4,argv
	lda	[argv]
	sta	arg
	ldy	#2
	lda	[argv],y
	sta	arg+2

	pei	(arg+2)
	pei	(arg)
	jsr	cstrlen
	inc	a
	inc	a
	pea	0
	pha
	~NEW
	sta	str
	stx	str+2
	pei	(arg+2)
	pei	(arg)
	phx
	pha
	jsl	decode

	pei	(str+2)
	pei	(str)
	pei	(func)
	jsl	bindkeyfunc

	pei	(str+2)
	pei	(str)
	jsl	nullfree

exit	return 2:status
	   
usage	dc	c'Usage: bindkey [-l] function string',h'0d00'
errstr	dc	c': undefined function',h'0d00'

liststr	dc	c'  backward-char        - move cursor left',h'0d'
	dc	c'  backward-delete-char - delete character to left',h'0d'
	dc	c'  backward-word        - move cursor left one word',h'0d'
	dc	c'  beginning-of-line    - move cursor to beginning of line',h'0d'
	dc	c'  clear-screen         - clear screen and redraw prompt',h'0d'
	dc	c'  complete-word        - perform filename completion',h'0d'
	dc	c'  delete-char          - delete character under cursor',h'0d'
	dc	c'  down-history         - replace command line with next history',h'0d'
	dc	c'  end-of-line          - move cursor to end of line',h'0d'
	dc	c'  forward-char         - move cursor to the right',h'0d'
	dc	c'  forward-word         - move cursor one word to the right',h'0d'
	dc	c'  kill-end-of-line     - delete line from cursor to end of line',h'0d'
	dc	c'  kill-whole-line      - delete the entire command line',h'0d'
	dc	c'  list-choices         - list file completion matches',h'0d'
	dc	c'  newline              - finished editing, accept command line',h'0d'
	dc	c'  raw-char             - character as-is',h'0d'
	dc	c'  redisplay            - redisplay the command line',h'0d'
	dc	c'  toggle-cursor        - toggle between insert and overwrite cursor',h'0d'
	dc	c'  undefined-char       - this key does nothing',h'0d'
	dc	c'  up-history           - replace command line with previous history',h'0d'
	dc	h'00'

nametbl	dc	i4'func1,func2,func3,func4,func5,func6,func7,func8'
	dc	i4'func9,func10,func11,func12,func13,func14,func15'
	dc	i4'func16,func17,func18,func19,func20,0'

func1	dc	c'backward-char',h'00'
func2	dc	c'backward-delete-char',h'00'
func3	dc	c'backward-word',h'00'
func4	dc	c'beginning-of-line',h'00'
func5	dc	c'clear-screen',h'00'
func6	dc	c'complete-word',h'00'
func7	dc	c'delete-char',h'00'
func8	dc	c'down-history',h'00'
func9	dc	c'end-of-line',h'00'
func10	dc	c'forward-char',h'00'
func11	dc	c'forward-word',h'00'
func12	dc	c'kill-end-of-line',h'00'
func13	dc	c'kill-whole-line',h'00'
func14	dc	c'list-choices',h'00'
func15	dc	c'newline',h'00'
func16	dc	c'raw-char',h'00'
func17	dc	c'redisplay',h'00'
func18	dc	c'toggle-cursor',h'00'
func19	dc	c'undefined-char',h'00'
func20	dc	c'up-history',h'00'

functbl	dc	i'backward_char'
	dc	i'backward_delete_char'
	dc	i'backward_word'
	dc	i'beginning_of_line'
	dc	i'clear_screen'
	dc	i'complete_word'
	dc	i'delete_char'
	dc	i'down_history'
	dc	i'end_of_line'
	dc	i'forward_char'
	dc	i'forward_word'
	dc	i'kill_end_of_line'
	dc	i'kill_whole_line'
	dc	i'list_choices'
	dc	i'newline_char'
	dc	i'raw_char'
	dc	i'redisplay'	
	dc	i'toggle_cursor'
	dc	i'undefined_char'
	dc	i'up_history'

	END

**************************************************************************
*
* decode does the grung work to decode the
* string escapes.
*
**************************************************************************

decode	START

ch	equ	1
space	equ	ch+2
cp	equ	space+3
str	equ	cp+4
end	equ	str+4

;	subroutine (4:str,4:cp),space

	tsc
	sec
	sbc	#space-1
	tcs
	phd
	tcd

	ldy	#0

loop	lda	[str],y
	and	#$FF
	jeq	breakloop
	iny

	cmp	#'^'
	bne	caseslash

	lda	[str],y
	and	#$1F
	iny
	bra	casebreak0

caseslash	cmp	#'\'
	bne	casebreak0

	lda	[str],y
	and	#$FF
	sta	ch
	iny

	ldx	#0
nextc	lda	dp1,x
	and	#$FF
	beq	nextslash
	cmp	ch
	beq	gotslash
	inx
	bra	nextc
gotslash	lda	dp2,x
	and	#$FF
	bra	casebreak0

nextslash	lda	ch
	cmp	#'0'
	bcc	casebreak0
	cmp	#'9'+1
	bcs	casebreak0

	sec
	sbc	#'0'
	sta	ch

	ldx	#2

numloop	asl	ch
	asl	ch
	asl	ch
	lda	[str],y
	and	#$FF
	sec
	sbc	#'0'
	clc
	adc	ch
	sta	ch
	iny
	dex
	beq	casebreak0
	lda	[str],y
	and	#$FF
	cmp	#'0'
	bcc	casebreak
	cmp	#'9'+1
	bcc	numloop

casebreak	lda	ch
casebreak0	sta	[cp]
	incad	cp
	jmp	loop

breakloop	lda	#0
	sta	[cp]

	lda	space+1
	sta	end-2
	lda	space
	sta	end-3
	pld
	tsc
	clc
	adc	#end-4
	tcs
	
	rtl
 
dp1	dc	c'E^\:nrtbf',h'00'
dp2	dc	h'1b',c'^\:',h'0a0d09080c'

	END
