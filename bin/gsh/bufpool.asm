**************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*
* $Id: bufpool.asm,v 1.5 1998/09/08 16:53:05 tribby Exp $
*
**************************************************************************
*
* BUFPOOL
*   By Tim Meekins
*   Modified by Dave Tribby for GNO 2.0.6 (256-byte buffer code removed)
*
* This is the buffer pool
*
* Note: text set up for tabs at col 16, 22, 41, 49, 57, 65
*              |     |                  |       |       |       |
*	^	^	^	^	^	^	
**************************************************************************
*
* Interfaces defined in this file:
*     The alloc routines are a jsl without any stack params.
*         Pointer to requested buffer is returned in X/A registers.
*   alloc1024	
*   free1024	
*
* bufpool data:
*    pool1024		dc   i4'0'
*    pool1024mutex	key
*		          
**************************************************************************

	mcopy /obj/gno/bin/gsh/bufpool.mac

dummybufpool	start		; ends up in .root
	end

**************************************************************************
*
* get a buffer of size 1024
*
**************************************************************************

alloc1024	START
	
	using	bufpool

	lock	pool1024mutex

	lda	pool1024
	ora	pool1024+2
	beq	allocbuf

	phd
	ph4	pool1024
	tsc
	tcd
	lda	[1]
	sta	pool1024
	ldy	#2
	lda	[1],y
	sta	pool1024+2
	unlock pool1024mutex
	pla
	plx
	pld
	rtl

allocbuf	unlock pool1024mutex
	ph4	#1024
	~NEW
	rtl

	END

**************************************************************************
*
* free a buffer of size 1024
*
**************************************************************************

free1024	START

	using bufpool

	phd
	phx
	pha
	tsc
	tcd
	lock pool1024mutex
	lda	pool1024
	sta	[1]
	ldy	#2
	lda	pool1024+2
	sta	[1],y
	lda	1
	sta	pool1024
	lda	3
	sta	pool1024+2
	unlock pool1024mutex
	pla
	plx	
	pld
	rtl

	END

**************************************************************************
*
* buffer pool data
*
**************************************************************************

bufpool	DATA

pool1024	dc	i4'0'
pool1024mutex	key
		         
	END
