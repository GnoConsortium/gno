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
* HASH.ASM
*   By Tim Meekins & Greg Thompson
*
* Command hashing routines
*
**************************************************************************

               keep  o/hash
               mcopy m/hash.mac

C1             gequ  11
C2             gequ  13
TAB_MULT       gequ  4
;
; Structure for filenames
;
fn_dirNum      gequ  0
fn_name        gequ  fn_dirNum+2
fn_next        gequ  fn_name+32
fn_size        gequ  fn_next+4
;
; Structure for hash table
;
tn_dirNum      gequ  0
tn_name        gequ  tn_dirNum+2
tn_size        gequ  tn_name+32

**************************************************************************
*
* Calculate hash value for a filename
*
**************************************************************************

hash           START

               using hashdata

space          equ   1
num            equ   space+2
name           equ   num+2
end            equ   name+4

               tsc
               phd
               tcd

               lda   num
               bne   hasher

               stz   h
               ldy   #0
loop           lda   [name],y
               and   #$FF
               beq   hasher
               sta	addit+1
               lda   h	;left shift 7
	xba
	and	#$FF00
	lsr	a
addit          adc   #0	;(cf=0)
               phy
               UDivide (@a,t_size),(@a,@a)
               sta   h
               ply
               iny
               bra   loop

hasher         lda   num	;num*num
	sta	tmp
	lda	#0
	ldx	#16
mulloop	asl	a
               asl   tmp
	bcc	nomul
	clc
	adc	num
nomul	dex
	bne	mulloop
		
	pha		;Acc * C2
	asl	a
	asl	a
	sec
	sbc	1,s
	asl	a
	asl	a
	adc	1,s
	sta	1,s

	lda	num	;num*C1 + (Acc*C2) + h
	asl	a
	adc	num
	asl	a
	asl	a
               adc   1,s
               adc   h
	sec
	sbc	num
               plx
               UDivide (@a,t_size),(@a,@y)

               lda   space
               sta   end-2
               pld
               tsc
               clc
               adc   #end-3
               tcs

               tya
               rts

h              ds    2
tmp	ds	2

               END

**************************************************************************
*
* dohash
*
**************************************************************************

dohash         START

               using hashdata

h              equ   1
temp           equ   h+2
qh             equ   temp+4
table          equ   qh+2
space          equ   table+4
files	equ	space+3
end	equ	files+4

;               subroutine (4:files),space

	tsc
	sec
	sbc	#space-1
	tcs
	phd
	tcd

               lda   hash_numexe
	bne	mktsize

	stz	table
	stz	table+2
	jmp	done
;
; t_size = (TAB_MULT * numexe) - 1
;     [Shift since TAB_MULT is 4, change later if needed]
;
mktsize        asl	a
	asl	a
               dec   a
               sta   t_size
;
; table = (tablenode **)malloc(sizeof(tablenode *) * t_size);
;
	inc	a	;safety precaution
	asl	a
	asl	a
               pea   0
               pha
               jsl   ~NEW
               sta   table
               stx   table+2
;
; for (i=0; i < t_size; ++i) table[i] = NULL;
;
               ldy   #0
               ldx   t_size
	tya
clrtbl         sta   [table],y
               iny
	iny
               sta   [table],y
               iny
	iny
               dex
               bne   clrtbl
;
; files = files->next
;
mainloop       ldy   #fn_next
               lda   [files],y
               tax
               ldy	#fn_next+2
               lda   [files],y
               sta   files+2
               stx   files
;
; while (files != NULL) {
;
               ora   files
               jeq   done
               stz   qh
;
; while (table[h = hash(files->name, qh))]) { ++qh; ++colls; }
;
hashloop       pei   (files+2)
               lda   files
	inc	a
	inc	a
               pha
               pei   (qh)
               jsr   hash
               asl  	a
	asl	a
               sta   h
               tay
               lda   [table],y
	tax
               iny
	iny
               ora   [table],y
               beq   gotit

; let's see if it's the same, skip if so...

;	pei	(files+2)
;	lda	files
;	inc	a
;	inc	a
;	pha
;	lda	[table],y
;	pha
;	phx
;	jsr	cmpcstr
;	beq	mainloop

               inc   qh
               bra   hashloop
;
; table[h] = (tablenode *)malloc(sizeof(tablenode))
;
gotit          ph4   #tn_size
               jsl   ~NEW
               sta   temp
               stx   temp+2
               ldy   h
               sta   [table],y
               iny
	iny
               txa
               sta   [table],y
;
; table[h]->dirnum = files->dirNum
;
               lda   [files]
               sta   [temp]
;
; strcpy(table[h]->name, files->name);
;
               pei   (files+2)
               lda   files
	inc	a
	inc	a
               pha
               pei   (temp+2)
               lda   temp
	inc	a
	inc	a
               pha
               jsr   copycstr
               jmp   mainloop

done	anop
;	return 4:table

	ldx	table+2
	ldy	table

	lda	space
	sta	end-3
	lda	space+1
	sta	end-2
	pld
	tsc
	clc
	adc	#end-4
	tcs
	
	tya
	rtl

               END

**************************************************************************
*
* Search the hash table
*
**************************************************************************

search         START

ptr            equ   1
full_path      equ   ptr+4
qh             equ   full_path+4
space          equ   qh+2
paths	equ	space+3
table	equ	paths+4
file	equ	table+4
end	equ	file+4

;               subroutine (4:file,4:table,4:paths),space

	tsc
	sec
	sbc	#space-1
	tcs
	phd
	tcd

               stz   qh
               stz   full_path
               stz   full_path+2

	lda	table
	ora	table+2
               jeq   done

       	pei   (file+2)
               pei   (file)
               jsr   lowercstr
mainloop       pei   (file+2)
               pei   (file)
               pei   (qh)
               jsr   hash
               asl  	a
	asl	a
               tay
               lda   [table],y
               sta   ptr
	tax
               iny
	iny
               ora   [table],y
	jeq	done
          	lda   [table],y
               sta   ptr+2
               pei   (file+2)
               pei   (file)
               pha		;ptr+2
	inx		;ptr + #2
	inx
               phx
               jsr   cmpcstr
               beq   found
               inc   qh
               bra   mainloop

found          lda   [ptr]
               asl	a
	asl	a
               ldx   paths+2
               adc   paths	;(cf=0)
               stx   ptr+2
               sta   ptr
               ldy   #2
               lda   [ptr],y
               pha
               lda   [ptr]
               pha
               jsr   cstrlen
               pha
               clc
               adc   #33
               pea   0
               pha
               jsl   ~NEW
               sta   full_path
               stx   full_path+2
               ldy   #2
               lda   [ptr],y
               pha
               lda   [ptr]
               pha
               pei   (full_path+2)
               pei   (full_path)
               jsr   copycstr
               pla                      ;length of path
               pei   (file+2)
               pei   (file)
               pei   (full_path+2)
               clc
               adc   full_path
               pha
               jsr   copycstr

done	ldx	full_path+2
	ldy	full_path
	lda	space
	sta	end-3
	lda	space+1
	sta	end-2
	pld
	tsc
	clc
	adc	#end-4
	tcs

	tya
	rtl

               END

**************************************************************************
*
* Dispose the hash table
*
**************************************************************************

dispose_table  START

               using hashdata

ptr            equ   0
count          equ   ptr+4
space          equ   count+2

               subroutine (4:table),space

               mv4   table,ptr
               mv2   t_size,count
loop           ldy   #2
               lda   [ptr],y
               pha
               lda   [ptr]
               pha
               jsl   nullfree
           	add2  ptr,#4,ptr
               dec   count
               bne   loop

               pei   (table+2)
               pei   (table)
               jsl   nullfree

               return

               END

**************************************************************************
*
* Dispose the file table
*
**************************************************************************

free_files     START

space          equ   0

               subroutine (4:files),space

loop           ora2  files,files+2,@a
               beq   done
               ldy   #fn_next
               lda   [files],y
               tax
               ldy	#fn_next+2
               lda   [files],y
               pei   (files+2)
               pei   (files)
	stx	files
	sta	files+2
               jsl   nullfree
               bra   loop

done           return

               END

**************************************************************************
*
* Directory search
*
**************************************************************************

dir_search     START

	using	hashdata

temp2          equ   0
temp           equ   temp2+4
entry          equ   temp+4
numEntries     equ   entry+2
ptr            equ   numEntries+2
space          equ   ptr+4

               subroutine (4:dir,2:dirNum,4:files),space
;
; Open directory
;
               ld2   3,ORec
               pei   (dir+2)            ;copy this string
               pei   (dir)
	jsr	c2gsstr
               sta   ORecPath
               stx   ORecPath+2
	phx
	pha
               Open  ORec
               bcc   goodopen
               jsl   nullfree
               jmp   exit

goodopen       jsl   nullfree
               mv2   ORecRef,DRecRef
               stz   DRecBase
               stz   DRecDisp
	jsl	alloc256
               sta   DRecName
               sta   ptr
               stx   DRecName+2
               stx   ptr+2
               lda   #254               ;Output buffer size (GT never did this?)
               sta   [ptr]
               GetDirEntry DRec
               mv2   DRecEntry,numEntries
               ld2   1,(DRecBase,DRecDisp)
               stz   entry

loop           lda   entry
               cmp   numEntries
               jge   done
               GetDirEntry DRec
               if2   DRecFileType,eq,#$B3,goodfile
               if2   @a,eq,#$B5,goodfile
               cmp   #$B0
               jne   nextfile
               lda   DRecAuxType
               cmp   #$06
               jne   nextfile
               lda   DRecAuxType+2
               jne   nextfile
goodfile       inc	hash_numexe
               ldy   #2
               lda   [ptr],y
               add2  @a,#4,@y
               lda   #0
               sta   [ptr],y
               add2  ptr,#4,@a
               pei   (ptr+2)            ;for copycstr
               pha
               pei   (ptr+2)
               pha
               jsr   lowercstr

               ldy   #fn_next
               lda   [files],y
               sta   temp
               ldy	#fn_next+2
               lda   [files],y
               sta   temp+2
               ph4   #fn_size
               jsl   ~NEW
               sta   temp2
               stx   temp2+2
               ldy   #fn_next
               sta   [files],y
               ldy	#fn_next+2
	txa
               pha
               sta   [files],y
               lda   temp2
               clc
               adc   #fn_name
               pha
               jsr   copycstr
               lda   dirNum
               sta   [temp2]
               ldy   #fn_next
               lda   temp
               sta   [temp2],y
               ldy	#fn_next+2
               lda   temp+2
               sta   [temp2],y

nextfile       inc   entry
               jmp   loop

done           ldx	DRecName+2
	lda	DRecName
	jsl	free256
               ld2   1,ORec
               Close ORec

exit           return

ORec           dc    i'3'
ORecRef        ds    2
ORecPath       ds    4
ORecAccess     dc    i'1'               ;read

DRec           dc    i'13'
DRecRef        ds    2
DRecFlag       ds    2
DRecBase       dc    i'0'
DRecDisp       dc    i'0'
DRecName       ds    4
DRecEntry      ds    2
DRecFileType   ds    2
DRecEOF        ds    4
DRecBlockCnt   ds    4
DRecCreate     ds    8
DRecMod        ds    8
DRecAccess     ds    2
DRecAuxType    ds    4

               END

**************************************************************************
*
* Hash the path variable
*
**************************************************************************

hashpath       START

               using hashdata

len            equ   1
pathnum        equ   len+2
ptr            equ   pathnum+2
files          equ   ptr+4
pathptr        equ   files+4
space          equ   pathptr+4
end            equ   space+3

               tsc
               sec
               sbc   #space-1
               tcs
               phd
               tcd
;
; allocate special file node
;
               ph4   #fn_size
               jsl   ~NEW
               sta   hash_files
               sta   files
               stx   hash_files+2
               stx   files+2

	jsl	alloc256
	sta	EPParm+6
	stx	EPParm+6+2
	sta	ptr
	stx	ptr+2
	lda	#254
	sta	[ptr]
;
; initialize counters and pointers
;
               lda   #0
               sta   hash_numexe
               sta   pathnum
               ldy   #fn_next
               sta   [files],y
               ldy	#fn_next+2
               sta   [files],y
               ldy   #fn_name
               sta   [files],y
               sta   [files]
;
; allocate memory for $path variable
;
	jsl	alloc256
               sta   pathparm+4
               stx   pathparm+4+2
	phx
	pha
	phx
	pha
;
; read $PATH
;
               Read_Variable pathparm

	jsr	p2cstr
	stx	pathptr+2
	sta	pathptr
	stx	pathparm+6	;for disposal only
	sta	pathparm+4
	pla
	plx
	jsl	free256	;pushed earlier
;
; begin parsing $path
;
loop           lda   [pathptr]
               and   #$FF
               jeq   done
;
; parse next pathname
;
               mv4   pathptr,ptr
               ldy   #0
despace        lda   [pathptr],y
               and   #$FF
               beq   gotspace0    
               if2   @a,eq,#' ',gotspace1
               if2   @a,eq,#009,gotspace1
               if2   @a,eq,#013,gotspace1
	if2	@a,eq,#'\',gotquote
               iny
               bra   despace
gotquote	iny2
	bra	despace
gotspace0	tyx
	bra   gotspace3
gotspace1      tyx
               short a
               lda   #0
               sta   [pathptr],y
               long  a
gotspace2      iny
               lda   [pathptr],y
               and   #$FF
               if2   @a,eq,#' ',gotspace2
               if2   @a,eq,#009,gotspace2
               if2   @a,eq,#013,gotspace2
gotspace3      anop
               clc
               tya
               adc   pathptr
               sta   pathptr

	lda	pathnum
	cmp	#32*4
	bcc	numok
	ldx	#^toomanyerr
	lda	#toomanyerr
	jsr	errputs
	jmp	done

numok	pei	(ptr+2)
	pei	(ptr)
	jsr	c2gsstr
	phx
	pha	
	sta	EPParm+2
	stx	EPParm+4
	ExpandPath EPParm
	bcc	epok

	ldx	#^eperrstr
	lda	#eperrstr
	jsr	errputs
	jsl	nullfree
	jmp	next

epok           jsl	nullfree

	lda	EPParm+6+2
	sta	ptr+2
	lda	EPParm+6
	inc2	a
	sta	ptr
	lda	[ptr]
	sta	len
	inc2	a
	tay
	lda	#0	
	sta   [ptr],y
	pea	0
	phy
	jsl	~NEW
	phx		;for dir_search
	pha
	pei	(ptr+2)
	inc2	ptr
	pei	(ptr)
	phx
	pha
	sta	ptr
	stx	ptr+2
               ldy   pathnum
               sta   hash_paths,y
               txa
               sta   hash_paths+2,y
               jsr   copycstr
               ldy   len
               beq   go4it
               dey
               lda   [ptr],y
               and   #$FF
               cmp   #':'
               beq   go4it
               iny
               lda   #':'
               sta   [ptr],y
               iny
               lda   #0
               sta   [ptr],y

go4it          lda   pathnum
               lsr2  a
               pha
               pei   (files+2)
               pei   (files)
               jsl   dir_search
               add2  pathnum,#4,pathnum

next           jmp   loop

done           ph4   pathparm+4
               jsl   nullfree

               lda   hash_print
               beq   noprint

               Int2Dec (hash_numexe,#hashnum,#3,#0)
	ldx	#^hashmsg
	lda	#hashmsg
	jsr	puts

noprint        ld2   1,hash_print

               ph4   hash_files
               jsl   dohash
               sta   hash_table
               stx   hash_table+2

	lda	EPParm+6
	ldx	EPParm+6+2
	jsl	free256

               pld
               tsc
               clc
               adc   #end-4
               tcs

               rtl

pathparm       dc    a4'pathvar'
               ds    4
pathvar        str   'path'

hashmsg        dc    c'hashed '
hashnum        dc    c'000 files',h'0d00'

EPParm	dc	i'2'
	ds	4
	ds	4

eperrstr	dc	c'rehash: Invalid pathname syntax.',h'0d00'
toomanyerr	dc	c'rehash: Too many paths specified.',h'0d00'

               END

**************************************************************************
*
* Dispose of hashing tables
*
**************************************************************************

dispose_hash   START

               using hashdata

	ora2	hash_table,hash_table+2,@a
	beq	done

               ldx   #32
               ldy   #0
loop1          phx
               phy
               lda   hash_paths+2,y
               pha
               lda   hash_paths,y
               pha
               lda   #0
               sta   hash_paths+2,y
               sta   hash_paths,y
	jsl	nullfree
next1          ply
               plx
               iny4
               dex
               bne   loop1

               ph4   hash_files
               jsl   free_files
               stz   hash_files
               stz   hash_files+2

               ph4   hash_table
               jsl   dispose_table
               stz   hash_table
               stz   hash_table+2

done           rts

               END

**************************************************************************
*
* Hash data
*
**************************************************************************

hashdata       DATA

t_size         ds    2

hash_paths     dc    32i4'0'            ;32 paths max for now.
hash_files     dc    i4'0'
hash_table     dc    i4'0'
hash_numexe    dc    i2'0'
hash_print     dc    i2'0'

               END
