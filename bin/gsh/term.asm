**************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*
* $Id: term.asm,v 1.2 1998/04/24 15:38:47 gdr-ftp Exp $
*
**************************************************************************
*
* TERM.ASM
*   By Tim Meekins
*
* Routines for dealing with Termcap under gsh.
*
**************************************************************************

	mcopy /obj/gno/bin/gsh/term.mac

dummy	start		; ends up in .root
	end

	setcom 60

TIOCGETP	gequ	$40067408

**************************************************************************
*
* Initialize the system for termcap - checks to see if $TERM exists
* and is set, if not, sets to GNOCON
*
**************************************************************************

InitTerm	START

	using	termdata

	Set_Variable setparm
	Export expparm
	ph4	#1024
	jsl	~NEW
               sta	bp
	stx	bp+2
	ph4	#1024
	jsl	~NEW
	sta	areabuf
	stx	areabuf+2
	rts

setparm	dc	i4'termname'	;default term type
	dc	i4'termval'

expparm	dc	i4'termname'
	dc	i2'1'

termname	str	'term'
termval	str	'gnocon'

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
	phx
	pha
	tgetent (bp,@xa)	;xa is pushed first
	beq	noentry
	dec	a
	beq	ok

	jsl	nullfree
	stz	termok
	ldx	#^error1
	lda	#error1
	jmp	errputs

noentry	jsl	nullfree
	stz	termok
	ldx	#^error2
	lda	#error2
	jsr	errputs
	ph4	#termname
	jsl	getenv
	jsr	errputs
	lda	#13
	jmp	errputchar

ok	jsl	nullfree
	lda	#1
	sta	termok
	mv4	areabuf,area          

	tgetstr (#isid,#area)
	jsr	puts
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

termname	dc	c'term',h'00'
error1	dc	c'Error reading termcap file!',h'0d0d00'
error2	dc	c'Termcap entry not found for ',h'00'
isid	dc	c'is',h'00'
leid	dc	c'le',h'00'
ndid	dc	c'nd',h'00'
veid	dc	c've',h'00'
viid	dc	c'vi',h'00'
vsid	dc	c'vs',h'00'
blid	dc	c'bl',h'00'
clid	dc	c'cl',h'00'
soid	dc	c'so',h'00'
seid	dc	c'se',h'00'
klid	dc	c'kl',h'00'
krid	dc	c'kr',h'00'
kuid	dc	c'ku',h'00'
kdid	dc	c'kd',h'00'
cdid	dc	c'cd',h'00'
ueid	dc	c'ue',h'00'
usid	dc	c'us',h'00'

sgtty	anop
	dc	i1'0'
sg_ospeed	dc	i1'0'
	dc	i1'0'
	dc	i1'0'
sg_flags	dc	i2'0'
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
insertflag     dc    i2'1'

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
                             
