
/*
 * Copyright (c) Kopriha Software,  1990-1991
 *       All Rights Reserved
 *
 * KS.CDev.h
 *
 * Description: This include file contains all the external
 *              variable and routine definitions (prototypes)
 *              for the SpoolMaster CDEV.
 *
 *
 * Table of contents:
 *
 *   Defined variables:
 *
 *      All CDEV flags passed the CDEV entry point
 *      All control IDs of the defined controls
 *      The port driver file/aux type
 *
 *
 *   External variables:
 *
 *      static_text_substitution
 *      text0_string
 *      text_control_handle
 *
 *
 *   External routines:
 *
 *      do_about()
 *      do_create()
 *      do_hit()
 *
 *      scroll_action()
 *
 *
 *
 *  History:May 18, 1991   Dave  Created this file
 *
 */

#ifndef _KS_CDEV_
#define _KS_CDEV_

/*  define HEY_DAVE_WE_MUST_DEBUG_MORE  */



#ifndef _KS_FILEIO_
#include "KS.FileIO.E"
#endif

#ifndef _KS_DEFINES_
#include "KS.Defines.h"
#endif

#ifndef _KS_ROUTINES_
#include "KS.Routines.h"
#endif

#ifndef _KS_MEMORY_
#include "ks.memory.h"
#endif

#ifndef _KS_SHARED_DATA_
#include "SharedData.h"
#endif

#ifndef __CONTROL__
#include <control.h>
#endif

#ifndef __LIST__
#include <list.h>
#endif

#ifndef __STDFILE__
#include <StdFile.h>
#endif

#ifndef  __RESOURCES__
#include <Resources.h>
#endif

#ifndef __LOADER__
#include <Loader.h>
#endif



/* ****************************************************************** *
 *  Macro definitions:                                                *
 * ****************************************************************** */


/* ****************************************************************** *
 *  The following macro simplifies by global variable definitions...  *
 *                                                                    *
 *  The reasoning goes like this:                                     *
 *    EXTERNAL is defined as extern by all modules that include this  *
 *      file but have not defined EXTERNAL (IE: All but Data.CC).     *
 *    Data.CC defines EXTERNAL to be nothing - thus we create the     *
 *      data in Data.CC but do not have to duplicate the global       *
 *      variable definitions...                                       *
 *                                                                    *
 *  This seems to simplify my maintenance - once copy of variable     *
 *   definitions to support...                                        *
 * ****************************************************************** */

#ifndef EXTERNAL
#define EXTERNAL extern
#endif



/* ****************************************************************** *
 *  The following are all of the currently defined 'events' that are  *
 *  passed to the CDEV entry point.                                   *
 * ****************************************************************** */

#define MachineCDEV     1
#define BootCDEV        2
#define Reserved        3
#define InitCDEV        4
#define CloseCDEV       5
#define EventsCDEV      6
#define CreateCDEV      7
#define AboutCDEV       8
#define RectCDEV        9
#define HitCDEV         10
#define RunCDEV         11


/* ****************************************************************** *
 *  The following is a list of all the controls we have defined for   *
 *  the spoolmaster CDEV                                              *
 *                                                                    *
 *  Note: Static text controls are not included in this list.         *
 * ****************************************************************** */

#define MainControlList       (1L)

#define PortListControl       (3L)
#define SpoolDirButtonControl (4L)
#define ScrollBarControl      (6L)
#define BufferSizeTextControl (7L)
#define PrintFileCheckBoxCtl  (8L)
#define DeleteFileCheckBoxCtl (9L)

#define HelpControl           (0x0100L)

#define CantWriteAlert        (1L)
#define CantWriteOkButton     (0)
#define CantWriteTryAgain     (1)

#define SpoolDirPrompt        (1L)



#ifdef HEY_DAVE_WE_MUST_DEBUG_MORE

#define DebugCloseAlert       (2L)
#define DebugRoutineEntry     (3L)
#define DebugRoutineExit      (4L)
#define DebugEndOfLoop        (5L)
#define DebugStandardFile     (6L)

#endif


/*
 * Macro definition of print task id (for resource file)
 */

#define PrintTask             (2L)


/*
 * Macro definitions of the saved user parameters
 */

#define SpoolDefaultsType     (0x0a13)
#define rMyCodeResource       (0x0a14)


#define SpoolOptions          (0x010L)
#define SpoolPrefix           (0x010L)

/*
 * Macro definitions of the standard file dialog template item numbers
 */

#define SFDT_SaveItem        1
#define SFDT_OpenItem        2
#define SFDT_CloseItem       3
#define SFDT_NextItem        4
#define SFDT_CancelItem      5
#define SFDT_ScrollItem      6
#define SFDT_PathItem        7
#define SFDT_FileItem        8
#define SFDT_PromptItem      9
#define SFDT_FilenameItem   10
#define SFDT_FreeSpaceItem  11
#define SFDT_NewFolderItem  12



/* ****************************************************************** *
 *  Port driver filetype and auxtype - used when we're building the   *
 *  port driver list...  Apple should have defined these...           *
 * ****************************************************************** */

#define PORT_DRIVER_TYPE    ((Word) 0x0bb)
#define PORT_DRIVER_AUXTYPE ((Long) 2)


/* ****************************************************************** *
 *  Structure for the loader (actually InitialLoad2 with flagWord==2. *
 * ****************************************************************** */

typedef struct
    {
    Pointer memoryAddress;
    Word    fileLength;
    } InitialLoad2_Pkt;



/* ****************************************************************** *
 *  Standard file name/path reference description for an undefined    *
 *  reference...  Apple should have defined this...                   *
 * ****************************************************************** */

#define STDFILE_REFERENCE_UNDEFINED ((Word) 3)


/* ****************************************************************** *
 *  Non-purgable purge level for memory blocks (setup by the memory   *
 *  manager on a SetPurge.  Apple should have defined this...         *
 * ****************************************************************** */

#define NOT_PURGEABLE ((Word) 0)



/* ****************************************************************** *
 *  Maximum string stubstitution index (apple should have defined     *
 *  this)                                                             *
 * ****************************************************************** */

#define MAX_SUB_INDEX  10



/* ****************************************************************** *
 *  Macros to get the filename/pathname from a standard file dialog.  *
 * ****************************************************************** */

#define DerefFileName ((** (ResultBuf255 **) ReplyRecord.nameRef).bufString)
#define DerefFilePath ((** (ResultBuf255 **) ReplyRecord.pathRef).bufString)



/* ****************************************************************** *
 * External definitions:                                              *
 * ****************************************************************** */

/*
 * static_text_substitution[] - is the static text substitution array.
 *
 * We use text substitution to display the selected buffer size.
 */

EXTERNAL Pointer static_text_substitution[MAX_SUB_INDEX];

/*
 * text0_string[] - this is the single substitution string we use.
 */

EXTERNAL char text0_string[10];


/*
 * cdev_window_handle - is the handle to the window of the CDEV.
 */

EXTERNAL WindowPtr cdev_window_handle;


/*
 * text_control_handle - handle to the static text that will
 *                       display the selected buffer size.
 *
 * Note: This is defined here as a performance improvement to
 *       the user interface (I see no reason to continually make
 *       the toolbox call to get this handle to redraw the text...).
 */

EXTERNAL CtlRecHndl text_control_handle;


/*
 * Flags indicating which toolsets needed to be started...
 */

EXTERNAL Word QDAuxStarted;
EXTERNAL Word IMStarted;
EXTERNAL Word FMStarted;
EXTERNAL Word TEStarted;
EXTERNAL Word LMStackPointer;
EXTERNAL Word OriginalStackPtr;


/*
 * Handles to the memory allocated for toolsets that we started.
 */

EXTERNAL Handle MemoryForFM;
EXTERNAL Handle MemoryForTE;
EXTERNAL Handle MemoryForLM;


/*
 * list_handle/list_pointer are the handle/pointer to the Member
 *  record for the port driver list.
 */

EXTERNAL MemRecHndl list_handle;
EXTERNAL MemRecPtr  list_pointer;
EXTERNAL CtlRecHndl list_control;
EXTERNAL Word       number_of_ports;
EXTERNAL Word       selected_port;

/*
 * orig_control_values - this is a SHARED_DATA structure that holds
 *  the original copy of all the control values.
 *
 * It is currently setup when we process the OPEN event of the CDEV!
 *  This will soon change to the INIT event.
 */

EXTERNAL KS_SHARED_DATA_HDL shared_data_handle;
EXTERNAL KS_SHARED_DATA_PTR shared_data_values;

EXTERNAL ResultBuf255Hndl shared_spooldir_hdl;
EXTERNAL ResultBuf255Ptr shared_spooldir_ptr;


EXTERNAL KS_SHARED_DATA_HDL new_shared_handle;
EXTERNAL KS_SHARED_DATA_PTR new_shared_values;



/*
 * dialog_filename - this is the temporary filename that we display
 *  with the standard file dialog.
 */

typedef struct
    {
    Word length;
    char text[64];
    } GSString64;

EXTERNAL GSString64 dialog_filename;
EXTERNAL GSString64 backup_dialog_filename;


/*
 * Prefix<0,8> - are handles we use to save prefix 0 and prefix 8.
 */

EXTERNAL Handle Prefix0;
EXTERNAL Handle Prefix8;


/*
 * SpoolPrefixDirectory - handle used to save the new spool directory
 */

EXTERNAL Handle SpoolPrefixDirectory;
EXTERNAL Handle SpoolPrefixBackup;


/*
 * handles/pointers to the control templates that are
 *  used only during the save of changed controls.
 */

EXTERNAL Handle CheckBoxHandle;
EXTERNAL RadioButtonTemplate *CheckBoxControlTemplate;
EXTERNAL Handle ScrollBarHandle;
EXTERNAL ScrollBarTemplate *ScrollBarControlTemplate;



/*
 * The following four variables are from the build_port_list()
 *  routine.  This is part of the effort to reduce stack usage
 *  so the finder can work for us...
 */

EXTERNAL KS_FILE_PTR     bpl_file_ptr;
EXTERNAL char           *bpl_port_name;
EXTERNAL DirEntryRecGS   bpl_dir_entry;
EXTERNAL ResultBuf32     bpl_filename;


/*
 * ReplyRecord - returned filled in by standard file dialog...
 */

EXTERNAL SFReplyRec2 ReplyRecord;


/*
 * il2_pkt - packet used to load a memory resident code section.
 */

EXTERNAL InitialLoad2_Pkt il2_pkt;


/*
 *  port_rec - used to load the print task code.
 */

EXTERNAL InitialLoadOutputRec port_rec;




/*
 * The following externals exist only when we are debugging
 *  the close code - if they are defined then when we are
 *  about to write the 'changed' data to the resource fork,
 *  then we will put up an alert window with all the variables
 *  displayed so we can see that they are correct.
 */

#ifdef HEY_DAVE_WE_MUST_DEBUG_MORE

EXTERNAL Pointer debug_sub_array[10];
EXTERNAL char    debug_sub1[80];
EXTERNAL char    debug_sub2[80];
EXTERNAL char    debug_sub3[80];
EXTERNAL char    debug_sub4[80];
EXTERNAL char    debug_sub5[80];
EXTERNAL Word    debug_stackptr;

#endif


/*
 * Pathname to directory to select port driver from...
 *
 * It is not declared with the EXTERNAL macro because of the
 *  static initialization...
 */

extern GSString32 port_dir_path;


/*
 * spoolmaster_path - the complete path to where the
 *  spoolmaster CDEV lives...  I'd like to remove this
 *  variable...
 */

extern GSString32 spoolmaster_path;


/*
 * my_sf_dialog_template - is my special standard file dialog templates
 */

extern Word my_sf_dialog320;
extern Word my_sf_dialog640;


/* ****************************************************************** *
 *  Function Prototypes:                                              *
 * ****************************************************************** */

/*
 * Functions from Spool.CC
 */

void do_boot(void);

void do_about(WindowPtr);

Long do_create(WindowPtr);

void do_close(void);

void do_hit(CtlRecHndl,
            Long);

/*
 * Functions from Controls.CC
 */

void scroll_action(CtlRecHndl,
                   Word);

void build_port_list(void);

void save_control_values(void);

void GetRidOfSFMemory(void);        /* Standard file support... */
int SFStart(int *, Handle *);
void SFStop(int, Handle);
void sfdialog_hook(Pointer, Word *);
int SFPut(void);
int PrefixToHandle (Word, Handle *);
void HandleToPrefix (Word, Handle);
int SavePrefixes(void);
void RestorePrefixes (void);


#endif
