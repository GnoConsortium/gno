**************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*
* $Id: stdio.asm,v 1.2 1998/04/24 15:38:43 gdr-ftp Exp $
*
**************************************************************************
*
* STDIO.ASM
*   By Tim Meekins
*
* This is the custom stdio for the shell
*
* stdout buffer size: 512 bytes
* stderr buffer size: 256 bytes
* stdin  buffer size: 128 bytes
*
**************************************************************************

	mcopy /obj/gno/bin/gsh/stdio.mac

dummy	start		; ends up in .root
	end

	setcom 60

**************************************************************************
*
* putchar - output a character to standard out
* On entry: A = character
*
**************************************************************************

putchar	START

	using	stdout
	
               tay		;lock destroys Acc
	lock	mutex
               tya
	and	#$FF
	ldx	index
	sta	stream,x	;we're still in long mode, note extra
	inx                      ; length of stream by one :)
	cmp	#13
	beq	_flush
	cpx	#512
	bcc	done
_flush	stx	index
               Write	WriteParm
	ldx	#0
done	stx	index
	unlock mutex
	rts

	END

**************************************************************************
*
* puts - output a c string to standard out
* On entry: A/X = pointer to string
*
**************************************************************************

puts	START

	using	stdout

               tay		;lock destroys Acc
	lock	mutex
               sty	getchar+1
	txa

	short	a
	sta	getchar+3
       	ora   getchar+2
       	ora   getchar+1
       	beq   exit

	ldy	index
               ldx	#0

getchar	lda	>$FFFFFF,x
	beq	done
	sta	stream,y
	iny
	inx
	cmp	#13
	beq	_flush
	cpy	#512
	bcc	getchar

_flush	sty	index
	phx
	long	a
               Write	WriteParm
	Flush	flushparm
	short	a        
	plx
	ldy	#0
               bra	getchar

done	sty	index
exit	long	a
	unlock mutex
	rts

	END                 

**************************************************************************
*
* putp - output a p string to standard out
* On entry: A/X = pointer to string
*
**************************************************************************

putp	START

	using	stdout

               tay		;lock destroys Acc
	lock	mutex
               sty	getchar+1
	sty	cmpchar+1
	txa

	short	a
	sta	getchar+3
	sta	cmpchar+3

	ldy	index
               ldx	#1
	bra	next

getchar	lda	>$FFFFFF,x
	sta	stream,y
	iny
	inx
	cmp	#13
	beq	_flush
next	txa
cmpchar	cmp	>$FFFFFF
	beq	check2
               bcs	done
check2	cpy	#512
	bcc	getchar

_flush	sty	index
	phx
	long	a
               Write	WriteParm
	Flush	flushparm
	short	a        
	plx
	ldy	#0
               bra	next

done	sty	index
	long	a
	unlock mutex
	rts

	END                 

**************************************************************************
*
* flush - flush stdout
*
**************************************************************************

flush	START

	using	stdout
	
               lock	mutex
	lda	index
	beq	skip
               Write	WriteParm
	Flush	flushparm
               stz	index            
skip	unlock mutex

	rts	

	END

**************************************************************************
*
* data for standard out
*
**************************************************************************

stdout	PRIVDATA

mutex	key

WriteParm	dc	i2'4'
	dc	i2'2'	;2 is standard out
	dc	i4'stream'
index	dc	i4'0'
	dc	i4'0'

flushparm	dc	i2'1'
	dc	i2'2'

stream	ds	512+1

	END

**************************************************************************
*
* errputchar - output a character to standard error
* On entry: A = character
*
**************************************************************************

errputchar	START

	using	stderr
	
               tay		;lock destroys Acc
	lock	errmutex
               tya
	and	#$FF
	ldx	errindex
	sta	errstream,x
	inx
	cmp	#13
	beq	_flush
	cpx	#256
	bcc	done
_flush	stx	errindex
               Write	errWriteParm
	ldx	#0
done	stx	errindex
	unlock errmutex
	rts

	END

**************************************************************************
*
* errputs - output a c string to standard error
* On entry: A/X = pointer to string
*
**************************************************************************

errputs	START

	using	stderr

               tay		;lock destroys Acc
	lock	errmutex
               sty	getchar+1
	txa

	short	a
	sta	getchar+3

	ldy	errindex
               ldx	#0

getchar	lda	>$FFFFFF,x
	beq	done
	sta	errstream,y
	iny
	inx
	cmp	#13
	beq	_flush
	cpy	#256
	bcc	getchar

_flush	sty	errindex
	phx
	long	a
               Write	errWriteParm
	short	a
	plx
	ldy	#0
               bra	getchar

done	sty	errindex
	long	a
	unlock errmutex
	rts

	END                 

**************************************************************************
*
* errputp - output a p string to standard error
* On entry: A/X = pointer to string
*
**************************************************************************

errputp	START

	using	stderr

               tay		;lock destroys Acc
	lock	errmutex
               sty	getchar+1
	sty	cmpchar+1
	txa

	short	a
	sta	getchar+3
	sta	cmpchar+3

	ldy	errindex
               ldx	#1

getchar	lda	>$FFFFFF,x
	sta	errstream,y
	iny
	inx
	cmp	#13
	beq	_flush
next	txa
cmpchar	cmp	>$FFFFFF
	beq	check2
               bcs	done
check2	cpy	#256
	bcc	getchar

_flush	sty	errindex
	phx
	long	a
               Write	errWriteParm
	short	a
	plx
	ldy	#0
               bra	next

done	sty	errindex
	long	a
	unlock errmutex
	rts

	END                 

**************************************************************************
*
* errflush - flush stderr
*
**************************************************************************

errflush	START

	using	stderr
	
               lock	errmutex
               Write	errWriteParm
               stz	errindex            
	unlock errmutex

	rts	

	END

**************************************************************************
*
* data for standard error
*
**************************************************************************

stderr	PRIVDATA

errmutex	key

errWriteParm	dc	i2'4'
	dc	i2'3'	;3 is standard err
	dc	i4'errstream'
errindex	dc	i4'0'
	dc	i4'0'

errstream	ds	256+1	;not as large as stdout

	END

**************************************************************************
*
* getchar - read a single character from standard input
* on exit: a = character, -1 if EOF
*
**************************************************************************

getchar	START

	using	stdin

	lock	inmutex

	lda	insize	;any characters in stream?
	bne	grabchar	;yup

readloop	Read	inReadParm
	bcc	okread
	ldy	#-1	;return EOF on ALL errors
               jmp	done2

okread	stz	inindex
	lda	insize
	bne	grabchar

	cop	$7F	;no characters ready, so wait
	bra	readloop

grabchar	ldx	inindex
	lda	instream,x
	and	#$7F
	inc	inindex
	dec	insize

	tay

done2	unlock inmutex
	tya
	rts

	END

**************************************************************************
*
* data for standard input
*
**************************************************************************

stdin	PRIVDATA

inmutex	key

inReadParm	dc	i2'4'
	dc	i2'1'	;1 is standard input
	dc	i4'instream'
inrequest	dc	i4'128'
insize	dc	i4'0'

inindex	dc	i2'0'

instream	ds	128+1

	END
