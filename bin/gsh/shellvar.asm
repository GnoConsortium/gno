**************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*   Derek Taubert
*
**************************************************************************
*
* SHELLVAR.ASM
*   By Tim Meekins
*
* Routines for handling variables in the shell
*
**************************************************************************

               keep  o/shellvar
               mcopy m/shellvar.mac

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
svwhoops	ld2	$201,Err
	Error Err
	jmp	exit

startshow      lda	#254
	sta	[valbuf]
	sta	[varbuf]
	lda   setmutex
	beq	wait0
	cop	$7F
	bra	startshow
wait0	inc	setmutex
	mv4	varbuf,idxParm+2
	mv4	valbuf,idxParm+6
	ld2	1,idxParm+10
showloop	GSOS 	$0148,idxParm	;ReadIndexed 2.0
	ldy	#2
	lda	[varbuf],y
	and	#$FF
	beq	showdone
	xba
	sta	[varbuf],y
	ldy	idxParm+12
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
	inc	idxParm+10
	bra	showloop

showdone	dec	setmutex
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
setvar	lda	setmutex
	beq	wait1
	cop	$7F
	bra	setvar
wait1	inc	setmutex
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

nextvar	dec	setmutex
skipvar	add2	argv,#4,argv
	dec	argc
	jmp	setvar

doneset	dec	setmutex

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

idxParm	dc	i2'4'
	ds	4
	ds	4
	ds	2
	ds	2

setmutex	dc	i'0'
showeq	dc	c' = ',h'00'
Usage	dc	c'Usage:',h'0d'
          dc	 c'  set                 - displays all variables',h'0d'
          dc	 c'  set ... [var]       - displays the value of var',h'0d'
          dc	 c'  set [var value]...  - sets var to value',h'0d'
          dc	 c'  set [var=value]...  - sets var to value',h'0d'
          dc	 h'00'
error1	dc	c'set: Variable not specified',h'0d00'
error2	dc	c'set: Variable not defined',h'0d00'

Err	ds	2

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
svwhoops	ld2	$201,Err
	Error Err
	jmp	exit

startshow      lda   setmutex
	beq	wait0
	cop	$7F
	bra	startshow
wait0	inc	setmutex
	mv4	varbuf,varParm+0
	mv4	valbuf,varParm+4
	ld2	1,varParm+8
	PushVariables 0
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

showdone	PopVariables 0
	dec	setmutex
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
setvar	lda	setmutex
	beq	wait1
	cop	$7F
	bra	setvar
wait1	inc	setmutex
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
	PushVariables 0
	Read_Variable varParm
	PopVariables 0
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

nextvar	dec	setmutex
skipvar	add2	argv,#4,argv
	dec	argc
	jmp	setvar

doneset	dec	setmutex

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
                             
setmutex	dc	i'0'
showeq	dc	c' = ',h'00'
Usage	dc	c'Usage:',h'0d'
          dc	 c'  setenv                 - displays all variables',h'0d'
          dc	 c'  setenv ... [var]       - displays the value of var',h'0d'
          dc	 c'  setenv [var value]...  - sets var to value',h'0d'
          dc	 c'  setenv [var=value]...  - sets var to value',h'0d'
          dc	 h'00'
error1	dc	c'setenv: Variable not specified',h'0d00'
error2	dc	c'setenv: Variable not defined',h'0d00'

Err	ds	2

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

wait	lda	expmutex
	beq	wait0
	cop	$7F
	bra	wait
wait0	inc	expmutex

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

	dec	expmutex

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

expmutex	dc	i'0'

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

wait	lda	unsmutex
	beq	wait0
	cop	$7F
	bra	wait
wait0	inc	unsmutex

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

	dec	unsmutex

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

unsmutex	dc	i'0'

unsetparm      ds    4

Usage	dc	c'Usage: unset var ...',h'0d00'

	END

;====================================================================
;
; update shell variable flags
;
; CHANGE TO JUST DO A READVARIABLES ON THE PASSED VAR
;
;====================================================================

updatevars	START

	using	vardata

space	equ	0

	subroutine (4:var,2:flag),space	;flag 1: set, 0: unset

	pei	(var+2)
	pei	(var)
	ph4	#varechoname
	jsr	cmpcstr
	bne	up2
	lda	flag
	sta	varecho
	jmp	done	

up2	pei	(var+2)
	pei	(var)
	ph4	#direxecname
	jsr	cmpcstr
	bne	up3
	lda	flag
	sta	vardirexec
	jmp	done	

up3	pei	(var+2)
	pei	(var)
	ph4	#newlinename
	jsr	cmpcstr
	bne	up4
	lda	flag
	sta	varnewline
	jmp	done	

up4	pei	(var+2)
	pei	(var)
	ph4	#noglobname
	jsr	cmpcstr
	bne	up5
	lda	flag
	sta	varnoglob
	jmp	done	

up5	pei	(var+2)
	pei	(var)
	ph4	#nobeepname
	jsr	cmpcstr
	bne	up6
	lda	flag
	sta	varnobeep
	jmp	done	

up6	pei	(var+2)
	pei	(var)
	ph4	#pushdsilname
	jsr	cmpcstr
	bne	up7
	lda	flag
	sta	varpushdsil
	jmp	done	

up7	pei	(var+2)
	pei	(var)
	ph4	#termname
	jsr	cmpcstr
	bne	up8
	jsr	readterm
	jmp	done

up8	pei	(var+2)
	pei	(var)
	ph4	#ignorename
	jsr	cmpcstr
	bne	up9
	lda	flag
	sta	varignore
	jmp	done	

up9	anop	
                         
done	return      

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

varecho	dc	i'0'
vardirexec	dc	i'0'
varnewline	dc	i'0'
varnoglob	dc	i'0'
varnobeep	dc	i'0'
varpushdsil	dc	i'0'
varignore	dc	i'0'

	END
