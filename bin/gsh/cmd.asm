**************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*
* $Id: cmd.asm,v 1.11 1999/02/08 17:26:50 tribby Exp $
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
*		4:pipesem,4:stream,4:awaitstatus)
*	Called by execute to act on a single command
*	Returns next token in Accumulator
*
*   argfree	subroutine (4:path,2:argc,4:argv)
*
*   ShellExec	subroutine (4:path,2:argc,4:argv,2:jobflag)
*	Reads and executes commands from an exec file.
*	Returns completion status in Accumulator
*				       
*   execute	subroutine (4:cmdline,2:jobflag)
*	Interpret a command line.
*	Returns completion status in Accumulator
*
*   system	Defined for libc; interface in <stdlib.h>
*	User interface to execute commands via shell
*	int system (char *command)
*
**************************************************************************

	mcopy /obj/gno/bin/gsh/cmd.mac

dummycmd	start		; ends up in .root
	end

	setcom 60

SIGINT	gequ	 2
SIGSTOP	gequ	17
SIGTSTP	gequ	18
SIGCHLD	gequ	20
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
               and	#$00FF
	sta	ch	ch = next character.
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
errappend	equ	pipefds+4
errfile	equ	errappend+2
srcfile	equ	errfile+4
dstfile	equ	srcfile+4
needq	equ	dstfile+4
argv	equ	needq+2
word	equ	argv+4
cmdline	equ	word+4
pid	equ	cmdline+4
append	equ	pid+2
temp	equ	append+2
argc	equ	temp+4
token	equ	argc+2
space	equ	token+2
awaitstatus	equ	space+3
stream	equ	awaitstatus+4
pipesem	equ	stream+4
inpipe2	equ	pipesem+4
jobflag	equ	inpipe2+2
inpipe	equ	jobflag+2
waitpid	equ	inpipe+2
end	equ	waitpid+4

;	 subroutine (4:waitpid,2:inpipe,2:jobflag,2:inpipe2,4:pipesem,4:stream,4:awaitstatus),space

	tsc
	sec
	sbc	#space-1
	tcs
	phd
	tcd

	ph4	maxline_size	Allocate maxline_size bytes
	~NEW		 and pointer in cmdline.
	sta	cmdline
	stx	cmdline+2
	lda	#0	Initialize to null C string.
	sta	[cmdline]

	jsl	allocmaxline	Allocate memory for token.
	sta	word
	stx	word+2

	ph4	#MAXARG*4	Allocate argv parameter pointer array
	~NEW
	sta	argv
	stx	argv+2

	stz	argc	Argument count = 0

	stz	srcfile	Clear I/O redirection pointers
	stz	srcfile+2
	stz	dstfile
	stz	dstfile+2
	stz	errfile
	stz	errfile+2
	stz	pipefds
	stz	pipefds+2

	lda	#-3
	sta	[waitpid]

loop	pei	(word+2)	Get next token from input stream.
	pei	(word)
	pei	(stream+2)
	pei	(stream)
	jsl	gettoken
	sta	token	Jump to appropriate token handler.
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

;
; Determine whether parameter needs surrounding quotes
;
	stz	needq
	lda	[word]	If this is a null parameter,
	and	#$FF
	beq	qneeded	  it needs to be quoted.
	ldy	#0
chkchar	if2	@a,eq,#' ',qneeded
	if2	@a,eq,#'&',qneeded
	if2	@a,eq,#'|',qneeded
	if2	@a,eq,#'<',qneeded
	if2	@a,eq,#'>',qneeded
	if2	@a,eq,#';',qneeded
	iny		Not special character.
	lda	[word],y	Get next character in parameter.
	and	#$FF
	beq	appword	Done if null character.
	bra	chkchar

qneeded	inc	needq	Quotes needed.

;
; Append word to command line (optionally, with quotes)
;
appword	anop
	pei	(word+2)
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
nospace	lda	needq	If special char is in parameter,
	beq	noquote
	lda	[word]
	and	#$FF
	cmp	#'"'	 and it doesn't start with '"',
	bne	doquote
	stz	needq
	bra	noquote
doquote	lda	#'"'	  add a '"' at start.
	sta	[cmdline],y
	iny
noquote	pei	(cmdline+2)
	add2	@y,cmdline,@a
	pha
	jsr	copycstr	Copy the parameter.
	lda	needq	If quote was added at start,
	beq	noquote2
	pei	(cmdline+2)
	pei	(cmdline)
	jsr	cstrlen
	tay
	lda	#'"'	   add another at end.
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
lt2	jsl	allocmaxline
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
gt1	jsl	allocmaxline
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
ga1	jsl	allocmaxline
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

	lda	argc	If number of arguments == 0,
	bne	nonnull

	lda	srcfile	  If any of the file pointers
	ora	srcfile+2	   are != NULL,
	ora	dstfile
	ora	dstfile+2
	ora	errfile
	ora	errfile+2
	beq	nulldone

	ldx	#^spcmdstr	  print error message:
	lda	#spcmdstr	   specify a command before redirecting.
	jsr	errputs

nulldone	anop
	pei	(cmdline+2)	Free buffers
	pei	(cmdline)	 allocated for
	jsl	nullfree	  command line
	pei	(argv+2)	   and argv.
	pei	(argv)
	jsl	nullfree

	lda	#0	Clear the waitpid
	sta	[waitpid]	 and return as if
	lda	#T_NULL	  nothing were parsed.
	jmp	exit

nonnull	asl2	a	Terminate the argv list
	tay		 with a null poiner.
	lda	#0
	sta	[argv],y
	iny2
	sta	[argv],y
;
; See if there is a conflict between > or >> and |
;
	lda	token
	if2	@a,ne,#T_BAR,runit
	lda	dstfile
	ora	dstfile+2
	beq	bar2
	lda	#err08	Yes: there is a conflict!
	jmp	error

bar2	clc		Calculate 32-bit address
	tdc		 of direct page variable
	adc	#pipefds	  pipefds in X and A.
	ldx	#0
	pipe	@xa	Allocate 2 file descriptor pipe
; >> NOTE: what if pipe returns error?

;
; Call invoke			param size:name
;
runit	pei	(argc)	2: argc
	pei	(argv+2)	4: argv
	pei	(argv)
	pei	(srcfile+2)	4: sfile
	pei	(srcfile)
	pei	(dstfile+2)	4: dfile
	pei	(dstfile)
	pei	(errfile+2)	4: efile
	pei	(errfile)
	pei	(append)	2: app
	pei	(errappend) 	2: eapp
	ldx	#0
	if2	token,ne,#T_AMP,run2
	inx		2: bg
run2	phx
	pei	(cmdline+2)	4: cline
	pei	(cmdline)
	pei	(jobflag)	2: jobflag
	pei	(inpipe)	2: pipein   (param passed in)
	pei	(pipefds+2)	2: pipeout  (allocated: read end)
	pei	(inpipe2)	2: pipein2  (param passed in)
	pei	(pipefds)	2: pipeout2 (allocated: write end)
	pei	(pipesem+2)	4: pipesem  (param passed in)
	pei	(pipesem)
	pei	(awaitstatus+2)	4: awaitstatus (address) [New for v2.0]
	pei	(awaitstatus)
	lda	#-1	Set waitstatus = -1; it will be set to
	sta	[awaitstatus]	 0 or 1 iff unforked builtin is called.
	jsl	invoke
	sta	pid
	cmp	#-1	If invoke detected an error,
	beq	exit	 all done.


; If next token is "|", recursively call command.

	if2	token,ne,#T_BAR,run3

	lda	#-1	Pre-set for error flag.
	ldx	pid	If no child was forked,
	beq	exit	 all done.

	pei	(waitpid+2)	4: waitpid
	pei	(waitpid)
	pei	(pipefds)	2: inpipe
	pei	(jobflag)	2: jobflag
	pei	(pipefds+2)	2: inpipe2
	pei	(pipesem+2)	4: pipesem
	pei	(pipesem)
	pei	(stream+2)	4: stream
	pei	(stream)
	pei	(awaitstatus+2)	4: awaitstatus
	pei	(awaitstatus)
	jsl	command
	bra	exit

run3	lda	pid
	sta	[waitpid]
	lda	token

;
; Free allocated memory and return to caller
;
exit	pha		Hold return status on stack.

	lda	dstfile
	ora	dstfile+2
	beq	ex1
	ldx	dstfile+2
	lda	dstfile
	jsl	freemaxline
ex1	anop

	lda	srcfile
	ora	srcfile+2
	beq	ex2
	ldx	srcfile+2
	lda	srcfile
	jsl	freemaxline
ex2	anop

	lda	errfile
	ora	errfile+2
	beq	ex3
	ldx	errfile+2
	lda	errfile
	jsl	freemaxline
ex3	anop
		   
	ldx	word+2
	lda	word
	jsl	freemaxline

	ply		Get return value.

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

;
; Print error message, deallocate memory, and return with value -1
;
error	ldx	#^err00	(Add high word of error address)
	jsr	errputs

tok_error	pei	(cmdline+2)
	pei	(cmdline)
	jsl	nullfree

	ph4	#0	(no path to be freed)
	pei	(argc)
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
spcmdstr	dc	c'gsh: Specify a command before redirecting.',h'0d00'
		    
	END

**************************************************************************
*
* dispose the argv
*
**************************************************************************

argfree	START

space	equ	0

	subroutine (4:path,2:argc,4:argv),space

	pei	path+2	Free the path.
	pei	path
	jsl	nullfree

free1	lda	argc	Free each of the argv elements.
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

free2	pei	(argv+2)	Free the argv array.
	pei	(argv)
	jsl	nullfree
	return

	END

**************************************************************************
*
* Read and execute commands from an exec file (shell script)
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
sptr	equ	ptr+4
status	equ	sptr+2
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
; Set the environment variables $0 ... argc
;
	lda	argc	Get number of variables.
	jeq	vars_set	If 0, there are none to set.

	stz	count	Start with argv[0].
parmloop	lda	count	Get index
	asl2	a	 into address array.
	tay
	lda	[argv],y	Copy argument
	sta	ptr	 pointer to
	iny2		  ptr.
	lda	[argv],y
	sta	ptr+2

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

;
; Build parameter block on stack and call SetGS (p 427 in ORCA/M manual)
;
set_value	anop
	tsx		Save current stack pointer.
	stx	sptr
	pea	0	Export flag
	ph4	ptr	Convert value string
	jsr	c2gsstr	 to a GS/OS string.
	stx	ptr+2	  (Save for later
	sta	ptr	   deallocation).
	phx		
	pha
	ph4	#pname	Address of variable name.
	pea	3	Number of parameters
	tsc		Calculate Param Block addr
	inc	a	 (stack ptr + 1)
	pea	0	  and push 32 bits
	pha		   onto stack
	pea	$0146	SetGS call number
	jsl	$E100B0	Call GS/OS
	lda	sptr	Restore stack pointer.
	tcs

	ph4	ptr	Free the value buffer.
	jsl	nullfree
	inc	count	Bump the parameter counter.
	lda	count	If more to do,
	cmp	argc
	jcc	parmloop	   stay in loop.

;
; $0 ... $n environment variables have all been set
;
vars_set	unlock mutex

;
; Allocate memory for GS/OS calls
;

	ph4	#4	CloseGS parms
	~NEW
	sta	CRec
	stx	CRec+2

	ph4	#10	OpenGS parameter block
	~NEW
	sta	ORec
	stx	ORec+2

	ph4	#12	NewLineGS parameter block
	~NEW
	sta	NRec
	stx	NRec+2

	ph4	#16	ReadGS parameter block
	~NEW
	sta	RRec
	stx	RRec+2

	ph4	#1000	Data buffer (1000 bytes)
	~NEW
	sta	data
	stx	data+2

;
; Set parameters in OpenGS parameter block
;
	lda	#3	Number of parameters = 3.
	sta	[ORec]

	pei	(path+2)	Convert exec file
	pei	(path)	 pathname to GS/OS string
	jsr	c2gsstr
	ldy	#4	Store in PB, offset bytes 4-7.
	sta	[ORec],y
	sta	ptr
	iny2
	txa
	sta	[ORec],y
	sta	ptr+2

	lda	#1	Read access only
	ldy	#8	Store in PB, offset bytes 8-9.
	sta	[ORec],y

	pei	(ORec+2)	OpenGS(ORec)
	pei	(ORec)
	ph2	#$2010
	jsl	$E100B0

	bcc	ok	If there was an error,
	cmp	#$46	 and it's not "file not found"
	bne	openerr	  on glogin or gshrc,
	ldx	jobflag
	bmi	godone
;
; Error opening source file. Assemble parameter block on stack and call
; ErrorGS (p 393 in ORCA/M manual)
;
openerr	anop
	tsx		Save current stack pointer.
	stx	sptr
	pha		Push error number
	pea	1	Push number of parameters
	tsc		Calculate PB addr
	inc	a	 (stack ptr + 1)
	pea	0	  and push 32 bits
	pha		   onto stack
	pea	$0145	ErrorGS call number
	jsl	$E100B0	Call GS/OS
	lda	sptr	Restore stack pointer.
	tcs

godone	jmp	done

ok	ldy	#2	Copy file ref num
	lda	[ORec],y	 from OpenGS PB into
	sta	[NRec],y	  NewLineGS PB,
	sta	[RRec],y	   ReadGS PB,
	sta	[CRec],y	    and CloseGS PB.

;
; Set parameters in NewLineGS parameter block
;
	lda	#4	Number of parameters = 4.
	sta	[NRec]

	ldy	#4	enableMask
	lda	#$7F	 = $7F (each input character
	sta	[NRec],y	  is ANDed with this mask)

	iny2		numChars
	lda	#1	 = 1 (number of newline chars
	sta	[NRec],y	  contained in newline char table)

	iny2		newlineTable pointer
	lda	#NLTable	 = NLTable
	sta	[NRec],y
	iny2
	lda	#^NLTable
	sta	[NRec],y

	pei	(NRec+2)	NewLineGS(NRec)
	pei	(NRec)
	ph2	#$2011
	jsl	$E100B0

	bcc	ok2	If there was an error,
	tsx			Save current stack pointer.
	stx	sptr
	pha
	pea	1
	tsc
	inc	a
	pea	0
	pha
	pea	$0145
	jsl	$E100B0		Call ErrorGS to print a message
	lda	sptr		Restore stack pointer.
	tcs

	jmp	close_ex

;
; Set up ReadGS parameter block
;
ok2	lda	#4	Number of parameters = 4
	sta	[RRec]

	tay		dataBuffer (offset 4)
	lda	data	 = data
	sta	[RRec],y
	iny2
	lda	data+2
	sta	[RRec],y

	iny2		requestCount (offset 4)
	lda	#999	 = 999 (save 1 byte for \0)
	sta	[RRec],y
	iny2
	lda	#0
	sta	[RRec],y

;
; Read lines from the exec file
;
ReadLoop	anop
	pei	(RRec+2)	ReadGS(RRec)
	pei	(RRec)
	ph2	#$2012
	jsl	$E100B0

	bcs	close_ex	Check for error

	ldy	#12	Get transferCount
	lda	[RRec],y

	tay		Store null byte
	lda	#0	 at end of buffer.
	sta	[data],y

	lda	varecho	If $ECHO environment
	beq	noecho	 variable is set,
	ldx	data+2
	lda	data
	jsr	puts		print the line
	jsr	newline		and a newline.

noecho	lda	[data]	If first character
	and	#$FF	 in data buffer is
	if2	@a,eq,#'#',ReadLoop  "#", read next line.

*   call execute: subroutine (4:cmdline,2:jobflag)
	pei	(data+2)
	pei	(data)
	lda	jobflag	Remove "startup" flag bit
	and	#$7FFF	 from jobflag before passing it.
	pha
	jsl	execute
	sta	status

	lda	exit_requested	If exit not requested,
	bne	close_ex
	bra	ReadLoop	  stay in read loop.

;
; Close the exec file
;
close_ex	anop
	lda	#1	Number of parameters = 1.
	sta	[CRec]
	pei	(CRec+2)	CloseGS(CRec)
	pei	(CRec)
	ph2	#$2014
	jsl	$E100B0
	stz	exit_requested	Clear the "exit gsh" flag.

done	pei	(CRec+2)	Free the parameter blocks,
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
	pei	(data+2)	 data buffer,
	pei	(data)
	jsl	nullfree
	pei	(ptr+2)	  and path name.
	pei	(ptr)
	jsl	nullfree

;
; Restore stack and return to caller
;
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

NLTable	dc	h'0d'	Newline Table

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
ptr_glob	equ	pipesem+2
waitstatus	equ	ptr_glob+4
ptr_envexp	equ	waitstatus+2
pid	equ	ptr_envexp+4
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
; Check for special quote characters
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
	sta	ptr_envexp
	stx	ptr_envexp+2

; Was there a variable error?
	ora	ptr_envexp+2
	bne	expglob
	pha		Put null pointer
	pha		 on stack
	jmp	errexit	  and go to error exit.

; Expand wildcard characters in the modified command line
expglob	phx
	lda	ptr_envexp
	pha
	jsl	glob
	sta	ptr_glob
	stx	ptr_glob+2

	ldx	ptr_envexp+2	Free memory allocated
	lda	ptr_envexp	 for env var expansion
	jsl	freemaxline

; Was there a globbing error?
	lda	ptr_glob
	ora	ptr_glob+2
	bne	expalias
	pha		Put null pointer
	pha		 on stack
	jmp	errexit	Go to error exit.

; Expand aliases in the modified command line (final expansion)
expalias	ldx	ptr_glob+2
	phx
	lda	ptr_glob
	pha
	jsl	expandalias
	sta	exebuf
	stx	exebuf+2

	phx		Put exebuf on stack for
	pha		 nullfree at endcmd.

	ldx	ptr_glob+2	Free memory allocated
	lda	ptr_glob            for globbing expansion.
	jsl	freemaxline

; Was there a alias error?
	lda	exebuf
	ora	exebuf+2
	jeq	errexit

* >> Temporary debug code: echo expanded command if echo is set.
	using	vardata
	ldy	varecho
	beq	noecho
	ldx	exebuf+2
	lda	exebuf
	jsr	puts
	jsr	newline
noecho	anop

*   command	subroutine (4:waitpid,2:inpipe,2:jobflag,2:inpipe2,
*		4:pipesem,4:stream,4:awaitstatus)
loop	pea	0	;Bank 0		waitpid (hi)
	tdc
	clc
	adc	#pid
	pha				waitpid (low)
	pea	0			inpipe = 0
	pei	(jobflag)			jobflag
	pea	0			inpipe2 = 0
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
	pea	0	;Bank 0		status (hi) [New: v2.0]
	tdc
	clc
	adc	#waitstatus
	pha				status (low)
	jsl	command  

	sta	term	Save result in term.
	jmi	errexit	If < 0, all done.

; If waitstatus != -1, executed command was a non-forked builtin,
; and waitstatus is its completion status.
	lda	waitstatus
	cmp	#-1
	beq	chkpid
	jsr	setstatus	Set $status.
	bra	godonewait	No need to wait.

chkpid	lda	pid	Get child process id.
	beq	godonewait	If 0 (no fork), no need to wait.
	cmp	#-1	If -1 (error), all done.
	jeq	errexit

	if2	term,eq,#T_AMP,jobwait If last token was "&"
	lda	jobflag	     or jobflag is set,
	beq	jobwait	      do more complicated wait.

;
; Uncomplicated wait: simply call wait() to get child's termination status
;
	signal (#SIGINT,#0)	Use default interrupt and
	phx		 keyboard stop signal handlers,
	pha		  and put address of current
	signal (#SIGTSTP,#0)	   handlers on the stack.
	phx
	pha

otherwait	anop
	ldx	#0
	clc
	tdc
	adc	#waitstatus
	wait	@xa	Wait for child completion.
	cmp	pid
	bne	otherwait
	lda	waitstatus
	and	#$FF
	cmp	#$7F	Check for WSTOPPED status.
	beq	otherwait	Something else...wait again.

	lda	waitstatus 
	jsr	setstatus	Set process's $status.

	pla		Restore gsh's interrupt and
	plx		 keyboard stop signal handlers.
	signal (#SIGTSTP,@xa)
	pla
	plx
	signal (#SIGINT,@xa)

godonewait	bra	donewait

;
; jobflag = 0: need more complicated wait for child
;
jobwait	anop
	signal (#SIGCHLD,#pchild)  Ensure child sig handler active.
	phx		  Save address of previous sig handler.
	pha
	kill	(pid,#0)	If child no longer exists
	beq	wait4job

	pei	pid
	jsl	removejentry	   Remove its pid from the list.
	beq	restoresigh	   Pid not in list: assume $status set.

	ldx	#0
	clc
	tdc
	adc	#waitstatus
	wait	@xa	   Get child completion status.
	lda	waitstatus 
	jsr	setstatus	   Set process's $status.
	bra	restoresigh
;
; Child is active: wait for it to complete and get its status.
; NOTE: $status is set by SIGCHLD signal handler, pchild
;
wait4job	jsl	pwait	Wait for child using pchild


restoresigh	pla		Restore previous child completion
	plx		 signal handler.
	signal (#SIGCHLD,@xa)

;
; Done waiting for completion status.  Check the token last parsed
; from the command line.
;
donewait	if2	term,eq,#T_EOF,endcmd If last token was EOF
	lda	[exebuf]	    or if next character is \0,
	and	#$FF
	beq	endcmd	     all done with this command.

	jmp	loop	  Process the next command.

;
; Underlying routine detected an error. Set waitstatus = -1
;
errexit	lda	#-1
	sta	waitstatus
	jsr	setstatus	   Set process's $status.

;
; We have completed processing of a command
;
endcmd	jsl	nullfree	Free exebuf (addr on stack).
	lda	term	Return -1 if error
	bmi	chk_cmd

	lda	waitstatus	Get completion status, and convert
	xba		 from wait() format to byte value.
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
	dec	a	  return -1 to caller.
	bra	setrtn

;
; Let execute(str,1) do the work
;
makecall	pei	(str+2)
	pei	(str)
	ph2	#1	jobflag=1
	jsl	execute
;
; Set status and go back to the caller
;
setrtn	sta	retval

	return 2:retval

	END
