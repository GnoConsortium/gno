
/*
 * Copyright (c) Kopriha Software,  1990-1991
 *       All Rights Reserved
 *
 *  Create.CC
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
 *   ks_file_create - Create a particular type/auxtype file           *
 *                                                                    *
 *   History: Mar 3, 1991  Dave  Created this routine                 *
 * ****************************************************************** */

#undef ROUTINE_NAME
#define ROUTINE_NAME "ks_file_create"

KS_E_ERROR ks_file_create(GSString255Ptr GSPathPtr,
                          Word file_type,
                          LongWord auxtype)

{
    /* ************************************************************** *
     *  Local declarations:                                           *
     * ************************************************************** */

    KS_E_ERROR       error;     /* Holds error codes for subroutines  */


    ROUTINE_ENTER();


    /* ************************************************************** *
     *  Create the file for our caller.                               *
     *                                                                *
     *  Note: The access is setup such that the file is readable,     *
     *        writable, can be renamed and can be deleted.            *
     * ************************************************************** */

    KSf_pkts.create.pCount = 4;
    KSf_pkts.create.pathname = GSPathPtr;
    KSf_pkts.create.access = readWriteEnable |
                             renameEnable |
                             destroyEnable;
    KSf_pkts.create.fileType = file_type;
    KSf_pkts.create.auxType = auxtype;

    CreateGS(&KSf_pkts.create);


    /* ************************************************************** *
     *  Return the success/error back to our caller.                  *
     * ************************************************************** */

    if ((error = GET_ERROR()) != KS_E_SUCCESS)
        {
        KS_ERROR(error, KS_FILE_ID);
        };

    KS_SUCCESS();

}   /* End of ks_file_create()                                        */


