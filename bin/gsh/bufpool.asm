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
* BUFPOOL
*   By Tim Meekins
*
* This is the buffer pool
*
**************************************************************************

               keep  o/bufpool
               mcopy m/bufpool.mac

**************************************************************************
*
* get a buffer of size 256
*
**************************************************************************

alloc256	START

	using	bufpool

	lock	pool256mutex

	lda	pool256
	ora	pool256+2
	beq	allocbuf

	phd
	ph4	pool256
	tsc
	tcd
	lda	[1]
	sta	pool256
	ldy	#2
	lda	[1],y
	sta	pool256+2
	unlock pool256mutex
               pla
	plx
	pld
	rtl

allocbuf	unlock pool256mutex
	ph4	#256
	jsl	~NEW
	rtl
	
	END

**************************************************************************
*
* free a buffer of size 256
*
**************************************************************************

free256	START

	using bufpool

	phd
	phx
	pha
	tsc
	tcd
	lock pool256mutex
	lda	pool256
	sta	[1]
	ldy	#2
	lda	pool256+2
	sta	[1],y
	lda	1
	sta	pool256
	lda	3
	sta	pool256+2
	unlock pool256mutex
	pla
	plx	
	pld
	rtl

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
