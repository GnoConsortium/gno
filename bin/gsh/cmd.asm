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
* CMD.ASM
*   By Tim Meekins
*
* Command line parsing routines.
*
**************************************************************************

               mcopy m/cmd.mac
               keep  o/cmd.mac

SIGINT	gequ	 2
SIGSTOP	gequ	17
;
; TOKENS used by the parser
;
T_WORD         gequ  1
T_BAR          gequ  2
T_AMP          gequ  3
T_SEMI         gequ  4
T_GT           gequ  5
T_GTGT         gequ  6
T_GTAMP        gequ  7
T_GTGTAMP      gequ  8
T_LT           gequ  9
T_NL           gequ  10
T_EOF          gequ  11
T_ERROR	gequ	12
T_NULL	gequ	13

MAXARG         gequ  256
BADFD          gequ  -2

**************************************************************************
*
* Read a token from the buffer.
*
* Input: word points to a buffer to place a word is parsed.
*
* Ouput: the token is placed in the accumulator
*
**************************************************************************

gettoken       START

buf            equ   1
state          equ   buf+4
ch             equ   state+2
space          equ   ch+2
stream	equ	space+3
word           equ   stream+4
end            equ   word+4

;               subroutine (4:word,4:stream),space

               tsc
               sec
               sbc   #space-1
               tcs
               phd
               tcd
;
; What state are we in
;
NEUTRAL        equ   0                  ;a neutral state, get anything
GTGT           equ   1                  ;looking for a second '>'
INQUOTE        equ   2                  ;parsing a quoted string
INWORD         equ   3                  ;parsing a word
SINGQUOTE      equ   4                  ;single quote string
;
; start in the neutral state
;
               ld2   NEUTRAL,state
	lda	[stream]
	sta	buf
	ldy	#2
	lda	[stream],y
	sta	buf+2
;
; the main loop
;
loop           lda   [buf]
               inc   buf
               and2  @a,#$FF,ch
               bne   switch

               if2   state,ne,#INWORD,loop2
               jmp   endword

loop2          if2   @a,ne,#GTGT,loop3
	dec	buf
	lda	#T_GT
	jmp	done

loop3	if2	@a,eq,#INQUOTE,error1
	if2	@a,eq,#SINGQUOTE,error2
	lda   #T_EOF
               jmp   done

error1	ldx	#^errstr1
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
switch         lda   state
               asl   a
               tax
               jmp   (statetbl,x)
statetbl       dc    a2'case_neutral'
               dc    a2'case_gtgt'
               dc    a2'case_inquote'
               dc    a2'case_inword'
               dc    a2'case_single'
;
; CASE NEUTRAL
;
case_neutral   if2   ch,ne,#';',neut1
               lda   #T_SEMI
               jmp   done
neut1          if2   @a,ne,#'&',neut2
               lda   #T_AMP
               jmp   done
neut2          if2   @a,ne,#'|',neut3
               lda   #T_BAR
               jmp   done
neut3          if2   @a,ne,#'<',neut4
               lda   #T_LT
               jmp   done
neut5          cmp	#' '	;space
	jeq	loop
	cmp	#9	;tab
	jeq	loop
               if2   @a,ne,#'>',neut6
               lda   #GTGT
               bra   neut10
neut4          if2   @a,ne,#13,neut4a   ;return
               lda   #T_NL
               jmp   done
neut4a         if2   @a,ne,#0,neut4b    ;EOF
               lda   #T_EOF
               jmp   done
neut4b	if2	@a,ne,#'#',neut5	;comment
neut4c	lda	[buf]
	and	#$7F
	beq	neut4d
               inc   buf
               if2   @a,eq,#13,neut4d
	if2	@a,eq,#10,neut4d
               bra   neut4c
neut4d	jmp	loop
neut6          if2   @a,ne,#'"',neut7
startquote     lda   #INQUOTE
               bra   neut10
neut7          if2   @a,ne,#"'",neut8
startsingle    lda   #SINGQUOTE
               bra   neut10
neut8          if2   @a,ne,#'\',neut9
               lda   [buf]
               and   #$FF
               inc   buf
	if2	@a,eq,#13,neut10a
neut9	sta   [word]             ;default
               inc   word
               lda   #INWORD
neut10         sta   state
neut10a        jmp   loop
;
; CASE GTGT
;
case_gtgt      if2   ch,eq,#'>',gtgt2
	if2	@a,eq,#'&',gtgt1
          	dec   buf
               lda   #T_GT
               jmp   done
gtgt1	lda	#T_GTAMP
	jmp	done
gtgt2          lda   [buf]
	and	#$FF
	if2	@a,eq,#'&',gtgt3
	lda   #T_GTGT
               jmp   done
gtgt3	inc	buf
	lda	#T_GTGTAMP
	jmp	done
;
; CASE INQUOTE
;
case_inquote   if2   ch,ne,#'\',quote2  ;is it a quoted character?
               lda   [buf]
               inc   buf
putword        sta   [word]
               inc   word
               jmp   loop
quote2         if2   @a,ne,#'"',putword
               ld2   INWORD,state
               jmp   loop

;
; CASE SINGLEQUOTE
;
case_single    anop
               if2   ch,ne,#"'",putword
               ld2   INWORD,state
               jmp   loop
;
; CASE INWORD
;
case_inword    if2   ch,eq,#000,endword
               if2   @a,eq,#';',endword
               if2   @a,eq,#'&',endword
               if2   @a,eq,#'|',endword
               if2   @a,eq,#'>',endword
	if2	@a,eq,#'<',endword
               if2   @a,eq,#' ',endword
               if2   @a,eq,#009,endword
               if2   @a,eq,#013,endword
               cmp   #'"'
               jeq   startquote
               cmp   #"'"
               jeq   startsingle
               if2   @a,ne,#'\',putword
               lda   [buf]
               inc   buf
	and	#$FF
	if2	@a,eq,#13,word2
               bra   putword
word2	jmp	loop
endword        dec   buf
finiword       lda   #0
               sta   [word]
               lda   #T_WORD

done           tax
	lda	buf
	sta	[stream]
	lda	buf+2
	ldy	#2
	sta	[stream],y

               lda   space
               sta   end-3
               lda   space+1
               sta   end-2
               pld
               tsc
               clc
               adc   #end-4
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

command        START

pipefds	equ	1
errappend      equ   pipefds+2
errfile        equ   errappend+2
srcfile        equ   errfile+4
dstfile        equ   srcfile+4
count          equ   dstfile+4
argv           equ   count+2
word           equ   argv+4
cmdline        equ   word+4
pid            equ   cmdline+4
append         equ   pid+2
temp           equ   append+2
argc           equ   temp+4
token          equ   argc+2
space          equ   token+2
stream	equ	space+3
pipesem	equ	stream+4
inpipe2	equ	pipesem+4
jobflag	equ	inpipe2+2
inpipe       	equ   jobflag+2
waitpid        equ   inpipe+2
end            equ   waitpid+4

;               subroutine (4:waitpid,2:inpipe,2:jobflag,2:inpipe2,4:pipesem,4:stream),space

               tsc
               sec
               sbc   #space-1
               tcs
               phd
               tcd

               ph4   #1024
               jsl   ~NEW
               sta   cmdline
               stx   cmdline+2
               lda   #0
               sta   [cmdline]

	jsl	alloc1024
               sta   word
               stx   word+2

               ph4   #MAXARG*4
               jsl   ~NEW
               sta   argv
               stx   argv+2

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
               sta   [waitpid]

loop           pei   (word+2)
               pei   (word)
	pei	(stream+2)
	pei	(stream)
               jsl   gettoken
               sta   token
               asl   a
               tax
               jmp   (toktbl,x)
toktbl         dc    a2'loop'
               dc    a2'tok_word'
               dc    a2'tok_bar'
               dc    a2'tok_amp'
               dc    a2'tok_semi'
               dc    a2'tok_gt'
               dc    a2'tok_gtgt'
	dc	a2'tok_gtamp'
	dc	a2'tok_gtgtamp'
               dc    a2'tok_lt'
               dc    a2'tok_nl'
               dc    a2'tok_eof'
	dc	a2'tok_error'
;
; Parse a word token
;
tok_word       if2   argc,ne,#MAXARG,word1
               lda   #err00             ;Too many arguments
               jmp   error
word1          pei   (word+2)
               pei   (word)
               jsr   cstrlen
               inc   a
               pea   0
               pha
               jsl   ~NEW
               sta   temp
               stx   temp+2
               ora   temp+2
               bne   word2
               lda   #err01             ;Out of memory for arguments
               jmp   error
word2          lda   argc               ;Copy word to argv[argc]
               asl2  a
               tay
               lda   temp
               sta   [argv],y
               lda   temp+2
               iny2
               sta   [argv],y
               pei   (word+2)
               pei   (word)
               pei   (temp+2)
               pei   (temp)
               jsr   copycstr

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

appword        pei   (word+2)           ;append word to current command line
               pei   (word)
               pei   (cmdline+2)
               pei   (cmdline)
               jsr   cstrlen
               tay
               ldx   argc
               beq   nospace
               lda   #' '
               sta   [cmdline],y
               iny
nospace        lda   count
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
noquote	pei   (cmdline+2)
               add2  @y,cmdline,@a
               pha
               jsr   copycstr
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
noquote2       inc   argc               ;increment argument count
goloop         jmp   loop
;
; Parse a '<' token
;
tok_lt         lda   srcfile
	ora	srcfile+2
               beq   lt1
               lda   #err02             ;Extra < encountered
               jmp   error
lt1            lda	inpipe
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
               jsl   gettoken
               if2   @a,eq,#T_WORD,goloop
               lda   #err03             ;Illegal < specified
               jmp   error
;
; Parse a '>' or '>>'
; 
tok_gt         anop
tok_gtgt	lda  	dstfile
               ora   dstfile+2
	beq	gt1
               lda   #err04             ;Extra > or >> encountered
               jmp   error
gt1            jsl	alloc1024
	stx	dstfile+2
	sta	dstfile
	phx
	pha
	pei	(stream+2)
	pei	(stream)
               jsl   gettoken
               if2   @a,eq,#T_WORD,gt2
               lda   #err05             ;Illegal > or >> specified
               jmp   error
gt2            stz   append
               if2   token,ne,#T_GTGT,gt3
               inc   append
gt3            jmp   loop
;
; Parse a '>&' or '>>&'
; 
tok_gtamp      anop
tok_gtgtamp    lda   errfile
               ora   errfile+2
	beq	ga1
               lda   #err06             ;Extra >& or >>& encountered
               jmp   error
ga1            jsl	alloc1024
	stx	errfile+2
	sta	errfile
	phx
	pha
	pei	(stream+2)
	pei	(stream)
               jsl   gettoken
               if2   @a,eq,#T_WORD,ga2
               lda   #err07             ;Illegal >& or >>& specified
               jmp   error
ga2            stz   errappend
               if2   token,ne,#T_GTGTAMP,ga3
               inc   errappend
ga3            jmp   loop
;
; Parse a command terminator
;
tok_bar        anop
tok_amp        anop
tok_semi       anop
tok_nl         anop
tok_eof        anop

               lda   argc	;terminate the argv list
               bne	nonnull
	lda	#0
               sta   [waitpid]
	lda	#T_NULL
	jmp	exit
nonnull        asl2  a
               tay
               lda   #0
               sta   [argv],y
               iny2
               sta   [argv],y
;
; see if there is a conflict between >,>> with |
;
	lda	token
               if2   @a,ne,#T_BAR,runit
	lda	dstfile
	ora	dstfile+2
	beq	bar2
               lda   #err08             ;> or >> conflicts with |
               jmp   error

bar2           clc
	tdc
	adc	#pipefds
	ldx	#0
	pipe	@xa
;what if pipes return errors?

runit	pei   (argc)
               pei   (argv+2)
               pei   (argv)
               pei   (srcfile+2)
	pei	(srcfile)
               pei   (dstfile+2)
	pei	(dstfile)
               pei   (errfile+2)
	pei	(errfile)
               pei   (append)
	pei	(errappend) 
               ldx   #0
               if2   token,ne,#T_AMP,run2
               inx
run2           phx
               pei   (cmdline+2)
               pei   (cmdline)
	pei	(jobflag)
	pei	(inpipe)
	pei	(pipefds+2)
	pei	(inpipe2)
	pei	(pipefds)
	pei	(pipesem+2)
	pei	(pipesem)
               jsl   invoke
               sta   pid
	cmp	#-1
	beq	exit

           	if2   token,ne,#T_BAR,run3

	pei   (waitpid+2)
               pei   (waitpid)
               pei	(pipefds)
	pei	(jobflag)
	pei	(pipefds+2)
	pei	(pipesem+2)
	pei	(pipesem)
	pei	(stream+2)
	pei	(stream)
               jsl   command
               bra	exit

run3           lda   pid
               sta   [waitpid]
           	lda   token

; clean up

exit           pha

	lda	dstfile
	ora	dstfile+2
	beq	ex1
	ldx	dstfile+2
	lda	dstfile
               jsl   free1024
ex1	anop

	lda	srcfile
	ora	srcfile+2
	beq	ex2
	ldx	srcfile+2
	lda	srcfile
               jsl   free1024
ex2	anop

	lda	errfile
	ora	errfile+2
	beq	ex3
	ldx	errfile+2
	lda	errfile
               jsl   free1024
ex3	anop
                        
          	ldx   word+2
               lda   word
               jsl   free1024

               ply

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

error	ldx	#^err00
	jsr	errputs

tok_error	pei	(cmdline+2)
	pei	(cmdline)
	jsl	nullfree

exit1a         pei	(argc)
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

free1	lda   argc
               beq   free2
               dec   a
               asl2  a
               tay
               lda   [argv],y
               tax
               iny2
               lda   [argv],y
               pha
               phx
               jsl   nullfree
               dec   argc
               bra   free1
free2         	pei   (argv+2)
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

ShellExec      START

	using	vardata
	using	global

count          equ   1
data           equ   count+2
CRec           equ   data+4
RRec           equ   CRec+4
NRec           equ   RRec+4
ORec           equ   NRec+4
ptr            equ   ORec+4
space          equ   ptr+4
jobflag	equ	space+3
argv           equ   jobflag+2
argc           equ   argv+4
path           equ   argc+2
end            equ   path+4

;               subroutine (4:path,2:argc,4:argv,2:jobflag),space

               tsc
               sec
               sbc   #space-1
               tcs
               phd
               tcd

wait           ldy   mutex
               beq   wait0
               cop   $7F
               bra   wait
wait0          inc   mutex
;
; set the variables 0..argc
;
	lda	argc
	jeq	skipvar
               stz   count
parmloop       lda   count
               asl2  a
               tay
               lda   [argv],y
               sta   setparm+4
               iny2
               lda   [argv],y
               sta   setparm+6
               lda   count
               cmp   #10
               bcs   num1
               short a
               adc   #'0'
               sta   num+2
               long  a
               ld4   num+2,setparm
               bra   setit
num1           cmp   #100
               bcs   num2
               Int2Dec (@a,#num+1,#2,#0)
               ld4   num+1,setparm
               bra   setit
num2           Int2Dec (@a,#num,#3,#0)
               ld4   num,setparm
setit          ph4	setparm
	jsr	c2pstr2
	phx
	pha
	stx	setparm+2
	sta	setparm
	ph4	setparm+4
	jsr	c2pstr2
	phx
	pha
	stx	setparm+4+2
	sta	setparm+4
	Set_Variable setparm
	jsl	nullfree
	jsl	nullfree
               inc   count
               lda   count
               cmp   argc
               jcc   parmloop

skipvar        dec   mutex

               ph4   #4                 ;Close parms
               jsl   ~NEW
               sta   CRec
               stx   CRec+2

               ph4   #10                ;Open parms
               jsl   ~NEW
               sta   ORec
               stx   ORec+2

               ph4   #12                ;NewLine parms
               jsl   ~NEW
               sta   NRec
               stx   NRec+2

               ph4   #16                ;Read parms
               jsl   ~NEW
               sta   RRec
               stx   RRec+2

               ph4   #1000           	;data buffer
               jsl   ~NEW
               sta   data
               stx   data+2

               pei	(path+2)	;Convert filename to GS/OS string
	pei	(path)
	jsr	c2gsstr
               ldy   #4
               sta   [ORec],y
               sta   ptr
               iny2
               txa
               sta   [ORec],y
               sta   ptr+2

	ldy	#8
	lda	#1	;Read access only
	sta	[ORec],y

               lda   #3                 ;Open the file
               sta   [ORec]
               pei   (ORec+2)
               pei   (ORec)
               ph2   #$2010             ;OPEN
               jsl   $E100B0
               bcc   ok
               sta   Err
               Error Err
               jmp   done
awshit         sta   Err
               Error Err
               jmp   almostdone

ok             ldy   #2                 ;Copy file ref num
               lda   [ORec],y
               sta   [NRec],y
               sta   [RRec],y
               sta   [CRec],y

               lda   #4                 ;Do NewLine
               sta   [NRec]
               ldy   #4
               lda   #$7F
               sta   [NRec],y
               iny2
               lda   #1
               sta   [NRec],y
               iny2
               lda   #NLTable
               sta   [NRec],y
               iny2
               lda   #^NLTable
               sta   [NRec],y
               pei   (NRec+2)
               pei   (NRec)
               ph2   #$2011             ;NEWLINE
               jsl   $E100B0
               bcs   awshit

               lda   #4                 ;Set up read parm
               sta   [RRec]
               tay
               lda   data
               sta   [RRec],y
               iny2
               lda   data+2
               sta   [RRec],y
               iny2
               lda   #1000
               sta   [RRec],y
               iny2
               lda   #0
               sta   [RRec],y

ReadLoop       anop
               pei   (RRec+2)
               pei   (RRec)
               ph2   #$2012             ;READ
               jsl   $E100B0
               bcs   almostdone
               ldy   #12
               lda   [RRec],y
               tay
               lda   #0
               sta   [data],y
               lda   varecho
               beq   noecho
	ldx	data+2
	lda	data
	jsr	puts
               jsr   newline
noecho         lda   [data]
               and   #$FF
               if2   @a,eq,#'#',ReadLoop
               pei   (data+2)
               pei   (data)
*	ph2	#0
*	ph2	#1
	pei	(jobflag)
               jsl   execute
	lda	exitamundo
	bne	almostdone
               bra   ReadLoop

almostdone     anop
	stz	exitamundo
               lda   #1
               sta   [CRec]
               pei   (CRec+2)
               pei   (CRec)
               ph2   #$2014             ;CLOSE
               jsl   $E100B0

done           pei   (CRec+2)
               pei   (CRec)
               jsl   nullfree
               pei   (NRec+2)
               pei   (NRec)
               jsl   nullfree
               pei   (RRec+2)
               pei   (RRec)
               jsl   nullfree
               pei   (ORec+2)
               pei   (ORec)
               jsl   nullfree
               pei   (data+2)
               pei   (data)
               jsl   nullfree
               pei   (ptr+2)
               pei   (ptr)
               jsl   nullfree

exit1a         anop

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

NLTable        dc    h'0d'

Err            ds    2

setparm        ds    4
               ds    4
num            dc    c'000',h'00'

mutex          dc    i'0'

               END

**************************************************************************
*
* Execute a command line by calling command for each command in the buffer
*
**************************************************************************

execute        START

exebuf	equ	1
pipesem	equ	exebuf+4
ptr2	equ	pipesem+2
waitstatus     equ   ptr2+4
ptr            equ   waitstatus+2
pid            equ   ptr+4
term           equ   pid+2
space          equ   term+2
jobflag	equ	space+3	;set if not a job
cmdline	equ   jobflag+2
end	equ   cmdline+4

;               subroutine (4:cmdline,2:jobflag),space

	tsc
	sec
	sbc	#space-1
	tcs
	phd
	tcd

	stz	pipesem
	stz	waitstatus

               pei   (cmdline+2)
               pei   (cmdline)
               jsl   expandvars
               phx
               pha
               sta   ptr
               stx   ptr+2
               jsl   glob
               phx                 
               pha
	sta	ptr2
	stx	ptr2+2
               jsl   expandalias
               phx            
               pha
               sta   exebuf
               stx   exebuf+2
               ldx   ptr+2
               lda   ptr
               jsl   free1024
               ldx   ptr2+2
               lda   ptr2
               jsl   free1024
                              
               lda   exebuf
               ora   exebuf+2
               bne   loop

               pla
               pla
               stz   term
	lda	#0
               jmp   ouch

loop	pea	0       	;Bank 0
               tdc
               clc
               adc   #pid
               pha
               pea   0
	pei	(jobflag)
	pea	0
	pea	0       	;Bank 0
               tdc
               clc
               adc   #pipesem
               pha
	pea	0       	;Bank 0
               tdc
               clc
               adc   #exebuf
               pha
               jsl   command  
               sta   term
	bmi	noerrexit

               lda   pid
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

nowait         if2   term,eq,#T_EOF,noerrexit
               lda   [exebuf]
               and   #$FF
               beq   exit
               jmp   loop

noerrexit	stz	waitstatus	;non-forked builtins cannot return an error
exit           jsl   nullfree
	lda	term	;make sure we return -1 if error
	bmi	ouch

	lda	waitstatus
	xba
	and	#$FF
ouch	tay
	lda	space+1
	sta	end-2
	lda	space
	sta	end-3
	pld
	tsc
	clc
	adc	#end-4
	tcs

               tya
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

	subroutine (4:str),space	        ;need the phk/plb

	pei	(str+2)
	pei	(str)
	ph2	#1	;tells execute we're called by system
	jsl	execute
	sta	retval

	return 2:retval

	END
