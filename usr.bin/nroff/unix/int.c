/*
 * This is an interface file for use with Insight.
 *
 * $Id: int.c,v 1.1 1997/03/14 06:22:27 gdr Exp $
 */

#include <stdarg.h>
#include "err.h"

void
verr(int eval, const char *fmt, va_list ap)
{
    iic_error(USER_ERROR, "verr traceback");
    verr(eval, fmt, ap);
}


void
verrx(int eval, const char *fmt, va_list ap)
{
    iic_error(USER_ERROR, "verrx traceback");
    verrx(eval, fmt, ap);
}

void
vwarn(const char *fmt, va_list ap)
{
    iic_error(USER_ERROR, "dummy traceback");
    vwarn(fmt, ap);
}


void
vwarnx(const char *fmt, va_list ap)
{
    iic_error(USER_ERROR, "dummy traceback");
    vwarnx(fmt, ap);
}
