/*
 * Work around a change in the ORCA/C libraries: function va_end
 * is now __va_end, which breaks some GNO/ME libraries.
 */

#include <stdarg.h>

#undef va_end

void va_end (va_list ap)
{
   __va_end (ap);
}
