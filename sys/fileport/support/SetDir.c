
/*
 * Copyright (c) Kopriha Software,  1990-1991
 *       All Rights Reserved
 *
 *  SetDir.CC
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
 *   ks_file_set_directory - Set the current directory (actually,     *
 *                           prefix 8).                               *
 *                                                                    *
 *   History: Mar 7, 1991  Dave  Created this routine                 *
 * ****************************************************************** */

#undef ROUTINE_NAME
#define ROUTINE_NAME "ks_file_set_directory"

KS_E_ERROR ks_file_set_directory(GSString255Ptr directory_ptr)
{
    /* ************************************************************** *
     *  Local declarations:                                           *
     * ************************************************************** */

    KS_E_ERROR       error;     /* Holds error codes for subroutines  */


    ROUTINE_ENTER();


    /* ************************************************************** *
     *  Setup the packet to set prefix 8 then make the set prefix     *
     *  call.                                                         *
     *                                                                *
     *  Note: We have to copy the prefix pathname into the buffer     *
     *        because of the way the GS/OS packet is set up.          *
     *        It would be nice to simply set a pointer... but we      *
     *        can't do that.                                          *
     * ************************************************************** */

    KSf_pkts.prefix.pCount = 2;
    KSf_pkts.prefix.prefixNum = 8;

    KSf_pkts.prefix.buffer.setPrefix = directory_ptr;

    SetPrefixGS(&KSf_pkts.prefix);


    /* ************************************************************** *
     *  Return to our caller.                                         *
     * ************************************************************** */

    if ((error = GET_ERROR()) != KS_E_SUCCESS)
        {
        KS_ERROR(error, KS_FILE_ID);
        };

    KS_SUCCESS();

}   /* End of ks_file_set_directory()                                 */


