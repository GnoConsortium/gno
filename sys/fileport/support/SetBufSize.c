
/*
 * Copyright (c) Kopriha Software,  1990-1991
 *       All Rights Reserved
 *
 *  SetBufSize.CC
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
 *   ks_file_set_buffer_size - set the specified I/O buffer size.     *
 *                                                                    *
 *   Note: Any open files that are using buffering will NOT change    *
 *         their buffer sizes until they need to do I/O!              *
 *                                                                    *
 *   History: Mar 3, 1990  Dave  Created this routine                 *
 * ****************************************************************** */

#undef ROUTINE_NAME
#define ROUTINE_NAME "ks_file_set_buffer_size"

KS_E_ERROR ks_file_set_buffer_size(LongWord buffer_size)
{
    /* ************************************************************** *
     *  Local declarations:                                           *
     * ************************************************************** */


    ROUTINE_ENTER();


    /* ************************************************************** *
     *  Set the buffer size back to our caller.                       *
     * ************************************************************** */

    KSf_FileBufferSize = buffer_size;

    KS_SUCCESS();

}   /* End of ks_file_set_buffer_size()                               */


