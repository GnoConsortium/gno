**************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*
* $Id: bufpool.asm,v 1.3 1998/06/30 17:25:09 tribby Exp $
*
**************************************************************************
*
* BUFPOOL
*   By Tim Meekins
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
*   alloc256	
*   free256	
*   alloc1024	
*   free1024	
*
* bufpool data:
*    pool256		dc   i4'0'
*    pool256mutex	key
*    pool1024		dc   i4'0'
*    pool1024mutex	key
*                              
**************************************************************************

	mcopy /obj/gno/bin/gsh/bufpool.mac

dummybufpool	start		; ends up in .root
	end

**************************************************************************
*
* get a buffer of size 256
*
**************************************************************************

alloc256	START

	using	bufpool

	lock	pool256mutex	Mutual exclusion lock.

	lda	pool256	Get address of
	ora	pool256+2	 256-byte pool.
	beq	allocbuf	If NULL, go allocate the buffer.

	phd		Hold on to Direct Page register.
	ph4	pool256	Put current contents of pool256 on stack.
	tsc		Copy stack pointer to
	tcd		 Direct Page register.
	lda	[1]	Store 4 bytes of data
	sta	pool256	 that pool256 pointed to
	ldy	#2	  in pool256.
	lda	[1],y
	sta	pool256+2

	unlock pool256mutex	Mutual exclusion unlock.

               pla		A = old pool256
	plx		X = old pool256+2
	pld		Restore Direct Page register.

	rtl		Return to caller.


allocbuf	unlock pool256mutex	Mutual exclusion unlock.
	ph4	#256	Request 256 bytes
	jsl	~NEW	 from system.
	rtl		Return to caller.
	
	END

**************************************************************************
*
* free a buffer of size 256
*
**************************************************************************

free256	START

	using bufpool

	phd		Save data bank register.
	phx		Put address to free
	pha		  on stack.
	tsc		Copy stack pointer into
	tcd		 direct page register.

	lock pool256mutex	Mutual exclusion lock.

	lda	pool256
	sta	[1]
	ldy	#2
	lda	pool256+2
	sta	[1],y
	lda	1
	sta	pool256
	lda	3
	sta	pool256+2

	unlock pool256mutex	Mutual exclusion unlock.

               pla		Restore Accumulator,
	plx		 X-register, and
	pld		  Direct Page register,

	rtl		Return to caller.

	END

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
	jsl	~NEW
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

pool256	dc	i4'0'
pool256mutex	key
pool1024	dc	i4'0'
pool1024mutex	key
                              
	END
