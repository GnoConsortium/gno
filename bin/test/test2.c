#line 1 ":trenco4:custom.src:test:test2.c"
/*
 * $Id: test2.c,v 1.1 1996/02/10 08:27:33 gdr Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include "operators.h"

#if __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

void
#if __STDC__
err(const char *fmt, ...)
#else
err(fmt, va_alist)
	char *fmt;
        va_dcl
#endif
{
	va_list ap;
#if __STDC__
	va_start(ap, fmt);
#else
	va_start(ap);
#endif
	(void)fprintf(stderr, "test: ");
	(void)vfprintf(stderr, fmt, ap);
	va_end(ap);
	(void)fprintf(stderr, "\n");
	exit(2);
	/* NOTREACHED */
}
