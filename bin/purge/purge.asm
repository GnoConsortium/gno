**************************************************************************
*
* GNO Purge 3.0
*
* Written by Tim Meekins, Procyon, Inc.
* Based on code originally written by Mike Westerfield.
*
* This program is public domain. enjoy it.
*
* Parts of this program are dependent on GNOlib and the ByteWorks SYSlib.
*
**************************************************************************

               keep  purge
               mcopy purge.mac

Purge          START

handle         equ   0
location       equ   0
nextHandle     equ   4
argv	equ	8
argc	equ	12
verbosity	equ	14
arg	equ	16
;
; memory manager record
;
attributes     equ   4
userID         equ   6
length         equ   8
last           equ   12
next           equ   16

	phk
	plb

	sta	~USER_ID
               sty	commandline
	stx	commandline+2
	stz	verbosity

	jsl	~MM_INIT

	ph4	commandline
	clc
	tdc
	adc	#argv
	pea	0
	pha
	jsl	GNO_PARSEARG
	sta	argc

	dec	a
	beq	start
	dec	a
	beq	doopt
showusage	WriteCString #Usage
               jmp	done

doopt	ldy	#4
	lda	[argv],y
	sta	arg
	iny2
	lda	[argv],y
	sta	arg+2

	lda	[arg]
	and	#$FF
	cmp	#'-'
	bne	showusage
	ldy	#1
	lda	[arg],y
	cmp	#'v'	;the 0 too..
	bne	doopt
	inc	verbosity

start          FreeMem before
	FindHandle #Purge,handle

lb1            ldy   #last
               lda   [handle],y
               tax
               iny2
               ora   [handle],y
               beq   lb1a
               lda   [handle],y
               sta   handle+2
               stx   handle
               bra   lb1


lb1a           lda	verbosity
	beq	lb2
	WriteCString #msg1

lb2            ora2  handle,handle+2,@a
               jeq   lb4
               ldy   #next
               lda   [handle],y
               sta   nextHandle
               iny2
               lda   [handle],y
               sta   nextHandle+2
               ldy   #attributes
               lda   [handle],y
               jmi   lb3
               and   #$0300
               jeq   lb3

           	lda	verbosity
	jeq	dopurge
               WriteChar #'$'
               ldx   handle+2
               lda   handle
               jsr   PrintHex4
               WriteString #msg2
               ldy   #2
               lda   [handle],y
               tax
               lda   [handle]
               jsr   PrintHex4
               WriteString #msg2
               ldy   #attributes
               lda   [handle],y
               jsr   PrintHex2
               WriteString #msg2
               ldy   #userID
               lda   [handle],y
               jsr   PrintHex2
               WriteString #msg2
               ldy   #length+2
               lda   [handle],y
               tax
               dey2
               lda   [handle],y
               jsr   PrintHex4
               WriteChar #' '
               stz   ref
apploop        ldy   #userID
               lda   [handle],y
               and   #%1111000011111111
               LGetPathname (@a,ref),@yx
               if2   @a,eq,#0,appput
               inc   ref
               if2   ref,cc,#128,apploop
               bra   oops
appput         WriteString @xy
oops           WriteLine #empty
dopurge        PurgeHandle handle
lb3            mv4   nextHandle,handle
               jmp   lb2
lb4            CompactMem
	TotalMem @yx
	NewHandle (@xy,~USER_ID,#$0000,#0),handle
	ora2	handle,handle+2,@a
	beq	showstat
	DisposeHandle handle

showstat	lda	verbosity
	beq	Done
	FreeMem After
	WriteCString #beforestr
	ldx	before+2
	lda	before
	jsr	PrintHex4
	WriteCString #leftstr
	WriteCString #afterstr
	ldx	after+2
	lda	after
	jsr	PrintHex4
	WriteCString #leftstr
                              
Done           lda   #0
               rtl

empty          str   ''
ref            ds    2
commandline	ds	4
before	ds	4
after	ds	4

msg1           dc    c'GNO Purge 3.0',h'0d0a0a'
               dc    c'Handle  Ptr     Attr  User  Length  App',h'0d0a00'
msg2           str   ' $'
Usage	dc	c'Usage: purge [-v]',h'0d0a00'
beforestr	dc	h'0d0a',c'Before: $',h'00'
afterstr	dc	c'After:  $',h'00'
leftstr	dc	c' bytes free',h'0d0a00'

PrintHex1      Int2Hex (@a,#hex1str+1,#2)
               WriteString #hex1str
               rts
hex1str        str   '00'

PrintHex4      pha
               txa
               jsr   PrintHex1
               pla

PrintHex2      Int2Hex (@a,#hex2str+1,#4)
               WriteString #hex2str
               rts
hex2str        str   '0000'

               END
