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
* $Id: syslog2.asm,v 1.2 1998/06/24 04:19:59 gdr-ftp Exp $
*
* This file is formatted for tab stops at positions 10, 16, 41, 40, 57,
* 65, and 73 (standard Orca/M systabs setting).
*
	keep	syslog
	mcopy	syslog.mac
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
	lda	>errno
	sta	lerrno
	lda	prio
	and	#7
	tax
	lda	>LogMask
lsrloop	lsr	a
	dex
	bpl	lsrloop
* carry = apropriate bit
	bcc	dolog

* get rif of parameters by running through sprintf
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

dolog	anop
	lda	prio
	and	#$3f8
	bne	gotone
	lda	prio
	ora	>LogFacility
	sta	prio
gotone	anop
	jsr	mksendhand
	jsr	cpsendhand

	lda	>LogTag
	sta	cptr
	lda	>LogTag+2
	sta	cptr+2
	ora	cptr
	beq	notag

	lda	>TagLen
	bne	already
	ldy	#0
	short	m
lppp	lda	[cptr],y
	beq	foundlen
	iny
	bra	lppp
foundlen	long	m
	tya
	sta	>TagLen

already	sta	[cumlen]
	tay
	short	m
fincp2	dey
	bmi	fincp
	lda	[cptr],y
	sta	[sendptr],y
	bra	fincp2
fincp	long	m
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
	pea	fmt|-16
	pea	fmt
	pei	sendptr+2
	pei	sendptr
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

nopid	anop
	lda	>LogTag
	ora	>LogTag+2
	beq	notagsecond
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

notagsecond	anop
	lda	lerrno
	bne	isone
	lda	#ptrtozero
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

none	ldx	#2	; = bytes needed in copy (one null+slop)
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
	phx		; length
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
	~NewHandle *,*,*,*
	pla
	plx
	bcs	impossible
	sta	chand
	stx	chand+2
	ldy	#2
	lda	[chand],y
	sta	cptr+2
	lda	[chand]
	sta	cptr

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
	sta	[cptr],y	; zero

impossible	anop	; jump here if malloc() failed
	pei	valist+2
	pei	valist
	pei	cptr+2
	pei	cptr
	pei	sendptr+2
	pei	sendptr
	jsl	vsprintf
	clc
	adc	[cumlen]
	sta	[cumlen]
	lda	>LogFlag
	and	#$20	; PERROR
	beq	noper

	ldx	cumlen
	ldy	cumlen+2
	lda	#3
	jsl	WriteGString	; echo on standard error
	ldx	#nlonly
	ldy	#^nlonly
	lda	#3
	jsl	WriteGString
		
noper	lda	chand
	ora	chand+2
	beq	nochand
	~DisposeHandle <chand

nochand	pei	sendhand+2
	pei	sendhand
	pea	portname|-16
	pea	portname
	jsl	pgetport
	cmp	#$ffff
	beq	nosyslogd
	pha
	jsl	psend
	ldy	#6
tryagain	lda	[sendhand],y
	cmp	>memid
	bne	return
	cop	$7f
	bra	tryagain

return	plb
	lda	argstart-3
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
	bcs	giveup
	ldy	#2
	lda	[sendhand],y
	sta	sendptr+2
	lda	[sendhand]
	sta	sendptr
	rts
giveup	pea	1
	jsl	sleep
	bra	mksendhand

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
	adc	#0
	sta	cumlen+2
	lda	#0
	sta	[cumlen]
	lda	cumlen
	clc
	adc	#2
	sta	sendptr
	lda	cumlen+2
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
