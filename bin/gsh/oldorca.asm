*************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*
**************************************************************************
*
* ORCA.ASM
*   By Tim Meekins
*
* Builtin commands for Orca compiling/editing, etc.
*
**************************************************************************

	keep	o/orca
	mcopy m/orca.mac

**************************************************************************
*
* EDIT: builtin command
* syntax: edit [pathname]
*
* Invokes an editor and edits a file.
*
**************************************************************************

edit	START

waitstatus	equ	0
pid	equ	waitstatus+4
editfile	equ	pid+2
outpath	equ	editfile+4
errcode	equ	outpath+4
space	equ	errcode+2

	subroutine (4:argv,2:argc),space

	stz	errcode

	lda	argc
	cmp	#2
	beq	ok
	bcc	toofew

	ldx	#^err2
	lda	#err2
	jmp	error
toofew	ldx	#^err1
	lda	#err1
error	jsr	errputs
	lda	#2
	sta	errcode
	jmp	done

ok	lock	mutex

	ldy	#6	;argv[1] -> ep.inputPath
	lda	[argv],y
	pha
	ldy	#4
	lda	[argv],y
	pha
	jsr	c2gsstr	;make it a GS/OS filename
	sta	ep_inputPath
	stx	ep_inputPath+2
	jsl	alloc256
	sta	outpath
	stx	outpath+2
	sta	ep_outputPath
	stx	ep_outputPath+2
	lda	#256
	sta	[outpath]
	ExpandPath ep
	bcc	ok2
	ldx	#^err3
	lda	#err3
	jsr	errputs
	lda	#-1
	sta	errcode
	ldx	outpath+2
	lda	outpath
	jsl	free256
	ph4	ep_inputpath
	jsl	nullfree
	unlock mutex
	jmp	done

ok2	ph4	ep_inputpath
	jsl	nullfree

	ldy	#2
	lda	[outpath],y
	xba
	sta	[outpath],y
	add4	outpath,#3,gl	;gl.sfile

; convert ':'s to '/'s

	lda	[outpath],y
	xba
	tax
	ldy	#4
	short	a
conv	lda	[outpath],y
	cmp	#':'
	bne	next
	lda	#'/'
	sta	[outpath],y
next	iny
	dex
	bne	conv
	long	a

	Set_LInfo gl	;now set up the environment

	ph4	#editorvar
	jsl	getenv
	sta	editfile	
	stx	editfile+2
	ora	editfile+2
	bne	gotedit
	ph4	#defedit
	jsr	p2cstr
	sta	editfile
	stx	editfile+2

gotedit	unlock mutex

	pei	(editfile+2)
	pei	(editfile)
	ph2	#0	;tells execute we're called by system
	jsl	execute
               sta	errcode                

	ldx	outpath+2
	lda	outpath
	jsl	free256

	pei	(editfile+2)
	pei	(editfile)
	jsl	nullfree

done	return 2:errcode

err1	dc	c'edit: no filename specified',h'0d00'
err2	dc	c'edit: only one filename allowed',h'0d00'
err3	dc	c'edit: invalid path name',h'0d00'

mutex	key

gl	dc	i4'0'
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

ep	dc	i2'2'
ep_inputPath	dc	i4'0'
ep_outputPath	dc	i4'0'

nullparm	dc	i2'0'

editorvar	dc	c'editor',h'00'
defedit	str	'4/editor'

	END
