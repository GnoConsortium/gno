
/*
 * Copyright (c) Kopriha Software,  1991
 *       All Rights Reserved
 *
 * SharedData.H
 *
 * Description: This include file contains the definition of the
 *              shared data structure that has a pointer to it posted
 *              to the message center.
 *
 *
 * Table of contents:
 *
 *   Structures:
 *     KS_SHARED_DATA . . . . . . . Shared data structure
 *
 *
 *  History:April 21, 1991  Dave  Created this file
 *
 */

#ifndef _KS_SHARED_DATA_
#define _KS_SHARED_DATA_

#ifndef _KS_DEFINES_
#include "KS.Defines.H"
#endif

#ifndef __GSOS__
#include <gsos.h>
#endif


/*
 *  Filetype/auxtype of spoolfiles.
 */

#define SPOOLFILE_FILETYPE ((Word) 4)
#define SPOOLFILE_AUXTYPE  ((LongWord) 0)
#define SPOOL_DONE_AUXTYPE ((LongWord) 256)



/*
 * Shared data spoolport flag states
 */

enum port_flags {PORT_NOT_ACTIVE = 1,
                 PORT_READY,
                 PORT_SPOOLING,
                 PORT_ERROR};

/*
 * Shared data spool flag states...
 */

enum spool_flags {SPOOL_INIT = 0,  /* First call - initialize everything  */
                  SPOOL_SCANNING,  /* Scanning spool directory for a file */
                  SPOOL_OPEN,      /* Setting up for printing             */
                  SPOOL_WAITING,   /* Waiting for port to be ready        */
                  SPOOL_PRINTING,  /* Printing a spoolfile                */
                  SPOOL_CLOSE,     /* Finished with a spoolfile           */
                  SPOOL_ERROR};    /* Some error stopped spooling         */


/*
 * KS_SHARED_DATA - generic shared data structure
 */

typedef struct
    {
    KS_STRUCT_ID struct_id;     /* Structure id                      */

    Word  port_driver_length;   /* Length of the port driver filename*/
    char  port_driver_name[64]; /* Name of port driver to print with */

    Word  spool_filename_length;/* Length of spool basename          */
    char  spool_filename[64];   /* Basename of spool file            */

    LongWord buffer_size;       /* User specified buffer size        */
                                /*  (This is the value of the        */
                                /*  scrollbar control)               */

    /*
     * Flag words (should use bits, but we'll be a little foolish):
     */
    Word  delete_flag;          /* Delete spoolfile after printing   */
    Word  print_flag;           /* Print new spoolfiles              */


    /*
     * Data for the spoolport driver
     */

    enum port_flags port_status;
    Word   port_error;
    Word   port_spoolnumber;


    /*
     * Now for the background task information.
     */

    Pointer start_address;      /* Start address of routine - pass   */
                                /*  this to add to run q!            */
    enum spool_flags spool_status;   /* Status of spooler            */
    Word    spool_error;        /* Saved Error from spooler          */
    Word    spool_spoolnumber;  /* Spoolfile number being printed    */
    Pointer file_ptr;           /* Pointer to the file we're spooling*/
    LongWord bytes_written;     /* Bytes written to the printer for  */
                                /*  the current spool file           */
    Word   driver_user_id;      /* Memory manager User ID for spooler*/

    Handle buffer_handle;       /* Handle to the I/O buffer          */
    Byte    *buffer;            /* Pointer to the I/O buffer         */
    LongWord buffer_bytesize;   /* Size of the buffer (in bytes)     */
    LongWord buffer_offset;     /* Offset of next available byte     */


    } KS_SHARED_DATA, *KS_SHARED_DATA_PTR, **KS_SHARED_DATA_HDL;



typedef struct
    {
    Word length;
    char text[768];
    } GSString768, *GSString768Ptr, **GSString768Hdl;


typedef struct
    {
    Word        bufSize;
    GSString768 buffer;
    } ResultBuf768, *ResultBuf768Ptr, **ResultBuf768Hdl;



/*
 * KS_DATA_MESSAGE - record pointer passed to message center by name
 *  This holds most of the user specifiable data.  The only structure
 *  missing is the pathname - and that is in the second message struct...
 */

typedef struct
    {
    Word blockLen;
    char name_length;          /*    123456789 123456789 12345678  */
    char message_name[28];     /* = "Kopriha Software SpoolMaster" */
    KS_SHARED_DATA_HDL shared_data_handle;
    ResultBuf768Hdl    shared_path_handle;
    } KS_NAMED_MESSAGE, *KS_NAMED_MSG_PTR, **KS_NAMED_MSG_HDL;



/*
 * Function prototypes (for message center routines):
 */

void send_message(KS_SHARED_DATA_HDL *,
                  ResultBuf255Hndl *);

void receive_message(KS_SHARED_DATA_HDL *,
                     ResultBuf255Hndl *);

#endif
