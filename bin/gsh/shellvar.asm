**************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*   Derek Taubert
*
* $Id: shellvar.asm,v 1.3 1998/06/30 17:25:57 tribby Exp $
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

	mcopy /obj/gno/bin/gsh/shellvar.mac

dummyshellvar	start		; ends up in .root
	end

	setcom 60

**************************************************************************
*
* SET: builtin command
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
space	equ	varbuf+4
argc	equ	space+3
argv	equ	argc+2
end	equ	argv+4

;	 subroutine (4:argv,2:argc),space

	tsc
	sec
	sbc	#space-1
	tcs
	phd
	tcd

	lda	argc
	dec	a
	beq	showvars
;
; If one parameter check for a '-' starting the parm
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
	jmp	exit
;
; show variables
;
showvars	anop

	jsl	alloc256
	sta	varbuf
	stx	varbuf+2
	ora	varbuf+2
	beq	svwhoops
	jsl	alloc256
	sta	valbuf
	stx	valbuf+2
	ora	valbuf+2
	bne	startshow
	ldx	varbuf+2
	lda	varbuf
	jsl	free256
svwhoops	ld2	$201,ErrError
	ErrorGS Err
	jmp	exit

startshow      lda	#254
	sta	[valbuf]
	lock	setmutex
	mv4	varbuf,idxName
	mv4	valbuf,idxValue
	ld2	1,idxIndex
showloop	ReadIndexedGS idxParm
	ldy	#2
	lda	[varbuf],y
	and	#$FF
	beq	showdone
	xba
	sta	[varbuf],y
	ldy	idxExport
	beq	noexp
	xba
               tax
	ldy	#4
	short	a
upper	lda	[varbuf],y
	cmp	#'a'
	bcc	upperfoo
	cmp	#'z'+1
	bcs	upperfoo
	sec
	sbc	#'a'-'A'
	sta	[varbuf],y	
upperfoo	iny
	dex
	bne	upper
	long	a
noexp	ldx	varbuf+2
	lda	varbuf
	clc	
	adc	#3
	jsr	putp
	ldx	#^showeq
	lda	#showeq
	jsr	puts
	ldy	#2
	lda	[valbuf],y
	xba
	sta	[valbuf],y
	lda	valbuf
	ldx	valbuf+2
	clc
	adc	#3
	jsr	putp
	jsr	newline
	inc	idxIndex
	bra	showloop

showdone	unlock setmutex
	ldx	varbuf+2
	lda	varbuf
	jsl	free256
	ldx	valbuf+2
	lda	valbuf
	jsl	free256
	jmp	exit
;
; set variables
;
setvar	lock	setmutex
	lda	argc
	jeq	doneset
	ldy	#2
	lda	[argv]
	sta	arg
	lda	[argv],y
	sta	arg+2
	ldy	#0
chkeql	lda	[arg],y
	and	#$FF
	beq	orcastyle
	cmp	#'='
	jeq	unixstyle
	iny
	bra	chkeql
;
; Orca style set. Uses two arguments.
;
orcastyle	add2	argv,#4,argv
	dec	argc
	beq	showonevar
	pei	(arg+2)
	pei	(arg)
	pea	1
	pei	(arg+2)
	pei	(arg)
	jsr	c2pstr2
	phx
	pha
	sta	varParm
	stx	varParm+2
	ldy	#2
	lda	[argv],y
	pha
	lda	[argv]
	pha
	jsr	c2pstr2
	phx
	pha
	sta	varParm+4
	stx	varParm+4+2
	Set_Variable varParm
	jsl	nullfree
	jsl	nullfree
	jsl	updatevars
	jmp	nextvar

showonevar     jsl	alloc256
	sta	valbuf
	sta	varParm+4
	stx	valbuf+2
	stx	varParm+4+2
	ora	varParm+4+2
	jeq	nextvar
	pei	(arg+2)
	pei	(arg)
	jsr	c2pstr2
	phx
	pha
	sta	varParm
	stx	varParm+2
	Read_Variable varParm
	lda	[valbuf]
	and	#$FF
	beq	notdef	
	lda	varParm
	ldx	varParm+2
	jsr	putp
	lda	#showeq
	ldx	#^showeq
	jsr	puts
	lda	varParm+4
	ldx	varParm+6
	jsr	putp
	jsr	newline
doneone	jsl	nullfree
	lda	valbuf
	ldx	valbuf+2
	jsl	free256
	bra	doneset

notdef	ldx	#^error2
	lda	#error2
	jsr	errputs
	bra	doneone

unixstyle      cpy   #0
	bne	unix0
	ldx	#^error1
	lda	#error1
	jsr	errputs
	bra	doneset
unix0	short	a
	lda	#0
	sta	[arg],y
	long	a
	tya
	sec
	adc	arg
	pei	(arg+2)
	pha
	jsr	c2pstr2
	phx
	pha
	sta	varParm+4
	stx	varParm+4+2
	pei	(arg+2)
	pei	(arg)
	pea	1
	pei	(arg+2)
	pei	(arg)
	jsr	c2pstr2
	phx
	pha
	sta	varParm
	stx	varParm+2		
	Set_Variable varParm
	jsl	nullfree
	jsl	updatevars
	jsl	nullfree

nextvar	unlock setmutex
skipvar	add2	argv,#4,argv
	dec	argc
	jmp	setvar

doneset	unlock setmutex

exit	lda	space
	sta	end-3
	lda	space+1
	sta	end-2
	pld
	tsc
	adc	#end-4
	tcs

	lda	#0

	rtl     

varParm	ds	4
	ds	4
	ds	2

; Parameter block for shell Read_Indexed call (p 421 in ORCA/M manual)
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
* SETENV: builtin command
* syntax: setenv	                - displays all variables
*         setenv ... [var]           - displays the value of var
*         setenv [var=value]...      - sets var to value
*         setenv [var value]...      - sets var to value
*
**************************************************************************

setenv	START

arg	equ	1
valbuf	equ	arg+4
varbuf	equ	valbuf+4
space	equ	varbuf+4
argc	equ	space+3
argv	equ	argc+2
end	equ	argv+4

;	 subroutine (4:argv,2:argc),space

	tsc
	sec
	sbc	#space-1
	tcs
	phd
	tcd

	lda	argc
	dec	a
	beq	showvars
;
; If one parameter check for a '-' starting the parm
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
	jmp	exit
;
; show variables
;
showvars	anop

	jsl	alloc256
	sta	varbuf
	stx	varbuf+2
	ora	varbuf+2
	beq	svwhoops
	jsl	alloc256
	sta	valbuf
	stx	valbuf+2
	ora	valbuf+2
	bne	startshow
	ldx	varbuf+2
	lda	varbuf
	jsl	free256
svwhoops	ld2	$201,ErrError
	ErrorGS Err
	jmp	exit

startshow	lock setmutex
	mv4	varbuf,varParm+0
	mv4	valbuf,varParm+4
	ld2	1,varParm+8
	PushVariablesGS NullPB
showloop	Read_Indexed varParm
	lda	[varbuf]
	and	#$FF
	beq	showdone
               tax
	ldy	#1
	short	a
upper	lda	[varbuf],y
	cmp	#'a'
	bcc	upperfoo
	cmp	#'z'+1
	bcs	upperfoo
	sec
	sbc	#'a'-'A'
	sta	[varbuf],y	
upperfoo	iny
	dex
	bne	upper
	long	a
	lda	varParm
	ldx	varParm+2
	jsr	putp
	ldx	#^showeq
	lda	#showeq
	jsr	puts
	lda	varParm+4
	ldx	varParm+6
	jsr	putp
	jsr	newline
	inc	varParm+8
	bra	showloop

showdone	PopVariablesGS NullPB
	unlock setmutex
	ldx	varbuf+2
	lda	varbuf
	jsl	free256
	ldx	valbuf+2
	lda	valbuf
	jsl	free256
	jmp	exit
;
; set variables
;
setvar	lock	setmutex
	lda	argc
	jeq	doneset
	ldy	#2
	lda	[argv]
	sta	arg
	lda	[argv],y
	sta	arg+2
	ldy	#0
chkeql	lda	[arg],y
	and	#$FF
	beq	orcastyle
	cmp	#'='
	jeq	unixstyle
	iny
	bra	chkeql
;
; Orca style set. Uses two arguments.
;
orcastyle	add2	argv,#4,argv
	dec	argc
	beq	showonevar
	pei	(arg+2)
	pei	(arg)
	pea	1
	pei	(arg+2)
	pei	(arg)
	jsr	c2pstr2
	phx
	pha
	sta	varParm
	stx	varParm+2
	sta	exportparm
	stx	exportparm+2
	ldy	#2
	lda	[argv],y
	pha
	lda	[argv]
	pha
	jsr	c2pstr2
	phx
	pha
	sta	varParm+4
	stx	varParm+4+2
	Set_Variable varParm
	Export exportparm
	jsl	nullfree
	jsl	nullfree
	jsl	updatevars
	jmp	nextvar

showonevar     jsl	alloc256
	sta	valbuf
	sta	varParm+4
	stx	valbuf+2
	stx	varParm+4+2
	ora	varParm+4+2
	jeq	nextvar
	pei	(arg+2)
	pei	(arg)
	jsr	c2pstr2
	phx
	pha
	sta	varParm
	stx	varParm+2
	PushVariablesGS NullPB
	Read_Variable varParm
	PopVariablesGS NullPB
	lda	[valbuf]
	and	#$FF
	beq	notthere
	lda	varParm
	ldx	varParm+2
	jsr	putp
	lda	#showeq
	ldx	#^showeq
	jsr	puts
	lda	varParm+4
	ldx	varParm+6
	jsr	putp
	jsr	newline
doneone	jsl	nullfree
	ldx	valbuf+2
	lda	valbuf
	jsl	free256
	jmp	doneset

notthere	ldx	#^error2
	lda	#error2
	jsr	errputs
	bra	doneone

unixstyle      cpy   #0
	bne	unix0
	ldx	#^error1
	lda	#error1
	jsr	errputs
	bra	doneset
unix0	short	a
	lda	#0
	sta	[arg],y
	long	a
	clc		;use sec and kill the iny :)
	iny
	tya
	adc	arg
	pei	(arg+2)
	pha
	jsr	c2pstr2
	phx
	pha
	sta	varParm+4
	stx	varParm+4+2
	pei	(arg+2)   
	pei	(arg)
	pea	1
	pei	(arg+2)   
	pei	(arg)
	jsr	c2pstr2
	phx
	pha
	sta	varParm
	stx	varParm+2
	sta	exportparm
	stx	exportparm+2		
	Set_Variable varParm
	Export exportparm
	jsl	nullfree
	jsl	updatevars
	jsl	nullfree

nextvar	unlock setmutex
skipvar	add2	argv,#4,argv
	dec	argc
	jmp	setvar

doneset	unlock setmutex

exit	lda	space
	sta	end-3
	lda	space+1
	sta	end-2
	pld
	tsc
	adc	#end-4
	tcs

	lda	#0

	rtl     

varParm	ds	4
	ds	4
	ds	2

exportparm     ds    4
	dc	i'1'
                             
setmutex	key
showeq	dc	c' = ',h'00'
Usage	dc	c'Usage:',h'0d'
	dc	 c'  setenv                 - displays all variables',h'0d'
	dc	 c'  setenv ... [var]       - displays the value of var',h'0d'
	dc	 c'  setenv [var value]...  - sets var to value',h'0d'
	dc	 c'  setenv [var=value]...  - sets var to value',h'0d'
	dc	 h'00'
error1	dc	c'setenv: Variable not specified',h'0d00'
error2	dc	c'setenv: Variable not defined',h'0d00'

; Parameter block for shell ErrorGS call (p 393 in ORCA/M manual)
Err	dc	i2'1'	pCount
ErrError	ds	2	Error number

; Null parameter block used for shell calls PushVariables
; (ORCA/M manual p.420) and PopVariablesGS (p. 419)
NullPB	dc	i2'0'	pCount

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

space	equ	1
argc	equ	space+3
argv	equ	argc+2
end	equ	argv+4

;	 subroutine (4:argv,2:argc),space

	tsc
	phd
	tcd

	lda	argc
	dec	a
	bne	loop
	ldx	#^Usage
	lda	#Usage
	jsr	errputs
	bra	done

loop	add2	argv,#4,argv
	dec	argc
	beq	done

wait	lock	expmutex

	ldy	#2
	lda	[argv],y
	pha
	lda	[argv]
	pha
	jsr	c2pstr2
	phx
	pha
	sta	exportparm
	stx	exportparm+2
	Export exportparm
	jsl	nullfree

	unlock expmutex

	bra	loop

done	lda	space
	sta	end-3
	lda	space+1
	sta	end-2
	pld
	tsc
	clc
	adc	#end-4
	tcs

	lda	#0

	rtl     

expmutex	key

exportparm     ds    4
	dc	i'1'

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

space	equ	1
argc	equ	space+3
argv	equ	argc+2
end	equ	argv+4

;	 subroutine (4:argv,2:argc),space

	tsc
	phd
	tcd

	lda	argc
	dec	a
	bne	loop
	
	ldx	#^Usage
	lda	#Usage
	jsr	errputs
	bra	done

loop	add2	argv,#4,argv
	dec	argc
	beq	done

	lock	unsmutex

	ldy	#2
	lda	[argv],y
	tax
	lda	[argv]
	phx
	pha
	pea	0
	phx
	pha
	jsr	c2pstr2
	phx
	pha
	sta	unsetparm
	stx	unsetparm+2
	UnsetVariable unsetparm
	jsl	nullfree
	jsl	updatevars

	unlock unsmutex

	bra	loop

done	lda	space
	sta	end-3
	lda	space+1
	sta	end-2
	pld
	tsc
	clc
	adc	#end-4
	tcs

	lda	#0

	rtl     

unsmutex	key

unsetparm      ds    4

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

; Table of flag values (must be in same order as string addresses)
evvaltbl	anop
varecho	dc	i2'0'
vardirexec	dc	i2'0'
varnewline	dc	i2'0'
varnoglob	dc	i2'0'
varnobeep	dc	i2'0'
varpushdsil	dc	i2'0'
varignore	dc	i2'0'

evvaltblsz	dc	i2'evvaltblsz-evvaltbl'	# bytes in table

	END
