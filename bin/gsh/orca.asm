*************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*
* $Id: orca.asm,v 1.2 1998/04/24 15:38:34 gdr-ftp Exp $
*
**************************************************************************
*
* ORCA.ASM
*   By Tim Meekins
*
* Builtin commands for Orca compiling/editing, etc.
*
**************************************************************************

	mcopy /obj/gno/bin/gsh/orca.mac

dummy	start		; ends up in .root
	end

	setcom 60

**************************************************************************
*
* EDIT: builtin command
* syntax: edit pathname...
*
* Invokes an editor and edits a file or files.
*
**************************************************************************

MAXPARMBUF	gequ	1023

edit	START
strPtr	equ	0
inLoopPtr	equ	strPtr+4
argLoopPtr	equ	inLoopPtr+4
sLen	equ	argLoopPtr+4
k	equ	sLen+2
j	equ	k+2
editcommand	equ	j+2
retval	equ	editcommand+4
sFile	equ	retval+2	GSString255Ptr
inPath	equ	sFile+4
outPath	equ	inPath+4
space	equ	outPath+4

	subroutine (4:argv,2:argc),space

	stz	retval
               stz	sLen

	lda	#1
	sta	j

	lda	argc	make sure there are two or
	cmp	#2	more parameters, otherwise we
	bcs	enoughparms

               ldx	#^enofile
	lda	#enofile
	jsr	errputs
               lda	#1
	sta	retval
	jmp	goaway

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

memerr1        ldx	#^enomem
	lda	#enomem
	jsr	errputs
               lda	#-1
	sta	retval
	jmp	goaway

noerr1	anop
	lda	sFile
	clc
	adc	#2
	sta	strPtr
	lda	sFile+2
	adc	#0
	sta	strPtr+2

doloop         lda	j
	cmp	#2
	bcc	nodelimit
               short	m
	lda	#10	; newline
	sta	[strPtr]
	long	m
	inc	sLen
	inc	strPtr
               bne	nodelimit
	inc	strPtr+2

nodelimit	anop
               lda	j
	asl	a	; get the proper argv pointer
	asl	a
	tay
	lda	[argv],y
	sta   argLoopPtr
	iny2
	lda	[argv],y
	sta	argLoopPtr+2

               lda	inPath	get text field of inPath
	clc
	adc	#2
	sta   inLoopPtr
	lda	inPath+2
	adc	#0
	sta	inLoopPtr+2

	stz	k

whileloop	lda	[argLoopPtr]
	and	#$00FF
               beq	donewhile

               short	m
	sta	[inLoopPtr]
               long	m
	inc	argLoopPtr
	bne	noinc2
	inc	argLoopPtr+2
noinc2	inc	inLoopPtr
	bne	noinc3
	inc	inLoopPtr+2
noinc3	inc	k
	lda	k
               cmp	#255
               bcc	whileloop
               ldx	#^einval
	lda	#einval
	jsr	errputs
               lda	#2
	sta	retval
	jmp	goaway
donewhile	lda	k
	sta	[inPath]
               lda	#1024
	sta	[outPath]
	mv4	inPath,ep_inputPath
	mv4	outPath,ep_outputPath

	ExpandPath ep
	bcc	noeperror
               ldx	#^err3
	lda	#err3
	jsr	errputs
               lda	#-1
	sta	retval
	jmp	goaway

noeperror	anop
	ldy	#2
	lda	[outPath],y
	sta	k
	clc
	adc	sLen
               cmp	#MAXPARMBUF
               bcs	doloopend

	pei	(k)
               pei	(outPath+2)
               lda	outPath
	clc
	adc	#4
	pha
	pei	(strPtr+2)
	pei	(strPtr)
	jsl	rmemcpy

	lda	sLen
	clc
	adc	k
	sta	sLen
	lda   strPtr
	clc
	adc	k
	sta	strPtr
	lda	strPtr+2
	adc	#0
	sta	strPtr+2
doloopend      inc	j
	lda	j
	cmp   argc
	jcc	doloop

	lda	sLen
               sta	[sFile]
               mv4	sFile,gl_sFile

	Set_LInfoGS gl	;now set up the environment

	ph4	#editorvar
	jsl	getenv
	sta	editcommand
	stx	editcommand+2
               ora	editcommand+2
	bne	goteditvar

               ph4   #defedit
               jsr   p2cstr
               sta	editcommand
	stx	editcommand+2

goteditvar	anop
	pei	(editcommand+2)
	pei	(editcommand)
	ph2	#0	;tells execute we're called by system
	jsl	execute
               sta	retval

	pei	(editcommand+2)
	pei	(editcommand)
	jsl   nullfree

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

donedealloc    return 2:retval

ep	dc	i2'2'
ep_inputPath	dc	i4'0'
ep_outputPath	dc	i4'0'

enofile	dc	c'edit: no filename specified',h'0D00'
enomem	dc	c'edit: out of memory',h'0D00'
einval	dc	c'edit: invalid argument (filename too long)',h'0D00'
err3	dc	c'edit: invalid path name',h'0d00'

gl	anop
	dc	i2'11'
gl_sfile	dc	i4'0'
	dc	i4'nullparm'
	dc	i4'nullparm'
	dc	i4'nullparm'
	dc	i1'8'
	dc	i1'0'
	dc	i1'0'
	dc	i1'0'
	dc	i4'0'
	dc	i4'$8000000'
	dc	i4'0'

nullparm	dc	i2'0'

editorvar	dc	c'editor',h'00'
defedit	str	'4:editor'
	END
