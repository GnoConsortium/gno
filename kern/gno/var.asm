*	$Id: var.asm,v 1.1 1998/02/02 08:20:02 taubert Exp $
**************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*   Derek Taubert
*
**************************************************************************
*
* VAR.ASM
*   By Tim Meekins
*   Optimized Nov 6,1991 by Jawaid Bazyar
*
* Routines for dealing with shell variables
*
* DATA STRUCTURES:
*
* varPtr points to a linked list of varNodes. Each varNode points to
* the next varNode and to the varTable for this level. When a Push Var
* is called when starting a shell file, a new varNode is added to the head
* of the list and a new varTable is created. All SETs will occur within
* this varTable. Reading variables will start by checking the current varTable.
* If not found, then the next lower varTable will be checked. This will
* insure proper scoping.
*
* A vartable is an array of size VTABSIZE. Each slot is a pointer to a
* variable itself. The variable consists of a pointer to its name, a 
* pointer to its value, a pointer to the next variable in case of hash
* collisions, and finally a flag which defines if the variable should
* be exported to the previous varTable after the current varTable is popped.
*
* varPtr:
*    [0] next: Ptr
*    [4] varTable: Ptr
*
* varTable[VTABSIZE]:
*    var: Ptr
*
* var:
*    [0] next: Ptr
*    [4] name: Ptr
*    [8] value: Ptr
*   [12] flag: word
*
* Nov 6, 1991 (jb) : I removed all references to kvm_xxxx routines.
*   The variable code will probably run about twice as fast now.
*
**************************************************************************

	case	on
	mcopy m/var.mac

VTABSIZE	gequ  39                 ;Size of variable table

VEXPORT	gequ  %0001              ;This is variable is exported.

;=========================================================================
;
; Deallocates the specified process' variable tables.  After this is done
; none of the variable calls will work properly, so disposevar MUST be
; called ONLY at process shutdown time.
;
;=========================================================================

disposevar	START
	using	VarData
	using	KernelStruct

	subroutine (2:process),0

displp	lda	process
	asl	a
	asl	a
	tax
	lda	varPtr,x
	ora	varPtr+2,x
	beq	done
	lda	process
	jsl	popvartbl		
	bra	displp
done	return
	END

;=========================================================================
;
; Initialize a new variable table. This is called at startup and also
; performs a PushStack. [THERE IS NO MEMORY ERROR CHECKING AT THIS TIME!!]!
;
;=========================================================================

initvar	START

	using VarData
	using KernelStruct

pidx128	equ   0
count	equ   pidx128+2
p	equ   count+2
tbl	equ   p+4
oldtbl	equ   tbl+4
space	equ   oldtbl+4

	subroutine (0:dummy),space

;
; Save the current table value
;
	lda   truepid
	beq   root
	asl   a
	tax
	lda   varDepth,x
	bne   pushptr

	lda   truepid            ; $$$
folpid	anop

	asl   a
	asl   a
	asl   a
	asl   a
	asl   a
	asl   a
	asl   a
	sta   pidx128
	tax
	lda   ParentProc,x       ; $$$
	bmi   root
	asl   a
	tax
	lda   varDepth,x
	bne   tryold

	ldx   pidx128
	lda   ParentProc,x
	bra   folpid

root	stz   oldtbl
	stz   oldtbl+2
	bra   insNode

tryold	anop

pushptr	txa
	asl   a
	tax
	lda   varPtr,x
	sta   p
	lda   varPtr+2,x
	sta   p+2
	ldy   #4
	lda   [p],y
	sta   oldtbl
	iny2
	lda   [p],y
	sta   oldtbl+2
;
; Insert a new varNode into the linked list
;
insNode	ph4   #4+4
	jsl   ~NEW
	sta   p
	stx   p+2
	lda   truepid
	asl2  a
	tax
	ldy   #2
	lda   varPtr,x
	sta   [p]
	lda   varPtr+2,x
	sta   [p],y
	lda   p
	sta   varPtr,x
	lda   p+2
	sta   varPtr+2,x
;
; Create a new varTable and add to linked list
;
	ph4   #VTABSIZE*4
	jsl   ~NEW
	sta   tbl
	stx   tbl+2
	ldy   #4
	sta   [p],y
	iny2
	txa
	sta   [p],y
;
; Set all table entries to NIL
;
	ldx   #VTABSIZE
	ldy   #0
	lda   #0
nilloop	sta   [tbl],y
	iny2
	sta   [tbl],y
	iny2
	dex
	bne   nilloop
;
; Set the var depth before calling SETVAR below
;
	lda   truepid
	asl   a
	tax
	lda   varDepth,x
	inc   a
	sta   varDepth,x
;
; If oldtbl == NIL then we're all done for now
;
	lda   oldtbl
	ora   oldtbl+2
	beq   done
;
; scan through the old table, get any exported variables and insert them
; into the new var table.
;
	ld2   VTABSIZE,count

loop1	ldy   #2
	lda   [oldtbl]
	sta   p
	lda   [oldtbl],y
	sta   p+2
	ora   p
	beq   nexttbl

loop2	ldy   #12
	lda   [p],y
	and   #VEXPORT
	beq   nextvar

	ldy   #4+2
	lda   [p],y
	pha
	dey2
	lda   [p],y
	pha
	ldy   #8+2
	lda   [p],y
	pha
	dey2
	lda   [p],y
	pha
	jsl   setvar             ;Copy over the new variable
	ldy   #4+2
	lda   [p],y
	pha
	dey2
	lda   [p],y
	pha
	ldy   #12
	lda   [p],y
	pha
	jsl   exportvar          ;Copy over the var flags

nextvar	ldy   #2
	lda   [p]
	tax
	lda   [p],y
	sta   p+2
	stx   p
	ora   p
	bne   loop2

nexttbl	add4  oldtbl,#4,oldtbl
	dec   count
	bne   loop1

done	return

	END

;=========================================================================
;
; Pop a variable table.
;
;=========================================================================

popvartbl	START

	using VarData
	using KernelStruct

tblptr	equ   0
pid	equ   tblptr+4
count	equ   pid+2
p	equ   count+2
tbl	equ   p+4
space	equ   tbl+4

	sta	>mylocalpid
	subroutine (0:dummy),space

	lda   mylocalpid
	sta   pid

	asl   a
	tax
	lda   varDepth,x
	bne   ok
	jmp   done

ok	dec   a
	sta   varDepth,x
;
; Point to tables, assume ptrs good because varDepth is good
;
	txa
	asl   a
	tax
	lda   varPtr,x
	sta   p
	lda   varPtr+2,x
	sta   p+2

	ldy   #4
	lda   [p],y
	sta   tbl
	sta   tblptr
	iny2
	lda   [p],y
	sta   tbl+2
	sta   tblptr+2

	ldy   #2
	lda   [p]
	tax
	lda   [p],y
	sta   p+2
	stx   p
;
; Dispose varNode pointing to table
;
	lda   pid
	asl2  a
	tax
	lda   varPtr+2,x
	pha
	lda   varPtr,x
	pha
	jsl   ~NDISPOSE
	lda   pid
	asl2  a
	tax
	lda   p
	sta   varPtr,x
	lda   p+2
	sta   varPtr+2,x
;
; Start looping through each variable in the table and then dispose them.
; If a variable is set to export then export it.
;
	ld2   VTABSIZE,count

loop1	ldy   #2
	lda   [tbl]
	sta   p
	lda   [tbl],y
	sta   p+2
	ora   p
	beq   nexttbl

loop2	anop
*	ldy   #12
*              lda   [p],y
*              and   #VEXPORT
*              beq   dispvar
*
*              ldy   #4+2
*              lda   [p],y
*              pha
*              dey2
*              lda   [p],y
*              pha
*              ldy   #8+2
*              lda   [p],y
*              pha
*              dey2
*              lda   [p],y
*              pha
*              jsl   setvar             ;Copy over the new variable
*              ldy   #4+2
*              lda   [p],y
*              pha
*              dey2
*              lda   [p],y
*              pha
*              ldy   #12
*              lda   [p],y
*              pha
*              jsl   exportvar          ;Copy over the var flags

dispvar	ldy   #4+2               ;dispose the variable
	lda   [p],y
	pha
	dey2
	lda   [p],y
	pha
	jsl   ~NDISPOSE
	ldy   #8+2
	lda   [p],y
	pha
	dey2
	lda   [p],y
	pha
	jsl   ~NDISPOSE
	pei   (p+2)
	pei   (p)
	ldy   #2
	lda   [p]
	tax
	lda   [p],y
	sta   p+2
	stx   p
	jsl   ~NDISPOSE
	lda   p+2
	ora   p
	bne   loop2

nexttbl	add4  tbl,#4,tbl
	dec   count
	jne   loop1

	pei   (tblptr+2)
	pei   (tblptr)
	jsl   ~NDISPOSE

done	return
mylocalpid	dc  i2'0'
	END

;=========================================================================
;
; Set a variable.
;
;=========================================================================

setvar	START

	using VarData
	using KernelStruct

pidx128	equ   0
var	equ   pidx128+2
tbl	equ   var+4
p	equ   tbl+4
q	equ   p+4
space	equ   q+4

	subroutine (4:name,4:value),space

	pei   (name+2)
	pei   (name)
	jsl   lowercstr

	lda   truepid

	asl   a
	tax
	lda   varDepth,x
	bne   pointit

	lda   truepid
folpid	anop
	asl2  a
	asl2  a
	asl2  a
	asl   a
	sta   pidx128
	tax
	lda   ParentProc,x       ;parent id
	jmi   done               ;can't do it
	asl   a
	tax
	lda   varDepth,x
	bne   tryit
	ldx   pidx128
	lda   ParentProc,x
	bra   folpid

tryit	anop

;
; Point to table
;
pointit	txa
	asl   a
	tax
	lda   varPtr,x
	sta   p
	lda   varPtr+2,x
	sta   p+2
	ldy   #4
	lda   [p],y
	sta   tbl
	iny2
	lda   [p],y
	sta   tbl+2
;
; Point to var list in hash table
;
	pei   (name+2)
	pei   (name)
	jsl   hashvar
	clc
	adc   tbl
	sta   p
	lda   tbl+2
	adc   #0
	sta   p+2
;
; See if it's already defined locally
;
	ldy   #2
	lda   [p],y
	pha
	lda   [p]
	pha
	pei   (name+2)
	pei   (name)
	jsl   followlocvar
	cmp   #0
	bne   changevar
	cpx   #0
	beq   newvar
;
; Modify an already defined variable
;
changevar	anop
	sta   var                ;point to variable
	stx   var+2
	ldy   #8+2               ;dispose old value
	lda   [var],y
	pha
	dey2
	lda   [var],y
	pha
	jsl   ~NDISPOSE
	jmp   setvalue           ;set the new value
;
; Create a new local variable
;
newvar	anop
	ph4   #14                ;get space for variable
	jsl   ~NEW
	sta   var
	stx   var+2
	ora   var+2
	beq   done
	ldy   #2                 ;insert variable into linked list
	lda   [p]
	sta   [var]
	lda   [p],y
	sta   [var],y
	lda   var
	sta   [p]
	lda   var+2
	sta   [p],y
	ldy   #12                ;initialize flag to 0
	lda   #0
	sta   [var],y
	pei   (name+2)           ;get memory for the string
	pei   (name)
	jsl   alloccstr
	sta   q
	stx   q+2
	ldy   #4                 ;point to name
	sta   [var],y
	txa
	iny2
	sta   [var],y
	pei   (name+2)           ;copy name to variable
	pei   (name)
	pei   (q+2)
	pei   (q)
	jsl   copycstr
setvalue	pei   (value+2)          ;get memory for the string
	pei   (value)
	jsl   alloccstr
	sta   q
	stx   q+2
	ldy   #8                 ;point to value
	sta   [var],y
	txa
	iny2
	sta   [var],y
	pei   (value+2)          ;copy value to variable
	pei   (value)
	pei   (q+2)
	pei   (q)
	jsl   copycstr

done	return

	END

;=========================================================================
;
; Returns the variable record of a named variable (same as what indexvar
; returns)
;
;=========================================================================

readvarrec	START

	using VarData
	using KernelStruct

pidx128	equ   0   
hashval	equ   pidx128+2
var	equ   hashval+2
tbl	equ   var+4
p	equ   tbl+4
space	equ   p+4

	subroutine (4:name),space

	pei   (name+2)
	pei   (name)
	jsl   lowercstr
	pei   (name+2)
	pei   (name)
	jsl   hashvar
	sta   hashval

	lda   truepid
	asl   a
	tax
	lda   varDepth,x
	bne   pointit

	lda   truepid
folpid	anop

	asl2  a
	asl2  a
	asl2  a
	asl   a
	sta   pidx128
	tax
	lda   ParentProc,x       ;parent id
	bpl   cont               ;can't do it
	stz   p
	stz   p+2
	jmp   done
cont	asl   a
	tax
	lda   varDepth,x
	bne   tryit
	ldx   pidx128
	lda   ParentProc,x
	bra   folpid

tryit	anop

;
; Point to table
;
pointit	txa
	asl   a
	tax
	lda   varPtr,x
	sta   p
	lda   varPtr+2,x
	sta   p+2

loop	clc
	ldy   #4
	lda   [p],y
	adc   hashval
	sta   tbl
	iny2
	lda   [p],y
	adc   #0
	sta   tbl+2
;
; See if it's already defined locally
;
	ldy   #2
	lda   [tbl],y
	pha
	lda   [tbl]
	pha
	pei   (name+2)
	pei   (name)
	jsl   followlocvar
;               cmp   #0
;               bne   found
;               cpx   #0
;               bne   found
;
; next table
;
;               ldy   #2
;               lda   [p]
;               tax
;               lda   [p],y
;               sta   p+2
;               stx   p
;               ora   p
;               bne   loop
;               bra   done

found	sta   var
	stx   var+2

done	return 4:var

	END


;=========================================================================
;
; Read a variable.
;
;=========================================================================

readvar	START

	using VarData
	using KernelStruct

pidx128	equ   0   
hashval	equ   pidx128+2
var	equ   hashval+2
tbl	equ   var+4
p	equ   tbl+4
space	equ   p+4

	subroutine (4:name),space

	pei   (name+2)
	pei   (name)
	jsl   lowercstr
	pei   (name+2)
	pei   (name)
	jsl   hashvar
	sta   hashval

	lda   truepid
	asl   a
	tax
	lda   varDepth,x
	bne   pointit

	lda   truepid
folpid	anop

	asl2  a
	asl2  a
	asl2  a
	asl   a
	sta   pidx128
	tax
	lda   ParentProc,x       ;parent id
	bpl   cont               ;can't do it
	stz   p
	stz   p+2
	jmp   done
cont	asl   a
	tax
	lda   varDepth,x
	bne   tryit
	ldx   pidx128
	lda   ParentProc,x
	bra   folpid

tryit	anop

;
; Point to table
;
pointit	txa
	asl   a
	tax
	lda   varPtr,x
	sta   p
	lda   varPtr+2,x
	sta   p+2

loop	clc
	ldy   #4
	lda   [p],y
	adc   hashval
	sta   tbl
	iny2
	lda   [p],y
	adc   #0
	sta   tbl+2
;
; See if it's already defined locally
;
	ldy   #2
	lda   [tbl],y
	pha
	lda   [tbl]
	pha
	pei   (name+2)
	pei   (name)
	jsl   followlocvar
	cmp   #0
	bne   found
	cpx   #0
	bne   found
;
; next table
;
;               ldy   #2
;               lda   [p]
;               tax
;               lda   [p],y
;               sta   p+2
;               stx   p
;               ora   p
;               bne   loop
	stz	p
	stz	p+2
	bra   done

found	sta   var
	stx   var+2
	ldy   #8
	lda   [var],y
	sta   p
	iny2
	lda   [var],y
	sta   p+2

done	return 4:p

	END

;=========================================================================
;
; Return the nth variable by its index
;
;=========================================================================

indexvar	START

	using VarData
	using KernelStruct

pidx128	equ   0
count	equ   pidx128+2
p	equ   count+2
tbl	equ   p+4
space	equ   tbl+4

	subroutine (2:index),space

	lda   index
	jeq   nope
	jmi   nope

	lda   truepid
	asl   a
	tax
	lda   varDepth,x
	bne   pointit

	lda   truepid
folpid	anop
	asl2  a
	asl2  a
	asl2  a
	asl   a
	sta   pidx128
	tax
	lda   ParentProc,x       ;parent id
	jmi   nope               ;can't do it
	asl   a
	tax
	lda   varDepth,x
	bne   tryit
	ldx   pidx128
	lda   ParentProc,x
	bra   folpid

tryit	anop

;
; Point to table
;
pointit	txa
	asl   a
	tax
	lda   varPtr,x
	sta   p
	lda   varPtr+2,x
	sta   p+2

	ldy   #4
	lda   [p],y
	sta   tbl
	iny2
	lda   [p],y
	sta   tbl+2

	ld2   VTABSIZE,count

loop1	ldy   #2
	lda   [tbl]
	sta   p
	lda   [tbl],y
	sta   p+2
	ora   p
	beq   nexttbl

loop2	dec   index
	beq   found
	ldy   #2
	lda   [p]
	tax
	lda   [p],y
	sta   p+2
	stx   p
	ora   p
	bne   loop2

nexttbl	add4  tbl,#4,tbl
	dec   count
	bne   loop1

nope	stz   p
	stz   p+2

found	return 4:p

	END

;=========================================================================
;
; Unset a variable.
;
;=========================================================================

unsetvar	START

	using VarData
	using KernelStruct

pidx128	equ   0
ch	equ   pidx128+2
thisname	equ   ch+2
hashval	equ   thisname+4
var	equ   hashval+2
tbl	equ   var+4
p	equ   tbl+4
q	equ   p+4
space	equ   q+4

	subroutine (4:name),space

	pei   (name+2)
	pei   (name)
	jsl   lowercstr
	pei   (name+2)
	pei   (name)
	jsl   hashvar
	sta   hashval

	lda   truepid
	asl   a
	tax
	lda   varDepth,x
	bne   pointit

	lda   truepid
folpid	anop
	asl2  a
	asl2  a
	asl2  a
	asl   a
	sta   pidx128
	tax
	lda   ParentProc,x       ;parent id
	jmi   done               ;can't do it
	asl   a
	tax
	lda   varDepth,x
	bne   tryit
	ldx   pidx128
	lda   ParentProc,x
	bra   folpid

tryit	anop

;
; Point to table
;
pointit	txa
	asl   a
	tax
	lda   varPtr,x
	sta   p
	lda   varPtr+2,x
	sta   p+2

	clc
	ldy   #4
	lda   [p],y
	adc   hashval
	sta   tbl
	iny2
	lda   [p],y
	adc   #0
	sta   tbl+2
;
; See if it's defined
;
	ldy   #2
	lda   [tbl],y
	sta   p+2
	lda   [tbl]
	sta   p

	mv4   tbl,q
;
; Check if end of list
;
loop	lda   p
	ora   p+2
	beq   done
;
; Point to this variable name
;
	ldy   #4
	lda   [p],y
	sta   thisname
	iny2
	lda   [p],y
	sta   thisname+2
;
; Do a string comparison
;
	pei   (name+2)
	pei   (name)
	pei   (thisname+2)
	pei   (thisname)
	jsl   cmpcstr
	cmp   #0
	beq   found
;
; follow link to next variable
;
next	mv4   p,q
	ldy   #2
	lda   [p]
	tax
	lda   [p],y
	sta   p+2
	stx   p
	bra   loop
;
; We found it, now unset it
;
found	ldy   #4+2               ;dispose the name
	lda   [p],y
	pha
	dey2
	lda   [p],y
	pha
	jsl   ~NDISPOSE
	ldy   #8+2               ;dispose the value
	lda   [p],y
	pha
	dey2
	lda   [p],y
	pha
	jsl   ~NDISPOSE
	ldy   #2                 ;move pointers to look around it
	lda   [p]
	sta   [q]
	lda   [p],y
	sta   [q],y
	pei   (p+2)              ;dispose the var
	pei   (p)
	jsl   ~NDISPOSE

done	return

	END

;=========================================================================
;
; Export a variable.
;
;=========================================================================

exportvar	START

	using VarData
	using KernelStruct

pidx128	equ   0
hashval	equ   pidx128+2
var	equ   hashval+2
tbl	equ   var+4
p	equ   tbl+4
space	equ   p+4

	subroutine (4:name,2:flag),space

	pei   (name+2)
	pei   (name)
	jsl   lowercstr
	pei   (name+2)
	pei   (name)
	jsl   hashvar
	sta   hashval

	lda   truepid
	asl   a
	tax
	lda   varDepth,x
	bne   pointit

	lda   truepid
folpid	anop
	asl2  a
	asl2  a
	asl2  a
	asl   a
	sta   pidx128
	tax
	lda   ParentProc,x       ;parent id
	jmi   done               ;can't do it
	asl   a
	tax
	lda   varDepth,x
	bne   tryit
	ldx   pidx128
	lda   ParentProc,x
	bra   folpid

tryit	anop

;
; Point to table
;
pointit	txa
	asl   a
	tax
	lda   varPtr,x
	sta   p
	lda   varPtr+2,x
	sta   p+2

	clc
	ldy   #4
	lda   [p],y
	adc   hashval
	sta   tbl
	iny2
	lda   [p],y
	adc   #0
	sta   tbl+2
;
; See if it's defined
;
	ldy   #2
	lda   [tbl],y
	pha
	lda   [tbl]
	pha
	pei   (name+2)
	pei   (name)
	jsl   followlocvar
	cmp   #0
	bne   found
	cpx   #0
	beq   done

found	sta   var
	stx   var+2
	ldy   #12
	lda   [var],y
	ora   flag
	sta   [var],y

done	return

	END

;=========================================================================
;
; Follows a linked list of local variables. Returns pointer to var or nil.
;
;=========================================================================

followlocvar	START

ch	equ   0
thisname	equ   ch+2
space	equ   thisname+4

	subroutine (4:p,4:varname),space
;
; Check if end of list
;
loop	lda   p
	ora   p+2
	beq   done
;
; Point to this variable name
;
	ldy   #4
	lda   [p],y
	sta   thisname
	iny2
	lda   [p],y
	sta   thisname+2
;
; Do a string comparison
;
	pei   (varname+2)
	pei   (varname)
	pei   (thisname+2)
	pei   (thisname)
	jsl   cmpcstr
	cmp   #0
	beq   found
;
; follow link to next variable
;
next	ldy   #2
	lda   [p]
	tax
	lda   [p],y
	sta   p+2
	stx   p
	bra   loop

done	stz   p                  ;we didn't find it
	stz   p+2
found	return 4:p         

	END

;=========================================================================
;
; Hash a variable
;
;=========================================================================

hashvar	PRIVATE

hashval	equ   0
space	equ   hashval+2

	subroutine (4:p),space

	lda   #11
	sta   hashval

	ldy   #0
loop	lda   hashval
	asl   a
	sta   hashval
	lda   [p],y
	and   #$FF
	beq   done
	clc
	adc   hashval
	sta   hashval
	iny
	bra   loop
done	UDivide (hashval,#VTABSIZE),(@a,hashval)

	asl2  a                  ;Make it an index.
	sta   hashval

	return 2:hashval

	END

;=========================================================================
;
; Variable Data        [How much does the data vary?] - TM
;                             (^^^ Oh, My Lord!)  - jb
;=========================================================================

VarData	DATA

varPtr	dc    32i4'0'
varDepth	dc    32i2'0'
	END
