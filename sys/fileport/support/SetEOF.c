
/*
 * Copyright (c) Kopriha Software,  1990-1991
 *       All Rights Reserved
 *
 *  SetEOF.CC
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
 *   ks_file_set_eof - Set the end of file of an open file            *
 *                                                                    *
 *   History: May 27, 1991  Dave  Created this routine                *
 * ****************************************************************** */

#undef ROUTINE_NAME
#define ROUTINE_NAME "ks_file_set_eof"

KS_E_ERROR ks_file_set_eof(KS_FILE_PTR file_ptr,
                           LongWord    new_eof)
{
    /* ************************************************************** *
     *  Local declarations:                                           *
     * ************************************************************** */

    KS_E_ERROR       error;     /* Holds error codes for subroutines  */


    ROUTINE_ENTER();


    /* ************************************************************** *
     *  Setup the packet then make the set EOF call...                *
     * ************************************************************** */

    KSf_pkts.position.pCount = 3;
    KSf_pkts.position.refNum = file_ptr->refNum;
    KSf_pkts.position.base = startPlus;
    KSf_pkts.position.displacement = new_eof;

    SetEOFGS(&KSf_pkts.position);


    /* ************************************************************** *
     *  Return to our caller.                                         *
     * ************************************************************** */

    if ((error = GET_ERROR()) != KS_E_SUCCESS)
        {
        KS_ERROR(error, KS_FILE_ID);
        };

    KS_SUCCESS();

}   /* End of ks_file_set_eof()                                       */



