
/*
 * Copyright (c) Kopriha Software,  1990-1991
 *       All Rights Reserved
 *
 *  Delete.CC
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
 *   ks_file_delete - Delete a file.                                  *
 *                                                                    *
 *   History: March 24, 1991  Dave  Created this routine              *
 * ****************************************************************** */

#undef ROUTINE_NAME
#define ROUTINE_NAME "ks_file_delete"

KS_E_ERROR ks_file_delete(GSString255Ptr GSPathPtr)
{
    /* ************************************************************** *
     *  Local declarations:                                           *
     * ************************************************************** */

    KS_E_ERROR       error;     /* Holds error codes for subroutines  */


    ROUTINE_ENTER();


    /* ************************************************************** *
     *  Setup the delete packet and try to get rid of the file.       *
     *  Any errors are returned to our caller.                        *
     * ************************************************************** */

    KSf_pkts.delete.pCount = 1;
    KSf_pkts.delete.pathname = GSPathPtr;

    DestroyGS(&KSf_pkts.delete);

    if ((error = GET_ERROR()) != KS_E_SUCCESS)
        {
        KS_ERROR(error, KS_FILE_ID);
        };


    /* ************************************************************** *
     *  Return the success back to our caller.                        *
     * ************************************************************** */

    KS_SUCCESS();

}   /* End of ks_file_delete()                                        */



