
/*
 * Copyright (c) Kopriha Software,  1990-1991
 *       All Rights Reserved
 *
 *  GetVolume.CC
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
 *   ks_file_get_volume - Return the volume name for the specified    *
 *                        pathname.                                   *
 *                                                                    *
 *   History: Mar 7, 1991  Dave  Created this routine                 *
 * ****************************************************************** */

#undef ROUTINE_NAME
#define ROUTINE_NAME "ks_file_get_volume"


KS_E_ERROR ks_file_get_volume(GSString255Ptr pathname_ptr,
                              GSString255Ptr volume_ptr)
{
    /* ************************************************************** *
     *  Local declarations:                                           *
     * ************************************************************** */

    Word  counter;              /* Counts characters copied           */


    ROUTINE_ENTER();


    /* ************************************************************** *
     *  Copy the volume name out of the pathname.  The volume name    *
     *  will start with a special delimitor, when we get to either    *
     *  the next delimitor or the end of the string then we have      *
     *  copied all of the volume name.                                *
     * ************************************************************** */

    volume_ptr->text[0] = pathname_ptr->text[0];

    for (counter = 1;
         ((counter < (pathname_ptr->length) ) &&
           ((pathname_ptr->text[0]) != (pathname_ptr->text[counter])));
         counter++)
        {
        volume_ptr->text[counter] = pathname_ptr->text[counter];
        };

    volume_ptr->length = counter;


    /* ************************************************************** *
     *  Return a success to our caller.                               *
     * ************************************************************** */

    KS_SUCCESS();

}   /* End of ks_file_get_volume()                                    */


