
/*
 * Copyright (c) Kopriha Software,  1990-1991
 *       All Rights Reserved
 *
 *  AddFile.CC
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
 *   ks_file_path_add_file - Append a filename (or partial filename)  *
 *                           to a pathname - although you can pass any*
 *                           pait of GS/OS strings.                   *
 *                                                                    *
 *   History: Mar 24, 1991  Dave  Created this routine                *
 * ****************************************************************** */

#undef ROUTINE_NAME
#define ROUTINE_NAME "ks_file_path_add_file"


KS_E_ERROR ks_file_path_add_file(GSString255Ptr path_ptr,
                                 GSString255Ptr file_ptr)
{
    /* ************************************************************** *
     *  Local declarations:                                           *
     * ************************************************************** */


    ROUTINE_ENTER();


    /* ************************************************************** *
     *  Append the filename to the pathname.                          *
     * ************************************************************** */

    COPY_BYTES(&file_ptr->text,
               0,
               &path_ptr->text[path_ptr->length],
               0,
               file_ptr->length);

    path_ptr->length = path_ptr->length + file_ptr->length;


    /* ************************************************************** *
     *  Return a success to our caller.                               *
     * ************************************************************** */

    KS_SUCCESS();

}   /* End of ks_file_path_add_file()                                 */



