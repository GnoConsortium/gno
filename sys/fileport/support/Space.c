
/*
 * Copyright (c) Kopriha Software,  1990-1991
 *       All Rights Reserved
 *
 *  Space.CC
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
 *   ks_file_get_volume_space - Return the total/unused space of      *
 *                              the specified volume                  *
 *                                                                    *
 *   History: Mar 13, 1991  Dave  Created this routine                *
 * ****************************************************************** */

#undef ROUTINE_NAME
#define ROUTINE_NAME "ks_file_get_volume_space"

KS_E_ERROR ks_file_get_volume_space(GSString255Ptr volume_ptr,
                                    LongWord *unused_space,
                                    LongWord *total_space)
{
    /* ************************************************************** *
     *  Local declarations:                                           *
     * ************************************************************** */

    KS_E_ERROR       error;     /* Holds error codes for subroutines  */


    ROUTINE_ENTER();


    /* ************************************************************** *
     *  Now lets generate some code that just shouldn't work...       *
     *                                                                *
     *  According to all the reference manuals, GetFileInfo does not  *
     *  return valid information on volumes or directories.           *
     *                                                                *
     *  The means we have to go back to the ProDOS 16 GET_FILE_INFO   *
     *  call that does return the volume size and number of allocated *
     *  blocks (even if the manuals seem to disagree with this).      *
     *                                                                *
     *  To do a ProDOS 16 call, you have to pass a pascal string...   *
     *  which is a length byte followed by the string.  Now a GS/OS   *
     *  string is a length word followed by the string.  We are going *
     *  to cheat once more by making the GS/OS string a Pascal string *
     *  for the duration of this call...                              *
     *                                                                *
     *  I don't like doing things this way, but the alternative is    *
     *  very ugly (start by scanning all devices with DInfo, the      *
     *  get the volume name of each disk device, then compare names   *
     *  until you find the specified volume, and finally you have     *
     *  the information for a Volume call).                           *
     *                                                                *
     *  We'll cheat now - this may need fixing in the future.         *
     * ************************************************************** */

    (((char *) volume_ptr)[1]) = (((char *) volume_ptr)[0]);

    KSf_pkts.fileinfo.pathname = &(((char *) volume_ptr)[1]);

    GET_FILE_INFO(&KSf_pkts.fileinfo);

    (((char *) volume_ptr)[1]) = '\0';

    if ((error = GET_ERROR()) != KS_E_SUCCESS)
        {
        KS_ERROR(error, KS_FILE_ID);
        };

    *unused_space = (KSf_pkts.fileinfo.auxType -
                     KSf_pkts.fileinfo.blocksUsed) / 2;

    *total_space = (KSf_pkts.fileinfo.auxType) / 2;


    /* ************************************************************** *
     *  We'll never reach this part of the code... However, if by     *
     *  some cosmic reason we do - let us return a success.           *
     * ************************************************************** */

    KS_SUCCESS();

}   /* End of ks_file_get_volume_space()                              */



