**************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*
**************************************************************************
*
* HISTORY.ASM
*   By Tim Meekins
*
* Routines for dealing with history buffers.
*
* History Data Structures:
*
*        HistoryPtr -> historyRec   [HistoryPtr is the most recent history]
*
* Where historyRec is:
*
*   [+0] NextHistory: pointer to historyRec
*   [+4] HistoryCmd:  string of characters
*
**************************************************************************

               keep  o/history
               mcopy m/history.mac

histNext       gequ  0
histCmd        gequ  4

;=========================================================================
;
; Add a C string to the head of the history buffer.
;
;=========================================================================

InsertHistory  START

               using HistoryData
               using global

ptr2	equ	0
ptr	equ	ptr2+4
len	equ	ptr+4
space	equ	len+2

	subroutine (4:cmd),space

	pei	(cmd+2)
	pei	(cmd)
	jsr	cstrlen
               sta   len                 
               pea   0
	clc
	adc	#4+1
               pha
               jsl   ~NEW
               sta   ptr
               stx   ptr+2
               ora   ptr+2
               beq   putdone

               inc   lasthist
               inc   numhist

               lda   historyptr         ;Set up linked list pointers
               sta   [ptr]
               ldy   #histNext+2
               lda   historyptr+2
               sta   [ptr],y
               mv4   ptr,historyptr

	pei	(cmd+2)
	pei	(cmd)
	pei	(ptr+2)
	clc
	lda	ptr
	adc	#4
	pha
	jsr	copycstr
;
; Now, find out what the maximum history is and prune to that value
;
putdone	anop

               Read_Variable RVParm
               lda   buffer
               and   #$FF
               beq   alldone
               Dec2Int (#buffer+1,@a,#0),@a
               sta   size
               beq   alldone
;
; Follow linked list until we reach size histories
;
               mv4   historyptr,ptr
               ldy   #histNext+2
follow         lda   [ptr]
               tax
               ora   [ptr],y
               beq   alldone            ;not enough
               dec   size
               beq   prune
               lda   [ptr],y
               sta   ptr+2
               stx   ptr
               bra   follow
;
; we have enough, start pruning
;
prune          lda   [ptr]
               sta   ptr2
               lda   [ptr],y
               sta   ptr2+2
               lda   #0
               sta   [ptr]
               sta   [ptr],y              ;terminate last history
;
; Dispose remaining
;
dispose        lda	ptr2
	ora	ptr2+2
	beq	alldone
	dec	numhist
               lda   [ptr2]
               sta   ptr
               lda   [ptr2],y
               sta   ptr+2
               pei   (ptr2+2)
               pei   (ptr2)
               jsl   nullfree
               lda   ptr
               sta   ptr2
               lda   ptr+2
               sta   ptr2+2
               ldy   #histNext+2
               bra   dispose

alldone        return

RVParm         dc    a4'historyStr'
               dc    a4'buffer'

size           ds    2

               END

;=========================================================================
;
; Places the previous history into the command buffer, called when pressing
; UP-ARROW.
;
;=========================================================================

PrevHistory    START

               using HistoryData
               using global
	using	termdata

               ora2  historyptr,historyptr+2,@a  ;If no history, then skip
               jeq   ctl5a

               ldx   cmdloc             ;Move cursor to beginning of line
	jsr	moveleft

	lda	cdcap
	ora	cdcap
	beq	ctl5b0
	tputs (cdcap,#0,#outc)
               bra   ctl5g

ctl5b0	ldx	cmdlen	;clear line
ctl5b	dex
	bmi	ctl5c
	phx
	lda	#' '
	jsr	putchar
	plx
	bra	ctl5b
ctl5c	ldx	cmdlen
	jsr	moveleft

ctl5g          ora2  currenthist,currenthist+2,@a
               bne   ctl5i              ;If not set up for current hist then
               lda   historyptr+2       ;Set up at start.
               ldx   historyptr
               ldy   #2
               bra   ctl5j

ctl5i          mv4   currenthist,0      ;Otherwise move to previous history
               stz   cmdlen
               stz   cmdloc
               ldy   #HistNext+2
               lda   [0]
               tax
               lda   [0],y

ctl5j          sta   0+2                ;Save some pointers
               stx   0
               sta   currenthist+2
               stx   currenthist
               ora   0                  ;If ptr is 0 then at end, quit
               beq   ctl5a

               ldx   #0                 ;Display and store command
               iny2
ctl5h          lda   [0],y
	and	#$FF
               sta   cmdline,x
	beq	ctl5ha
               inx
               iny
               bra   ctl5h
ctl5ha         stx	cmdlen
	stx	cmdloc

	ldx	#^cmdline
	lda	#cmdline
	jsr	puts

ctl5a          rts

               END

;=========================================================================
;
; Places the next history into the command buffer, called when pressing
; DOWN-ARROW.
;
;=========================================================================

NextHistory    START

               using HistoryData
               using global
	using	termdata

               ora2  historyptr,historyptr+2,@a  ;No hist, then skip
               jeq   ctl6a

               stz   4          ;Walk through linked list searching
               stz   4+2        ; for hist prior to last hist.
               mv4   historyptr,0
ctl6i          if2   0,ne,currenthist,ctl6j
               if2   0+2,eq,currenthist+2,ctl6k
ctl6j          mv4   0,4
               ldy   #HistNext+2
               lda   [0]
               tax
               lda   [0],y
               sta   0+2
               stx   0
               bra   ctl6i

ctl6k          ldx   cmdloc             ;Move cursor to left
	jsr	moveleft

	lda	cdcap
	ora	cdcap
	beq	ctl6b0
	tputs (cdcap,#0,#outc)
               bra   ctl6g

ctl6b0	ldx	cmdlen	;clear line
ctl6b	dex
	bmi	ctl6c
	phx
	lda	#' '
	jsr	putchar
	plx
	bra	ctl6b
ctl6c	ldx	cmdlen
	jsr	moveleft

ctl6g          stz   cmdlen             ;Get hist info.
               stz   cmdloc
               mv4   4,currenthist
               ora2  4,4+2,@a
               beq   ctl6a              ;Whoops, back to end, quit

               ldx   #0                 ;Output the new command
               ldy	#4
ctl6h          lda   [4],y
	and	#$ff
               sta   cmdline,x
	beq	ctl6ha
               iny
               inx
               bra   ctl6h
ctl6ha	stx	cmdlen
	stx	cmdloc

	ldx	#^cmdline
	lda	#cmdline
	jsr	puts

ctl6a          rts

               END

;=========================================================================
;
; Save History if variable set
;
;=========================================================================

SaveHistory    START

               using HistoryData
               using global

	lda	historyFN
	sta	DestroyParm+2
	sta	CreateParm+2
	sta	OpenParm+4
	lda	historyFN+2
	sta	DestroyParm+4
	sta	CreateParm+4
	sta	OpenParm+6

               Read_Variable RVParm
               lda   buffer
               and   #$FF
               jeq   done
               Dec2Int (#buffer+1,@a,#0),size
               lda   size
               jeq   done
;
; Create and write history to file
;
               Destroy DestroyParm
               Create CreateParm
               jcs   done
               Open  OpenParm
               bcs   done
               mv2   OpenRef,(WriteRef,WriteCRRef,CloseRef)

loop1          mv4   historyptr,0
               mv2   size,count
               ldy   #histNext+2
loop2          lda   0
               ora   0+2
               beq   next
               lda   [0]
               tax
               dec   count
               beq   write
               lda   [0],y
               sta   0+2
               stx   0
               bra   loop2
write          clc
	lda	0
	adc	#4
	tax
	sta	WriteBuf
	lda	0+2
	adc	#0
	sta	WriteBuf+2
	pha
	phx
	jsr	cstrlen
               sta   WriteReq
               Write WriteParm
               bcs   doneclose
               Write WriteCR
               bcs   doneclose
next           dec   size
               bne   loop1

doneclose      Close CloseParm

done           rts

RVParm         dc    a4'savehistStr'
               dc    a4'buffer'

DestroyParm    dc    i2'1'
               dc    a4'historyFN'

CreateParm     dc    i2'3'
               dc    a4'historyFN'
               dc    i2'$C3'
               dc    i2'0'

OpenParm       dc    i2'2'
OpenRef        ds    2
               dc    a4'historyFN'

WriteParm      dc    i2'4'
WriteRef       ds    2
WriteBuf       dc    a4'buffer'
WriteReq       ds    4
               ds    4

WriteCR        dc    i2'4'
WriteCRRef     ds    2
               dc    a4'CRBuf'
               dc    i4'1'
               ds    4
CRBuf          dc    i1'13'

CloseParm      dc    i2'1'
CloseRef       ds    2

size           ds    2
count          ds    2

               END

;=========================================================================
;
; Read History file
;
;=========================================================================

ReadHistory    START

               using HistoryData
               using global

	lda	historyFN
	sta	OpenParm+4
               lda   historyFN+2
	sta	OpenParm+6

               lda   #0
               sta   historyptr
               sta   historyptr+2

               Open  OpenParm
               bcs   done
               mv2   OpenRef,(ReadRef,NLRef,CloseRef)
               NewLine NLParm
               bcs   doneclose

loop           anop
               Read  ReadParm
               bcs   doneclose
               ldy   ReadTrans
               beq   doneclose
               dey
	lda	#0
	sta	buffer,y
	ph4	#buffer
               jsl   InsertHistory
               bra   loop

doneclose      Close CloseParm

done           rts

OpenParm       dc    i2'2'
OpenRef        ds    2
               dc    a4'historyFN'

NLParm         dc    i2'4'
NLRef          ds    2
               dc    i2'$7F'
               dc    i2'1'
               dc    a4'NLTable'
NLTable        dc    i1'13'

ReadParm       dc    i2'4'
ReadRef        ds    2
               dc    a4'buffer'
               dc    i4'1024'
ReadTrans      ds    4

CloseParm      dc    i2'1'
CloseRef       ds    2

size           ds    2

               END

;=========================================================================
;
; Init History
;
;=========================================================================
InitHistory	START
	using	HistoryData

	ph4	#histName
	jsl	AppendHome
	stx	historyFN+2
	sta	historyFN
	rts
	END

;=========================================================================
;
; Print History
;
;=========================================================================

PrintHistory   START

               using HistoryData
               using global

ptr            equ   0
space          equ   ptr+4

               subroutine (2:argc,4:argv),space

               lda   historyptr
               ora   historyptr+2
               beq   done

               lda   numhist
               sta   num
loop1          mv4   historyptr,ptr
               lda   num
               bmi   done
               sta   count
loop2          lda   count
               beq   print
               ldy   #histNext+2
               lda   [ptr]
               tax
               lda   [ptr],y
               sta   ptr+2
               stx   ptr
               ora   ptr
               beq   next
               dec   count
               bra   loop2

print          sub2  lasthist,num,@a
               Int2Dec (@a,#numbstr,#4,#0)
	ldx	#^numbstr
	lda	#numbstr
	jsr	puts
               clc
               ldx   ptr+2
               lda   ptr
               adc   #4
ok             jsr	puts
	jsr	newline

next           dec   num
               bra   loop1

done           return

numbstr        dc    c'0000:  ',h'00'
num            ds    2
count          ds    2

               END

;=========================================================================
;
; History Data
;
;=========================================================================

HistoryData    DATA

lasthist       dc    i2'0'
numhist        dc    i2'0'
currenthist    ds    4
historyptr     dc    i4'0'
historyStr     str   'HISTORY'
savehistStr    str   'SAVEHIST'
histName       dc	c'/history',i1'0'
historyFN	ds	4

               END
