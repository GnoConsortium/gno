**************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*
* $Id: bufpool.asm,v 1.6 1998/12/21 23:57:04 tribby Exp $
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
*     The alloc routine is a jsl without any stack params.
*         Pointer to requested buffer is returned in X/A registers.
*   alloc1024	
*     The free routine takes the address from the X/A registers
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
* Get a buffer of size 1024
*
**************************************************************************

alloc1024	START
	
	using	bufpool

	lock	pool1024mutex

	lda	pool1024	If pool pointer
	ora	pool1024+2	 isn't NULL,
	beq	allocbuf

	phd
	ph4	pool1024		Push pool pointer on stack.
	tsc
	tcd
	lda	[1]		Replace pool pointer with
	sta	pool1024		 the address it points to.
	ldy	#2
	lda	[1],y
	sta	pool1024+2
	unlock pool1024mutex
	pla
	plx
	pld
	rtl			Return to caller.

;
; No memory in free pool; must allocate a new block.
;
allocbuf	unlock pool1024mutex
	ph4	#1024
	~NEW
	rtl

	END

**************************************************************************
*
* Free a buffer of size 1024, putting it into the free pool
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
	lda	pool1024	Move current head of pool list
	sta	[1]	 into the buffer being freed.
	ldy	#2
	lda	pool1024+2
	sta	[1],y
	lda	1	Put address of buffer being freed
	sta	pool1024	 into the pool list head.
	lda	3
	sta	pool1024+2
	unlock pool1024mutex
	pla
	plx	
	pld
	rtl		Return to caller.

	END

**************************************************************************
*
* Buffer pool data
*
**************************************************************************

bufpool	DATA

pool1024	dc	i4'0'	Head of free pool list.

pool1024mutex	key		Mutual exclusion when modifying list.
		         
	END
