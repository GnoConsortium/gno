
/*
 * Copyright (c) Kopriha Software,  1990
 *       All Rights Reserved
 *
 * Portable.C.h
 *
 * Description: This include file contains all the portable C language
 *              definitions for Font Forge.
 *
 *
 * Table of contents:
 *
 *   Structures:
 *
 *   Macros:
 *
 *     COPY_BYTES . . . . . . . . . . Copy bytes from one place to another
 *     MAX  . . . . . . . . . . . . . Return the maximum of two values
 *     MIN  . . . . . . . . . . . . . Return the minimum of two values
 *     TO_POINTER . . . . . . . . . . Convert some pointer type to Pointer
 *     ZERO . . . . . . . . . . . . . Initialize a structure to zero
 *
 *
 *  Notes: Wherever possible, the macros defined in this module
 *         will be used!!!!
 *
 *  History:August 10, 1990  Dave  Created this file
 *
 */

#ifndef _PORTABLE_C_
#define _PORTABLE_C_

#ifndef __stddef__
#include <stddef.h>
#endif


/*
 * COPY_BYTES() - Copies the specified number of bytes
 */

#define COPY_BYTES(_src_ptr, _src_offset, _dest_ptr, _dest_offset, _size)\
                                                                \
    BlockMove( &(((char *) (_src_ptr))[(_src_offset)]),         \
               &(((char *) (_dest_ptr))[(_dest_offset)]),       \
               (Long) (_size))



/*
 * GET_ERROR() - returns the error from the last toolbox call.
 *            Use this macro only in a assignment statement
 *            (IE: z = MAX(x,y);)
 */

#define GET_ERROR()                                             \
                                                                \
    toolerror()



/*
 * MAX(a,b) - returns the minimum value, use this macro only
 *            in a assignment statement (IE: z = MAX(x,y);)
 */

#define MAX(_arg1, _arg2)                                       \
                                                                \
    (((_arg1) > (_arg2)) ? (_arg1) : (_arg2))



/*
 * MIN(a,b) - returns the minimum value, use this macro only
 *            in a assignment statement (IE: z = MIN(x,y);)
 */

#define MIN(_arg1, _arg2)                                       \
                                                                \
    (((_arg1) < (_arg2)) ? (_arg1) : (_arg2))



/*
 * TO_POINTER(some_pointer) - returns some_pointer typecase as
 *            a Pointer.  Use this macro only in a assignment
 *            statement (IE: z = MIN(x,y);)
 */

#define TO_POINTER(_some_pointer)                               \
                                                                \
    ((Pointer) (_some_pointer))



/*
 * ZERO(structure) - sets a structure to zero.
 */

#define ZERO(_struct)                                           \
                                                                \
    memset( (char *) &(_struct),                                \
            0,                                                  \
            ((size_t) sizeof(_struct)) )


#endif
