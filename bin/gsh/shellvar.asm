**************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*   Derek Taubert
*
* $Id: shellvar.asm,v 1.7 1998/12/21 23:57:08 tribby Exp $
*
**************************************************************************
*
* SHELLVAR.ASM
*   By Tim Meekins
*   Modified by Dave Tribby for GNO 2.0.6
*
* Routines for handling variables in the shell
*
* Note: text set up for tabs at col 16, 22, 41, 49, 57, 65
*              |     |                  |       |       |       |
*	^	^	^	^	^	^	
**************************************************************************
*
* Interfaces defined in this file:
*
* updatevars	subroutine (4:var,2:flag),space	;flag 1: set, 0: unset
* InitVars	jsr without any parameters
*
* Remainder are interfaces to builtin commands with interface
*	subroutine (4:argv,2:argc)
*	returns status in accumulator
* export
* set	(setenv is alternate entry point)
* unset
*
**************************************************************************

	mcopy /obj/gno/bin/gsh/shellvar.mac

dummyshellvar	start		; ends up in .root
	end

	setcom 60


**************************************************************************
*
* SET/SETENV: builtin command
* syntax: set		             - displays all variables
*         set ... [var]           - displays the value of var
*         set [var=value]...      - sets var to value
*         set [var value]...      - sets var to value
*
**************************************************************************

set	START

arg	equ	1
valbuf	equ	arg+4
varbuf	equ	valbuf+4
exflag	equ	varbuf+4
status	equ	exflag+2
space	equ	status+2
argc	equ	space+3
argv	equ	argc+2
end	equ	argv+4

;
; Entry point for set command: clear export flag after setting up params.
;

;	 subroutine (4:argv,2:argc),space

	tsc
	sec
	sbc	#space-1
	tcs
	phd
	tcd

	stz	exflag
	bra	startcmd	Go start the command.


;
; Entry point for setenv command: set export flag after setting up params.
;
setenv	ENTRY

;	 subroutine (4:argv,2:argc),space

	tsc
	sec
	sbc	#space-1
	tcs
	phd
	tcd

	lda	#1
	sta	exflag

;
; Beginning of main code for both set and setenv commands
;
startcmd	anop

	stz	status

	lda	argc	If no parameter provided,
	dec	a
	beq	showvars	 list all variables.

;
; If parameter provided, check for an illegal '-' starting the parm
;
	ldy	#4
	lda	[argv],y
	sta	arg
	iny2
	lda	[argv],y
	sta	arg+2
	lda	[arg]
	and	#$FF
	cmp	#'-'
	beq	showusage
	jmp	skipvar

showusage	ldx	#^Usage
	lda	#Usage
	jsr	errputs
	inc	status	Return status = 1.
	jmp	exit

;
; Show all environment variables
;
showvars	anop
	ph4	#261
	~NEW		Allocate 261 bytes
	sta	varbuf	 for name buffer.
	stx	varbuf+2
	ora	varbuf+2
	beq	svwhoops
	jsl	alloc1024	Allocate 1024 bytes
	sta	valbuf	 for result buffer.
	stx	valbuf+2
	ora	valbuf+2	If memory was not allocated,
	bne	startshow
	ph4	varbuf
	jsl	nullfree
svwhoops	ld2	$201,ErrError		report memory error
	ErrorGS Err
	inc	status	         set return status = 1
	jmp	exit		  and exit.

startshow	anop
	lda	#1022	Store buffer len == 1022 in value
	sta	[valbuf]	 buffer (save 2 bytes at end for 0).
	lda	#260	Store buffer len == 260 in name
	sta	[varbuf]	 buffer.
	lock	setmutex
	mv4	varbuf,idxName	Initialize ReadIndexedGS
	mv4	valbuf,idxValue	 parameter block.
	ld2	1,idxIndex	Start index at 1.

showloop	ReadIndexedGS idxParm	Get next indexed variable.
	ldy	#2	Get length of name.
	lda	[varbuf],y
	beq	showdone	If 0, we've got all the names.
	cmp	#257	If len > 256,
	bcs	bumpindx	  we didn't get it.

	tay		Store 0 at end of
	iny4		 name string so it
	short	a	  can be treated like
	lda	#0	   a c-string.
	sta	[varbuf],y
	long	a

	ldx	idxExport	X = variable's export flag.
	ldy	#4	Y = offset in varname to text.
	jsr	prnameval	Print varname and varval.

bumpindx	inc	idxIndex	Bump index number.
	bra	showloop	Handle the next env variable.

;
; Done showing the list of all variables.
;
showdone	anop
	unlock setmutex	Unlock mutual exclusion.
	ph4	varbuf
	jsl	nullfree	Free the name buffer.
	ldx	valbuf+2
	lda	valbuf
	jsl	free1024	Free the value buffer.
	jmp	exit	Exit.


;
; Set the value of a variable (loop begins here)
;
setvar	lock	setmutex
	lda	argc	If we've run out of parameters,
	jeq	doneset	 we are done setting values.
	lda	[argv]
	sta	arg
	ldy	#2
	lda	[argv],y
	sta	arg+2
;
; Examine characters in second argument to determine syntax style
;
	ldy	#0
chkeql	lda	[arg],y
	and	#$FF
	beq	orcastyle	No "=": user ORCA-style parsing
	cmp	#'='	"=" found: use UNIX-style parsing
	jeq	unixstyle
	iny
	bra	chkeql

;
; No "=" found in second argument. Either ORCA style or a single var show.
;
orcastyle	add2	argv,#4,argv	Point to next argument.
	dec	argc
	jeq	showonevar	If only one arg, it's a single show.

	ldy	#2
	lda	[argv],y
	pha
	lda	[argv]	Create GS/OS string
	pha		 that contains the value.
	bra	set1	Complete operation in UNIX-style code.

;
; UNIX style set. Uses two arguments separated by "=".
; When we get here, Y-reg = index of "=" character.
;
unixstyle	cpy	#0
	bne	unix0
	ldx	#^error1
	lda	#error1		Print error message:
	jsr	errputs		 'Variable not specified'
	inc	status	Return status = 1.
	jmp	doneset
unix0	short	a	Store '\0' on
	lda	#0	 on top of '='
	sta	[arg],y	  so it looks
	long	a	   like a c-string.
	tya		Add length of variable name
	sec		 to address of arg to get
	adc	arg	  address of value.
	pei	(arg+2)
	pha

set1	jsr	c2gsstr	Convert value to GS/OS string.
	sta	RSvalue
	stx	RSvalue+2

	pei	(arg+2)
	pei	(arg)
	jsr	c2gsstr	Convert name to GS/OS string.
	sta	RSname
	stx	RSname+2

	lda	exflag	Set export flag in parameter block.
	sta	RSexport
		
	SetGS ReadSetVar	Set variable value & export flag.

	pei	(arg+2)
	pei	(arg)
	pea	1
	jsl	updatevars	Update special shell flags.

	ph4	RSname
	jsl	nullfree	Free name buffer.
	ph4	RSvalue
	jsl	nullfree	Free value buffer.

nextvar	unlock setmutex
skipvar	add2	argv,#4,argv
	dec	argc
	jmp	setvar

;
; Display the value of a single variable
;
showonevar	anop

	jsl	alloc1024	Allocate 1024 bytes
	sta	valbuf	 for result buffer.
	sta	RSvalue
	stx	valbuf+2
	stx	RSvalue+2
	ora	valbuf+2	Check for memory error.
	jeq	nextvar
	lda	#1022	Store max len == 1022 in result
	sta	[valbuf]	 buffer (save 2 bytes at end for 0).

	pei	(arg+2)	Create GS/OS string that
	pei	(arg)	 contains the variable name.
	jsr	c2gsstr
	sta	varbuf
	stx	varbuf+2
	sta	RSname
	stx	RSname+2
		  
	stz	RSexport

	ReadVariableGS ReadSetVar 	Read value of variable.

	lda	RSexport	If export flag is set, it's defined.
	bne	def
	lda	exflag	If export is required,
	bne	notdef	 report 'not defined'.
	ldy	#2
	lda	[valbuf],y	If there is no value length
	bne	def	  print error message:

notdef	ldx	#^error2		'Variable not defined'
	lda	#error2
	jsr	errputs
	inc	status	Return status = 1.
	bra	doneone

def	ldx	RSexport	X = export flag.
	ldy	#2	Y = offset in varname to text.
	jsr	prnameval	Print varname and varval.

doneone	anop
	ldx	valbuf+2
	lda	valbuf
	jsl	free1024	Free valbuf.
	ph4	varbuf
	jsl	nullfree	Free varbuf.


doneset	unlock setmutex

exit	ldy	status
	lda	space
	sta	end-3
	lda	space+1
	sta	end-2
	pld
	tsc
	adc	#end-4
	tcs

	tya		Return status

	rtl	  

;
; Utility subroutine to print name and value in varname and varval
;  Call with X = export flag, Y = index to text in varbuf.
;
prnameval	anop
	phy		Hold name length offset on stack.
	cpx	#0	If export flag is set,
	bne	needshift	 go upshift the name.
	ldx	exflag	If we're listing all vars, it's OK.
	beq	nameok
	ply		Otherwise, remove length offset
	bra	goback	 and skip the printing.
;
; Variable is exported: need to upshift its name:
;
needshift	short	a	Switch to 1-byte memory access.

upper	lda	[varbuf],y	Get next character.
	beq	golong	If 0, at end.
	cmp	#'a'	If >= 'a'
	bcc	noshift	 and <= 'z',
	cmp	#'z'+1
	bcs	noshift
	and	#$5F		upshift the char.
	sta	[varbuf],y
noshift	iny		Bump the index and
	bra	upper	 stay in upshift loop until done.

golong	long	a	Switch back to 1-word access.
;
; Name is ready for printing
;
nameok	ldx	varbuf+2	
	clc
	pla		Get text offset from stack.
	adc	varbuf	Add starting address,
	bcc	prname
	inx		 adjusting high-order word if needed.
prname	jsr	puts	Print name

	ldx	#^showeq
	lda	#showeq
	jsr	puts	Print " = "

	ldy	#2	Get length word of value.
	lda	[valbuf],y	If zero,
	beq	newln	 skip printing the value.
	tay		Set Y to point to the end of the string.
	iny4
	lda	#0	Store zero word at end so it can
	sta	[valbuf],y	 be treated like a c-string.
	lda	valbuf
	ldx	valbuf+2
	clc
	adc	#4
	bcc	prval
	inx
prval	jsr	puts	Print value (c-string).

newln	jsr	newline	Print blank line.

goback	rts		Return to caller



; Parameter block for shell ReadVariableGS/SetGS calls
ReadSetVar	anop
	dc	i2'3'	pCount
RSname	ds	4	Name (pointer to GS/OS string)
RSvalue	ds	4	Value (ptr to result buf or string)
RSexport	ds	2	Export flag

; Parameter block for shell ReadIndexedGS call (p 421 in ORCA/M manual)
idxParm	anop
	dc	i2'4'	pCount
idxName	ds	4	Name (pointer to GS/OS result buf)
idxValue	ds	4	Value (pointer to GS/OS result buf)
idxIndex	ds	2	Index number
idxExport	ds	2	Export flag


setmutex	key
showeq	dc	c' = ',h'00'
Usage	dc	c'Usage:',h'0d'
	dc	c'  set                 - displays all variables',h'0d'
	dc	c'  set ... [var]       - displays the value of var',h'0d'
	dc	c'  set [var value]...  - sets var to value',h'0d'
	dc	c'  set [var=value]...  - sets var to value',h'0d'
	dc	h'00'
error1	dc	c'set: Variable not specified',h'0d00'
error2	dc	c'set: Variable not defined',h'0d00'

; Parameter block for shell ErrorGS call (p 393 in ORCA/M manual)
Err	dc	i2'1'	pCount
ErrError	ds	2	Error number

	END


**************************************************************************
*
* EXPORT: builtin command
* syntax: export [var] ...
*
* exports each variable given in the list
*
**************************************************************************

export	START

status	equ	0
space	equ	status+2

	 subroutine (4:argv,2:argc),space

	stz	status
	lda	argc	Get parameter count.
	dec	a	If < 1
	bne	loop
	ldx	#^Usage		Print usage string
	lda	#Usage
	jsr	errputs
	inc	status
	bra	done		  and terminate.

;
; Loop to process all the variables to export
;
loop	anop
	dec	argc	Decrement argument counter.
	beq	done	If zero, all done.
	add4	argv,#4,argv	Bump argument address pointer.

	lock	expmutex

	ldy	#2	Convert argv string
	lda	[argv],y	 from c-string
	pha		  to a GS/OS string.
	lda	[argv]
	pha
	jsr	c2gsstr
	sta	ExpName	Store result in
	stx	ExpName+2	 ExportGS parameter block.

	ExportGS ExportPB	Export the named parameter

	ph4	ExpName	Deallocate the GS/OS string.
	jsl	nullfree

	unlock expmutex

	bra	loop

done	return 2:status

expmutex	key

;
; Parameter block for shell ExportGS call (p 398 in ORCA/M manual)
;
ExportPB	anop
	dc	i2'2'	pCount
ExpName	ds	4	Name  (pointer to GS/OS string)
	dc	i2'1'	Export flag (always on)

Usage	dc	c'Usage: export var ...',h'0d00'

	END

**************************************************************************
*
* UNSET: builtin command
* syntax: unset [var] ...
*
* unsets each variable given in the list
*
**************************************************************************

unset	START

status	equ	0
space	equ	status+2

	subroutine (4:argv,2:argc),space

	stz	status

	lda	argc	Get parameter count.
	dec	a	If < 1
	bne	loop
	ldx	#^Usage		Print usage string
	lda	#Usage
	jsr	errputs
	inc	status		  set return status = 1
	bra	done		   and terminate.

;
; Loop to process all the variables to export
;
loop	anop
	dec	argc	Decrement argument counter.
	beq	done	If zero, all done.
	add4	argv,#4,argv	Bump argument address pointer.

	lock	unsmutex

	ldy	#2	Convert argv string
	lda	[argv],y	 from c-string
	pha		  to a GS/OS string.
	lda	[argv]
	pha
	jsr	c2gsstr
	sta	UnsetName	Store result in
	stx	UnsetName+2	 UnsetVariableGS param block.

	UnsetVariableGS UnsetPB	Unset the named parameter.

	ph4	UnsetName	Deallocate the GS/OS string.
	jsl	nullfree

	ldy	#2	Update special shell flags.
	lda	[argv],y
	pha
	lda	[argv]
	pha
	pea	0
	jsl	updatevars

	unlock unsmutex

	bra	loop

done	return 2:status

unsmutex	key

;
; Parameter block for shell UnsetVariableGS call (p 439 in ORCA/M manual)
;
UnsetPB	anop
	dc	i2'2'	pCount
UnsetName	ds	4	Name  (pointer to GS/OS string)

Usage	dc	c'Usage: unset var ...',h'0d00'

	END

;====================================================================
;
; update shell variable flags
;
;====================================================================

updatevars	START

	using	vardata

space	equ	0

	subroutine (4:var,2:flag),space	;flag 1: set, 0: unset

	pei	(var+2)
	pei	(var)
	ph4	#varechoname
	jsr	cmpdcstr
	bne	up2
	lda	flag
	sta	varecho
	jmp	done	

up2	pei	(var+2)
	pei	(var)
	ph4	#direxecname
	jsr	cmpdcstr
	bne	up3
	lda	flag
	sta	vardirexec
	jmp	done	

up3	pei	(var+2)
	pei	(var)
	ph4	#newlinename
	jsr	cmpdcstr
	bne	up4
	lda	flag
	sta	varnewline
	jmp	done	

up4	pei	(var+2)
	pei	(var)
	ph4	#noglobname
	jsr	cmpdcstr
	bne	up5
	lda	flag
	sta	varnoglob
	jmp	done	

up5	pei	(var+2)
	pei	(var)
	ph4	#nobeepname
	jsr	cmpdcstr
	bne	up6
	lda	flag
	sta	varnobeep
	jmp	done	

up6	pei	(var+2)
	pei	(var)
	ph4	#pushdsilname
	jsr	cmpdcstr
	bne	up7
	lda	flag
	sta	varpushdsil
	jmp	done	

up7	pei	(var+2)
	pei	(var)
	ph4	#termname
	jsr	cmpdcstr
	bne	up8
	jsr	readterm
	jmp	done

up8	pei	(var+2)
	pei	(var)
	ph4	#ignorename
	jsr	cmpdcstr
	bne	up9
	lda	flag
	sta	varignore
	jmp	done	

up9	anop	
	pei	(var+2)
	pei	(var)
	ph4	#oldpmodename
	jsr	cmpdcstr
	bne	done
	lda	flag
	sta	varoldpmode
		     
done	return      

	END

;====================================================================
;
; Update all shell variable flags from environment
;
;====================================================================

InitVars	START

	using	vardata

	lock gvmutex	Mutual exclusion lock.

	ldx	#0	Use X-reg to index value table.
loop	phx		Hold onto value table index.
	txa
	asl	a	Double the value array index
	tax		 to get addr table index.
	lda	evstrtbl,x	Store address of variable name
	sta	RVname	 in ReadVariableGS parameter block.
	lda	evstrtbl+2,x
	sta	RVname+2
	stz	RVexpflag	Clear export flag.

	ReadVariableGS ReadVar	Read the value of the named variable.

	plx		Restore value table index.
	lda	RBlen	If variable length != 0
	bne	set	 it is set.
	lda	RVexpflag	It could be exported with len = 0.
set	sta	evvaltbl,x	Save flag in variable.
	inx2		Bump index.
	cpx	evvaltblsz	If not at end,
	bcc	loop	 stay in loop.

	unlock gvmutex	Mutual exclusion unlock.

	rts

gvmutex	key		Key for mutual exclusion.

; Parameter block for shell ReadVariableGS call (p 423 in ORCA/M manual)
ReadVar	anop
	dc	i2'3'	pCount
RVname	ds	4	Pointer to name
RVresult	dc	a4'ResultBuf'	GS/OS Output buffer ptr
RVexpflag	ds	2	export flag

; GS/OS result buffer for testing whether a variable is defined.
; It doesn't have enough room for > 1 byte to be returned, but we
; only need to get the length of the value.
ResultBuf	dc	i2'5'	Only five bytes total.
RBlen	ds	2	Value's length returned here.
	ds	1	Only 1 byte for value.

; GS/OS strings
echostr	gsstr	'echo'
nodirexecstr	gsstr	'nodirexec'
nonewlinestr	gsstr	'nonewline'
noglobstr	gsstr	'noglob'
nobeepstr	gsstr	'nobeep'
pushdsilentstr	gsstr	'pushdsilent'
termstr	gsstr	'term'
ignoreofstr	gsstr	'ignoreeof'
oldpathmodestr	gsstr	'oldpathmode'

; Table of GS/OS string addresses
evstrtbl	anop
	dc	a4'echostr'
	dc	a4'nodirexecstr'
	dc	a4'nonewlinestr'
	dc	a4'noglobstr'
	dc	a4'nobeepstr'
	dc	a4'pushdsilentstr'
	dc	a4'termstr'
	dc	a4'ignoreofstr'
	dc	a4'oldpathmodestr'

	END

;===================================================================
;
; variable settings
;
;===================================================================

vardata	DATA

varechoname	dc	c'echo',h'00'
direxecname	dc	c'nodirexec',h'00'
newlinename	dc	c'nonewline',h'00'
noglobname	dc	c'noglob',h'00'
nobeepname	dc	c'nobeep',h'00'
pushdsilname	dc	c'pushdsilent',h'00'
termname	dc	c'term',h'00'
ignorename	dc	c'ignoreeof',h'00'
oldpmodename	dc	c'oldpathmode',h'00'

; Table of flag values (must be in same order as string addresses)
evvaltbl	anop
varecho	dc	i2'0'
vardirexec	dc	i2'0'
varnewline	dc	i2'0'
varnoglob	dc	i2'0'
varnobeep	dc	i2'0'
varpushdsil	dc	i2'0'
varignore	dc	i2'0'
varoldpmode	dc	i2'0'

evvaltblsz	dc	i2'evvaltblsz-evvaltbl'	# bytes in table

	END
