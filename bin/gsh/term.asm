**************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*
* $Id: term.asm,v 1.7 1998/10/26 17:04:51 tribby Exp $
*
**************************************************************************
*
* TERM.ASM
*   By Tim Meekins
*   Modified by Dave Tribby for GNO 2.0.6
*
* Routines for dealing with Termcap under gsh.
*
* Note: text set up for tabs at col 16, 22, 41, 49, 57, 65
*              |     |                  |       |       |       |
*	^	^	^	^	^	^	
**************************************************************************

	mcopy /obj/gno/bin/gsh/term.mac

dummyterm	start		; ends up in .root
	end

	setcom 60

TIOCGETP	gequ	$40067408

**************************************************************************
*
* Initialize the system for termcap - checks to see if $TERM exists
* and is set, if not, sets to GNOCON; and allocate termcap buffers.
*
**************************************************************************

InitTerm	START
	
	using	termdata

;
; See if $TERM exists
;
	ReadVariableGS ReadVarPB

	lda	term_len	Get length of $TERM
	bne	allocate	If 0,
	SetGS	SetPB		set to default ("gnocon")

allocate	anop		Allocate termcap buffers.

	ph4	#1024
	~NEW
	sta	bp
	stx	bp+2

	ph4	#1024
	~NEW
	sta	areabuf
	stx	areabuf+2

	rts		Return to caller.

;
; Parameter block for shell ReadVariableGS call (p 423 in ORCA/M manual)
;
ReadVarPB	anop
	dc	i2'3'	pCount
	dc	i4'term'	Name  (pointer to GS/OS string)
	dc	i4'dummyresult'	GS/OS Output buffer ptr
	ds	2	export flag
;
; GS/OS result buffer for getting length of TERM env var.
;
dummyresult	dc	i2'5'	Only five bytes total.
term_len	ds	2	Value's length returned here.
	ds	1	Only 1 byte for value.

;
; Parameter block for shell SetGS calls (p 427 in ORCA/M manual)
;
SetPB	anop
	dc	i2'3'	pCount
	dc	i4'term'	Name  (pointer to GS/OS string)
	dc	i4'gnocon'	Value (pointer to GS/OS string)
	dc	i2'1'	Export flag

term	gsstr	'term'
gnocon	gsstr	'gnocon'

	END

**************************************************************************
*
* read new temcap information
*
**************************************************************************

readterm	START

	using	termdata

backward_char	equ	3
forward_char	equ	4
up_history	equ	5
down_history	equ	6

	lda	#1
	sta	didReadTerm

	ph4	#termname
	jsl	getenv
	phx		Push allocated buffer on stack
	pha		 for later call to nullfree.
	clc		Add 4 to GS/OS result buffer
	adc	#4	 to get pointer to text.
	bcc	val_addr_set
	inx
val_addr_set	sta	hold_term_val
	stx	hold_term_val+2
	tgetent (bp,@xa)	;xa is pushed first
	beq	noentry
	dec	a
	beq	ok

	jsl	nullfree	Free buffer allocated by getenv.
	stz	termok
	ldx	#^error1
	lda	#error1
	jmp	errputs

noentry	anop
	stz	termok
	ldx	#^error2
	lda	#error2	Print error message:
	jsr	errputs	  'Termcap entry not found for '
	lda	hold_term_val	Get text from buffer allocated
	ldx	hold_term_val+2	 by getenv
	jsr	errputs	  and print it.
	jsl	nullfree	Free buffer allocated by getenv.
	lda	#13
	jmp	errputchar

ok	jsl	nullfree	Free buffer allocated by getenv.
	lda	#1
	sta	termok
	mv4	areabuf,area	   

;
; Get addresses of termcap strings
;
	tgetstr (#isid,#area)
	jsr	puts	Send initialization string to term.
	tgetstr (#leid,#area)
	sta	lecap
	stx	lecap+2
	tgetstr (#ndid,#area)
	sta	ndcap
	stx	ndcap+2
	tgetstr (#veid,#area)
	sta	vecap
	stx	vecap+2
	tgetstr (#viid,#area)
	sta	vicap
	stx	vicap+2
	tgetstr (#vsid,#area)
	sta	vscap
	stx	vscap+2
	tgetstr (#blid,#area)
	sta	blcap
	stx	blcap+2
	tgetstr (#clid,#area)
	sta	clcap
	stx	clcap+2
	tgetstr (#soid,#area)
	sta	socap
	stx	socap+2
	tgetstr (#seid,#area)
	sta	secap
	stx	secap+2
	tgetstr (#cdid,#area)
	sta	cdcap
	stx	cdcap+2
	tgetstr (#ueid,#area)
	sta	uecap
	stx	uecap+2
	tgetstr (#usid,#area)
	sta	uscap
	stx	uscap+2

;
; Bind keyboard characters
;           
	tgetstr (#klid,#area)
	phx
	pha
	ph2	#backward_char
	jsl	bindkeyfunc
	tgetstr (#krid,#area)
	phx
	pha
	ph2	#forward_char
	jsl	bindkeyfunc
	tgetstr (#kuid,#area)
	phx
	pha
	ph2	#up_history
	jsl	bindkeyfunc
	tgetstr (#kdid,#area)
	phx
	pha
	ph2	#down_history
	jsl	bindkeyfunc

; the following is VERY important. It doesn't seem so, but I actually tested
; a terminal that dropped characters w/o it.
            
	ioctl	(#1,#TIOCGETP,#sgtty)
	lda	sg_ospeed	;let termcap know our speed
	case	on
	sta	>ospeed
	case	off

	rts 

termname	gsstr	'term'
error1	dc	c'Error reading termcap file!',h'0d0d00'
error2	dc	c'Termcap entry not found for ',h'00'

;
; Termcap identification strings
;
isid	dc	c'is',h'00'	Initialization
leid	dc	c'le',h'00'	Out of keypad transmit mode
ndid	dc	c'nd',h'00'	Non-destructive space
veid	dc	c've',h'00'	Normal cursor visible
viid	dc	c'vi',h'00'	Cursor unvisible
vsid	dc	c'vs',h'00'	Standout cursor
blid	dc	c'bl',h'00'	Bell
clid	dc	c'cl',h'00'	Clear screen and home cursor
soid	dc	c'so',h'00'	Begin standout mode
seid	dc	c'se',h'00'	End standout mode
cdid	dc	c'cd',h'00'	Clear to end of display
ueid	dc	c'ue',h'00'	End underscore mode
usid	dc	c'us',h'00'	Begin underscore mode
klid	dc	c'kl',h'00'	Left arrow key
krid	dc	c'kr',h'00'	Right arrow key
kuid	dc	c'ku',h'00'	Up key
kdid	dc	c'kd',h'00'	Down key

sgtty	anop
	dc	i1'0'
sg_ospeed	dc	i1'0'
	dc	i1'0'
	dc	i1'0'
sg_flags	dc	i2'0'

; Hold the address of the value of $TERM
hold_term_val	ds	4

	END
		        
**************************************************************************
*
* outc for outputting characters by termcap
*
**************************************************************************

outc	START

space	equ	0

	subroutine (2:char),space

	lda	char
	jsr	putchar

	return

	END

**************************************************************************
*
* move left x characters
*
**************************************************************************

moveleft	START

	using	termdata

	lda	termok
	beq	done

loop	dex
	bmi	done
	phx
	tputs (lecap,#0,#outc)
	plx
	bra	loop

done	rts

	END

**************************************************************************
*
* move right x characters
*
**************************************************************************

moveright	START

	using	termdata

	lda	termok
	beq	done

loop	dex
	bmi	done
	phx
	tputs (ndcap,#0,#outc)
	plx
	bra	loop

done	rts

	END

**************************************************************************
*
* cursor off
*
**************************************************************************

cursoroff	START

	using	termdata

	lda	termok
	beq	done
	lda	vicap
	ldx	vicap+2
	jmp	puts
done	rts

	END

**************************************************************************
*
* cursor on
*
**************************************************************************

cursoron	START

	using	termdata

	lda	termok
	beq	done
	lda	insertflag
	beq	dovs

	lda	vecap
	ldx	vecap+2
	jmp	puts

dovs	lda	vscap
	ldx	vscap+2
	jmp	puts
done	rts

	END

**************************************************************************
*
* Beep the bell if it's ok.
*
**************************************************************************

beep	START

	using	vardata
	using	termdata

	lda	varnobeep
	bne	beepdone
	lda	termok
	beq	beepdone
	tputs (blcap,#0,#outc)
beepdone	rts

	END

**************************************************************************
*
* clear the screen
*
**************************************************************************

clearscrn	START

	using	termdata

	lda	termok
	beq	done
	tputs (clcap,#0,#outc)
done	rts

	END

**************************************************************************
*
* begin standout mode
*
**************************************************************************

standout	START

	using	termdata
	
	lda	termok
	beq	done

	tputs (socap,#0,#outc)

done	rts

	END

**************************************************************************
*
* end standout mode
*
**************************************************************************

standend	START

	using	termdata
	
	lda	termok
	beq	done

	tputs (secap,#0,#outc)

done	rts

	END

**************************************************************************
*
* begin underline mode
*
**************************************************************************

underline	START

	using	termdata
	
	lda	termok
	beq	done

	tputs (uscap,#0,#outc)

done	rts

	END

**************************************************************************
*
* end underline mode
*
**************************************************************************

underend	START

	using	termdata
	
	lda	termok
	beq	done

	tputs (uecap,#0,#outc)

done	rts

	END

**************************************************************************
*
* TSET: builtin command
* syntax: tset
*
* reset the termcap for gsh
*
**************************************************************************

tset	START

	using	global

space	equ	0

	subroutine (4:argv,2:argc),space

	jsr	readterm

	return

	END

**************************************************************************
*
* termcap data
*
**************************************************************************

termdata	DATA

didReadTerm	dc	i2'0'

termok	dc	i2'0'
insertflag	dc	i2'1'

bp	ds	4
areabuf	ds	4
area	ds	4

blcap	ds	4
cdcap	ds	4
clcap	ds	4
lecap	ds	4
ndcap	ds	4
secap	ds	4
socap	ds	4
uecap	ds	4
uscap	ds	4
vecap	ds	4       
vicap	ds	4
vscap	ds	4

	END
		        
