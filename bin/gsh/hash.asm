**************************************************************************
*
* The GNO Shell Project
*
* Developed by:
*   Jawaid Bazyar
*   Tim Meekins
*
* $Id: hash.asm,v 1.4 1998/07/20 16:23:06 tribby Exp $
*
**************************************************************************
*
* HASH.ASM
*   By Tim Meekins & Greg Thompson
*
* Command hashing routines
*
* Note: text set up for tabs at col 16, 22, 41, 49, 57, 65
*              |     |                  |       |       |       |
*	^	^	^	^	^	^	
**************************************************************************
*
* Interfaces defined in this file:
*
* hash	jsr with params: (2:num, 4:name)
*
* dohash	subroutine (4:files)
*	return 4:table
*
* search	subroutine (4:file,4:table,4:paths)
*	return 4:full_path
*
* dispose_table subroutine (4:table)
*	 return
*
* free_files	subroutine (4:files)
*	return
* 
* dir_search	subroutine (4:dir,2:dirNum,4:files)
*	return
* 
* hashpath	jsl with no parameters
*	no returned value
*
* dispose_hash	jsr with no parameters
*	no returned value
*                                    
**************************************************************************

	mcopy /obj/gno/bin/gsh/hash.mac

dummyhash	start		; ends up in .root
	end

	setcom 60

C1             gequ  11
C2             gequ  13
TAB_MULT       gequ  4

;
; Structure for filenames
;
;	struct filenode {
;		short      dirnum;
;		char       name[32];
;		filenode  *next;
;		};
fn_dirNum      gequ  0
fn_name        gequ  fn_dirNum+2
fn_next        gequ  fn_name+32
fn_size        gequ  fn_next+4

;
; Structure for hash table
;
;	struct tablenode {
;		short      dirnum;
;		char      *name[32];
;		};
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
               iny2
               ora   [table],y
               beq   gotit

; If it's the same name, skip the duplicate entry

               ldy	h	Calculate address
	clc		 of hash entry's
	lda	[table],y	  name field.
	adc	#tn_name
	tax
	iny2
	lda	[table],y
	adc	#0
	pha		High-order word of address.
	phx		Low-order word of address.
	pei	(files+2)
	lda	files
	inc	a
	inc	a
	pha
	jsr	cmpcstr
	beq	mainloop

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
               stz   full_path	Set result to NULL.
               stz   full_path+2

	lda	table	If hash table hasn't
	ora	table+2	 been allocated,
               jeq   done	  return null string.

       	pei   (file+2)
               pei   (file)
               jsr   lowercstr
mainloop       pei   (file+2)	Get hash(qh,file)
               pei   (file)
               pei   (qh)
               jsr   hash
               asl  	a	Multiply by 4
	asl	a
               tay		Use as index into table.
               lda   [table],y	ptr = table[hash(qh,file)]
               sta   ptr
	tax
               iny
	iny
               ora   [table],y	If == 0,
	jeq	done	 all done.

          	lda   [table],y
               sta   ptr+2

               pei   (file+2)
               pei   (file)
               pha		;ptr+2
	inx		;ptr + #2
	inx
               phx
               jsr   cmpcstr	Compare filename against entry.
               beq   found
               inc   qh
               bra   mainloop

;
; Found an entry that matches the filename. Calculate full path.
;
found          lda   [ptr]
               asl	a
	asl	a
               adc   paths	;(cf=0)
               sta   ptr
               ldx   paths+2
               stx   ptr+2
               ldy   #2
               lda   [ptr],y
               pha
               lda   [ptr]
               pha
               jsr   cstrlen	Get length of path.
               pha
               clc
               adc   #33	Add 33 (max prog name size + 1)
               pea   0
               pha
               jsl   ~NEW	Allocate memory,
               sta   full_path	 storing address at
               stx   full_path+2	  functional return value.

               ldy   #2
               lda   [ptr],y
               pha
               lda   [ptr]
               pha
               pei   (full_path+2)
               pei   (full_path)
               jsr   copycstr	Copy pathname into buffer.

               pla                      ;length of path
               pei   (file+2)
               pei   (file)
               pei   (full_path+2)
               clc
               adc   full_path
               pha
               jsr   copycstr	Put filename at end of pathname.

done	ldx	full_path+2	Load return value into Y- & X- regs
	ldy	full_path

; Adjust stack in preparation for return
	lda	space
	sta	end-3
	lda	space+1
	sta	end-2
	pld
	tsc
	clc
	adc	#end-4
	tcs

	tya		A- & X- regs contain ptr (or NULL)

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
; Open directory name passed as 1st parameter
;
               ld2   3,ORec
               pei   (dir+2)            Turn "dir" c-string into
               pei   (dir)	 a GS/OS string, allocated
	jsr	c2gsstr	  via ~NEW.
               sta   ORecPath
               stx   ORecPath+2
	phx		Put GS/OS string addr on stack
	pha		 so it can be deallocated.
               Open  ORec	Open that file.
               bcc   goodopen	If there was an error,
               jsl   nullfree	 Free the GS/OS string
               jmp   exit	  and exit.

goodopen       jsl   nullfree	Free the GS/OS string.

;
; Set up parameter block for GetDirEntry
;
               mv2   ORecRef,DRecRef	Copy the file ref num from open.
               stz   DRecBase	Zero the base and
               stz   DRecDisp	 displacement.
	jsl	alloc256	Get 256 bytes for name.
               sta   DRecName	Store address in param block
               sta   ptr	 and also in direct page var.
               stx   DRecName+2
               stx   ptr+2
               lda   #254               Set total length of GS/OS buffer in
               sta   [ptr]	 bufsize word (save byte for 0 at end).
               GetDirEntry DRec	Make DirEntry call.

               mv2   DRecEntry,numEntries	Save number of entries.
               ld2   1,(DRecBase,DRecDisp)
               stz   entry		# processed entries = 0.

loop           lda   entry	If number of processed entries
               cmp   numEntries	 equals the total number,
               jge   done	  we are all done.
               GetDirEntry DRec

; Check for filetype $B3: GS/OS Application (S16)
               if2   DRecFileType,eq,#$B3,goodfile

; Check for filetype $B5: GS/OS Shell Application (EXE)
               if2   @a,eq,#$B5,goodfile

; Check for filetype $B0, subtype $0006: Shell command file (EXEC)
               cmp   #$B0
               bne   nextfile
               lda   DRecAuxType
               cmp   #$06
               bne   nextfile
               lda   DRecAuxType+2
               bne   nextfile
;
; This directory entry points to an executable file.
; Included it in the file list.
;
goodfile       inc	hash_numexe	Bump the (global) # files.
               ldy   #2	Get length word from GS/OS string
               lda   [ptr],y	 in result buffer.
               add2  @a,#4,@y	Use length + 4 as index
               lda   #0	 to store terminating
               sta   [ptr],y	  null byte.
               add2  ptr,#4,@a
               pei   (ptr+2)            ;for copycstr
               pha
               pei   (ptr+2)
               pha
               jsr   lowercstr	Convert name to lower case.

               ldy   #fn_next	temp = files->next.
               lda   [files],y
               sta   temp
               ldy	#fn_next+2
               lda   [files],y
               sta   temp+2

               ph4   #fn_size	temp2 = new entry.
               jsl   ~NEW
               sta   temp2
               stx   temp2+2

               ldy   #fn_next	files->next = temp2
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

               lda   dirNum	temp2->dirnum = dirNum
               sta   [temp2]	

               ldy   #fn_next           temp2->next = temp
               lda   temp
               sta   [temp2],y	
               ldy	#fn_next+2
               lda   temp+2
               sta   [temp2],y

nextfile       inc   entry	Bump entry number
               jmp   loop	 and stay in the loop.

;
; Done adding entries to the hash table from this directory
;
done           ldx	DRecName+2	Free the Name buffer.
	lda	DRecName
	jsl	free256

               ld2   1,ORec	ORec.pCount = 1
               Close ORec

exit           return


; Parameter block for GS/OS Open and Close calls
ORec           dc    i'3'	pCount (3 for Open, 1 for Close)
ORecRef        ds    2	refNum
ORecPath       ds    4	pathname (result buf)
ORecAccess     dc    i'1'               requested access = read

; Parameter block for GS/OS GetDirEntry call
DRec           dc    i'13'	pCount
DRecRef        ds    2	refNum
DRecFlag       ds    2	flags: extended/not
DRecBase       dc    i'0'	base: displacement is absolute entry #
DRecDisp       dc    i'0'	displacement: get tot # active entries
DRecName       ds    4	name: result buf
DRecEntry      ds    2	entryNum: entry # whose info is rtrned
DRecFileType   ds    2	fileType
DRecEOF        ds    4	eof: # bytes in data fork
DRecBlockCnt   ds    4	blockCount: # blocks in data fork
DRecCreate     ds    8	createDateTime
DRecMod        ds    8	modDateTime
DRecAccess     ds    2	access attribute
DRecAuxType    ds    4	auxType

               END

**************************************************************************
*
* Hash the path variable
*
**************************************************************************

hashpath       START

               using hashdata
	using	vardata

len            equ   1
pathnum        equ   len+2
ptr            equ   pathnum+2
files          equ   ptr+4
pathptr        equ   files+4
qflag	equ	pathptr+4
qptr	equ	qflag+2
gsosbuf        equ	qptr+4
space          equ	gsosbuf+4
end            equ   space+3

;
; Allocate space on stack for direct page variables
;
               tsc
               sec
               sbc   #space-1
               tcs
               phd
               tcd

	lock	hashmutex
;
; Allocate special file node
;
               ph4   #fn_size
               jsl   ~NEW
               sta   hash_files
               sta   files
               stx   hash_files+2
               stx   files+2

;
; Allocate memory for ExpandPath GS/OS result string
;
	jsl	alloc256
	sta	EPoutputPath
	stx	EPoutputPath+2
	sta	ptr
	stx	ptr+2
	lda	#254
	sta	[ptr]
;
; Initialize counters and pointers
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
; Determine length of $PATH environment variable string
;
	ph4	#pathname
	jsl	getenv
	sta	gsosbuf	Save address of allocated buffer.
	stx	gsosbuf+2
	ora	gsosbuf+2	If null,
	bne	setptr
	ldx	#^nopatherr		print error message
	lda	#nopatherr
	jsr	errputs
	jmp	noprint		  and exit.

setptr	clc		Add 4 bytes to
	lda	gsosbuf	 direct page pointer
	adc	#4	  to get the addr of
	sta	pathptr	   beginning of text.
	lda	gsosbuf+2
	adc	#0
	sta	pathptr+2

;
; Begin parsing $PATH
;
loop           lda   [pathptr]
               and   #$FF
               jeq   pathdone
;
; parse next pathname
;
	stz	qflag	Clear quote flag for this path

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

; Found "\"
gotquote	anop
	iny2
	ldx	qflag	If quote flag hasn't already been set,
	bne	despace
	sty	qflag	 set it to index of first "\" + 2.
	bra	despace

; Found null byte
gotspace0	tyx			Why put Y-reg in X???
	bra   gotspace3

; Found " ", tab, or creturn
gotspace1      tyx			Why put Y-reg in X???
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
               clc		Bump pathptr by
               tya		 the number of bytes
               adc   pathptr	  indicated in Y-reg.
               sta   pathptr
	lda	pathptr+2
	adc	#0
	sta	pathptr+2

	lda	pathnum
	cmp	#32*4
	bcc	numok
	ldx	#^toomanyerr
	lda	#toomanyerr
	jsr	errputs
	jmp	pathdone

;
; Convert c string to GS/OS string (allocating space for it)
;
numok	pei	(ptr+2)
	pei	(ptr)
	jsr	c2gsstr
	phx		Push allocated address onto
	pha		 stack for later deallocation.
	sta	EPinputPath	Save address in ExpandPath
	stx	EPinputPath+2	 parameter block.
;
; If any quoted characters were included, the "\" chars must be removed
;
	ldy	qflag	Get quote flag (index to "\" char).
	beq	xpandit	If no "\", go ahead with expansion.

	sta	qptr	Save EPinputPath pointer in
	stx	qptr+2	 direct page variable.
	lda	[qptr]	Store length + 2 (since we're indexing
	inc2	a	 from before length word) in qflag.
	sta	qflag
	tyx		X = index of 1st overwritten "\".
	short	a	Use 1-byte accumulator
;
; Copy characters toward front of string, removing "\" chars
;
chkloop2	lda	[qptr],y	Get next character.
	cmp	#'\'	If it's a quote,
	bne	storeit
	lda	[qptr]		Decrement length.
	dec	a
	sta	[qptr]
	iny			Skip over "\".
	lda	[qptr],y		Get character following.
storeit	phy		Push source index onto stack
	txy		 so destination index can be
	sta	[qptr],y	  used to store the character.
	ply		Restore the source index.
	inx		Bump destination and
	iny		 source index registers.
	cpy	qflag	If source index < length,
	bcc	chkloop2	 stay in copy loop.

	long	a	Restore long accumulator.

;
; Convert the input pathname into the corresponding
; full pathname with colons as separators.
;
xpandit	ExpandPath EPParm
	bcc	epok

	ldx	#^eperrstr	Print error message:
	lda	#eperrstr	 "Invalid pathname syntax."
	jsr	errputs
	jsl	nullfree	 Free GS/OS string (pushed earlier).
	jmp	next	 Get the next one.

epok           jsl	nullfree	Free GS/OS string (addr on stack)

	clc		Set ptr to GS/OS string
	lda	EPoutputPath	 portion of result buffer.
	adc	#2
	sta	ptr
	lda	EPoutputPath+2
	adc	#0
	sta	ptr+2

	lda	[ptr]	Get GS/OS string's length word
	sta	len	 and store in len.

	inc2	a	Store 0 at end of text
	tay		 in string.
	lda	#0	
	sta   [ptr],y

	pea	0	Allocate memory the
	phy		 size of the expanded path.
	jsl	~NEW

	pei	(ptr+2)
	inc2	ptr
	pei	(ptr)
	phx
	pha
	sta	ptr
	stx	ptr+2

               ldy   pathnum
               sta   hash_paths,y	Store address of this
               txa		 path's address in the
               sta   hash_paths+2,y	  hash path table.

               jsr   copycstr
               ldy   len
               beq   bumppnum
               dey
               lda   [ptr],y	If last character
               and   #$FF	 of path name
               cmp   #':'	  isn't ":",
               beq   bumppnum
               iny
               lda   #':'		store ":\0"
               sta   [ptr],y		 at end of string.

bumppnum	anop
               add2  pathnum,#4,pathnum	 Bump path pointer.
next           jmp   loop	Stay in loop.

;
; The $PATH entries have been created. Now we need to search each of the
; directories for executable files and add them to the "files" list.
; The earliest versions of gsh added files to the list in the order that
; directories appeared in $PATH, which put the earliest directories' files
; at the end of the list. Check for existence of $OLDPATHMODE environment
; variable to see if the user wants this, or would rather have them hashed
; in the expected order.
;
pathdone	anop
	lda	varoldpmode
	beq	neworder

;
; Search directories and add executables to file list starting at the
; beginning of $PATH and working to the end.
;
	stz	pathnum	Start at beginning of path table.

nextpath1	ldy	pathnum	Get offset into hash table.
	cpy	#32*4
	bcs	filesdone
	lda	hash_paths,y	If address of this path
	ora	hash_paths+2,y	 has not been set,
	beq	filesdone	  all done.
	lda   hash_paths,y	Get address of this
	ldx	hash_paths+2,y	 path's address in the
	phx		  hash path table.
	pha
	tya		Directory number =
	lsr2	a	 offset / 4
	pha
	pei	(files+2)	Pointer to file list.
	pei	(files)
	jsl	dir_search	Add executables from this directory.
               add2  pathnum,#4,pathnum	 Bump path offset.
	bra	nextpath1

;
; Search directories and add executables to file list starting at end
; of $PATH and working back to the beginning. (Note: Loop begins at
; "neworder", but structuring the code this ways saves an instruction.)
;
nextpath2	dey4		Decrement path offset.
	sty	pathnum
               lda   hash_paths,y	Get address of this
               ldx	hash_paths+2,y	 path's address in the
               phx   	  hash path table.
	pha
	tya		Directory number =
	lsr2	a	 offset / 4
	pha
	pei	(files+2)	Pointer to file list.
	pei	(files)
	jsl	dir_search	Add executables from this directory.
neworder	ldy   pathnum	Get offset into hash table.
	bne	nextpath2	When == 0, no more to do.


;
; Executable files in $PATH have been added to the list. Print
; number of files, then build the hash table.
;
filesdone	anop
	ph4	gsosbuf	Free memory allocated for
	jsl	nullfree	 $PATH string.

               lda   hash_print	If this is the first time,
               beq   noprint	 don't print the # of files.

               Int2Dec (hash_numexe,#hashnum,#3,#0)
	ldx	#^hashmsg
	lda	#hashmsg
	jsr	puts

noprint        ld2   1,hash_print	Set print flag.

;
; Free memory allocated for ExpandPath output string
;
	lda	EPoutputPath
	ldx	EPoutputPath+2
	jsl	free256

;
; Create the hash table from the file list.
;
               ph4   hash_files
               jsl   dohash
               sta   hash_table
               stx   hash_table+2

	unlock hashmutex
               
               pld
               tsc
               clc
               adc   #end-4
               tcs

               rtl

hashmutex	key		Mutual exclusion key

pathname	gsstr	'path'

hashmsg        dc    c'hashed '
hashnum        dc    c'000 files',h'0d00'

; Parameter block for GS/OS call ExpandPath
EPParm	dc	i'2'	pCount = 2
EPinputPath	ds	4	ptr to inputPath (GS/OS string)
EPoutputPath	ds	4	ptr to outputPath (Result buffer)

eperrstr	dc	c'rehash: Invalid pathname syntax.',h'0d00'
toomanyerr	dc	c'rehash: Too many paths specified.',h'0d00'
nopatherr	dc	c'rehash: PATH string is not set.',h'0d00'

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

               ldx   #32	32 different paths, maximum
               ldy   #0	Start looking at the first entry.

loop1          phx		Save path counter
               phy		 and index.
               lda   hash_paths+2,y	Put address for this
               pha		 path table entry on
               lda   hash_paths,y	  the stack.
               pha
               lda   #0	Zero out the table entry.
               sta   hash_paths+2,y
               sta   hash_paths,y
	jsl	nullfree	Free the entry's memory.
next1          ply		Restore path index
               plx		 and counter.
               iny4		Bump pointer to next address.
               dex		If more paths to process,
               bne   loop1	 stay in the loop.

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

t_size         ds    2	t_size = (TAB_MULT * numexe) - 1

hash_paths     dc    32i4'0'            32 paths max for now.
hash_files     dc    i4'0'
hash_table     dc    i4'0'	Pointer to table (t_size entries)
hash_numexe    dc    i2'0'	Number of hashed executables
hash_print     dc    i2'0'	Print flag; 0 first time through

               END
