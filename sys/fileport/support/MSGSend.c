
/*
 * Copyright (c) Kopriha Software,  1991
 *       All Rights Reserved
 *
 * MSGSend.CC
 *
 * Description:
 *     This module is message center options from the menus..
 *
 *
 *   Table of Contents:
 *
 *     send_message . . . . Send a message to the message center (by name)
 *     retrieve_message . . Get message by name for our caller...
 *
 *  History:April 23, 1991  Dave  Created this file
 *
 *          July   2, 1991  Dave  Changed the parameters to pointers to
 *                                handles (so our caller can work with
 *                                the real shared data - not the original
 *                                copy that will be purged soon).
 *
 */

/*
 *  define DEBUG_CODE
 *                     - add # to define to create the local
 *                       debug code (IE:module)
 */

#include <string.h>

#ifndef __LOCATOR__
#include <Locator.h>
#endif

#ifndef __MISCTOOL__
#include <MiscTool.h>
#endif

#ifndef _KS_DEFINES_
#include "KS.Defines.h"
#endif

#ifndef _KS_MEMORY_
#include "KS.Memory.H"
#endif

#ifndef _KS_SHARED_DATA_
#include "SharedData.H"
#endif


#pragma noroot



/* ****************************************************************** *
 *  send_message() - Send a few handles to the message center by name.*
 *                   (IE: Create some shared memory for the different *
 *                        parts of the print spooler).                *
 *                                                                    *
 *   Note: This routine used to take a pointer to the data items to   *
 *         setup in shared memory (by copying into memory that is     *
 *         allocated under a different user id).  The problem with    *
 *         that scheme is that the caller does not have access to the *
 *         shared structure on our return.  The simple change is to   *
 *         pass pointers to the handle to the data.  This routine     *
 *         then change the handles to point to the real shared data   *
 *         so our caller can work with it.                            *
 *                                                                    *
 *   History: Apr 23, 1991  Dave  Created this routine                *
 * ****************************************************************** */

#define ROUTINE_NAME "send_message"

void send_message(KS_SHARED_DATA_HDL *shared_data_template,
                  ResultBuf255Hndl   *shared_path_template)
{

    ResponseRecord   response_rec;
    KS_NAMED_MESSAGE named_msg;
    Word             user_id;
    Handle           new_handle;
    KS_SHARED_DATA_PTR data_ptr;
    ResultBuf768Ptr    path_ptr;
    KS_E_ERROR         error;


    /* ************************************************************** *
     *  Setup the header of the message center message... We're       *
     *  doing this here because the name of the message is not a      *
     *  C string to the system - here we can overwrite the element    *
     *  beyond of the message name without causing a problem (like    *
     *  blowing away the first byte of that element...).              *
     * ************************************************************** */

    named_msg.blockLen = sizeof(KS_NAMED_MESSAGE);

    strcpy(named_msg.message_name, "Kopriha Software SpoolMaster");

    named_msg.name_length = strlen(named_msg.message_name);


    /* ************************************************************** *
     *  Get a new user id for us to allocate the shared structures.   *
     * ************************************************************** */

    user_id = GetNewID((Word) 0xA000); /* Using a setup file id...   */

    (**shared_data_template)->driver_user_id = user_id;



    /* ************************************************************** *
     *  Grab a couple of chunks of memory to setup the shared data    *
     *  structures                                                    *
     * ************************************************************** */

    KS_MEMORY_ALLOCATE(attrNoSpec + attrLocked,
                       sizeof(KS_SHARED_DATA),
                       user_id,
                       new_handle,
                       error);

    if (error != KS_E_SUCCESS)
        {
        return;
        };

    data_ptr = (KS_SHARED_DATA_PTR) *new_handle;
    named_msg.shared_data_handle = (KS_SHARED_DATA_HDL) new_handle;


    KS_MEMORY_ALLOCATE(attrNoSpec + attrLocked,
                       sizeof(ResultBuf768),
                       user_id,
                       new_handle,
                       error);

    if (error != KS_E_SUCCESS)
        {
        return;
        };

    path_ptr = (ResultBuf768Ptr) *new_handle;
    named_msg.shared_path_handle = (ResultBuf768Hdl) new_handle;


    /* ************************************************************** *
     *  Copy the passed in structures to the new shared structure.    *
     * ************************************************************** */

    COPY_BYTES(**shared_data_template,
               0,
               data_ptr,
               0,
               sizeof(KS_SHARED_DATA));


    COPY_BYTES(**shared_path_template,
               0,
               path_ptr,
               0,
               sizeof(ResultBuf768));


    /* ************************************************************** *
     *  Send the pointer to our shared data by name to the message    *
     *  center                                                        *
     * ************************************************************** */

    response_rec = MessageByName(TRUE,
                                 (void *) &named_msg);


    /* ************************************************************** *
     *  Ok - We've sent the message to the message center - return    *
     * ************************************************************** */

    *shared_data_template = named_msg.shared_data_handle;

    return;

}   /* End of send_message()                                          */
