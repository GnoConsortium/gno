/*
 * $Id: bmem.c,v 1.1 1997/02/28 05:12:43 gdr Exp $
 *
 * This file is formatted with tabs every 8 columns.
 */

#ifdef __ORCAC__
segment "libc_gen__";
#endif

#pragma optimize 0
#pragma debug 0
#pragma memorymodel 0

#include <string.h>

void
bzero(void *buf, size_t len) {
	memset(buf, 0, len);
}

void
bcopy(const void *src, const void *dest, size_t len) {
	memmove(dest, src, len);
}
