
/*
 * Copyright (c) Kopriha Software,  1990-1991
 *       All Rights Reserved
 *
 *  Gnfn.CC
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
 *   ks_file_get_next_file - Return the next filename in a directory. *
 *                                                                    *
 *   History: Mar 5, 1991  Dave  Created this routine                 *
 * ****************************************************************** */

#undef ROUTINE_NAME
#define ROUTINE_NAME "ks_file_get_next_file"

KS_E_ERROR ks_file_get_next_file(KS_FILE_PTR file_ptr,
                                 DirEntryRecPtrGS dir_entry_ptr)
{
    /* ************************************************************** *
     *  Local declarations:                                           *
     * ************************************************************** */

    KS_E_ERROR       error;     /* Holds error codes for subroutines  */


    ROUTINE_ENTER();


    /* ************************************************************** *
     *  Set the parameters up and get the next directory entry.       *
     *                                                                *
     *  If we are at the end of the directory, then we have to do a   *
     *  special call to get the first entry - the open code will set  *
     *  the flag making it look like we're at the end of the directory*
     *  for our first call.                                           *
     * ************************************************************** */

    dir_entry_ptr->pCount = 13;
    dir_entry_ptr->refNum = file_ptr->refNum;
    if  (file_ptr->end_of_dir == TRUE)
        {
        dir_entry_ptr->base = 0;
        dir_entry_ptr->displacement = 0;
        file_ptr->end_of_dir = FALSE;
        }
    else
        {
        dir_entry_ptr->base = 1;
        dir_entry_ptr->displacement = 1;
        };

    GetDirEntryGS(dir_entry_ptr);


    /* ************************************************************** *
     *  Now check for errors.  If we are at the end of the directory  *
     *  then send the end of directory flag.                          *
     *                                                                *
     *  The last thing to do is to return to our caller.              *
     * ************************************************************** */

    if ((error = GET_ERROR()) != KS_E_SUCCESS)
        {
        if (error == endOfDir)
            {
            file_ptr->end_of_dir = TRUE;
            };

        KS_ERROR(error, KS_FILE_ID);
        };

    KS_SUCCESS();

}   /* End of ks_file_get_next_file()                                 */
