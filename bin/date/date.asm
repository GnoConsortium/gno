* asm version of date utility

* Phillip Vandry, May 1993

		mcopy	date.mac
		keep	date
		copy	13/asmdefs/gno/e16.ioctl

cline		gequ	0

main		start
		phk
		plb
		sty	cline
		stx	cline+2
		stz	uflag
		ldy	#8
lpp15		lda	[cline],y
		and	#$ff
		beq	ok
		iny
		cmp	#$20
		bne	lpp15
lpp20		lda	[cline],y
		and	#$ff
		beq	ok
		iny
		cmp	#$20
		beq	lpp20
		cmp	#$22
		beq	lpp20
		cmp	#'-'
		bne	showusg
		lda	[cline],y
		and	#$ff
		cmp	#'V'
		beq	showvers
		cmp	#'c'
		beq	updateflag
showusg		~ErrWriteCString #usage
		bra	getoutPASS
showvers	~WriteCString #versstr
getoutPASS	brl	getout
updateflag	inc	uflag
ok		anop
		lda	#2		; SIGINT
		jsr	handle
		lda	#15		; SIGTERM
		jsr	handle

		pha
		pea	2		; stdout
		pea	TIOCGETP|-16
		pea	TIOCGETP
		pea	sgttyb|-16
		pea	sgttyb
		pea	errno|-16
		pea	errno
		ldx	#$2603
		jsl	$e10008
		pla

		lda	sgttyb+4
		sta	oldflags
		and	#$ffef
		sta	sgttyb+4

		jsr	csetp

		lda	oldflags
		sta	sgttyb+4

refresh		anop
		pha
		pha
		pha
		pha
		~ReadTimeHex
		lda	#year+3
		sta	cline
		lda	4,s
		and	#$ff
		clc
		adc	#1900
		ldx	#4
		jsr	insert
		lda	1,s
		and	#$ff
		ldx	#2
		jsr	insert
		jsr	unpad
		pla
		xba
		and	#$ff
		ldx	#2
		jsr	insert
		jsr	unpad
		pla
		and	#$ff
		ldx	#2
		jsr	insert
		jsr	unpad
		lda	1,s
		and	#$ff
		inc	a
		ldx	#2
		jsr	insert
		phk
		plb
		pla
		jsr	lookup
		lda	months,x
		sta	month
		lda	months+1,x
		sta	month+1
		pla
		jsr	lookup
		lda	weekdays-3,x
		sta	weekday
		lda	weekdays-2,x
		sta	weekday+1
		WriteGS	ramit
		lda	uflag
		beq	exitall
		pea	1
		case	on
		jsl	sleep
		case	off
		brl	refresh
exitall		WriteGS justnewl
		jsr	csetp
getout		QuitGS	qtrec

csetp		anop
		pha
		pea	2
		pea	TIOCSETP|-16
		pea	TIOCSETP
		pea	sgttyb|-16
		pea	sgttyb
		pea	errno|-16
		pea	errno
		ldx	#$2603
		jsl	$e10008
		pla
		rts

uflag		ds	2

handler		anop
		phb
		plx
		ply
		pla
		pla
		phy
		phx
		plb
		lda	#1
		case	on
		sta	>ringring
		case	off
		lda	#0
		sta	>uflag
		rtl

handle		anop
		pha
		pha
		pha
		pea	handler|-16
		pea	handler
		pea	errno|-16
		pea	errno
		ldx	#$1603
		jsl	$e10008
		pla
		pla
		rts

lookup		xba
		and	#$ff
		pha
		asl	a
		asl	a
		sec
		sbc	1,s
		tax
		pla
		rts

unpad		ldx	cline
		phk
		plb
		short	m
		lda	|0,x
		cmp	#$20
		bne	allok
		lda	#'0'
		sta	|0,x
allok		long	m
		rts

insert		pha			; thenum
		pea	0
		plb
		phk
		lda	cline
		dec	a
		dec	a
		dec	a
		sta	cline
		pha
		phx
		pea	0
		~Int2Dec *,*,*,*
		rts

string		anop

qtrec		dc	i'2'
		dc	a4'0'
		dc	i'$4000'

ramit		dc	i'4'
		dc	i'2'
		dc	a4'strbeg'
		dc	i4'strend-strbeg'
		ds	4

sgttyb		ds	6
oldflags	ds	2

strbeg		anop

weekday		dc	c'xxx '
month		dc	c'xxx '
day		dc	c'xx '
hour		dc	c'xx:'
minute		dc	c'xx:'
second		dc	c'xx '
year		dc	c'xxxx'

		dc	h'0d'

strend		anop

justnewl	dc	i'4'
		dc	i'2'
		dc	a4'justnewl1'
		dc	i4'1'
		ds	4
justnewl1	dc	h'0a'

weekdays	dc	c'SunMonTueWedThuFriSat'
months		dc	c'JanFebMarAprMayJunJulAugSepOctNovDec'

versstr		dc	c'date utility, asm version 1.1'
		dc	h'0d 0a'
usage		dc	c'usage: date [-cV]',h'0d 0a'
		dc	h'0'
		end

errno		data
		ds	2
		end

		copy	modsleep.asm

dpstk		data	~Direct
		kind	$12
		ds	512
		end

