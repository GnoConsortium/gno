*
* getvers version 2.0 for Apple IIGS
*
*  Original getvers written by Ian Schmidt (Copyright 1993 Two Meg Software)
*  Version 2.0 was created from a disassembly of getvers 1.2.
*   by Dave Tribby for GNO/ME 2.0.6  *  April 1998
*  See README.getvers for additional implementation notes.
*
* $Id
*

; Macro library
	MCOPY	getvers.mac

; Predefined labels:
GSOS     gequ  $E100A8

;
; Locations used on direct page
;
CmdLn        gequ  $00	00-03 Command line address
RezHndl      gequ  $04	04-07 rVersion resource handle
RezAddr      gequ  $08	08-0B rVersion resource address
rezFieldAddr gequ  $0C	0C-0F rVersion field address
AdrPStr      gequ  $10	10-13 pString addr for WritePString
CntryPStrAdr gequ  $14	14-17 addr of country pString


* -----------------------------------------------------------------

;
; Beginning of program
;
getvers  start GETVERS
         longa on
         longi on

; Handle the GS/OS startup protocol for shell text programs
         phk		Use program bank register
         plb		 to set data bank register.
         sty   CmdLn	Save address of
         stx   CmdLn+2	 command line.
; (Not used) sta   UserID	Save the user ID number.

; Setup for using stdout
         jsr   GetStdOutNum

; Start the Resource manager
         pha
         _ResourceStartUp

; Were parameters provided?
         ldy   #8	Start beyond shell identifier.
         jsr   GetNxtPrm	1st parameter is program name.
         stx   BufLen	
ChkPrm   lda   [CmdLn],y	Get next command line
         and   #$FF	 character.
         bne   GetParam	If not end-of-string, continue.

; No filename parameter: print the usage message and exit from the program
DoUsage  PH4   #UsageMsg
         jsr   WritePString

         PH4   #PStrNewLn
         jsr   WritePString

         lda	#1	Set return
	sta   rval	 status to 1.
;
; Shutdown and exit the program
;
AllDone  _ResourceShutDown	Close the resource manager.
         jsr   CloseStdout	Close stdout.
         lda	rval	Set status and
         rtl                             exit from program.


;
; Process program's filename parameters
;
GetParam	jsr   GetNxtPrm	Get filename
         stx   BufLen	Set dest length.
         sty	CmdIndx	Save cmd line pointer.

; Was it a valid option?
         lda   EndOptFlg	If end-of-options flag is set,
         bne   PRF	 process parameter as pathname.

         SHORT M	Use short mode to access chars.
         lda   Buffer	Get parameter.
         cmp   #'-'	If 1st character isn't '-',
         bne   FrstName	  it must be a filename.

         ldx   #1
GetPCh   lda   Buffer,x	Get option character.
         cmp   #'b'	If it's 'b',
         bne   ChkPc
         sta   bflag	    set bflag.
         bra   BumpIt
ChkPc    cmp   #'c'	If it's 'c',
         bne   ChkPf
         sta   cflag	    set cflag.
         bra   BumpIt
ChkPf    cmp   #'f'	If it's 'f',
         bne   ChkPq
         sta   fflag	    set fflag.
         bra   BumpIt
ChkPq    cmp   #'q'	If it's 'q',
         bne   BadParam
         sta   qflag	    set qflag.
BumpIt   inx		Increment index.
         cpx   BufLen	If not at end of buffer,
         bcc   GetPCh	 get the next parameter character.
         LONG  M	Otherwise, restore long mode
         ldy	CmdIndx	Restore cmd line pointer
         bra   ChkPrm	  and check if params are provided.

; First filename encountered
Frstname LONG  M	Restore long mode.
         inc   EndOptFlg	Set end-of-options flag.

; Try to open the file and print rVersion information
PRF      lda   bflag	Use "brief" flag for initial
         sta   no_sep	 setting of "no separator" flag.

         jsr   ProcessRezFile	Handle this file.

         ldy	CmdIndx	Restore cmd line pointer.

         lda   [CmdLn],y	Get next command line
         and   #$FF	 character.
         beq   AllDone	If end-of-string, all done.
         lda   no_sep	If "no separator" flag set,
         bne   GetParam
         PH4   #PStrNewLn	   print newline.
         jsr   WritePString
         bra   GetParam	Get next filename.

	
; A parameter was offered, but it's not "b", "c", "f", or "q".
BadParam sta   BadOpChr	Save character in error message.
         LONG  M	Restore long mode.
         PH4   #BadOption	Print "Illegal option"
         jsr   WritePString

         PH4   #PStrNewLn
         jsr   WritePString

         brl   DoUsage	Print usage and exit program.


;
; Subroutine to open and process the rez fork of the file named in Buffer
;
ProcessRezFile pha	Word for result.
         PH2   #1	Access mode = read.
         PH4   #0	Ptr to map in mem (0: load from disk).
         PH4   #BufLen	Filename (GS/OS string format).
         _OpenResourceFile	Open the resource fork.
         bcc   OpenedOK	If no error, go print the information.

         ply		Discard return value.

PrintErr cmp   #$0063	If error = $0063,
         beq   NoInfo	 handle as "no resource information".
         ldy   qflag	If -q option was specified,
         bne   ErRtn	 don't print error message.

         sta   ErrGS_error	Save error value.

         jsr   PrName	Print the filename.

         lda   ErrGS_error
         cmp   #$0046	If error = $0046,
         PH4   #E0046Msg	    print "File not found"
         jsr   WritePString
         PH4   #PStrNewLn
         jsr   WritePString

ErRtn    lda   #1	Set error status.
         sta   rval
WarnRtn  lda   qflag	If -q option was specified,
         ora   no_sep                    make sure no linefeed separator
         sta   no_sep	  is printed.
         rts		   and return from ProcessRezFile.
         
; Print a generic error message
Generr   jsl   >GSOS                       Shell:ErrorGS
         dc    i2'$0145'
         dc    a4'ErrorGSbuf'

         bra   ErRtn

;
; Resource file was opened without error
;
OpenedOK PL2   RezRefNum	Save rtn value (file ref num).

; Get file ID of resource file containing rVersion resource ID # 1

         pha		Space for result.
         PH2   #$8029	Resource type: rVersion
         PH4   #1	ID: 1
         _HomeResourceFile

         pla		If return value
         cmp   RezRefNum	 != loaded rez file number,
         beq   LoadVRez

NoInfo   ldy   qflag	  and -q option wasn't specified,
         bne   WarnRtn

         jsr   PrName	   print the filename,

         PH4   #NoInfoMsg	   print error message,
         jsr   WritePString
         PH4   #PStrNewLn
         jsr   WritePString

         bra   WarnRtn	Set err stat & rtn from ProcessRezFile.

;
; The resource exists in the target file. Load it.
;
LoadVRez pha		Reserve space for
         pha		 return value.
         PH2   #$8029	Resource type: rVersion
         PH4   #1	ID: 1
         _LoadResource
         bcc   GotRez	If error getting resource,

         plx		   Remove the
         plx		    returned value,
         brl   PrintErr	   Print error msg and exit.

;
; The rVersion resource was successfully loaded. Print its information.
;
GotRez   PL4	RezHndl	Rtn value = resource handle.

         ldy   #4	Make sure resource
         lda   [RezHndl],y	 doesn't move by
         ora   #$8000	  setting AttrLocked bit
         sta   [RezHndl],y	   in handle.

         lda   [RezHndl]	De-reference
         sta   RezAddr	 handle
         ldy   #2	  and
         lda   [RezHndl],y	   save
         sta   RezAddr+2	    address.

; Check for "print file name" mode
         lda   fflag	If -f option was specified,
         beq   CvtVrs
         jsr   PrName	  print the filename.

; Convert version field to pString using _VersionString
CvtVrs   pea   0	Flags (= 0).
         lda   [RezAddr],y	Version (long).
         pha	
         lda   [RezAddr]
         pha
         PH4   #VersPStr	Address of result.
         _VersionString

         ldy   #4
         lda   [RezAddr],y	Get country code.
         cmp   #55	If >= 55,
         bcc   shift
         lda   #30	  use 30 (an unknown country).
shift    asl   a	Multiply country code
         asl   a                         by 4
         tax		  and use as index.
         lda   CntryAddrTbl,x	Get address of pString
         sta   CntryPStrAdr	 of country name and
         lda   CntryAddrTbl+2,x           store in CntryPStrAdr.
         sta   CntryPStrAdr+2

         clc
         lda   RezAddr	Get rVersion address,
         adc   #6	 add 6
         sta   rezFieldAddr	  and store pointer
         lda   RezAddr+2                  to name of
         adc   #0	    product.
         sta   rezFieldAddr+2

; Print product name
         PH4   rezFieldAddr
         jsr   Wrt8bitPStr

; Print a single space
         PH4   #PstrSpace
         jsr   WritePString

; Print version number
         PH4   #VersPStr
         jsr   WritePString

; Print newline
         PH4   #PStrNewLn
         jsr   WritePString

; If "brief" option is set, skip moreInfo and Country.
         lda   bflag
         bne   ChkCflag

; Calculate address of moreInfo field
         lda   [rezFieldAddr]	Get product name's
         and   #$FF	  length byte.
         inc   a	Add one for length byte itself.
         clc
         adc   rezFieldAddr	Add to addr of product name.
         sta   rezFieldAddr	Store addr of moreInfo field.
         bcc   PrintMI
         inc   rezFieldAddr+2

; Print moreInfo field
PrintMI  PH4   rezFieldAddr
         jsr   Wrt8bitPStr

; Print newline
         PH4   #PStrNewLn
         jsr   WritePString


; Print "Country: "
         PH4   #CountryPStrK
         jsr   WritePString

; Print country name
         PH4   CntryPStrAdr
         jsr   WritePString

; Print newline
         PH4   #PStrNewLn
         jsr   WritePString

; If "comment" option is set, load and print Comment resource.
ChkCflag lda   cflag
         beq   CloseRez

; Get file ID of resource file containing rComment resource ID # 1

         pha		Space for result.
         PH2   #$802A	Resource type: rVersion
         PH4   #1	ID: 1
         _HomeResourceFile

         pla		If return value
         cmp   RezRefNum	 != loaded rez file number,
         bne   CloseRez                   skip it.

         pha		Reserve space for
         pha		 return value.
         PH2   #$802A	Resource type: rVersion
         PH4   #1	ID: 1
         _LoadResource
         bcs   CloseRez	If error, skip it.

;
; The rComment resource was successfully loaded. Print its information.
;
         PL4	RezHndl	Rtn value = resource handle.

         ldy   #4	Make sure resource
         lda   [RezHndl],y	 doesn't move by
         ora   #$8000	  setting AttrLocked bit
         sta   [RezHndl],y	   in handle.

         lda   [RezHndl]	De-reference
         sta   GSWriteAdr	 handle
         ldy   #2	  and
         lda   [RezHndl],y	   save
         sta   GSWriteAdr+2	    address.

         pha		Reserve space for
         pha                             result.
         PH4   RezHndl                  Get size of
         _GetHandleSize                  rComment resource.
         PL4   GSWriteLen	Save length.

         jsr   Wrt8bitStr	Convert and write comment string.

; Print newline
         PH4   #PStrNewLn
         jsr   WritePString
               

; Done with the resource file; close it
CloseRez PH2	RezRefNum	fileID
         _CloseResourceFile

Rtn      rts		Return from ProcessRezFile.


;
; Subroutine to print the pathname stored at Buffer (length in BufLen)
;
PrName   PH2   BufLen	   Write the pathname,
         PH4   #Buffer
         jsr   WriteBuf
         PH4   #PstrColTb	     a colon, and a tab.
         jsr   WritePString
         rts		Return from PrName.


;
; Subroutine to copy next word of input line into Buffer
; Upon entry: Y-reg = index into CmdLn (beginning of parameter)
; Upon exit:  Y-reg = index into CmdLn (beyond end of parameter)
;             A-reg = char beyond end: [CmdLn],y (0 or ' ')
;             X-reg = length of parameter
;
GetNxtPrm ldx  #0	Destination index and len = 0.
         SHORT M	Use short mode to access chars.

; Skip leading blanks.
SkipSp   lda   [CmdLn],y                Get next character.
         beq   EndParam	If null character, at end of string.
         cmp   #' '	If not blank,
         bne   SaveCh	  found start of parameter.
         iny		If blank, skip it
         bra   SkipSp	  and get the next character.

SaveCh   sta   Buffer,x	Save char in Buffer.
         iny		Increment source and
         inx		 destination pointers.
         lda   [CmdLn],y                Get next character.
         beq   EndParam	If null character, found end.
         cmp   #' '	If not a blank,
         bne   SaveCh	  keep getting characters.

; Found the end of parameter (or end of string)
EndParam LONG  M	Restore long mode.
         rts		Return from GetNxtPrm.


;
; Subroutine to ensure that stdout is open
;
GetStdOutNum jsl   >GSOS	GetStdRefNum
         dc    i2'$2037'
         dc    a4'GetSRefBlk'
         lda   GSRrefNum	Use current refnum, unless
         bcc   SvRefNum	 stdout isn't already open.
         jsl   >GSOS                       GSOpen "Console"
         dc    i2'$2010'
         dc    a4'OpenBlock'
         lda   OpenRefNum	    and use that refnum.
         inc   OpenFlag	    Set "open" flag.
SvRefNum sta   StdOutRefNum	Save stdout refnum.
         rts		Return from GetStdOutNum.


;
; Subroutine to close stdout
;
CloseStdout lda OpenFlag	If this program opened .CONSOLE
         beq   GoBack	 for use as stdout,
         lda   OpenRefNum
         sta   GSClose_ref
         jsl   >GSOS                       Close the file.
         dc    i2'$2014'
         dc    a4'GSClosePB'
GoBack   rts		Return from CloseStdout.


;
; Subroutine to write the Pascal string whose address is on the stack
;
WritePString plx	Hold the return address.
         PL4   AdrPStr	Get pString address.
         phx		Restore return address.

         clc		Text starts at one
         lda   AdrPStr	 byte beyond the
         adc   #1	  beginning of the
         sta   GSWriteAdr	   pString. Store
         lda   AdrPStr+2	    addr in GSWrite
         adc   #0	     Param Block.
         sta   GSWriteAdr+2

         lda   [AdrPStr]	Get pString length
         and   #$FF	 (byte).
         sta   GSWriteLen	Store in Param Block
         stz   GSWriteLen+2	 (long).

         jsl   >GSOS	GSWrite
         dc    i2'$2013'
         dc    a4'GSWritePB'
         rts		Return from WritePString.

;
; Subroutine to write the Pascal string whose address is on the stack
; and whose contents may contain 8-bit characters.
;
Wrt8bitPStr plx	Hold the return address.
         PL4   AdrPStr	Get pString address.
         phx		Restore return address.

         clc		Text starts at one
         lda   AdrPStr	 byte beyond the
         adc   #1	  beginning of the
         sta   GSWriteAdr	   pString. Store
         lda   AdrPStr+2	    addr in GSWrite
         adc   #0	     Param Block.
         sta   GSWriteAdr+2

         lda   [AdrPStr]	Store pString length
         and   #$FF	 in GSWrite Param Block.
         sta   GSWriteLen
;
; Alternate entry point to write 8-bit characters, with address
; and length already stored at GSWriteAdr and GSWriteLen.
;
Wrt8bitStr anop

; Reset Buffer to contain "$80" characters. These will be translated
; by _StringToText, so any that are left are not part of the translation.

         ldx   #254
         lda   #$8080	Value to be stored.
SetBuf   sta   Buffer,x	Save special value in buffer.
         dex		Decrement count
         dex		  by two.
	bpl   SetBuf	If x >= 0, stay in loop.

; Call _StringToText to convert 8-bit characters to printable
         pha		Reserve space for
         pha		 return values.
         PH2   #$5000	Flags: allow long subs; pass ctl chrs.
         PH4   GSWriteAdr               Pointer to source text.
         PH2   GSWriteLen	Source text length.
         PH4   #GSOSBuf	Pointer to destination.
         _StringToText	
         plx		Hold printable len in X-reg.
         stx   Printable
         pla
         sta   ResultFlags	Save results flags.

         PH4   #Buffer	Set GSWrite address
         PL4   GSWriteAdr                to the result buffer.

; _StringToText does not count non-printables (e.g. tab, carriage-return)
; in its printable length. Find first $80 in buffer to determine the true
; length of the string.

         SHORT M	Use short mode to access chars.
CkLen	cpx   #255	Make sure that
         beq   SvLen	 length <= 255.
         lda   Buffer,x	Get next byte.
	cmp   #$80	If it's the special value,
         beq	SvLen	  done looking.
         inx		Bump length
	bra	CkLen	 and read the next character.
SvLen    stx   GSWriteLen	Save length.
         LONG  M	Restore long mode.

	lda   ResultFlags	If any translations took place,
         bmi   ChkNP	 need to check for non-printables.
         cpx   Printable	If current length == # printable,
         beq   DoWrite                   don't need to check.

; Need to see if any of the non-printables need to be removed.

ChkNP    ldx	#0	Start looking at beginning of Buffer.
         SHORT M	Use short mode to access chars.
; Skip over leading printable characters.
GetAtStart lda Buffer,x	Get next character from Buffer.
         jsr   ChkChar	Is it printable?
         bcs   CopyLoop	No -- continue in copy loop.
         inx		Yes -- just bump index
         bra   GetAtStart	  and keep looking.

; Copy printable characters, but skip non-printable ones.
CopyLoop txy		Copy source index to destination index.

ChkEnd   cmp   #$80	If it's the special character,
         beq   CopyDone	 all done with checking.

NextCh	inx		Bump source index.
         lda   Buffer,x	Get next character from Buffer.
         jsr   ChkChar	Is it printable?
         bcs   ChkEnd	No -- see if we're at the end.
         sta   Buffer,y	Yes -- store in buffer
         iny		 and increment destination index.
         bra   NextCh

CopyDone LONG  M	Restore long mode.
         sty   GSWriteLen	Save true length.


DoWrite  jsl   >GSOS	GSWrite
         dc    i2'$2013'
         dc    a4'GSWritePB'

         rts		Return from Wrt8bitPStr/Wrt8bitStr.

; Variables used in subroutine.
Printable   dc i2'0'
ResultFlags dc i2'0'
               
;
; Subroutine called by Wrt8bit subroutines to determine whether the
; character in the accumulator is printable. Return carry flag to
; indicate the result: Set == non-printable; Clear == printable.
;
         longa off	Always called in SHORT M mode

ChkChar	cmp   #$80	If >= $80,
         bcs   ChkDone	  it's not printable.
         cmp   #$20	If < $80 && > 20,
	bcs   RtnClr	  it is printable.
         cmp   #$0D	If == carriage-return
	beq   RtnClr	  it is printable.
         cmp   #$09	If == tab
	beq   RtnClr	  it is printable.

         sec		If it is none of these,
         bra   ChkDone	  it is not printable.
                                                             
RtnClr   clc		Printable: clear carry flag.

ChkDone  rts		Return from ChkChar w/result in carry

         longa on

;
; Subroutine to write text; Addr and Len passed on stack
;
WriteBuf plx		Hold return address.
         PL4   GSWriteAdr	Move parameters into
         PL2   GSWriteLen	 GS Write
         stz   GSWriteLen+2	  Param Block.
         phx		Restore return address.
         jsl   >GSOS	GSWrite
         dc    i2'$2013'
         dc    a4'GSWritePB'
         rts		Return from WriteBuf.

; -------------------------------------------------------------------
;  Miscellaneous program constants and storage
; -------------------------------------------------------------------

; Usage message
UsageMsg dw    'usage: getvers [-b] [-c] [-f] [-q] file ...'

; Bad option error message (pString)
BadOption dc   i1'BadOpChr-BadOption'
         dc	c'illegal option -- '
BadOpChr dc    c' '
                    
; File open error pStrings
E0046Msg  dw   'File not found'
NoInfoMsg dw   'No version information'

; Indicates whether this program opened stdout
OpenFlag dc    i2'0'

; index into Command line
CmdIndx  dc    i2'0'

; ref num for resource file
RezRefNum dc   i2'0'

; ID number of program (Not used)
; UserID    dc   i2'0'

; Status value returned to shell
rval	dc	i2'0'

; Parameter flags
EndOptFlg dc   i2'0'	Has end-of-options been reached?
bflag    dc    i2'0'	Has -b option been specified?
cflag    dc    i2'0'	Has -c option been specified?
fflag    dc    i2'0'	Has -f option been specified?
qflag    dc    i2'0'	Has -q option been specified?

no_sep   dc    i2'0'	Is newline separator needed?

; Miscellaneous pString constants:
PstrColTb dc   h'023A09'	Colon and tab
PstrSpace dw   ' '	Space
PStrNewLn dc   h'010D'	Newline

; pString Version (filled in by call to SetVersion)
VersPStr dw    '          '

; Constants used for printing country information

CountryPStrK dw 'Country: '	Label

; Table of addresses for country pStrings
CntryAddrTbl dc a4'US,France,Britain,Germany,Italy,Netherlands,BelgiumLux'
         dc    a4'Sweden,Spain,Denmark,Portugal,FrCanada,Norway,Israel'
         dc    a4'Japan,Australia,Arabia,Finland,FrSwiss,GrSwiss,Greece'
         dc    a4'Iceland,Malta,Cyprus,Turkey,Yugoslavia'
         dc    a4'Unknown,Unknown,Unknown,Unknown,Unknown,Unknown,Unknown'
         dc    a4'India,Pakistan'
         dc    a4'Unknown,Unknown,Unknown,Unknown,Unknown,Unknown'
         dc    a4'Lithuania,Poland,Hungary,Estonia,Latvia,Lapland'
         dc    a4'FaeroeIsl,Iran,Russia,Ireland,Korea,China,Taiwan'
         dc    a4'Thailand'
; Message printed when country is not valid
Unknown  dw    'unknown to this version of getvers'

; pStrings for the various countries
US	dw    'United States'
France   dw    'France'
Britain  dw    'Britain'
Germany  dw    'Germany'
Italy    dw    'Italy'
Netherlands dw 'Netherlands'
BelgiumLux dw  'Belgium/Luxembourg'
Sweden   dw    'Sweden'
Spain    dw    'Spain'
Denmark  dw    'Denmark'
Portugal dw    'Portugal'
FrCanada dw    'French Canadian'
Norway   dw    'Norway'
Israel   dw    'Israel'
Japan    dw    'Japan'
Australia dw   'Australia'
Arabia   dw    'Arabia'
Finland  dw    'Finland'
FrSwiss  dw    'French Swiss'
GrSwiss  dw    'German Swiss'
Greece   dw    'Greece'
Iceland  dw    'Iceland'
Malta    dw    'Malta'
Cyprus   dw    'Cyprus'
Turkey   dw    'Turkey'
Yugoslavia dw  'Bosnia/Herzegovena/Yugoslavia/Croatia'
India	dw	'India'
Pakistan	dw	'Pakistan'
Lithuania dw	'Lithuania'
Poland	dw	'Poland'
Hungary	dw	'Hungary'
Estonia	dw	'Estonia'
Latvia	dw	'Latvia'
Lapland	dw	'Lapland'
FaeroeIsl dw	'Faeroe Islands'
Iran	dw	'Iran'
Russia	dw	'Russia'
Ireland  dw    'Ireland'
Korea    dw    'Korea'
China    dw    'China'
Taiwan   dw    'Taiwan'
Thailand dw    'Thailand'


; GS/OS input/result string: two length words followed by 256 bytes
GSOSBuf  anop
BufSize  dc    i2'260'	Total size (when used as result buf)
BufLen   dc    i2'0'                    Num chars used in buffer
Buffer   ds    256	Storage area (256 bytes)
	dc	h'80'	Special character, flags end of buffer

; Parameter Block used for ErrorGS
ErrorGSbuf  dc i2'1'	Number of parameters
ErrGS_error dc i2'0'	Error code

; Parameter block for  GSOpen of .CONSOLE
OpenBlock  dc  i2'4'	pCount
OpenRefNum dc  i2'0'	refNum
OpenPath   dc  i4'ConName'	pathname
OpenAccess dc  i2'2'	requestAccess (write)
OpenRNum   dc  i2'0'	resourceNumber (open data fork)

; ".CONSOLE" as a GS/OS input string
ConName    dc  i2'8'
           dc  c'.CONSOLE'

; GetStdRefNum parameter block
GetSRefBlk  dc i2'2'	pCount
GSRpfixNum  dc i2'11'	prefixNum
GSRrefNum   dc i2'0'	refNum (returned)

; Parameter block for GSClose
GSClosePB   dc i2'1'	pCount
GSClose_ref dc i2'0'	refNum

; Parameter block used for GSWrite in WriteBuf and WritePString
GSWritePB    dc i2'4'	pCount
StdOutRefNum dc i2'0'	device number
GSWriteAdr   dc a4'0'	buffer
GSWriteLen   dc i4'0'	request count
             dc i4'0'	starting block

         end

; -----------------------------------------------------------------

; Segment for direct-page and stack

stack    data  STACK
         kind  $12

; Fill the direct-page/stack pages with question marks,
; so they can be examined for use during execution.

         dc    128c'?'
         dc    128c'?'

         dc    128c'?'
         dc    128c'?'

         end
