* These syslog() related routines use syslogd. They replace the old
* syslog() routines in the lbsd library.
*
* If syslogd is not found, syslog() calls old_syslog with the handle it
* would have passed to syslogd. If you are sure your program will have
* syslogd available when it is run, you can replace old_syslog() with
* a dummy routine that does nothing and returns. This will make your
* program smaller.
*
* syslogd is built into init and is automatically started by it. I
* recommend that you use init with GNO/ME 2.0
*
* Phillip Vandry, August 1993
*
* $Id: syslog2.asm,v 1.3 1999/01/04 05:10:36 gdr-ftp Exp $
*
* This file is formatted for tab stops at positions 10, 16, 41, 40, 57,
* 65, and 73 (standard Orca/M systabs setting).
*
	keep	syslog2
	mcopy	syslog2.mac
	case	on

dummy	start		; ends up in .root
	end

syslog	start	libc_gen__
	tsc
	sec
	sbc	#$8
	tcs
	phd
	tcd
* Direct: 1=va_list, 9=RTL, 12=prio, 14=char *, 18=args

	lda	#0
	sta	3
	sta	7
	tdc
	clc
	adc	#18
	sta	1
	sta	5

	pea	0
	tdc
	inc	a
	pha
	pei	16
	pei	14
	pei	12
	jsl	vsyslog

	lda	1
	clc
	sec
	sbc	5
	tax		; num extra parms

* Direct: 1=va_list, 9=RTL, 12=prio, 14=char *, 18=args
	lda	10
	sta	16,x
	lda	9
	sta	15,x
	pld
	tsc
	clc
	adc	#14
	phx
	adc	1,s
	tcs
	rtl
	end

vsyslog	start	libc_gen__
	using	syslog_dat

space	equ	26
argstart	equ	space+4

	tsc
	sec
	sbc	#space
	tcs
	phd
	tcd
	phb

prio	equ	argstart
format	equ	argstart+2
valist	equ	argstart+6

sendhand	equ	1
sendptr	equ	5
lerrno	equ	9
errlen	equ	9
chand	equ	11
cptr	equ	15
cumlen	equ	19
error	equ	23

	lda	>~USER_ID	; uck! bad name!
	sta	>memid

* Log only if bit clear in LogMask
	lda	>errno		; save value of errno in lerrno for future
	sta	lerrno		; reference
	lda	prio
	and	#7
	tax
	lda	>LogMask
lsrloop	lsr	a
	dex
	bpl	lsrloop
* carry = apropriate bit
	bcc	dolog

* LogMask was such that we're not logging the message; get rid of parameters
* before returning by running through sprintf
	jsr	mksendhand
	pei	valist+2
	pei	valist
	pei	format+2
	pei	format
	pei	sendptr+2
	pei	sendptr
	jsl	vsprintf
	~DisposeHandle <sendhand
	brl	return

* We will be logging.  Use the given prio if nonzero, else use the default
* facility specified in the openlog() call.
dolog	anop
	lda	prio
	and	#$3f8
	bne	gotone
	lda	prio
	ora	>LogFacility
	sta	prio
gotone	anop
	jsr	mksendhand	; allocate mem, init sendhand and sendptr
	jsr	cpsendhand	; init version, prio, numstrings, and offset
				; in mem block.  Init locals cumlen and sendptr

	lda	>LogTag		; if the tag has not been initialized, skip
	sta	cptr		; to the 'notag' label
	lda	>LogTag+2
	sta	cptr+2
	ora	cptr
	beq	notag

	lda	>TagLen		; set y reg to length of LogTag string, store
	bne	already		; the result in TagLen.  If we've already
	ldy	#0		; determined it, use the cached value.
	short	m		; Leave the TagLen value in the accumulator
lppp	lda	[cptr],y
	beq	foundlen
	iny
	bra	lppp
foundlen	long	m
	tya
	sta	>TagLen

already	sta	[cumlen]	; store TagLen into length word of GS string
	tay
	short	m
fincp2	dey			; copy the LogTag into the GS string
	bmi	fincp
	lda	[cptr],y
	sta	[sendptr],y
	bra	fincp2
fincp	long	m		; make sendptr point to end of copied tag
	lda	[cumlen]
	clc
	adc	sendptr
	sta	sendptr
	lda	sendptr+2
	adc	#0
	sta	sendptr+2

notag	anop
	lda	>LogFlag
	lsr	a
	bcc	nopid

	pha
	ldx	#$0903
	jsl	$e10008	; getpid
	pea	fmt|-16		; if the LOG_PID flag is set, append a text
	pea	fmt		; representation of the pid to the GS string.
	pei	sendptr+2	; update the length word and make sendptr
	pei	sendptr		; point to the end of the string.
	jsl	sprintf
	pha
	clc
	adc	[cumlen]
	sta	[cumlen]
	pla
	clc
	adc	sendptr
	sta	sendptr
	lda	sendptr+2
	adc	#0
	sta	sendptr+2

nopid	anop			; if we had a log tag, append a " :" to
	lda	>LogTag		; the GS string.  Update the length word
	ora	>LogTag+2	; and make sendptr point to the end of the
	beq	notagsecond	; string
	lda	[cumlen]
	inc	a
	inc	a
	sta	[cumlen]
	lda	#$203a
	sta	[sendptr]
	lda	sendptr
	clc
	adc	#2
	sta	sendptr
	lda	sendptr+2
	adc	#0
	sta	sendptr+2

notagsecond	anop		; If errno was nonzero, make error point
	lda	lerrno		; to the appropriate error string, otherwise
	bne	isone		; make it a pointer to the empty string ("\0").
	lda	#ptrtozero	; Set errlen to the length of the error string.
	sta	error
	lda	#^ptrtozero
	sta	error+2
	bra	none
isone	pha
	jsl	strerror
	sta	error
	stx	error+2
	phx
	pha
	jsl	strlen
	sta	errlen

				; determine what the size of the format
				; string will be after expanding all "%m"
				; specifiers
none	ldx	#2		; = bytes needed in copy (one null+slop)
	ldy	#0
runthrough lda	[format],y
	cmp	#$6d25	; '%m'
	beq	account
	and	#$ff
	beq	endstring
	iny
	inx
	bra	runthrough
account	iny
	iny
	txa
	clc
	adc	errlen
	tax
	bra	runthrough
endstring	pha
	pha
	pea	0
	phx			; length
	stz	chand
	stz	chand+2
	lda	format
	sta	cptr
	lda	format+2
	sta	cptr+2
	lda	>memid
	pha
	pea	$4000
	pha
	pha
	~NewHandle *,*,*,*	; allocate memory for copied format string,
	pla			; handle is chand, dereferenced handle is cptr
	plx
	bcs	impossible
	sta	chand
	stx	chand+2
	ldy	#2
	lda	[chand],y
	sta	cptr+2
	lda	[chand]
	sta	cptr

				; copy format string to cptr, substituting
				; %m specifiers along the way
	pea	0	; offset in source
	ldy	#0	; offset in dest
realtime	tyx
	ply
	lda	[format],y
	cmp	#$6d25	; '%m'
	beq	coper
	and	#$ff
	beq	excited
	iny
	phy
	txy
	sta	[cptr],y
	iny
	bra	realtime
coper	iny
	iny
	phy
	txy		; Y=offset in destination
	pea	0	; offset in source
anotherreal	tyx
	ply
	lda	[error],y
	and	#$ff
	beq	donerr
	iny
	phy
	txy
	sta	[cptr],y
	iny
	bra	anotherreal
donerr	txy
	bra	realtime
excited	txy
	sta	[cptr],y	; null terminate cptr

impossible	anop		; jump here if mem alloc for copying format
				; string failed, in which case cptr points to
				; the original format string.
				; We also fall through to here if we have
				; successfully copied the format string.
	pei	valist+2
	pei	valist
	pei	cptr+2		; use vsprintf to print format and any args
	pei	cptr		; to the GS string.  Update the length word
	pei	sendptr+2	; of the GS string.
	pei	sendptr
	jsl	vsprintf
	clc
	adc	[cumlen]
	sta	[cumlen]
	lda	>LogFlag
	and	#$20	; PERROR
	beq	noper

	ldx	cumlen		; Write the message to stderr if the necessary
	ldy	cumlen+2	; bit was set.
	lda	#3
	jsl	WriteGString
	ldx	#nlonly
	ldy	#^nlonly
	lda	#3
	jsl	WriteGString
		
noper	lda	chand		; Release the mem from the copied format
	ora	chand+2		; string, if it was successfully allocated
	beq	nochand
	~DisposeHandle <chand

nochand	pei	sendhand+2	; see if syslogd is running
	pei	sendhand
	pea	portname|-16
	pea	portname
	jsl	pgetport
	cmp	#$ffff		; if not, use the old_syslog routine to print
	beq	nosyslogd	; the message
	pha
	jsl	psend		; send the message to syslogd
	ldy	#6
tryagain	lda	[sendhand],y
	cmp	>memid		; busy wait until we no longer own the
	bne	return		; memory block.  Use COP to force a context
	cop	$7f		; switch to minimize wasted clock cycles
	bra	tryagain

return	plb
	lda	argstart-3	; va_end()
	sta	valist+1
	lda	argstart-2
	sta	valist+2
	pld
	tsc
	clc
	adc	#(space+10)
	tcs
	rtl
nosyslogd jsl	old_syslog
	bra	return

* Allocate a memory region of 1024 bytes.  Create it locked.  Store the
* handle into 'sendhand'.  Deref the handle and store the result into 'sendptr'

mksendhand	pha
	pha
	pea	0
	pea	1024
	lda	>memid
	pha
	pea	$4000
	pha
	pha
	~NewHandle *,*,*,*
	plx
	stx	sendhand
	plx
	stx	sendhand+2
	bcs	giveup		; if alloc fails, sleep 1 second and try again
	ldy	#2
	lda	[sendhand],y
	sta	sendptr+2
	lda	[sendhand]
	sta	sendptr
	rts
giveup	pea	1
	jsl	sleep
	bra	mksendhand

* - Copy the five words of Xsyslog into the region pointed to by sendptr.
*   This initializes the version, prio, numstrings, string1 offset, and
*   sizeof(region) to 0, 0, 1, 0, and 10, respectively.
* - Reset the prio field to the appropriate value
* - Make cumlen point to the length word of the GS string
* - Make sendptr point to the start of the text field of the GS string.

cpsendhand ldy	#(Xthis-Xsyslog-2)	; is even
	phb
	phk
	plb
still	lda	Xsyslog,y
	sta	[sendptr],y
	dey
	dey
	bpl	still
	plb
	ldy	#2
	lda	prio
	sta	[sendptr],y
	lda	sendptr
	clc
	adc	#(Xthis-Xsyslog)
	sta	cumlen
	lda	sendptr+2
	adc	#0		; cumlen now holds a pointer to the length
	sta	cumlen+2	; word of the first GS string.
	lda	#0
	sta	[cumlen]	; zero the length word
	lda	cumlen
	clc
	adc	#2
	sta	sendptr		; sendptr now points to the text field of
	lda	cumlen+2	; the GS string.
	adc	#0
	sta	sendptr+2
	rts
	end

setlogmask start libc_gen__
	using	syslog_dat
* Stack: 1:RTL, 4:mask
	lda	4,s
	tax
	lda	>LogMask
	sta	4,s
	txa
	sta	>LogMask
	phb
	plx
	ply
	pla
	phy
	phx
	plb
	rtl
	end

closelog	start	libc_gen__
	using	syslog_dat
	rtl
	end

openlog	start	libc_gen__
	using	syslog_dat
	phb
	plx
	ply
* Stack: 1:char *, 5:int, 7:int
	lda	1,s
	ora	3,s
	beq	notag
	pla
	sta	>LogTag
	pla
	sta	>LogTag+2
	lda	#0
	sta	>TagLen
	dc	h'a9'
notag	pla
	pla
wastag	pla
	sta	>LogFlag
	pla
	sta	>LogFacility
	phy
	phx
	plb
	rtl
	end

syslog_dat privdata libc_gen__
LogTag	ds	4
TagLen	ds	2
LogFlag	ds	2
LogMask	ds	2
LogFacility ds	2
memid	ds	2

Xsyslog	dc	i'0,0,1,0,Xthis-Xsyslog'
Xthis	anop

fmt	dc	c'[%u] ',h'0'
portname	dc	c'syslogd'
ptrtozero dc	h'0'

nlonly	dc	h'01 00 0d'
	end
