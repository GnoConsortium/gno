
/*
 * Copyright (c) Kopriha Software,  1990-1991
 *       All Rights Reserved
 *
 *  Rename.CC
 *
 * Description:
 *     This module exists to abstract the data of the file I/O
 *     primitives of GS/OS.
 *
 *
 *  History:Jun 30, 1991  Dave  Created this file
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
 *   ks_file_rename - Change the name of a file (given complete       *
 *                    pathnames of the source/destination).           *
 *                                                                    *
 *   History: June 30, 1991  Dave  Created this routine               *
 * ****************************************************************** */

#undef ROUTINE_NAME
#define ROUTINE_NAME "ks_file_rename"

KS_E_ERROR ks_file_rename(GSString255Ptr old_path,
                          GSString255Ptr new_path)
{
    /* ************************************************************** *
     *  Local declarations:                                           *
     * ************************************************************** */

    KS_E_ERROR       error;     /* Holds error codes for subroutines  */


    ROUTINE_ENTER();


    /* ************************************************************** *
     *  Fill in the change path packet then issue the system call...  *
     * ************************************************************** */

    KSf_pkts.rename.pCount = 2;
    KSf_pkts.rename.pathname = old_path;
    KSf_pkts.rename.newPathname = new_path;

    ChangePathGS(&KSf_pkts.rename);

    if ((error = GET_ERROR()) != KS_E_SUCCESS)
        {
        KS_ERROR(error, KS_FILE_ID);
        };


    /* ************************************************************** *
     *  Return the success back to our caller.                        *
     * ************************************************************** */

    KS_SUCCESS();

}   /* End of ks_file_rename()                                        */



