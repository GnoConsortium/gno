*	$Id: conpatch.asm,v 1.1 1998/02/02 08:17:53 taubert Exp $

	case	on
	mcopy	../drivers/conpatch.mac

PatchDeskManager	START
	phd
	pha
	pha
	pea	$0000			; system tool set
	pea	$0005			; Desk Manager
	_GetTSPtr
	tsc
	tcd

	ldy	#$002d			; ($b * 4) + 1
	lda	[$1],y
	sta	>CONOLDSAVEALL+1
	dey
	lda	[$1],y
	inc	A
	sta	>CONOLDSAVEALL
	iny
	lda	#>ConSaveAllPatch	; -1
	sta	[$1],y
	dey
	lda	#ConSaveAllPatch-1
	sta	[$1],y

	ldy	#$0031			; ($c * 4) + 1
	lda	[$1],y
	sta	>CONOLDRESTALL+1
	dey
	lda	[$1],y
	inc	A
	sta	>CONOLDRESTALL
	iny
	lda	#>ConRestAllPatch	; -1
	sta	[$1],y
	dey
	lda	#ConRestAllPatch-1
	sta	[$1],y

	pla
	pla
	pld
	rtl
	END
	
UnpatchDeskManager	START
	phd
	pha
	pha
	pea	$0000			; system tool set
	pea	$0005			; Desk Manager
	_GetTSPtr
	tsc
	tcd

	ldy	#$002d			; ($b * 4) + 1
	lda	>CONOLDSAVEALL+1
	sta	[$1],y
	dey
	lda	>CONOLDSAVEALL
	dec	A
	sta	[$1],y

	ldy	#$0031			; ($c * 4) + 1
	lda	>CONOLDRESTALL+1
	sta	[$1],y
	dey
	lda	>CONOLDRESTALL
	dec	A
	sta	[$1],y

	pla
	pla
	pld
	rtl
	END

