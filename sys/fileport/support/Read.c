
/*
 * Copyright (c) Kopriha Software,  1990-1991
 *       All Rights Reserved
 *
 *  Read.CC
 *
 * Description:
 *     This module exists to abstract the data of the file I/O
 *     primitives of GS/OS.
 *
 *
 *  History:Oct 13, 1990  Dave  Created this file
 *
 *          Feb 25, 1991  Dave  Added I/O buffering
 *
 *          May 26, 1991  Dave  Added set EOF
 *
 *          Jun 07, 1991  Dave  Broke the single source into lots
 *                              of small sources so we can build
 *                              a library to use...
 *
 *          Jun 30, 1991  Dave  Added support for no target buffer
 *                              (IE: This allows me to open a file
 *                              with a buffer then do I/O to only
 *                              that buffer - we will assume that
 *                              the caller will know what he is doing).
 *
 *                              This functionallity is invoked when
 *                              we are called with a data_buffer
 *                              pointer to NULL (then the user must
 *                              understand that the file_ptr->buffer
 *                              is where he will find his data).
 *
 *                              Note: This functionallity is only
 *                              implemented for the buffered I/O case!
 *
 */

/*
 *  define DEBUG_CODE
 *                     - add # to define to create the local
 *                       debug code (IE:module)
 */

#ifndef _KS_FILEIO_
#include "ks.fileio.h"
#endif

#pragma noroot


/* ****************************************************************** *
 *   ks_file_read - Perform an read from an open file (possibly using *
 *                  an internal buffer).                              *
 *                                                                    *
 *   History: Feb 26, 1991  Dave  Created this routine                *
 * ****************************************************************** */

#undef ROUTINE_NAME
#define ROUTINE_NAME "ks_file_read"

KS_E_ERROR ks_file_read(KS_FILE_PTR file_ptr,
                        LongWord position,
                        LongWord data_size,
                        Pointer data_buffer)
{
    /* ************************************************************** *
     *  Local declarations:                                           *
     * ************************************************************** */

    KS_E_ERROR   error;            /* Holds error codes for subroutine*/
                                   /*  calls                          */
    LongWord     data_offset;      /* Offset into the buffer to return*/
    LongWord     remaining_space;  /* Space remaining in file buffer  */
    LongWord     buffer_request;   /* Size of each copy from the      */
                                   /*  file buffer                    */


    ROUTINE_ENTER();


    /* ************************************************************** *
     *  Verify the structure ID passed in is the correct one.         *
     * ************************************************************** */

    if (file_ptr->struct_id != KS_FILE_ID)
        {
        KS_ERROR(KS_E_INVALID_STRUCT_ID, KS_FILE_ID);
        };


    /* ************************************************************** *
     *  Zero the number of bytes transfered in the KS_FILE structure. *
     * ************************************************************** */

    file_ptr->data_size = 0;


    /* ************************************************************** *
     *  If there is a buffer, then lets get data from the file buffer.*
     * ************************************************************** */

    if (file_ptr->buffer_size != NULL)
        {

        /* ********************************************************** *
         *  If we hit the end of file last time, then we have no      *
         *  choice but to return an error.                            *
         * ********************************************************** */

        if (file_ptr->end_of_file == TRUE)
            {
            error = eofEncountered;
            goto EXIT_NOW;
            };


        /* ********************************************************** *
         *  Loop till we satisfy the request (or take an error)       *
         * ********************************************************** */

        data_offset = 0;

        while (data_size > 0)
            {

            /* ****************************************************** *
             *  Calculate the remaining space in the buffer.  If      *
             *  there is any space left in the buffer then lets copy  *
             *  as much as we need to into the output buffer.         *
             * ****************************************************** */

            remaining_space = (file_ptr->buffer_available) -
                              (file_ptr->buffer_offset);

            if (remaining_space > 0)
                {
                buffer_request = MIN(data_size,
                                     remaining_space);


                /* ************************************************** *
                 *  Copy the available bytes (or the required bytes)  *
                 *  to the target buffer (if one was supplied).       *
                 * ************************************************** */

                if (data_buffer != NULL)
                    {
                    COPY_BYTES(file_ptr->buffer,
                               file_ptr->buffer_offset,
                               data_buffer,
                               data_offset,
                               buffer_request);
                    };


                /* ************************************************** *
                 *  Now modify the parameters of the buffers by:      *
                 *                                                    *
                 *  1) Adding the size of the request to the file     *
                 *  buffer ofset and the data offset (IE: Indices to  *
                 *  the file buffer and the read request buffer).     *
                 *                                                    *
                 *  2) Subtracting the request size from the read     *
                 *  request size and the remaining number of          *
                 *  characters in the file buffer.                    *
                 * ************************************************** */

                file_ptr->buffer_offset = file_ptr->buffer_offset +
                                          buffer_request;

                data_offset = data_offset + buffer_request;

                file_ptr->data_size = data_offset; 

                data_size = data_size - buffer_request;

                remaining_space = remaining_space - buffer_request;
                };


            /* ****************************************************** *
             *  If isn't anything in the file buffer, the we have to  *
             *  re-fill it.  The problem is that the buffer size may  *
             *  have changed due to what our user wants (users are    *
             *  bound to be the end of all computing...).  This means *
             *  that we'll junp through a few hoops if we must change *
             *  buffer sizes - so expect some weirdness here.         *
             * ****************************************************** */

            if (remaining_space == 0)
                {

                /* ************************************************** *
                 *  This is the above mentioned weirdness - if the    *
                 *  user specified a different size buffer we will    *
                 *  no comply with their wishes.                      *
                 * ************************************************** */

                if (file_ptr->buffer_size != KSf_FileBufferSize)
                    {
                    KS_MEMORY_DEALLOCATE(file_ptr->buffer_handle,
                                         error);

                    if (error != KS_E_SUCCESS)
                        {
                        goto EXIT_NOW;
                        };

                    KS_MEMORY_ALLOCATE(attrFixed + attrLocked,
                                       KSf_FileBufferSize,
                                       BUFFER_USERID,
                                       file_ptr->buffer_handle,
                                       error);

                    if (error != KS_E_SUCCESS)
                        {
                        goto EXIT_NOW;
                        };

                    file_ptr->buffer = (Byte *)
                                         *(file_ptr->buffer_handle);

                    file_ptr->buffer_size = KSf_FileBufferSize;

                    file_ptr->buffer_available = KSf_FileBufferSize;
                    };


                /* ************************************************** *
                 *  Issue a Read to the file into our buffer.         *
                 * ************************************************** */

                KSf_pkts.IO.pCount = 4;
                KSf_pkts.IO.refNum = file_ptr->refNum;
                KSf_pkts.IO.dataBuffer = TO_POINTER(file_ptr->buffer);
                KSf_pkts.IO.requestCount = file_ptr->buffer_size;

                ReadGS(&KSf_pkts.IO);


                /* ************************************************** *
                 *  Now for the error processing.                     *
                 *                                                    *
                 *  Any error means we return to our caller.          *
                 *                                                    *
                 *  The end of file error (eofEncountered or $4c) is  *
                 *  special.  At EOF we mark the KS_FILE structure    *
                 *  so the next READ call will return EOF right away. *
                 *  We will return SUCCESS in this case because the   *
                 *  user will be able to find some data in the input  *
                 *  buffer (total amount == ).
                 * ************************************************** */

                if ((error = GET_ERROR()) != KS_E_SUCCESS)
                    {
                    if (error == eofEncountered)
                        {
                        error = KS_E_SUCCESS;
                        file_ptr->end_of_file = TRUE;
                        }
                    goto EXIT_NOW;
                    };

                file_ptr->buffer_available = KSf_pkts.IO.transferCount;
                file_ptr->buffer_offset = 0;

                };  /* End if there is no remaining buffer space      */

            };  /* End while there are characters to be read...       */

        KS_SUCCESS();

        };  /* End if we are doing buffer I/O from the file           */




    /* ************************************************************** *
     *  Ok, we've done enough buffering... lets do some real input... *
     *                                                                *
     *  Position the 'mark' (where we will read from) in the file.    *
     *  Note: We'll move the mark only if our user asks us to.        *
     * ************************************************************** */

    if (position != KS_NEXT_FILE_POSITION)
        {
        KSf_pkts.position.pCount = 3;
        KSf_pkts.position.refNum = file_ptr->refNum;
        KSf_pkts.position.base = startPlus;
        KSf_pkts.position.displacement = position;

        SetMarkGS(&KSf_pkts.position);

        if ((error = GET_ERROR()) != KS_E_SUCCESS)
            {
            goto EXIT_NOW;
            };

        };  /* End if we must change the file position                */


    /* ************************************************************** *
     *  Setup the I/O packet and read what our user is asking for.    *
     * ************************************************************** */

    KSf_pkts.IO.pCount = 4;
    KSf_pkts.IO.refNum = file_ptr->refNum;
    KSf_pkts.IO.dataBuffer = data_buffer;
    KSf_pkts.IO.requestCount = data_size;

    ReadGS(&KSf_pkts.IO);

    if ((error = GET_ERROR()) != KS_E_SUCCESS)
        {
        goto EXIT_NOW;
        };


    /* ************************************************************** *
     *  Save the number of bytes transfered in the KS_FILE structure. *
     * ************************************************************** */

    file_ptr->data_size = KSf_pkts.IO.transferCount;


    /* ************************************************************** *
     *  Return the status back to our caller.                         *
     * ************************************************************** */

EXIT_NOW:

    if (error != KS_E_SUCCESS)
        {
        KS_ERROR(error, KS_FILE_ID);
        };

    KS_SUCCESS();

}   /* End of ks_file_read()                                          */


