**************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*
* $Id: stdio.asm,v 1.5 1998/09/08 16:53:14 tribby Exp $
*
**************************************************************************
*
* STDIO.ASM
*   By Tim Meekins
*   Modified by Dave Tribby for GNO 2.0.6
*
* This is the custom stdio for the shell
*
* stdout buffer size: 512 bytes
* stderr buffer size: 256 bytes
* stdin  buffer size: 128 bytes
*
* Note: text set up for tabs at col 16, 22, 41, 49, 57, 65
*              |     |                  |       |       |       |
*	^	^	^	^	^	^	
**************************************************************************

	mcopy /obj/gno/bin/gsh/stdio.mac

dummystdio	start		; ends up in .root
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
	
	tay		Note: lock destroys Acc
	lock	mutex	Wait for others to leave, and lock.
	tya
	and	#$FF	Isolate the single character.
	ldx	index	Get num of chars already in stream.
	sta	stream,x	Store this char + null byte.
	inx		Bump length of stream by one.
	cmp	#13	If character was newline,
	beq	_flush	 go write the stream.
	cpx	#512	If length < 512,
	bcc	done	 all done.
_flush	stx	index	Save current length.
	Write	WriteParm	Write the stream.
	ldx	#0	Set new length to 0.

done	stx	index	Save stream length in global.
	unlock mutex	Allow others through.
	rts		Return to caller.

	END

**************************************************************************
*
* puts - output a c string to standard out
* On entry: A/X = pointer to string
*
**************************************************************************

puts	START

	using	stdout

	tay		Note: lock destroys Acc
	lock	mutex	Wait for others to leave, and lock.
	sty	getchar+1	Save low-order bytes of address.
	txa

	short	a	SWITCH TO SINGLE-BYTE MEMORY MODE.
	sta	getchar+3	Store high-order byte of address.
	ora	getchar+2	If string address
	ora	getchar+1	 is 00/0000,
	beq	exit	  don't do the write.

	ldy	index	Get current number of chars in stream.
	ldx	#0	Clear source string offset.

getchar	lda	>$FFFFFF,x	Get next character from string.
	beq	done	Done when we see a null byte.
	sta	stream,y	Store in output stream.
	iny		Bump the stream and
	inx		 string pointers.
	cmp	#13	If newline was encountered,
	beq	_flush	 go write & flush the stream.
	cpy	#512	If stream length < 512,
	bcc	getchar	 continue copying characters.

_flush	sty	index	Save length of stream.
	phx		Hold source string offset on stack.
	long	a	SWITCH TO FULL-WORD MEMORY MODE.
	Write	WriteParm	Write the stream to stdout
	Flush	flushparm	 and flush it.
	short	a	SWITCH TO SINGLE-BYTE MEMORY MODE.
	plx		Restore source string offset to X-reg.
	ldy	#0	Set stream length to 0.
	bra	getchar	Continue copying characters.

; Arrive here when null character is encountered.
done	sty	index	Save stream length in global.

exit	long	a	SWITCH TO FULL-WORD MEMORY MODE.
	unlock mutex	Allow others through.
	rts		Return to caller.

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
