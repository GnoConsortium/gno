
/*
 * Copyright (c) Kopriha Software,  1990-1991
 *       All Rights Reserved
 *
 *  Open.CC
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
 *   ks_file_open - open a file for reading/writing.                  *
 *                                                                    *
 *   History: Oct 12, 1990  Dave  Created this routine                *
 * ****************************************************************** */

#undef ROUTINE_NAME
#define ROUTINE_NAME "ks_file_open"

KS_E_ERROR ks_file_open(GSString255Ptr GSPathPtr,
                        Word access,
                        Word file_fork,
                        Word buffer_flag,
                        KS_FILE_HDL file_hdl)
{
    /* ************************************************************** *
     *  Local declarations:                                           *
     * ************************************************************** */

    KS_E_ERROR       error;     /* Holds error codes for subroutines  */
    KS_FILE_PTR      file_ptr;  /* Pointer to the new KS_FILE struct  */
    Handle           file_handle;  /* Handle to the file pointer      */
    Word             file_ref;  /* File reference number (for errors) */


    ROUTINE_ENTER();


    /* ************************************************************** *
     *  Open the file for our caller.                                 *
     * ************************************************************** */

    KSf_pkts.open.pCount = 15;
    KSf_pkts.open.pathname = GSPathPtr;
    KSf_pkts.open.requestAccess = access;
    KSf_pkts.open.resourceNumber = file_fork;
    KSf_pkts.open.optionList = (ResultBuf255Ptr)
                                 &(KSf_pkts2.optionList[0]);
    KSf_pkts2.optionList[0] = sizeof(KSf_pkts2.optionList)
                                - sizeof(Word);

    OpenGS(&KSf_pkts.open);

    if ((error = GET_ERROR()) != KS_E_SUCCESS)
        {
        goto EXIT_NOW;
        };

    file_ref = KSf_pkts.open.refNum;


    /* ************************************************************** *
     *  The file is open, now lets setup the KS_FILE structure.       *
     *                                                                *
     *  Allocate a KS_FILE structure, fill it in and return the       *
     *  file pointer (actually, store it through the handle).         *
     * ************************************************************** */

    KS_MEMORY_ALLOCATE(attrNoSpec + attrLocked,
                       sizeof(KS_FILE),
                       BUFFER_USERID,
                       file_handle,
                       error);

    if (error != KS_E_SUCCESS)
        {
        goto CLOSE_EXIT;
        };

    file_ptr = (KS_FILE_PTR) *file_handle;

    file_ptr->file_handle = file_handle;
    file_ptr->access = access;
    file_ptr->refNum = KSf_pkts.open.refNum;
    file_ptr->eof = KSf_pkts.open.eof;
    file_ptr->resource_eof = KSf_pkts.open.resourceEOF;
    file_ptr->fileType = KSf_pkts.open.fileType;
    file_ptr->auxType = KSf_pkts.open.auxType;
    file_ptr->end_of_file = FALSE;
    file_ptr->end_of_dir = TRUE;
    file_ptr->struct_id = KS_FILE_ID;


    /* ************************************************************** *
     *  We have a good KS_FILE structure, lets try to setup a buffer  *
     *  to do I/O to/from if our caller wants one.                    *
     * ************************************************************** */

    if (buffer_flag == KS_FILE_BUFFER_IO)
        {
        KS_MEMORY_ALLOCATE(attrNoSpec + attrLocked,
                           KSf_FileBufferSize,
                           BUFFER_USERID,
                           file_ptr->buffer_handle,
                           error);

        if (error != KS_E_SUCCESS)
            {
            goto FREE_CLOSE_EXIT;
            };

        file_ptr->buffer = (Byte *) *(file_ptr->buffer_handle);

        file_ptr->buffer_size = KSf_FileBufferSize;
        file_ptr->buffer_offset = 0;

        if (access == KS_FILE_WRITE_ACCESS)
            {
            file_ptr->buffer_available = KSf_FileBufferSize;
            }
        else
            {
            file_ptr->buffer_available = 0;
            };
        }
    else
        {
        file_ptr->buffer_size = NULL;
        };


    /* ************************************************************** *
     *  Return the open file back to our caller.                      *
     * ************************************************************** */

    *file_hdl = file_ptr;

    KS_SUCCESS();


    /* ************************************************************** *
     *  Return the error back to our caller after we close file file. *
     * ************************************************************** */

FREE_CLOSE_EXIT:

    file_ptr->struct_id = 0;

    KS_MEMORY_DEALLOCATE(file_handle,
                         error);

CLOSE_EXIT:

    KSf_pkts.close.pCount = 1;
    KSf_pkts.close.refNum = file_ref;

    CloseGS(&KSf_pkts.close);

EXIT_NOW:

    KS_ERROR(error, KS_FILE_ID);

}   /* End of ks_file_open()                                          */
