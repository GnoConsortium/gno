**************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*
* $Id: cmd.asm,v 1.6 1998/09/08 16:53:07 tribby Exp $
*
**************************************************************************
*
* CMD.ASM
*   By Tim Meekins
*   Modified by Dave Tribby for GNO 2.0.6
*
* Command line parsing routines.
*
* Note: text set up for tabs at col 16, 22, 41, 49, 57, 65
*              |     |                  |       |       |       |
*	^	^	^	^	^	^	
**************************************************************************
*
* Interfaces defined in this file:
*
*   gettoken	subroutine (4:word,4:stream)
*	Returns value of token in Accumulator
*
*   command	subroutine (4:waitpid,2:inpipe,2:jobflag,2:inpipe2,
*		4:pipesem,4:stream)
*	Returns next token in Accumulator
*
*   argfree	subroutine (2:argc,4:argv)
*
*   ShellExec	subroutine (4:path,2:argc,4:argv,2:jobflag)
*	Returns completion status in Accumulator
*				       
*   execute	subroutine (4:cmdline,2:jobflag)
*	Returns completion status in Accumulator
*
*   system	Defined for libc; interface in <stdlib.h>
*	int system (char *command)
*
**************************************************************************

	mcopy /obj/gno/bin/gsh/cmd.mac

dummycmd	start		; ends up in .root
	end

	setcom 60

SIGINT	gequ	 2
SIGSTOP	gequ	17
;
; TOKENS used by the parser
;
T_WORD	gequ	1
T_BAR	gequ	2
T_AMP	gequ	3
T_SEMI	gequ	4
T_GT	gequ	5
T_GTGT	gequ	6
T_GTAMP	gequ	7
T_GTGTAMP	gequ	8
T_LT	gequ	9
T_NL	gequ	10
T_EOF	gequ	11
T_ERROR	gequ	12
T_NULL	gequ	13

MAXARG	gequ	256
BADFD	gequ	-2

**************************************************************************
*
* Read a token from the buffer.
*
* Input: word points to a buffer to place a word is parsed.
*
* Ouput: the token is placed in the accumulator
*
**************************************************************************

gettoken	START

buf	equ	1
state	equ	buf+4
ch	equ	state+2
space	equ	ch+2
stream	equ	space+3
word	equ	stream+4
end	equ	word+4

;	 subroutine (4:word,4:stream),space

	tsc
	sec
	sbc	#space-1
	tcs
	phd
	tcd
;
; What state are we in
;
NEUTRAL	equ	0	;a neutral state, get anything
GTGT	equ	1	;looking for a second '>'
INQUOTE	equ	2	;parsing a quoted string
INWORD	equ	3	;parsing a word
SINGQUOTE	equ	4	;single quote string
;
; Start in the neutral state
;
	ld2	NEUTRAL,state
	lda	[stream]
	sta	buf
	ldy	#2
	lda	[stream],y
	sta	buf+2
;
; Main loop: get character and take action based upon state.
;
loop	lda	[buf]
	incad	buf
	and2	@a,#$FF,ch	ch = next character.
	bne	switch

; End of string detected. Action depends upon current state.

	if2	state,ne,#INWORD,loop2
	jmp	endword	state INWORD: end the word.

loop2	anop
	dec	buf	Went too far: back up pointer.
	if2	@a,ne,#GTGT,loop3
	lda	#T_GT	state GTGT: return single GT.
	jmp	done

loop3	if2	@a,eq,#INQUOTE,error1	INQUOTE: error.
	if2	@a,eq,#SINGQUOTE,error2	SINGQUOTE: error.
	lda	#T_EOF	must be NEUTRAL: return EOF.
	jmp	done

error1	ldx	#^errstr1	Report string errors.
	lda	#errstr1
	bra	error0
error2	ldx	#^errstr2
	lda	#errstr2
error0	jsr	errputs
	lda	#T_ERROR
	jmp	done
;
; jump to the current state
;
switch	lda	state
	asl	a
	tax
	jmp	(statetbl,x)
statetbl	dc	a2'case_neutral'
	dc	a2'case_gtgt'
	dc	a2'case_inquote'
	dc	a2'case_inword'
	dc	a2'case_single'
;
; CASE NEUTRAL
;  Check for special characters:
;     ; & | < creturn EOF -- set token value and go to done
;     space tab -- ignore and stay in loop
;     > -- Change state to GTGT and stay in loop
;     # -- Eat characters to creturn or lf, then stay in loop
;     " -- Change state to INQUOTE and stay in loop
;     ' -- Change state to SINGQUOTE and stay in loop
;     \ -- Get next character, change state to INWORD, and stay in loop
;  All other characters: change state to INWORD and stay in loop

case_neutral	if2	ch,ne,#';',neut1
	lda	#T_SEMI	
	jmp	done
neut1	if2	@a,ne,#'&',neut2
	lda	#T_AMP
	jmp	done
neut2	if2	@a,ne,#'|',neut3
	lda	#T_BAR
	jmp	done
neut3	if2	@a,ne,#'<',neut4
	lda	#T_LT
	jmp	done

neut5	cmp	#' '	;space
	jeq	loop
	cmp	#9	;tab
	jeq	loop
	if2	@a,ne,#'>',neut6
	lda	#GTGT
	bra	neut10

neut4	if2	@a,ne,#13,neut4a	;return
	lda	#T_NL
	jmp	done
neut4a	if2	@a,ne,#0,neut4b	;EOF [Is this possible?? DMT]
	lda	#T_EOF
	jmp	done

neut4b	if2	@a,ne,#'#',neut5	;comment
neut4c	lda	[buf]
	and	#$7F
	beq	neut4d
	incad	buf
	if2	@a,eq,#13,neut4d
	if2	@a,eq,#10,neut4d
	bra	neut4c
neut4d	jmp	loop

neut6	if2	@a,ne,#'"',neut7
startquote	lda	#INQUOTE
	bra	neut10
neut7	if2	@a,ne,#"'",neut8
startsingle	lda	#SINGQUOTE
	bra	neut10
neut8	if2	@a,ne,#'\',neut9
	lda	[buf]
	and	#$FF
	incad	buf
	if2	@a,eq,#13,neut10a
neut9	sta	[word]	;default
	incad	word
	lda	#INWORD
neut10	sta	state
neut10a	jmp	loop

;
; CASE GTGT
;
case_gtgt	if2	ch,eq,#'>',gtgt2
	if2	@a,eq,#'&',gtgt1
	dec	buf
	lda	#T_GT
	jmp	done
gtgt1	lda	#T_GTAMP
	jmp	done
gtgt2	lda	[buf]
	and	#$FF
	if2	@a,eq,#'&',gtgt3
	lda	#T_GTGT
	jmp	done
gtgt3	incad	buf
	lda	#T_GTGTAMP
	jmp	done
;
; CASE INQUOTE
;
case_inquote	if2	ch,ne,#'\',quote2	;is it a quoted character?
	lda	[buf]
	incad	buf
putword	sta	[word]
	incad	word
	jmp	loop
quote2	if2	@a,ne,#'"',putword
	ld2	INWORD,state
	jmp	loop

;
; CASE SINGLEQUOTE
;
case_single	anop
	if2	ch,ne,#"'",putword
	ld2	INWORD,state
	jmp	loop
;
; CASE INWORD
;
case_inword	if2	ch,eq,#000,endword
	if2	@a,eq,#';',endword
	if2	@a,eq,#'&',endword
	if2	@a,eq,#'|',endword
	if2	@a,eq,#'>',endword
	if2	@a,eq,#'<',endword
	if2	@a,eq,#' ',endword
	if2	@a,eq,#009,endword
	if2	@a,eq,#013,endword
	cmp	#'"'
	jeq	startquote
	cmp	#"'"
	jeq	startsingle
	if2	@a,ne,#'\',putword
	lda	[buf]
	incad	buf
	and	#$FF
	if2	@a,eq,#13,word2
	bra	putword
word2	jmp	loop
endword	dec	buf
finiword	lda	#0
	sta	[word]
	lda	#T_WORD

done	tax
	lda	buf
	sta	[stream]
	lda	buf+2
	ldy	#2
	sta	[stream],y

	lda	space
	sta	end-3
	lda	space+1
	sta	end-2
	pld
	tsc
	clc
	adc	#end-4
	tcs

	txa
	rtl

errstr1	dc	c'gsh: Missing ending ".',h'0d00'
errstr2	dc	c"gsh: Missing ending '.",h'0d00'

	END

**************************************************************************
*
* Parse a single command
*
**************************************************************************

command	START
	
pipefds	equ	1
errappend	equ	pipefds+2
errfile	equ	errappend+2
srcfile	equ	errfile+4
dstfile	equ	srcfile+4
count	equ	dstfile+4
argv	equ	count+2
word	equ	argv+4
cmdline	equ	word+4
pid	equ	cmdline+4
append	equ	pid+2
temp	equ	append+2
argc	equ	temp+4
token	equ	argc+2
space	equ	token+2
stream	equ	space+3
pipesem	equ	stream+4
inpipe2	equ	pipesem+4
jobflag	equ	inpipe2+2
inpipe	equ	jobflag+2
waitpid	equ	inpipe+2
end	equ	waitpid+4

;	 subroutine (4:waitpid,2:inpipe,2:jobflag,2:inpipe2,4:pipesem,4:stream),space

	tsc
	sec
	sbc	#space-1
	tcs
	phd
	tcd

	ph4	#1024	Allocate 1024 bytes
	~NEW		 and pointer in cmdline.
	sta	cmdline
	stx	cmdline+2
	lda	#0	Initialize to null C string.
	sta	[cmdline]

	jsl	alloc1024
	sta	word
	stx	word+2

	ph4	#MAXARG*4
	~NEW
	sta	argv
	stx	argv+2

	stz	srcfile
	stz	srcfile+2
	stz	dstfile
	stz	dstfile+2
	stz	errfile
	stz	errfile+2
	stz	argc
	stz	pipefds
	stz	pipefds+2
	lda	#-3
	sta	[waitpid]

loop	pei	(word+2)
	pei	(word)
	pei	(stream+2)
	pei	(stream)
	jsl	gettoken
	sta	token
	asl	a
	tax
	jmp	(toktbl,x)
toktbl	dc	a2'loop'
	dc	a2'tok_word'
	dc	a2'tok_bar'
	dc	a2'tok_amp'
	dc	a2'tok_semi'
	dc	a2'tok_gt'
	dc	a2'tok_gtgt'
	dc	a2'tok_gtamp'
	dc	a2'tok_gtgtamp'
	dc	a2'tok_lt'
	dc	a2'tok_nl'
	dc	a2'tok_eof'
	dc	a2'tok_error'

;
; Parse a word token
;
tok_word	if2	argc,ne,#MAXARG,word1
	lda	#err00	;Too many arguments
	jmp	error
word1	pei	(word+2)
	pei	(word)
	jsr	cstrlen
	inc	a
	pea	0
	pha
	~NEW
	sta	temp
	stx	temp+2
	ora	temp+2
	bne	word2
	lda	#err01	;Out of memory for arguments
	jmp	error
word2	lda	argc	;Copy word to argv[argc]
	asl2	a
	tay
	lda	temp
	sta	[argv],y
	lda	temp+2
	iny2
	sta	[argv],y
	pei	(word+2)
	pei	(word)
	pei	(temp+2)
	pei	(temp)
	jsr	copycstr

	stz	count	;count illegal characters in word
	ldy	#0
illword	lda	[word],y
	and	#$FF
	beq	appword
	if2	@a,eq,#' ',incword
	if2	@a,eq,#'&',incword
	if2	@a,eq,#'|',incword
	if2	@a,eq,#'<',incword
	if2	@a,eq,#'>',incword
	if2	@a,eq,#';',incword
	bra	nextword
incword	inc	count
nextword	iny
	bra	illword

appword	pei	(word+2)	;append word to current command line
	pei	(word)
	pei	(cmdline+2)
	pei	(cmdline)
	jsr	cstrlen
	tay
	ldx	argc
	beq	nospace
	lda	#' '
	sta	[cmdline],y
	iny
nospace	lda	count
	beq	noquote
	lda	[word]
	and	#$FF
	cmp	#'"'
	bne	doquote
	stz	count
	bra	noquote
doquote	lda	#'"'
	sta	[cmdline],y
	iny
	inc	count
noquote	pei	(cmdline+2)
	add2	@y,cmdline,@a
	pha
	jsr	copycstr
	lda	count
	beq	noquote2
	pei	(cmdline+2)
	pei	(cmdline)
	jsr	cstrlen
	tay
	lda	#'"'
	sta	[cmdline],y
	iny
	lda	#0
	sta	[cmdline],y
noquote2	inc	argc	;increment argument count
goloop	jmp	loop
;
; Parse a '<' token
;
tok_lt	lda	srcfile
	ora	srcfile+2
	beq	lt1
	lda	#err02	;Extra < encountered
	jmp	error
lt1	lda	inpipe
	beq	lt2
	lda	#err09	;< conflicts with |
	jmp	error
lt2	jsl	alloc1024
	stx	srcfile+2
	sta	srcfile
	phx
	pha
	pei	(stream+2)
	pei	(stream)
	jsl	gettoken
	if2	@a,eq,#T_WORD,goloop
	lda	#err03	;Illegal < specified
	jmp	error
;
; Parse a '>' or '>>'
; 
tok_gt	anop
tok_gtgt	lda	dstfile
	ora	dstfile+2
	beq	gt1
	lda	#err04	;Extra > or >> encountered
	jmp	error
gt1	jsl	alloc1024
	stx	dstfile+2
	sta	dstfile
	phx
	pha
	pei	(stream+2)
	pei	(stream)
	jsl	gettoken
	if2	@a,eq,#T_WORD,gt2
	lda	#err05	;Illegal > or >> specified
	jmp	error
gt2	stz	append
	if2	token,ne,#T_GTGT,gt3
	inc	append
gt3	jmp	loop
;
; Parse a '>&' or '>>&'
; 
tok_gtamp	anop
tok_gtgtamp	lda	errfile
	ora	errfile+2
	beq	ga1
	lda	#err06	;Extra >& or >>& encountered
	jmp	error
ga1	jsl	alloc1024
	stx	errfile+2
	sta	errfile
	phx
	pha
	pei	(stream+2)
	pei	(stream)
	jsl	gettoken
	if2	@a,eq,#T_WORD,ga2
	lda	#err07	;Illegal >& or >>& specified
	jmp	error
ga2	stz	errappend
	if2	token,ne,#T_GTGTAMP,ga3
	inc	errappend
ga3	jmp	loop
;
; Parse a command terminator
;
tok_bar	anop
tok_amp	anop
tok_semi	anop
tok_nl	anop
tok_eof	anop

	lda	argc
	bne	nonnull
	lda	#0
	sta	[waitpid]
	lda	#T_NULL
	jmp	exit

nonnull	asl2	a	;terminate the argv list
	tay
	lda	#0
	sta	[argv],y
	iny2
	sta	[argv],y
;
; see if there is a conflict between >,>> with |
;
	lda	token
	if2	@a,ne,#T_BAR,runit
	lda	dstfile
	ora	dstfile+2
	beq	bar2
	lda	#err08	;> or >> conflicts with |
	jmp	error

bar2	clc
	tdc
	adc	#pipefds
	ldx	#0
	pipe	@xa
;what if pipes return errors?

runit	pei	(argc)
	pei	(argv+2)
	pei	(argv)
	pei	(srcfile+2)
	pei	(srcfile)
	pei	(dstfile+2)
	pei	(dstfile)
	pei	(errfile+2)
	pei	(errfile)
	pei	(append)
	pei	(errappend) 
	ldx	#0
	if2	token,ne,#T_AMP,run2
	inx
run2	phx
	pei	(cmdline+2)
	pei	(cmdline)
	pei	(jobflag)
	pei	(inpipe)
	pei	(pipefds+2)
	pei	(inpipe2)
	pei	(pipefds)
	pei	(pipesem+2)
	pei	(pipesem)
	jsl	invoke
	sta	pid
	cmp	#-1
	beq	exit

	if2	token,ne,#T_BAR,run3	If next token is "|",

	pei	(waitpid+2)		  recursively call command.
	pei	(waitpid)
	pei	(pipefds)
	pei	(jobflag)
	pei	(pipefds+2)
	pei	(pipesem+2)
	pei	(pipesem)
	pei	(stream+2)
	pei	(stream)
	jsl	command
	bra	exit

run3	lda	pid
	sta	[waitpid]
	lda	token

; clean up

exit	pha

	lda	dstfile
	ora	dstfile+2
	beq	ex1
	ldx	dstfile+2
	lda	dstfile
	jsl	free1024
ex1	anop

	lda	srcfile
	ora	srcfile+2
	beq	ex2
	ldx	srcfile+2
	lda	srcfile
	jsl	free1024
ex2	anop

	lda	errfile
	ora	errfile+2
	beq	ex3
	ldx	errfile+2
	lda	errfile
	jsl	free1024
ex3	anop
		   
	ldx	word+2
	lda	word
	jsl	free1024

	ply

	lda	space
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

error	ldx	#^err00
	jsr	errputs

tok_error	pei	(cmdline+2)
	pei	(cmdline)
	jsl	nullfree

exit1a	pei	(argc)
	pei	(argv+2)
	pei	(argv)
	jsl	argfree
	lda	#-1
	jmp	exit

err00	dc	c'gsh: Too many arguments, so no dessert tonight.',h'0d00'
err01	dc	c'gsh: Not enough memory for arguments.',h'0d00'
err02	dc	c'gsh: Extra ''<'' encountered.',h'0d00'
err03	dc	c'gsh: No file specified for ''<''.',h'0d00'
err04	dc	c'gsh: Extra ''>'' or ''>>'' encountered.',h'0d00'
err05	dc	c'gsh: No file specified for ''>'' or ''>>''.',h'0d00'
err06	dc	c'gsh: Extra ''>&'' or ''>>&'' encountered.',h'0d00'
err07	dc	c'gsh: No file specified for ''>&'' or ''>>&''.',h'0d00'
err08	dc	c'gsh: ''|'' conflicts with ''>'' or ''>>''.',h'0d00'
err09	dc	c'gsh: ''|'' conflicts with ''<''.',h'0d00'
		    
	END

**************************************************************************
*
* dispose the argv
*
**************************************************************************

argfree	START

space	equ	0

	subroutine (2:argc,4:argv),space

free1	lda	argc
	beq	free2
	dec	a
	asl2	a
	tay
	lda	[argv],y
	tax
	iny2
	lda	[argv],y
	pha
	phx
	jsl	nullfree
	dec	argc
	bra	free1
free2	pei	(argv+2)
	pei	(argv)
	jsl	nullfree
	return

	END

**************************************************************************
*
* Interpret a shell script
* This is overly complicated so that it can be run concurrently.
*
**************************************************************************

ShellExec	START
	
	using	vardata
	using	global

count	equ	1
data	equ	count+2
CRec	equ	data+4
RRec	equ	CRec+4
NRec	equ	RRec+4
ORec	equ	NRec+4
ptr	equ	ORec+4
status	equ	ptr+4
space	equ	status+2
jobflag	equ	space+3
argv	equ	jobflag+2
argc	equ	argv+4
path	equ	argc+2
end	equ	path+4

;	 subroutine (4:path,2:argc,4:argv,2:jobflag),space

	tsc
	sec
	sbc	#space-1
	tcs
	phd
	tcd

	lock	mutex
;
; Set the variables 0..argc
;
	lda	argc	Get number of variables.
	jeq	vars_set	If 0, there are none to set.

	stz	count	Start with argv[0].
parmloop	lda	count	Get index
	asl2	a	 into address array.
	tay
	lda	[argv],y	Copy argument
	sta	SetValue	 pointer to
	iny2		  SetValue
	lda	[argv],y
	sta	SetValue+2

	lda	count	If parameter number
	cmp	#10
	bcs	digits2or3	 < 10,
	adc	#'0'		Convert to single digit
	sta	pname_text		 and store in name string.
	lda	#1		Set length of string to 1.
	sta	pname
	bra	set_value

digits2or3	cmp	#100	If parameter number
	bcs	digits3	 >= 10 && < 99,
	ldx	#2		length = 2
	bra	setit	otherwise
digits3	ldx	#3		length = 3
;
; Store length (2 or 3) and convert number to text
;
setit	stx	pname
	Int2Dec (@a,#pname_text,pname,#0)

set_value	anop
	ph4	SetValue	Convert value string
	jsr	c2gsstr	 to a GS/OS string
	stx	SetValue+2	  and save in SetGS
	sta	SetValue	   parameter block.

	SetGS	SetPB	Set $count to the argv string.

	ph4	SetValue	Free the value buffer.
	jsl	nullfree
	inc	count	Bump the parameter counter.
	lda	count	If more to do,
	cmp	argc
	jcc	parmloop	   stay in loop.

;
; Variables have all been set
;
vars_set	unlock mutex

	ph4	#4	;Close parms
	~NEW
	sta	CRec
	stx	CRec+2

	ph4	#10	;Open parms
	~NEW
	sta	ORec
	stx	ORec+2

	ph4	#12	;NewLine parms
	~NEW
	sta	NRec
	stx	NRec+2

	ph4	#16	;Read parms
	~NEW
	sta	RRec
	stx	RRec+2

	ph4	#1000	;data buffer
	~NEW
	sta	data
	stx	data+2

	pei	(path+2)	;Convert filename to GS/OS string
	pei	(path)
	jsr	c2gsstr
	ldy	#4
	sta	[ORec],y
	sta	ptr
	iny2
	txa
	sta	[ORec],y
	sta	ptr+2

	ldy	#8
	lda	#1	;Read access only
	sta	[ORec],y

	lda	#3	;Open the file
	sta	[ORec]
	pei	(ORec+2)
	pei	(ORec)
	ph2	#$2010	;OPEN
	jsl	$E100B0
	bcc	ok
	sta	ErrError
	ErrorGS Err
	jmp	done

awshit	sta	ErrError
	ErrorGS Err
	jmp	almostdone

ok	ldy	#2	;Copy file ref num
	lda	[ORec],y
	sta	[NRec],y
	sta	[RRec],y
	sta	[CRec],y

	lda	#4	;Do NewLine
	sta	[NRec]
	ldy	#4
	lda	#$7F
	sta	[NRec],y
	iny2
	lda	#1
	sta	[NRec],y
	iny2
	lda	#NLTable
	sta	[NRec],y
	iny2
	lda	#^NLTable
	sta	[NRec],y
	pei	(NRec+2)
	pei	(NRec)
	ph2	#$2011	;NEWLINE
	jsl	$E100B0
	bcs	awshit

	lda	#4	;Set up read parm
	sta	[RRec]
	tay
	lda	data
	sta	[RRec],y
	iny2
	lda	data+2
	sta	[RRec],y
	iny2
	lda	#1000
	sta	[RRec],y
	iny2
	lda	#0
	sta	[RRec],y

ReadLoop	anop
	pei	(RRec+2)
	pei	(RRec)
	ph2	#$2012	;READ
	jsl	$E100B0
	bcs	almostdone
	ldy	#12
	lda	[RRec],y
	tay
	lda	#0
	sta	[data],y
	lda	varecho
	beq	noecho
	ldx	data+2
	lda	data
	jsr	puts
	jsr	newline
noecho	lda	[data]
	and	#$FF
	if2	@a,eq,#'#',ReadLoop

*   call execute: subroutine (4:cmdline,2:jobflag)
	pei	(data+2)
	pei	(data)
	pei	(jobflag)
	jsl	execute
	sta	status

	lda	exit_requested
	bne	almostdone
	bra	ReadLoop

almostdone	anop
	stz	exit_requested
	lda	#1
	sta	[CRec]
	pei	(CRec+2)
	pei	(CRec)
	ph2	#$2014	;CLOSE
	jsl	$E100B0

done	pei	(CRec+2)
	pei	(CRec)
	jsl	nullfree
	pei	(NRec+2)
	pei	(NRec)
	jsl	nullfree
	pei	(RRec+2)
	pei	(RRec)
	jsl	nullfree
	pei	(ORec+2)
	pei	(ORec)
	jsl	nullfree
	pei	(data+2)
	pei	(data)
	jsl	nullfree
	pei	(ptr+2)
	pei	(ptr)
	jsl	nullfree

exit1a	anop

	lda	space+1
	sta	end-2
	lda	space
	sta	end-3
	pld
	tsc
	clc
	adc	#end-4
	tcs

	lda	status	Pass back status value.
	rtl

NLTable	dc	h'0d'

; Parameter block for shell ErrorGS call (p 393 in ORCA/M manual)
Err	dc	i2'1'	pCount
ErrError	ds	2	Error number

;
; Parameter block for shell SetGS calls (p 427 in ORCA/M manual)
;
SetPB	anop
	dc	i2'3'	pCount
SetName	dc	i4'pname'	Name  (pointer to GS/OS string)
SetValue	ds	4	Value (pointer to GS/OS string)
SetExport	ds	2	Export flag
;
; Name of argv parameter ($1 to $999) to be set; GS/OS string
;
pname	ds	2	Length
pname_text	dc	c'000'	Text (up to 3 digits)

mutex	key

	END

**************************************************************************
*
* Execute a command line by calling command for each command in the buffer
*
**************************************************************************

execute	START

exebuf	equ	1
pipesem	equ	exebuf+4
ptr2	equ	pipesem+2
waitstatus	equ	ptr2+4
ptr	equ	waitstatus+2
pid	equ	ptr+4
term	equ	pid+2
cmdstrt	equ	term+2
cmdend	equ	cmdstrt+4
end_char	equ	cmdend+4
inquote	equ	end_char+2
space	equ	inquote+2
jobflag	equ	space+3	;set if not a job
cmdline	equ	jobflag+2
end	equ	cmdline+4

;	 subroutine (4:cmdline,2:jobflag),space

	tsc
	sec
	sbc	#space-1
	tcs
	phd
	tcd

; ---------------------------------------------------------------

; New code for gsh 2.0 (Dave Tribby):  within execute, loop through
; the command line to separate each command and expand it separately,
; rather than passing multiple commands to the next level.  This is
; done so that commands that depend upon each other will work, e.g.
;    set testnum=2 ; echo "This is test $testnum"

;
; Find beginning and end of next command in the command line
;
	lda	cmdline	Initialize cmdstrt to
	sta	cmdstrt	 beginning of cmdline.
	lda	cmdline+2
	sta	cmdstrt+2

; Skip over leading whitespace

chkws	lda	[cmdstrt]	Get next character.
	and	#$FF
	jeq	goback	If at end of line, nothing to do!
	cmp	#" "	If it's a space
	beq	bump_strt
	cmp	#9	 or a tab,
	bne	found_start
bump_strt	incad	cmdstrt	   bump the start pointer
	bra	chkws	    and look for more whitespace.

; Initialize pointer to end of command

found_start	anop

; Scan the command line for next semi-colon. Need to account for
; quoted strings, backslash-escaped characters, and comments.

; Take advantage of 65816's BIT command for the "in quotes" flag. Use
; $8000 for single-quote bit and $4000 for double-quote bit. After
;	bit inquote
; can do a bmi to check for $8000 set and bvs for $4000 set. Can
; check for either with  lda inquote  followed by beq or bne.

	stz	inquote	Clear the "in quotes" flag.

	ldy	#$FFFF	Clear index into cmdstrt

find_end	anop
	iny
	lda	[cmdstrt],y	Get next character.
	and	#$FF	If at end of string,
	beq	found_end	 all done looking.
; Check for special characters
	cmp	#"'"
	beq	s_quote
	cmp	#'"'
	beq	d_quote
	cmp	#'\'
	beq	b_slash
; "#" and ";" are special only if we aren't in a quoted string
	ldx	inquote
	bne	find_end
	cmp	#"#"
	beq	found_end
	cmp	#";"
	beq	found_end
; Not a special character. Keep looking.
	bra	find_end


; "'" found
s_quote	bit	inquote	Check the "in quotes" flag.
	bvs	find_end	In double quotes...keep looking.
	lda	inquote	Toggle the single_quote
	eor	#$8000	 bit in the "in quotes" flag.
	sta	inquote
	bra	find_end	Keep looking.

; '"' found
d_quote	bit	inquote	Check the "in quotes" flag.
	bmi	find_end	In single quotes...keep looking.
	lda	inquote	Toggle the double_quote
	eor	#$4000	 bit in the "in quotes" flag.
	sta	inquote
	bra	find_end	Keep looking.

; "\" found: accept next character without examining it in detail
b_slash	iny		Bump index.
	lda	[cmdstrt],y	Get next character.
	and	#$FF	If not at end of string,
	bne	find_end	 keep looking.

;
; Found a ";", "#", or null byte.
;
found_end	anop
	sta	end_char	Save the ending character.

	tya		Get number of bytes in command.
	jeq	goback	If none, just skip it.

	clc		Add command length to
	adc	cmdstrt	 starting address to
	sta	cmdend	  get ending address.
	lda	#0
	adc	cmdstrt+2
	sta	cmdend+2

	lda	end_char	Get the termination character.
	beq	expand	If it's not a null byte,
	lda	#0
	short	m
	sta	[cmdend]	  store null byte in string.
	long	m

; Continue with command-line expansions for the single command

expand	anop

; ---------------------------------------------------------------

	stz	pipesem
	stz	waitstatus

; Expand $ (environment variables) and ~ in the raw command line
	pei	(cmdstrt+2)
	pei	(cmdstrt)
	jsl	expandvars

; Expand wildcard characters in the modified command line
	phx
	pha
	sta	ptr
	stx	ptr+2
	jsl	glob

; Expand aliases in the modified command line
	phx	              
	pha
	sta	ptr2
	stx	ptr2+2
	jsl	expandalias

	phx	         
	pha
	sta	exebuf
	stx	exebuf+2

* >> Temporary debug code: echo expanded command if echo is set.
	using	vardata
	lda	varecho
	beq	noecho
	ldx	exebuf+2
	lda	exebuf
	jsr	puts
	jsr	newline
noecho	anop


	ldx	ptr+2
	lda	ptr
	jsl	free1024
	ldx	ptr2+2
	lda	ptr2
	jsl	free1024
		         
	lda	exebuf
	ora	exebuf+2
	bne	loop

	pla
	pla
	stz	term
	lda	#0
	jmp	chk_cmd


*   command	subroutine (4:waitpid,2:inpipe,2:jobflag,2:inpipe2,
*		4:pipesem,4:stream)
loop	pea	0	;Bank 0		waitpid (hi)
	tdc
	clc
	adc	#pid
	pha				waitpid (low)
	pea	0			inpipe
	pei	(jobflag)			jobflag
	pea	0			inpipe2
	pea	0	;Bank 0		pipesem (hi)
	tdc
	clc
	adc	#pipesem
	pha				pipesem (low)
	pea	0	;Bank 0		stream (hi)
	tdc
	clc
	adc	#exebuf
	pha				stream (low)
	jsl	command  

	sta	term
	bmi	noerrexit

	lda	pid
	beq	nowait
	cmp	#-1
	beq	noerrexit

	lda	jobflag
	beq	jobwait

	signal (#SIGINT,#0)
	phx
	pha
	signal (#SIGSTOP,#0)
	phx
	pha

otherwait	ldx	#0
	clc
	tdc
	adc	#waitstatus
	wait	@xa
	cmp	pid
	bne	otherwait
	lda	waitstatus
	and	#$FF
	cmp	#$7F
	beq	otherwait
	lda	waitstatus 
	jsr	setstatus

	pla
	plx
	signal (#SIGSTOP,@xa)
	pla
	plx
	signal (#SIGINT,@xa)

	bra	nowait

jobwait	jsl	pwait
	sta	waitstatus

; If command detected EOF terminator, all done
nowait	if2	term,eq,#T_EOF,noerrexit
	lda	[exebuf]	If not at end of line,
	and	#$FF
	beq	exit	
	jmp	loop	  process the next command.

;
; NOTE: non-forked builtins have no mechanism to return command status
;

noerrexit	stz	waitstatus

exit	jsl	nullfree
	lda	term	;make sure we return -1 if error
	bmi	chk_cmd

	lda	waitstatus
	xba
	and	#$FF

;
; Is there another command waiting in the buffer?
;
chk_cmd	ldx	end_char	Was the original ending character
	cpx	#";"	 a semi-colon?
	bne	goback	NO -- all done.

; Set cmdstrt to point to the character beyond cmdend
	lda	cmdend
	ldx	cmdend+2
	ina
	bne	set_strt
	inx
set_strt	sta	cmdstrt
	stx	cmdstrt+2

	jmp	chkws	Parse the next command.

;
; All done.
;
goback	tay		Hold return status in Y-reg.

	lda	space+1	Set up stack
	sta	end-2	 for rtl.
	lda	space
	sta	end-3
	pld
	tsc
	clc
	adc	#end-4
	tcs

	tya		Restore return status from Y-reg.

	rtl

	END

**************************************************************************
*
* System() call vector
*
**************************************************************************

system	START

retval	equ	0
space	equ	retval+2

	subroutine (4:str),space		;need the phk/plb

	lda	str	If user passes a
	ora	str+2	 null pointer,
	bne	makecall
	ina		  return 1 to caller.

;
; Let execute(str) do the work
;
makecall	pei	(str+2)
	pei	(str)
	ph2	#1	;tells execute we're called by system
	jsl	execute
;
; Set status and go back to the caller
;
setrtn	sta	retval

	return 2:retval

	END
