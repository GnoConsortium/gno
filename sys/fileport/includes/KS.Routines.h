
/*
 * Copyright (c) Kopriha Software,  1990-1991
 *       All Rights Reserved
 *
 * ks.routines.h
 *
 * Description: This include file contains the routine entry/exit
 *              macro definitions.
 *
 *
 * Table of contents:
 *
 *   Macros:
 *
 *     ROUTINE_ENTER()  . . . . . . . Macro to indicate a routine
 *                                    was entered.
 *     ROUTINE_EXIT() . . . . . . . . Macro to indicate a routine
 *                                    was exited (success or error).
 *
 *
 * Notes: These macros will be expanded to check the stack pointer
 *        in debugging cases.  Also, a performance set of macros
 *        will be created that will count # of entries to a routine
 *        along with total time spent in the routine.
 *
 *
 *  History:July 13, 1990  Dave  Created this file
 *
 */

#ifndef _KS_ROUTINES_
#define _KS_ROUTINES_

#ifdef DEBUG_CODE
#ifndef __STDIO__
#include <stdio.h>
#endif
#endif



/* ****************************************************************** *
 * Macro definitions:                                                 *
 * ****************************************************************** */

/*
 *  define DEBUG_CODE
 *                     - add # to define to create all modules
 *                       with debug code.
 */

/*
 * ROUTINE_ENTER macro - used on entry to a routine.
 */

#ifndef DEBUG_CODE
#define ROUTINE_ENTER()
#endif

#ifdef DEBUG_CODE
#define ROUTINE_ENTER()                                              \
                                                                     \
        printf("%s: entered\n",                                      \
               ROUTINE_NAME);
#endif


#ifdef HEY_DAVE_WE_MUST_DEBUG_MORE_RIGHT_NOW

#undef ROUTINE_ENTER

#define ROUTINE_ENTER()                                              \
                                                                     \
        asm                                                          \
            {                                                        \
            tsc                                                      \
            sta   >debug_stackptr                                    \
            };                                                       \
                                                                     \
        sprintf(debug_sub1, "%s",                                    \
                ROUTINE_NAME);                                       \
        sprintf(debug_sub2, "%x",                                    \
                debug_stackptr);                                     \
                                                                     \
        debug_sub_array[0] = (Pointer) &debug_sub1;                  \
        debug_sub_array[1] = (Pointer) &debug_sub2;                  \
                                                                     \
        /*                                                           \
         *  Resource reference with C string substitution array...   \
         */                                                          \
                                                                     \
        debug_stackptr = AlertWindow((Word) 4,                       \
                                     (Long) &debug_sub_array,        \
                                     DebugRoutineEntry)

#endif



/*
 * ROUTINE_EXIT macro - used to return to our caller.
 */

#ifndef DEBUG_CODE
#define ROUTINE_EXIT(_error_code)
#endif

#ifdef DEBUG_CODE
#define ROUTINE_EXIT(_error_code)                                    \
                                                                     \
        printf("%s: error return.  Error = %d.\n",                   \
               ROUTINE_NAME,                                         \
               _error_code);
#endif


#ifdef HEY_DAVE_WE_MUST_DEBUG_MORE_RIGHT_NOW

#undef ROUTINE_EXIT

#define ROUTINE_EXIT(_error_code)                                    \
                                                                     \
        asm                                                          \
            {                                                        \
            tsc                                                      \
            sta   >debug_stackptr                                    \
            };                                                       \
                                                                     \
        sprintf(debug_sub1, "%s",                                    \
                ROUTINE_NAME);                                       \
        sprintf(debug_sub2, "%x",                                    \
                debug_stackptr);                                     \
        sprintf(debug_sub3, "%lx",                                   \
                (Long) (_error_code));                               \
                                                                     \
        debug_sub_array[0] = (Pointer) &debug_sub1;                  \
        debug_sub_array[1] = (Pointer) &debug_sub2;                  \
        debug_sub_array[2] = (Pointer) &debug_sub3;                  \
                                                                     \
        /*                                                           \
         *  Resource reference with C string substitution array...   \
         */                                                          \
                                                                     \
        debug_stackptr = AlertWindow((Word) 4,                       \
                                     (Long) &debug_sub_array,        \
                                     DebugRoutineExit)

#endif

#endif
