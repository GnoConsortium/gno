* Serial Interrupt Manager Permanent INIT file

	case	on
	mcopy	sim.mac

************************************************************************

* Global Firmware Equates

v_Main	gequ	$E10010
v_AppleTalk	gequ	$E10020
v_Serial	gequ	$E10024

* Address of SerFlag is same for all three ROM revisions of the IIGS
SerFlag	gequ	$E10104

* This value determines whether SIM is installed as a user tool or
* as a system tool.
sysFlag	gequ	0	make '$8000' to install as user tool

* This value determines not only which tool set number SIM should be
* installed as, but also the error codes that are returned

toolNum        gequ  $4000              change to $0400 for user tool

* Current version of SIM in tool version format
Version	gequ	$0100

	copy	simequates.equ

* Request proc dataOut map
Gcount	gequ	0
Gerror	gequ	2
Gversion	gequ	4

* Input buffer data structure for Install and Remove calls
SIMport	gequ	0
SIMaddress	gequ	2


************************************************************************

* Install our RequestProc, and install the new interrupt handlers, etc.

SIMINIT	START
	using	SIMData


	phb
	phk
	plb

	lda	#0
	sta   IntVectPrinter	indicate that no vectors are
	sta	IntVectPrinter+2	installed
	sta	IntVectModem
	sta	IntVectModem+2
               sta	ExtVect1
               sta	ExtVect1+2
               sta	ExtVect2
               sta	ExtVect2+2
	sta	ourSerFlag	no SCC interrupts are handled

* Turn off interrupts while we do the dirty deed

               php
	sei
* set the IIGS main interrupt vector to point to our version of this
* routine.  This code only works on a ROM 01 at this point due to the
* different code locations.
* We check the ROM version here and install the appropriate routine

	jsr	GetROMVersion
               cmp	#0
	beq	doRom0
	cmp	#1
	beq	doRom0

	lda	jmpit3
	sta	>v_Main
	lda	jmpit3+2
	sta	>v_Main+2
	bra	donePatch

doRom0	lda	jmpit1
	sta	>v_Main
	lda	jmpit1+2
	sta	>v_Main+2

donePatch	plp

	ph4	#SIMName
	pha
	_MMStartUp
	pla
	sta	SIMuserID
	pha
	ph4	#SIMRequest
	_AcceptRequests

	jsl	CheckPort

	plb
	rtl
jmpit1	jmp	>ROM01IntVect
jmpit3	jmp	>ROM03IntVect
	END

ROM03IntVect	START
	using	SIMData

	clc                    (E1/0010 JMPs here, FF/BC6C)
	xce
	long	mx
	php
	phb
;         Assume B=00E1
	pea   $E1E1
	plb
	sta   |$0108	; ASave
	lda   |$C035	; shadow register
 	sta   |$0119	; ShadSave
	ora   #$8000
	and   #$9F3E
	sta   |$C035	; shadow register
	stx   |$010A	; XSave
	sty   |$010C	; YSave
	tdc
	sta   |$0110	; DPSave
	lda   #$0000
	tcd
	short	mx
	bcc   NotSure                  ; check emulation mode
	lda   $04,s	; and check for brk
	and   #$10
	adc   #$70
NotSure        bvc	hmm3
* $$$  ROM DEPENDENT ENTRY POINT  $$$
	jmp	>$FFBD16
hmm3	lda   #$03
	sta   |$C039	SCCAREG
	lda   |$C039	SCCAREG
	bit   |$0103	ATlkFlag
	beq   SerInt2
	pha
	and   #$07
	bne   ChanB
	lda   |$C03B	SCCADATA
	sta   |$0105	SerIntData
	lda   |$C03B	SCCADATA
	sta   |$0106	SerIntData+1
	jsl   >v_AppleTalk
	bra   Next
ChanB	lda   $C03A	SCCBDATA
	sta   |$0105	SerIntData
	lda   $C03A	SCCBDATA
	sta   |$0106	SerIntData+1
	jsl   >v_AppleTalk	IRQ_ATalk
Next	lda   >$010101
	sta   |$011C
	lda   #$00
	ror   a
	sta   |$E10101
	pla
	bra   Next2		not appletalk?
SerInt2	pha
	lda   >$010101
	sta   |$011C
	stz   |$E10101
	jsl   >$E1021C                 MIDI?
	pla
	bcc   handledIt	was a midi interrupt, exit intr
	clc
Next2	anop
	pha
	and	>ourSerFlag
	beq	notOurSerIntr
               lda	1,s
	bit	#%00000111
	beq	doPortPrinter
	jsl	>IntVectModem
	rol	$0101

doPortPrinter	anop
	lda	1,s
               and	>ourSerFlag
	beq	notOurSerIntr1
	bit	#%00111000
	beq	notOurSerIntr1
               jsl	>IntVectPrinter
               rol	$0101
notOurSerIntr1	pla
	lda	$0101
	beq	handledIt
* $$$ ROM DEPENDENT ENTRY POINT $$$
* was a SCC interrupt, but we didn't install it!
               jmp   >$FFBD15

notOurSerIntr  anop
* $$$ ROM DEPENDENT ENTRY POINT
* check for 'external' interrupts here
	jsr	checkExternal
	bcs	handledIt1
	lda	1,s
	jmp	>$FFBCFA	; Back to the real intr handler

handledIt1	pla
handledIt	long  mx
	plb
* $$$ ROM DEPENDENT ENTRY POINT $$$
	jmp   >$FFBF58	; Exit

	END

ROM01IntVect	START
	using	SIMData

	clc
	xce
	long	mx
	php
	phb
	pea	$E1E1
	plb
	sta	$0108
	lda	$C035
	sta	$0119
	ora	#$8000
	and	#$9F1E
	sta	$C035
	stx	$010A
	sty	$010C
	tdc
	sta	$0110
	lda	#0
	tcd
	short	mx
               bcc	sw2	emulation mode?
	lda	4,s	yes check to see if this intr is a BRK
	and	#$10
	adc	#$70
sw2       	bvc	switch1
* $$$ ROM DEPENDENT ENTRY POINT
	jmp	>$FFB85F	it was a BRK, let the firmware handle it
switch1	lda	#3
	sta   $C039
	lda	$C039
               bit   $0103
               beq	X_B83F
               pha
               and	#$07
               bne	X_B826
               lda	$C03B
               sta	$0105
               lda	$C03B
               sta	$0106
               jsl	>v_AppleTalk
               bra	X_B836
X_B826	lda	$C03A
	sta	$0105
	lda	$C03A
	sta	$0106
	jsl	>v_AppleTalk
X_B836	lda	#0
	ror	a
	sta	$0101
	pla
	bra	X_B842
X_B83F	stz	$0101
X_B842	pha
	and	>ourSerFlag
	beq	notOurSerIntr
               lda	1,s
	bit	#%00000111
	beq	doPortPrinter
	jsl	>IntVectModem
	rol	$0101

doPortPrinter	anop
	lda	1,s
               and	>ourSerFlag
	beq	notOurSerIntr1
	bit	#%00111000
	beq	notOurSerIntr1
               jsl	>IntVectPrinter
               rol	$0101

notOurSerIntr1	pla
	lda	$0101
	beq	handledIt
* $$$ ROM DEPENDENT ENTRY POINT
	jmp	>$FFB85E

handledIt1	pla
handledIt	long	mx
	plb
* $$$ ROM DEPENDENT ENTRY POINT
	jmp	>$FFBA86

notOurSerIntr  anop
* $$$ ROM DEPENDENT ENTRY POINT
* check 'external' interrupt sources here
	jsr	checkExternal
	bcs	handledIt1
	lda	1,s
	jmp	>$FFB843	; Back to the real intr handler
	END

SIMRequest	START
	using	SIMData

misc	equ	3
result	equ	14+misc
reqCode	equ	12+misc
dataIn	equ	8+misc
dataOut	equ	4+misc
rtl	equ	1+misc


	phb
	phk
	plb
	phd
	tsc
	tcd

	lda	reqCode
               cmp	#$8003
	jcs	exitRequestErr
	cmp	#$8000
	jcc	exitRequestErr

	and	#$00FF
	asl	a
	tax
	jmp	(requestProcs,X)

requestProcs	dc	a2'doInstall'
	dc	a2'doRemove'
	dc	a2'doVersion'

doInstall	anop
	ldy	#SIMport
	lda	[dataIn],y
	jsr	ValidatePort
	bcs	notValidPort

	cmp	|ApTalkPort	is appletalk running on this port?
	beq	ohNoItsATalk

	cmp	#3	; external port?
	bcs	installExt

	dec	a
	asl	a
	asl	a
	tax
               lda	IntVectPrinter,x
	ora	IntVectPrinter+2,x
	bne	intInstalled

* Turn off interrupts, so we don't get caught with our pants down
	php
	sei
* install the new vector in our interrupt handler table
               lda	#$5C
	sta	IntVectPrinter,x
	ldy   #SIMaddress
	lda	[dataIn],y
	sta	IntVectPrinter+1,x
               ldy   #SIMaddress+1
	lda	[dataIn],y
	sta	IntVectPrinter+2,x

* Now set OurSerFlag to indicate that we now have a handler on that
* port
	ldx	#%00000111
	ldy	#SIMport
	lda	[dataIn],y
	cmp	#2
	beq	port2
               ldx	#%00111000
port2	txa
	ora	ourSerFlag
	sta	ourSerFlag

* And reset the firmware's SerFlag to indicate we're in control
	ldx	#%11111000
	ldy	#SIMport
	lda	[dataIn],y
	cmp	#SIMModemPort
	beq	port2a
               ldx	#%11000111
port2a	txa
               and   >SerFlag
               sta   >SerFlag

* Restore interrupts
               plp

okayno1	lda	#SIMNoError
	bra	goodbye
ohNoItsATalk	lda	#SIMATalkActive
	bra	goodbye
notValidPort	lda	#SIMInvalidPort
	bra	goodbye
intInstalled	lda	#SIMAlreadyInst	a handler is already installed
goodbye	anop
	ldy	#Gerror
	sta	[dataOut],y
	jmp	exitRequest

installExt     dec	a
	dec	a
	dec	a
	asl	a
	asl	a
	tax
	lda	ExtVect1,x
	cmp	ExtVect1+2,x
               bne	intInstalled
               php
	sei
	lda	#$5C
	sta	ExtVect1,x
	ldy   #SIMaddress
	lda	[dataIn],y
	sta	ExtVect1+1,x
               ldy   #SIMaddress+1
	lda	[dataIn],y
	sta	ExtVect1+2,x
               plp
               bra	okayno1

doRemove	anop
* Calculate index into our handler list by quadrupling "port"
	ldy	#SIMport
	lda	[dataIn],y
               jsr	ValidatePort
	bcs	notValidPort

	cmp	#3
	jcs	removeExternal

	dec	a
	asl	a
	asl	a
	tax
* If there is no handler installed on that port, return an error
               lda	IntVectPrinter,x
	ora	IntVectPrinter+2,x
	beq	noIntInstalled
* If the handler on the port isn't the one the application claims,
* return an error
	lda	IntVectPrinter+1,x
               ldy	#SIMaddress
	cmp	[dataIn],y
	bne	notSameVect
	lda	IntVectPrinter+2,x
	ldy	#SIMaddress+1
	cmp	[dataIn],y
	bne	notSameVect

* otherwise, we're cool, so zero out the handler vector
	php
	sei
	lda	#0
	sta	IntVectPrinter,x
	sta	IntVectPrinter+2,x

* and modify serFlag to say "no handlers here, bub!"
	ldx	#%11111000
	ldy	#SIMport
	lda	[dataIn],y
	cmp	#SIMModemPort
	beq	port3
               ldx	#%11000111
port3          txa
	and	ourSerFlag
	sta	ourSerFlag

* We don't re-enable the firmware's SerFlag because there is nobody
* installed now!

	plp

noerrgoaway	lda	#0
	bra	goodbye2

notSameVect	lda	#SIMInvalidAddr
	jmp	goodbye
noIntInstalled lda   #SIMNotInstalled
goodbye2       ldy	#Gerror
	sta	[dataOut],y
               jmp	exitRequest

removeExternal anop
               dec   a
               dec   a
               dec   a
               asl   a
               asl   a
               tax

* if no handler is installed, signal an error
               lda	ExtVect1,x
	ora	ExtVect1+2,x
	beq	noIntInstalled

* if handler is not the same as is there, signal an error
	lda	ExtVect1+1,x
               ldy	#SIMaddress
	cmp	[dataIn],y
	bne	notSameVect
	lda	ExtVect1+2,x
	ldy	#SIMaddress+1
	cmp	[dataIn],y
	bne	notSameVect
               php
	sei
	lda	#0
	sta	ExtVect1,X
	sta	ExtVect1+2,X
	plp
               bra	noerrgoaway

doVersion	anop
	lda	#Version
               ldy	#Gversion
	sta	[dataOut],y
	lda	#SIMNoError
	ldy	#Gerror
	sta	[dataOut],y
	bra	exitRequest

exitRequestErr	anop
	lda	#0
	sta	result
	bra	exitRequest1
exitRequest	lda	#$8000
	sta	result
exitRequest1	anop
	pld
	plb
	lda	2,s
	sta	12,s
	lda	1,s
	sta	11,s
	ply
	ply
	ply
	ply
	ply
	rtl

	END

* Does not modify the port number in A
ValidatePort	START
               cmp	#1
	bcc	bad
	cmp	#7
	bcs	bad
	clc
	rts
bad	sec
	rts
	END

GetROMVersion	START
               pha
	pha
	pha
	pha
	lda	#0
	pha
	pha
	pha
	pea	$FE1F
	_FWEntry
	ply
	pla
	pla
	pla
	tya
	rts
	END

;
; This routine checks to see which serial port, if any, AppleTalk is using.
; The routine sets a flag byte, ApTalkPort, and the accumulator to indicate
; which port (if any) AppleTalk is using.
;    $0000 = AppleTalk is not using a serial port
;    $0001 = AppleTalk is using serial port 1 (printer port)
;    $0002 = AppleTalk is using serial port 2 (modem port)
; Note:  This method should be used under GS/OS only.
;
; Enter routine in native 16 bit mode
;
	longa	on
	longi	on

CheckPort	START

GetPort	equ $8001	The .AppleTalk DStatus subcall to get
;			the port number AppleTalk is currently
;			using.

	phb		save data bank
	phk		data bank = code bank
	plb

	lda	#$0001	start with device #1
	sta	DIdevNum

FindATDriver	anop
	_DInfoGS DInfoParms	call Dinfo
	bcs	DIError	stop searching if error
	lda	DIdeviceIDNum
	cmp	#$001D	is it the AppleTalk main driver?
	beq	ATDriverFound	yes
	inc	DIdevNum	check the
	bra	FindATDriver	next device number

ATDriverFound	anop
	lda	DIdevNum	store device number
	sta	DSdevNum        in the DStatus parm list
	_DStatusGS DStatusParms	call DStatus
	lda	portNum	get the port number
	sta	ApTalkPort
	bra	Exit

DIError	anop
	cmp	#$0011	invalid device number, so the
	beq	NotFound	AppleTalk main driver wasn't found
;
; Add your code to handle any other errors from DInfo here, because the
; end of the device list was not found.

NotFound	stz	ApTalkPort	neither port is in use
	bra	Exit

Exit           anop
               lda 	ApTalkPort
               plb		restore data bank
               rtl		return to caller

ApTalkPort	entry
	ds	2	will be 0, 1, or 2

DInfoParms	anop
	dc	i2'8'	pCount = 8 parameters
DIdevNum	dc i2'1'	devNum
               dc	a4'NameBuffer'	devName
	ds	2	characteristics
               ds	4	totalBlocks
	ds	2	slotNum
	ds	2	unitNum
               ds	2		version
DIdeviceIDNum	ds	2		deviceIDNum

NameBuffer     anop
               dc	i2'31'	Class 1 input string. Max Length=31
               ds	33

DStatusParms   anop
               dc	i2'5'            	pCount = 5 parameters
DSdevNum       ds	2                	devNum
               dc	i2'GetPort'      	statusCode = GetPort
               dc	a4'GetPortSList' 	statusList = GetPortSList
               dc	i4'2'            	requestCount = 2
               ds	4                	transferCount

GetPortSList   anop	   	the GetPort subcall's statusList
portNum        ds	2	Port appletalk is using
	dc	i2'0'
	END

SIMData	DATA
SIMName        str	'SerialIntrMgr~Entry~'
userID	dc	i2'0'

ourSerFlag	dc	i2'0'
SIMuserID	dc	i2'0'

IntVectPrinter ENTRY
	dc	i4'0'
IntVectModem	dc	i4'0'
ExtVect1       ENTRY
	dc	i4'0'
	dc	i4'0'
	dc	i4'0'
	dc	i4'0'
	END

checkExternal	START
* check first external vector
	lda	>ExtVect1
	ora	>ExtVect1+2
	beq	checksecond
	jsl	ExtVect1
	bcs	gotit
checksecond	lda	>ExtVect2
	ora	>ExtVect2+2
	beq	neither
	jsl	ExtVect2
gotit	rts
neither	clc
	rts
	END
