
;***********************************************************************
;*
;*  Copyright (C) Kopriha Software, 1990 - 1991
;*  All rights reserved.
;*  Licensed material -- property of Kopriha Software
;*
;*  This software is made available solely pursuant to the terms
;*  of a Kopriha Software license agreement which governs its use.
;*
;*
;* File Printer Port Driver
;*
;*
;* Description:
;*  The goal of this GS port driver is to allow the user of any printer
;*  driver to save their printer driver output in a user specified file.
;*
;*  The user interface includes a standard file dialog box of where to
;*  save the printer driver output.
;*
;*  If this port driver encounters any error, a dialog box will be
;*  displayed with the error code and a message saying that the printer
;*  output could not be saved.  The user will have two buttons -
;*  DELETE_OUTPUT and OK.  Delete output will simply delete the output
;*  file and act as an OK button.
;*
;*  Rational for this driver: I want to be able to 'print' a file on
;*  my GS, bring the file to work and print the file on a printer here.
;*  (An additional utility (hopefully a CDEV) will do the inverse of
;*  what this driver does - it will put up a dialog box to get the user
;*  to select a printer data file to to printed.  Lets get this to work
;*  first).
;*
;*
;* Notes:
;*  This printer port driver is complies to the version 3.0 printer
;*  driver specs.  See IIGS technotes 35 (dated March 1990) and
;*  technote 36 (dated September 1989) for the printer/port driver
;*  specificiations.
;*
;*  The resulting port driver must be given filetype $BB 
;*  auxiliary type $02 (IE: Port driver for local printer).
;*
;*  The entire file port driver is designed to fit within one 64K 
;*  segment/bank of memory.  If it must span more than one memory
;*  bank, then you will have to correct the dispatch routine and
;*  possibly some references to the internal data structures.
;*
;*  Further References:
;*    Apple IIGS Toolbox Reference, Volumes 1 & 2
;*    Apple IIGS Technical Note #35, Printer Driver Specifications
;*    Apple IIGS Technical Note #36, Port Driver Specifications
;*
;*
;* History: Nov 11 1990 Dave   Began coding this port driver.
;*                             (started with IIGS technotes 35 & 36)
;*
;*          Jan 19 1991 Dave   Got this code compiled and attempted
;*                             the first couple of tests... results
;*                             are that the calling convention isn't
;*                             what I expected (Orca/C...) - technote
;*                             #35 confirmed my guess... time for
;*                             some macros to replace csubroutine &
;*                             creturn.
;*
;*          Jan 20 1991 Dave   Changed all routines to use the new
;*                             set of port_subroutine/return/complete
;*                             macros.
;*                             Now its time to check the macros and
;*                             after that try the driver again...
;*
;*          Jan 21 1991 Dave   Changed return macro to include the
;*                             completion routine - this was after
;*                             hours of looking at the 'Printer' port
;*                             driver.
;*
;*                             This revision caused 'Printer
;*                             Communication Error's from the
;*                             Imagewriter printer driver.
;*
;*                             The epson printer driver was happy...
;*
;*          Jan 31 1991 Dave   After a trip into the Orca/Disassembler
;*                             to peek into the port drivers from
;*                             Bill Heinman to see the return procedure
;*                             from all the entry points, now we have
;*                             another port_return macro (named
;*                             port_return2).
;*
;*                             Another 'Printer Communication Error'...
;*
;*          Feb 3 1991 Dave    Well... that didn't work out too well.
;*                             Besides, I didn't really like the second
;*                             return macro.
;*
;*                             The second macro is gone now and I've
;*                             changed the return macro so that it now
;*                             JSL's to the completion routine and
;*                             the RTL was removed from the macro.
;*                             This allows me to add the CMP #1 for
;*                             the routines that need it (like lots
;*                             of them, but not all of them...) and
;*                             then you have to pop an RTL in the code.
;*
;*                             With these changes, I have gotten the
;*                             Imagewriter driver to complete (and
;*                             now I have to test the rest, once I
;*                             finish with the comments here and in
;*                             the macro source...)
;*
;*                             This version works correctly with the
;*                             Imagewriter and Epson printer drivers.
;*                             The Imagewriter LQ driver returns a
;*                             'Printer not Imagewriter LQ, not ready
;*                             or out of paper.' (cancel) dialog.
;*                             The Laserwriter (renamed) creates a
;*                             blank dialog box (no message) then hangs.
;*
;*          Feb 3 1991 Dave    I added the calls to the completion
;*                             routines for the async_read & write
;*                             background calls and a set of counters
;*                             for each port call.
;*
;*                             Results same as before...
;*
;*          Feb 12 1991 Dave   Well, after debugging to find out that
;*                             the ImageWriter LQ problem is related
;*                             to the information read from the printer
;*                             it was time to disassemble the code until
;*                             I found out what we can do about it.
;*
;*                             The IWLQ driver puts up the status dialog
;*                             after the following calls:
;*                               Open (no problem here)
;*                               Write
;*                                 6 bytes ($1b,$5a,$00,$02,$1b,$3f)
;*                               Read
;*                                 8 bytes (we returned zero)
;*                               Write
;*                                 8 bytes (All $ff...)
;*                               Status
;*                                 Bits checked $87 - one at a time
;*                               Close (no problem here)
;*
;*                             Now after disassembling the IWLQ
;*                             driver, I found that the string to
;*                             return on the Read is as follows:
;*                               5 to 8 bytes returned
;*                               string: $4c,$51,$31,<don't care>,
;*                                       $XX,$XX,$XX
;*                               Where XX will cause a bit to get set:
;*                                $45 - bit $4  Multiple bins
;*                                $46 - bit $4  Multiple bins
;*                                $43 - bit $2  Color
;*
;*                             I've added this...
;*
;*          Feb 13 1991 Dave   Problem is that the same dialog came
;*                             up - after some debugging I found that
;*                             we are returning the bytes in the
;*                             wrong order (I fixed them to see it
;*                             work!)
;*
;*                             I tried the beta version of the DeskJet
;*                             printer drivers and they worked like a
;*                             charm!
;*
;*
;*                             Since I got the ImWr LQ printer driver
;*                             to work, it seemed like a good time to
;*                             generalize the printer identification
;*                             routines.  I've also defined what to
;*                             expect/return from/to the ImageWriter LQ
;*                             and the ImageWriter printer drivers.
;*
;*
;*                             At this point in time the NULL port
;*                             driver is complete (YA HOO!)!!!
;*
;*
;***********************************************************************

         case  on                       Set case sensitivity in assembler
         objcase on                     Set case sensitivity in object

         gen off                        List generated code
         trace off                      List all macro generation

         mcopy port.macros              Include the port driver macros

TRUE     gequ  1                        Define TRUE and FALSE
FALSE    gequ  0

PT_Unknown       gequ 0                 Define the special types
PT_ImageWriter   gequ 4                  of printers (These printers
PT_ImageWriterLQ gequ 8                  respond to a read after being
;                                        sent a ID/status command)

;***********************************************************************
;*
StartOfDriver start
;*
;* Description:
;*  Dispatch to each of the port driver support drivers.
;*
;* Note:
;*  This routine must be the first in the port driver - the first two
;*  words are used to determine what version port driver specification
;*  is installed.
;*
;* History:
;*  Most of this routine was provided by GS technote # 35.
;*
;***********************************************************************

             using PortData

; Setup the header of the driver (as per the technote)
 
              dc  i2'0'                    ; version 3.0 style driver
              dc  i2'12'                   ; Supported # of port calls


; Set the data bank to our own, saving the previous one for later

              phb                          ; Save data bank
              phk
              plb                          ; Set data bank to our own


; Increment a counter of port calls.  This counter is used to
;  determine the printer driver type (after an open, there is
;  usually a write, followed by a read).  If we can return the
;  right string on the read then all of the options are enabled
;  in the printer job dialog.

              inc PortCallCounter


; Finally, lets dispatch to the port driver routine (X was preset,
;  see the technote for details).

              jmp (PrPortDrvList,x)

 
;                     Routine name        Toolbox call #
PrPortDrvList dc  a4'PrDevPrChanged'       ; $1913
              dc  a4'PrDevStartUp'         ; $1A13
              dc  a4'PrDevShutDown'        ; $1B13
              dc  a4'PrDevOpen'            ; $1C13
              dc  a4'PrDevRead'            ; $1D13
              dc  a4'PrDevWrite'           ; $1E13
              dc  a4'PrDevClose'           ; $1F13
              dc  a4'PrDevStatus'          ; $2013
              dc  a4'PrDevAsyncRead'       ; $2113 (alias PrDevInitBack)
              dc  a4'PrDevWriteBackground' ; $2213 (alias PrDevFillBack)
              dc  a4'PrPortVer'            ; $2413
              dc  a4'PrDevIsItSafe'        ; $3013

              end                          ; of StartOfDriver



;***********************************************************************
;*
PrDevPrChanged start
;* 
;*  Description:
;*    The Print Manager makes this call every time the user accepts
;*    this port driver in the Choose Printer dialog.
;*
;*    Our version of this routine will set the printer type to Unknown
;* 
;*  Input:                   LONG        printer name pointer
;* 
;***********************************************************************

         using PortData

         port_subroutine  (4:printername)

; Set the printer type to unknown
;
; Note: I refused to use stz because that will avoid the predefined
;       constants - if I change Unknown to some other value, a STZ
;       wouldn't work any more!

         lda    #PT_Unknown
         sta    PrinterType

         port_return
         rtl

         end                                ; of PrDevPrChanged



;***********************************************************************
;*
PrDevStartUp   start
;* 
;*  Description:
;*    This call is not required to do anything.  However, if your
;*    driver needs to initialize itself by allocating memory or other
;*    setup tasks, this is the place to do it.  Network drivers should
;*    copy the printer name to a local storage area for later use.
;*
;*    Our version of this routine will do nothing.
;* 
;*  Input:                   LONG        printer name pointer
;*                           LONG        zone name pointer
;* 
;***********************************************************************

         port_subroutine (4:zonename,4:printername)

         port_return
         cmp    #1
         rtl

         end                                ; of PrDevPrChanged



;***********************************************************************
;*
PrDevShutDown  start
;* 
;*  Description:
;*    This call, like PrDevStartUp, is not required to do anything.
;*    However, if your driver performs other tasks when it starts,
;*    from the normal (allocating memory) to the obscure (installing
;*    heartbeat tasks), it should undo them here.  If you allocate
;*    anything when you start, you should deallocate it when you
;*    shutdown.  Note that this call may be made without a balancing
;*    PrDevStartUp, so be prepared for this instance.  For example, do 
;*    not try to blindly deallocate a handle that your PrDevStartUp
;*    routine allocates and stores in local storage; if you have not
;*    called PrDevStartUp, there is no telling what will be in your
;*    local storage area.
;*
;*    Our version of this routine will do nothing.
;* 
;*  Input:                   none
;* 
;***********************************************************************

         port_subroutine 

         port_return
         rtl

         end                                ; of PrDevShutDown



;***********************************************************************
;*
PrDevOpen      start
;* 
;*  Description:
;*    This call basically prepares the firmware for printing.  It
;*    must initialize the firmware for both input and output. 
;*    Input is required so the connected printer may be polled
;*    for its status.
;*
;*    Our version of this routine will display a dialog box (if we
;*    haven't already opened a file) and open the specified file
;*    for output.  Since we don't really have a printer that could
;*    respond, I suppose we can't really get it ready for input
;*    (besides that, I don't believe all printer interfaces allow
;*    input - a parallel interface doesn't have input wires from
;*    the printer!)
;*
;*  Note: We will have to JSL to the completion routine when we
;*    are done!  (or RTL if the completion pointer is NIL).
;* 
;*  Input:                   LONG        completion routine pointer 
;*                           LONG        reserved long
;* 
;***********************************************************************

         using PortData

         port_subroutine (4:reserved,4:completion_routine)

; Clear the port driver call counter (This counter is used to determine
;  what type of printer driver is calling us).

         stz   PortCallCounter


; Now do the spooling code...
;    if (channel_already_open? == NO)
;        {
;        Dialog_box()
;        if Dialog_box answer == cancel
;            set channel already open = CANCELLED;
;        else
;            {
;            _allocate()
;            (create file)
;            open file
;            set channel = channel
;            }
;        }
;

         port_return 0:stump,completion_routine
         cmp   #1
         rtl

         end                                ; of PrDevOpen



;***********************************************************************
;*
PrDevRead      start
;* 
;*  Description:
;*    This call reads input from the printer.
;*
;*    Our version of this routine will return a string based on
;*    what we think the printer driver did:
;*
;*      ImageWriter:    Return Color option
;*      ImageWriter LQ: Return Model ID, Multiple bins and Color options
;*      Unknown:        Return nothing (String of zero length)
;*
;* 
;*  Input:                   WORD        space for result
;*                           LONG        buffer pointer
;*                           WORD        number of bytes to transfer
;*   
;*  Output:                  WORD        number of bytes transferred 
;* 
;***********************************************************************

         using PortData

         port_subroutine (2:bytes_to_send,4:buffer,2:bytes_transfered)


; Dispatch to return the appropriate string (depending on the printer
;  type - we determined the printer type from the last two commands...
;  see the open/write routine comments for details)

         ldx   PrinterType
         jmp   (ReadDispatch,x)

;                Routine name          Printer type
ReadDispatch anop
         dc    a4'ReadUnknown'          ; Unknown
         dc    a4'ReadIMWR'             ; ImageWriter
         dc    a4'ReadIMWR_LQ'          ; ImageWriter LQ


; For most printers, there aren't any bytes to be read.  Return
;  the standard zero length string.

ReadUnknown anop
         lda   #0
         sta   bytes_transfered
         bra   read_return


; Set the output buffer up for a ImageWriter returned string...
;  (IE: Color)

ReadIMWR anop
         lda   ImWr_ReadLen
         sta   bytes_transfered

         ldy   ImWr_ReadLen
         dey
ReadIMWR2 anop
         dey
         lda   ImWr_Read,y
         sta   [buffer],y
         cpy   #0
         bne   ReadIMWR2
         bra   read_return


; Set the output buffer up for a ImageWriter LQ returned string...
;  (IE: Multiple bins and Color)

ReadIMWR_LQ anop
         lda   ImWr_LQ_ReadLen
         sta   bytes_transfered

         ldy   ImWr_LQ_ReadLen
         dey
ReadIMWR_LQ2 anop
         dey
         lda   ImWr_LQ_Read,y
         sta   [buffer],y
         cpy   #0
         bne   ReadIMWR_LQ2


; Back to our caller we go...

read_return anop

         port_return 2:bytes_transfered
         rtl

         end                                ; of PrDevRead


 
;***********************************************************************
;*
PrDevWrite     start
;* 
;*  Description:
;*    Writes the data in the buffer to the printer and calls
;*    the completion routine.
;*
;*    Our version of this routine will copy the information to
;*    our buffer.  Once the buffer is full (check before the copy
;*    and if the copy would fill it then only copy what will fill
;*    it).  If the buffer is full then write it to the output
;*    file.  If there is still stuff to in the input buffer, then
;*    loop to the beginning of this routine again.
;*    Once we're done, its time to call the completion routine.
;* 
;*  Input:                   LONG    write completion pointer
;*                           LONG    buffer pointer
;*                           WORD    buffer length
;* 
;***********************************************************************

         using PortData

         port_subroutine (2:bytes_to_send,4:buffer,4:completion_routine)

; Lets see if we can determine the type of printer driver (for special
;  printers).  We only do this specialness on the first write after an
;  open.

         lda   PortCallCounter
         cmp   #1
         bne   WriteNotSpecial


; Check for an ImageWriter (check length of buffer, then the buffer)

WriteIMWR anop
         lda   bytes_to_send
         cmp   ImWr_WriteLen
         bne   WriteIMWR_LQ

         ldy   ImWr_WriteLen
         dey
WriteIMWR2 anop
         dey
         lda   ImWr_Write,y
         cmp   [buffer],y
         bne   WriteIMWR_LQ
         cpy   #0
         bne   WriteIMWR2


; Looks like and ImageWrite status string to me.
;  Set the special printer flag for later.

         lda   #PT_ImageWriter
         sta   PrinterType
         bra   WriteNotSpecial


; Check for an ImageWriter LQ (check length of buffer, then the buffer)

WriteIMWR_LQ anop
         lda   bytes_to_send
         cmp   ImWr_LQ_WriteLen
         bne   WriteNotSpecial

         ldy   ImWr_LQ_WriteLen
         dey
WriteIMWR_LQ2 anop
         dey
         lda   ImWr_LQ_Write,y
         cmp   [buffer],y
         bne   WriteNotSpecial
         cpy   #0
         bne   WriteIMWR_LQ2


; Looks like and ImageWrite LQ status string to me.
;  Set the special printer flag for later.

         lda   #PT_ImageWriterLQ
         sta   PrinterType
;        bra   WriteNotSpecial


; Continue with the standard port write

WriteNotSpecial anop

;    number_of_bytes_transferred = 0;
;    if (out_buffer_size+in_buffer_size > out_buffer_space)
;        {
;        copy(out_buffer(out_buffer_free),
;             in_buffer(0),
;             out_buffer_space)
;        in_buffer += out_buffer_space;
;
;. . . . blah blah blah . . . . 

         port_return 0:stump,completion_routine
         cmp   #1
         rtl

         end                                ; of PrDevWrite

 
 
;***********************************************************************
;*
PrDevClose     start
;* 
;*  Description:
;*
;*    This call is not required to do anything.  However, if
;*    you allocate any system resources with PrDevOpen, you
;*    should deallocate them at this time.  As with start and
;*    shutdown, note that PrDevClose could be called without a 
;*    balancing PrDevOpen (the reverse is not true), and you
;*    must be prepared for this if you try to deallocate
;*    resources which were never allocated.
;*
;*    Our version of this routine will close the file (if it
;*    is open).  If we find a CANCELLED flag, then we will
;*    just reset the flag.
;* 
;*  Input:                   none
;* 
;***********************************************************************

         port_subroutine 

;    if (buffer allocated)
;        deallocate()
;
;    if (channel open)
;        close (channel)
;
;    if (CANCELLED flag)
;        flag = OK;

         port_return
         rtl

         end                                ; of PrDevClose
 

 
;***********************************************************************
;*
PrDevStatus    start
;* 
;*  Description:
;*
;*    This call performs differently for direct connect and
;*    network drivers.  For direct connect drivers, it currently
;*    has no required function, although it may return the status
;*    of the port in the future.  For network drivers, it calls
;*    an AppleTalk status routine, which returns a status string
;*    in the buffer (normally a string like "Status:  The print
;*    server is spooling your document").
;*
;*    Our version of this routine will do nothing.
;* 
;*  Input:                   LONG        status buffer pointer
;* 
;***********************************************************************

         port_subroutine (4:status)

         port_return
         cmp   #1
         rtl

         end                                ; of PrDevStatus

 
 
;***********************************************************************
;*
PrDevAsyncRead start
;* 
;*  Description:
;*    Since PrDevRead cannot read asynchronously, this call is
;*    provided for that task.  Note that this does nothing for
;*    direct connect drivers, and if the completion pointer is
;*    NIL, it behaves for network drivers exactly as PrDevRead
;*    does.
;*
;*    Our version of this routine will do nothing.
;* 
;*  Input:                   WORD        space for result
;*                           LONG        completion pointer
;*                           WORD        buffer length
;*                           LONG        buffer pointer
;*   
;*  Output:                  WORD        number of bytes transferred
;* 
;***********************************************************************

         port_subroutine (4:buffer,2:length,4:cmp_rtn,2:bytes_transfered)

         lda        #0
         sta        bytes_transfered

         port_return 2:bytes_transfered,cmp_rtn
         cmp   #1
         rtl

         end                                ; of PrDevAsyncRead

 
 
;***********************************************************************
;*
PrDevWriteBackground start
;* 
;*  Description:
;*    This routine is not implemented at this time.
;*
;*    Our version of this routine will do nothing.
;* 
;*  Input:                   LONG        completion procedure pointer
;*                           WORD        buffer length
;*                           LONG        buffer pointer
;* 
;***********************************************************************
 
         port_subroutine (4:buffer,2:length,4:completion_routine)

         port_return 0:stump,completion_routine
         cmp   #1
         rtl

         end                                ; of PrDevWriteBackground

 
 
;***********************************************************************
;*
PrPortVer     start
;* 
;*  Description:
;*    Returns the version number of the currently installed
;*    port driver.
;*
;*    Our version of this routine will do nothing.
;* 
;*  Note:  The internal version number is stored as a major
;*    byte and a minor byte (i.e., $0103 represents version 1.3)
;*
;*  Input:                   WORD        space for result
;* 
;*  Output:                  WORD        Port driver's version number
;* 
;***********************************************************************
 
         port_subroutine (2:version_number)

         lda        #$0401                        ; Rev 4.1
         sta        version_number                ; Save it

         port_return 2:version_number
         rtl

         end                                ; of PrPortVer

 
 
;***********************************************************************
;*
PrDevIsItSafe start
;* 
;*  Description:
;*    This call checks to see if the port or card which your
;*    driver controls is enabled.  It should check at least
;*    the corresponding bit of $E0C02D, and checking the Battery
;*    RAM settings wouldn't hurt any either.
;*
;*    Our version of this routine will return TRUE - we don't
;*    really drive any port, so we can't really check to see if
;*    we are allowed to access the port/slot or not.
;* 
;*  Input:                   WORD        space for result
;* 
;*  Output:                  WORD        Boolean indicating if port
;*                                       is enabled 
;* 
;***********************************************************************
 
         port_subroutine (2:port_safe_flag)

         lda        #TRUE                        ; Its always safe...
         sta        port_safe_flag               ; Save it

         port_return 2:port_safe_flag
         rtl

         end                                ; of PrDevIsItSafe

 
 
;***********************************************************************
;*
PortData data
;*
;*  File Port Driver Data Section
;* 
;*  Description:
;*    This section is all of the data that the file port driver
;*    requires (not very much really...).
;*
;***********************************************************************

; Special Printer Types

PrinterType ds  2


; Count of the Port calls made since an Open

PortCallCounter dc i2'0'


; ImageWriter model ID/status/options string (written)

ImWr_Write     dc  i1'$1b,$3f'
ImWr_WriteLen  dc  i2'*-ImWr_Write'


; ImageWriter model ID/status/options string (read)

ImWr_Read     dc  i1'$49,$57,$31,$30,$43'
ImWr_ReadLen  dc  i2'*-ImWr_Read'


; ImageWriter LQ model ID/status/options string (written)

ImWr_LQ_Write     dc  i1'$1b,$5a,$00,$02,$1b,$3f'
ImWr_LQ_WriteLen  dc  i2'*-ImWr_LQ_Write'


; ImageWriter LQ model ID/status/options string (read)

ImWr_LQ_Read     dc  i1'$4c,$51,$31,$45,$43'
ImWr_LQ_ReadLen  dc  i2'*-ImWr_LQ_Read'

 
 
         end                                ; End of File Port Driver











