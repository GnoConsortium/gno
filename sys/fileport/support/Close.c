
/*
 * Copyright (c) Kopriha Software,  1990-1991
 *       All Rights Reserved
 *
 *  Close.CC
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
 *   ks_file_close - close an open file.                              *
 *                                                                    *
 *   History: Oct 13, 1990  Dave  Created this routine                *
 * ****************************************************************** */

#define ROUTINE_NAME "ks_file_close"

KS_E_ERROR ks_file_close(KS_FILE_PTR file_ptr)
{
    /* ************************************************************** *
     *  Local declarations:                                           *
     * ************************************************************** */

    KS_E_ERROR       error;     /* Holds error codes for subroutines  */



    ROUTINE_ENTER();


    /* ************************************************************** *
     *  Verify the structure ID passed in is the correct one.         *
     * ************************************************************** */

    if (file_ptr->struct_id != KS_FILE_ID)
        {
        KS_ERROR(KS_E_INVALID_STRUCT_ID, KS_FILE_ID);
        };


    /* ************************************************************** *
     *  Flush any remaining buffer to disk (if there is a buffer and  *
     *  we are writing to the file).                                  *
     * ************************************************************** */

    if (file_ptr->buffer_size != NULL)
        {
        if (file_ptr->access == KS_FILE_WRITE_ACCESS)
            {
            if (file_ptr->buffer_offset != 0)
                {

                /* ************************************************** *
                 *  We have something in a buffer - write the final   *
                 *  buffer to disk.                                   *
                 * ************************************************** */

                KSf_pkts.IO.pCount = 4;
                KSf_pkts.IO.refNum = file_ptr->refNum;
                KSf_pkts.IO.dataBuffer = TO_POINTER(file_ptr->buffer);
                KSf_pkts.IO.requestCount = file_ptr->buffer_offset;

                WriteGS(&KSf_pkts.IO);

                if ((error = GET_ERROR()) != KS_E_SUCCESS)
                    {
                    KS_ERROR(error, KS_FILE_ID);
                    };

                };  /* End if there was something in the buffer       */

            };  /* End if we were writing to the file                 */

        /* ********************************************************** *
         *  We used buffered I/O on this file - now lets free the     *
         *  buffer back to free memory.                               *
         * ********************************************************** */

        KS_MEMORY_DEALLOCATE(file_ptr->buffer_handle,
                             error);

        };  /* End if we used buffered I/O to the file                */


    /* ************************************************************** *
     *  Set up the close packet and close the file.                   *
     * ************************************************************** */

    KSf_pkts.close.pCount = 1;
    KSf_pkts.close.refNum = file_ptr->refNum;

    CloseGS(&KSf_pkts.close);


    /* ************************************************************** *
     *  Clear the FILE structure and give it back to free memory.     *
     * ************************************************************** */

    file_ptr->struct_id = 0;

    KS_MEMORY_DEALLOCATE(file_ptr->file_handle,
                         error);


    /* ************************************************************** *
     *  Return the success back to our caller.                        *
     * ************************************************************** */

    KS_SUCCESS();

}   /* End of ks_file_close()                                         */


