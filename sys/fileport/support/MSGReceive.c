
/*
 * Copyright (c) Kopriha Software,  1991
 *       All Rights Reserved
 *
 * MSGReceive.CC
 *
 * Description:
 *     This module is message center options from the menus..
 *
 *
 *   Table of Contents:
 *
 *     retrieve_message . . Get message by name for our caller...
 *
 *  History:April 23, 1991  Dave  Created this file
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
 *  receive_message() - Get a pointer to the port data to the message *
 *                      center by name...                             *
 *                                                                    *
 *   History: Apr 23, 1991  Dave  Created this routine                *
 * ****************************************************************** */

#undef ROUTINE_NAME
#define ROUTINE_NAME "receive_message"

void receive_message(KS_SHARED_DATA_HDL *data_hdl,
                     ResultBuf255Hndl   *path_hdl)
{

    ResponseRecord   response_rec;
    KS_NAMED_MESSAGE named_msg;
    KS_NAMED_MSG_PTR msg_rec_ptr;
    Word             message_id;
    Handle           msg_handle;
    KS_E_ERROR       error;
    extern Word      ProgramID;


    *data_hdl = NULL;
    *path_hdl = NULL;


    /* ************************************************************** *
     *  Start by getting the message id for the named message         *
     * ************************************************************** */

    named_msg.blockLen = sizeof(KS_NAMED_MESSAGE);

    strcpy(named_msg.message_name, "Kopriha Software SpoolMaster");

    named_msg.name_length = strlen(named_msg.message_name);

    response_rec = MessageByName(FALSE,
                                 (void *) &named_msg);

    if (GET_ERROR() != KS_E_SUCCESS)
        {
        return;
        };

    message_id = (response_rec & 0xffff);


    /* ************************************************************** *
     *  Get the complete message from the message center              *
     * ************************************************************** */

    KS_MEMORY_ALLOCATE(attrNoSpec + attrNoCross,
                       sizeof(KS_NAMED_MESSAGE), /* could be anything */
                       ProgramID,
                       msg_handle,
                       error);

    if (error != KS_E_SUCCESS)
        {
        return;
        };

    MessageCenter((Word) getMessage,
                  (Word) message_id,
                  (Handle) msg_handle);

    if (GET_ERROR() == KS_E_SUCCESS)
        {
        msg_rec_ptr = (KS_NAMED_MSG_PTR)
                       &(((MessageRecPtr) *msg_handle)->messageData);

        *data_hdl = msg_rec_ptr->shared_data_handle;
        *path_hdl = (ResultBuf255Hndl) msg_rec_ptr->shared_path_handle;
        };


    KS_MEMORY_DEALLOCATE(msg_handle,
                         error);


    /* ************************************************************** *
     *  Return to our caller.                                         *
     * ************************************************************** */

    return;

}   /* End of receive_message()                                       */
