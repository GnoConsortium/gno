*************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*
* $Id: orca.asm,v 1.3 1998/06/30 17:25:48 tribby Exp $
*
**************************************************************************
*
* ORCA.ASM
*   By Tim Meekins
*   Modified by Dave Tribby for GNO 2.0.6
*
* Builtin command for ORCA editor
*
* Note: text set up for tabs at col 16, 22, 41, 49, 57, 65
*              |     |                  |       |       |       |
*	^	^	^	^	^	^	
**************************************************************************

	mcopy /obj/gno/bin/gsh/orca.mac

dummyorca	start		; ends up in .root
	end

	setcom 60

**************************************************************************
*
* EDIT: builtin command
* syntax: edit pathname...
*
* Invokes the ORCA editor and edits a file or files.
*
**************************************************************************

MAXPARMBUF	gequ	1023

edit	START
strPtr	equ	0
inLoopPtr	equ	strPtr+4
argLoopPtr	equ	inLoopPtr+4
sLen	equ	argLoopPtr+4
pathLen	equ	sLen+2
pnum	equ	pathLen+2
editcommand	equ	pnum+2
retval	equ	editcommand+4
sFile	equ	retval+2	GSString255Ptr
inPath	equ	sFile+4
outPath	equ	inPath+4
space	equ	outPath+4

	subroutine (4:argv,2:argc),space

	lda	argc	Make sure there are two or
	cmp	#2	 more parameters.
	bcs	enoughparms	  Otherwise,
               ldx	#^enofile	    report error:
	lda	#enofile	     no filename specified
	jsr	errputs
               lda	#1
	bra	seterr


; Allocate memory for sFile, inPath, and outPath

enoughparms	anop
               jsl	alloc1024
	sta	sFile
	stx	sFile+2
	ora	sFile+2
               beq	memerr1

               jsl	alloc1024
	sta	inPath
	stx	inPath+2
	ora	inPath+2
               beq	memerr1

               jsl	alloc1024
	sta	outPath
	stx	outPath+2
	ora	outPath+2
               bne	noerr1

memerr1        ldx	#^enomem	Report error:
	lda	#enomem	 out of memory
	jsr	errputs
               lda	#-1
seterr	sta	retval
	jmp	goaway

; Parameters were provided, and memory has been allocated.
; Ready to start processing the filename(s).
noerr1	anop
	stz	retval	Zero return status
               stz	sLen	 and length of source names.

	lda	#1                 Initialize parameter
	sta	pnum	 number to 1.

	lda	sFile	strPtr = sFile + 2
	clc
	adc	#2
	sta	strPtr
	lda	sFile+2
	adc	#0
	sta	strPtr+2

	bra	nodelimit	Skip delimiter for 1st file.

; Loop for getting name, converting it to a full path, and
; appending it to sFile

doloop         short	m	Between parameters:
	lda	#10	Store newline as delimiter
	sta	[strPtr]	 character in strPtr.
	long	m
	inc	sLen	Bump string length
	inc	strPtr	 and pointer.
               bne	nodelimit
	inc	strPtr+2

nodelimit	anop
               lda	pnum	Get parameter number
	asl	a	 and turn it into an
	asl	a	  index to the proper
	tay		   argv pointer.
	lda	[argv],y	Store address in
	sta   argLoopPtr	 direct page variable
	iny2		  argLoopPtr.
	lda	[argv],y
	sta	argLoopPtr+2

               lda	inPath	Get address of text field
	clc		 in inPath.
	adc	#2
	sta   inLoopPtr
	lda	inPath+2
	adc	#0
	sta	inLoopPtr+2

; Move argument into inPath, counting characters

	ldy	#0	

whileloop	lda	[argLoopPtr],y	Get next character of name.
	and	#$00FF	If it's the terminating null,
               beq	donewhile	 done copying.

	sta	[inLoopPtr],y	Store character (and null byte)
	iny
               cpy	#255	If < 255,
               bcc	whileloop	  stay in loop.

               ldx	#^einval	Print error:
	lda	#einval	 invalid argument (filename too long)
	jsr	errputs
               lda	#2
	bra	seterr

donewhile	tya 		Set length of GS/OS string inPath.
	sta	[inPath]
               lda	#1024	Set max len of result buffer outPath.
	sta	[outPath]

; Set up GS/OS ExpandPath parameter buffer and make call.

	mv4	inPath,ep_inputPath
	mv4	outPath,ep_outputPath
	ExpandPath ep	Expand the pathname.

; Note: The GS/OS reference says ExpandPath can detect invalid pathname
;       syntax error, but I can't get this to happen (even with names like
;       "::/^" and " "). Ignore errors and let the editor report them.

	ldy	#2	Get length of result string
	lda	[outPath],y
	sta	pathLen	 and store in pathLen.
	clc
	adc	sLen	If accumulated length is
               cmp	#MAXPARMBUF	 beyond the maximum,
               bcs	doloopend	  don't add this name.

	sta	sLen	Store accumulated length in sLen.

	pei	(pathLen)	Append outPath string
               pei	(outPath+2)	 to the end of sFile's text.
               lda	outPath
	clc
	adc	#4
	pha
	pei	(strPtr+2)
	pei	(strPtr)
	jsl	rmemcpy

	lda   strPtr	Add pathLen to strPtr.
	clc
	adc	pathLen
	sta	strPtr
	lda	strPtr+2
	adc	#0
	sta	strPtr+2

doloopend      inc	pnum	pnum++
	lda	pnum               if pnum < argc,
	cmp   argc
	jcc	doloop	  continue processing filenames.

; All of the arguments have been processed.

	lda	sLen	Save length in
               sta	[sFile]	 GS/OS buffer.

; Set up shell SetLInfo parameter buffer and make call.

               mv4	sFile,gl_sFile
	SetLInfoGS gl	Set the edit environment.

	ph4	#editorvar	Get value of environment
	jsl	getenv	 variable "editor".
	sta	editcommand
	stx	editcommand+2
               ora	editcommand+2
	bne	goteditvar	If $editor is not defined,
               ph4   #defedit	  use default value.
	bra	execit

goteditvar	anop
	pei	(editcommand+2)
	pei	(editcommand)
execit	ph2	#0	;tells execute we're called by system
	jsl	execute
               sta	retval

	lda	editcommand	If getenv allocated it,
	ora	editcommand+2
	beq	goaway

	pei	(editcommand+2)	  free the "editcommand" string.
	pei	(editcommand)
	jsl   nullfree


; See which GS/OS buffers need to be deallocated

goaway         lda	sFile
	ora	sFile+2
	beq	donedealloc
	ldx	sFile+2
	lda	sFile
               jsl   free1024

               lda   inPath
               ora   inPath+2
               beq   donedealloc
	ldx	inPath+2
	lda	inPath
	jsl	free1024

               lda   outPath
               ora   outPath+2
               beq   donedealloc
	ldx	outPath+2
	lda	outPath
	jsl	free1024


; Return to caller with status set to value in retval

donedealloc    return 2:retval


; Parameter block for GS/OS ExpandPath call (p. 140 in GS/OS Reference)
ep	dc	i2'2'	pCount
ep_inputPath	dc	i4'0'	input pointer (GS/OS string)
ep_outputPath	dc	i4'0'	output pointer (GS/OS result buf)

; Error messages
enofile	dc	c'edit: no filename specified',h'0D00'
enomem	dc	c'edit: out of memory',h'0D00'
einval	dc	c'edit: invalid argument (filename too long)',h'0D00'

; Parameter block for shell SetLInfo call (p. 433 in ORCA/M book)
gl	anop
	dc	i2'11'	pCount
gl_sfile	dc	i4'0'	source file name (GS/OS string)
	dc	i4'nullparm'	output file name (compile/link)
	dc	i4'nullparm'	names in NAMES parameter list (compile)
	dc	i4'nullparm'	compiler commands (compiler)
	dc	i1'0'	max err level allowed (compile/link)
	dc	i1'0'	max err level found (compile/link)
	dc	i1'0'	operations flags (compile/link)
	dc	i1'0'	keep flag (compile)
	dc	i4'0'	minus flags (see ASML)
	dc	i4'$08000000'	plus flags [+E] (see ASML)
	dc	i4'0'	origin (link)

nullparm	dc	i2'0'

editorvar	dc	c'editor',h'00'	Name of editor environment variable
defedit	dc	c'4:editor',h'00'	Default value for editor
	END
