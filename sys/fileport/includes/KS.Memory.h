
/*
 * Copyright (c) Kopriha Software,  1991
 *       All Rights Reserved
 *
 * ks.memory.h
 *
 * Description: This include file contains all the memory
 *              support macros.
 *
 *
 * Table of contents:
 *
 *   Macros:
 *
 *     KS_MEMORY_ALLOCATE()  . . . . . Allocate a block of memory
 *     KS_MEMORY_FREE()  . . . . . . . Free a block of memory
 *
 *
 *  Notes: Why create these macros instead of using the standard
 *         C I/O routines?  I believe that the support
 *         routines will require some user interface work to handle
 *         cases like when we run out of memory.  Standard C memory
 *         routines don't give me the ability to do such processing.
 *
 *         For now, these macros will simply call the C library
 *         routines (malloc and free).  I'm sure this will change
 *         in the future.
 *
 *
 *  History: March 2, 1991  Dave  Created this file
 *
 */

#ifndef _KS_MEMORY_
#define _KS_MEMORY_

#ifndef _PORTABLE_C_
#include "Portable.C.h"
#endif

#ifndef _stdlib_
#include <stdlib.h>
#endif

#ifndef __MEMORY__
#include <memory.h>
#endif





/*
 * KS_MEMORY_ALLOCATE() - Allocate some memory
 */

#define KS_MEMORY_ALLOCATE(_memory_attr,_memory_size,_memory_id,_memory_hdl,_error)\
                                                                       \
    (_memory_hdl) = NewHandle((Long) (_memory_size),                   \
                              (_memory_id),                            \
                              (_memory_attr),                          \
                              0L);                                     \
    (_error) = GET_ERROR()



/*
 * KS_MEMORY_DEALLOCATE() - Release a block of memory back to the
 *                          free pool of memory
 */

#define KS_MEMORY_DEALLOCATE(_memory_hdl, _error)                      \
                                                                       \
    DisposeHandle( (_memory_hdl));                                     \
    _error = KS_E_SUCCESS




/* ****************************************************************** *
 *  Memory routine prototypes:                                        *
 * ****************************************************************** */

#endif
