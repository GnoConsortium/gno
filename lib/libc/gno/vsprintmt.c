/*
 * Thread-safe and buffer-overrun-safe versions of sprintf and vsprintf
 * Written to support thread-safe versions of the syslog routines.
 *
 * Devin Reade, October 1998
 *
 * $Id: vsprintmt.c,v 1.1 1998/10/31 17:22:05 gdr-ftp Exp $
 */

#ifdef __ORCAC__
#pragma memorymodel 1
segment "libc_gno__";
#endif

#include <string.h>
#include <stdarg.h>
#include <sys/errno.h>
#ifdef __GNO__
#include <gno/gno.h>	/* prototype consistency check */
#endif

/*
 * sprintmt, vsprintmt
 *
 * These are sprintf- and vsprintf-variants that are thread-safe.
 *
 * <buffer> is the place where the formatted string will be put.
 * <blen> is the length of <buffer>.  No more than <blen>-1 characters
 *       will be placed into <buffer>.
 * <format> gives the printf-like format string.
 * <ap>	is the usual vararg thing.
 *
 * RETURNS:	A pointer the the NULL-terminator at the end of <buffer>
 *
 * Only a subset of printf format specifiers are permitted.  They are:
 *	%d	signed int (decimal output)
 *	%u	unsigned int (decimal output)
 *	%x	unsigned int (decimal output)
 *	%s	string
 *	%%	the literal, "%"
 * There is also an extra format specifier:
 *	%m	the string currently returned by strerror();
 *
 * In addition, the %d, %u, and %x format specifiers may use the 'l'
 * modifier (%ld, %lu, %lx) for long signed and unsigned ints.
 *
 * Floating point format specifiers are not supported.
 *
 * Any other character following the '%' is treated as a literal.  For
 * example, '%A' would be printed as "A".
 *
 * LIMITATION:
 *	This routines assumes that shorts are the same length as ints.
 */

#define PRINTCHAR(c) \
{ \
  if (blen > 0) { \
    *bptr++ = (c); \
    --blen; \
  } \
}
#define MAX_ULONG_CHARS 11	/* > number of digits in an unsigned long */

char *
vsprintmt (char *buffer, size_t blen, const char *format, va_list ap)
{
  unsigned long uival;
  long int ival;
  const char *fptr;
  char *bptr, *sval;
  int i, j;
  short base, islong, capX;
  char intbuffer[MAX_ULONG_CHARS];

  fptr = format-1;  /* we'll increment this right away */
  bptr = buffer;
  --blen;           /* leave space for the null-terminator */

nextchar:
  fptr++;
  islong = 0;
  capX = 0;

  if (*fptr == '\0') {
    goto done;
  }

  if (*fptr != '%') {
    /* regular character */
    PRINTCHAR(*fptr);
    goto nextchar;
  }
    
  fptr++;   /* skip the '%' */

  if (*fptr == '\0') {
    goto done;
  }

  if (*fptr == '%') {
    /*
     * '%%' is a literal percent.  Check it here so that we don't
     * treat "%l%..." the same as "%%...".
     */
    PRINTCHAR(*fptr);
    goto nextchar;
  }
      
  if (*fptr == 'l') {
    /* 
     * long qualifier, unless at end of string, in which case it's
     * ignored. "%ls" and "%lm" are treated like "%s" and "%lm", for
     * simplicity's sake
     */
    islong = 1;
    fptr++;
    if (*fptr == '\0') {
      goto done;
    }
  }

  /* At this point, we're at the main type specifier */
  switch (*fptr) {
  case '\0':
    /* '%' at end of format string?  Ignore it. */
    goto done;

  case 'd':
    if (islong) {
      ival = va_arg(ap, long int);
    } else {
      ival = va_arg(ap, int);
    }
    goto printint;

  case 'X':
    capX = 1;
    /*FALLTHROUGH*/
  case 'x':
  case 'u':
    if (islong) {
      uival = va_arg(ap, unsigned long int);
    } else {
      uival = va_arg(ap, unsigned int);
    }
    if (*fptr == 'u') {
      goto printuint;
    } else {
      goto printhex;
    }
    /*NOTREACHED*/

  case 's':
  case 'm':
    
    if (*fptr == 's') {
      sval = va_arg(ap, char *);
    } else {
      sval = strerror(errno);
    }
    while ((blen > 0) && *sval) {
      *bptr++ = *sval++;
      --blen;
    }
    goto nextchar;

  default:
    /* treat everything else following '%' as a literal */
    PRINTCHAR(*fptr);
    goto nextchar;
  }

  /*
   * Nothing falls through this point.  In order to get here, you have
   * to jump your way in.  However, to be on the safe side, we put the
   * 'done' label here.
   */
done:
  *bptr = '\0';
  return bptr;

printhex:
  /*
   * Prereq:  uival contains an unsigned integer
   */
  base = 16;
  goto printunsigned;
  
printuint:
  /* 
   * Prereq:  uival contains an unsigned integer
   */
  base = 10;
  goto printunsigned;
  
printint:
  /*
   * Prereq:  ival contains a signed integer
   */
  if (ival < 0) {
    uival = -ival;
    PRINTCHAR('-');
  } else {
    uival = ival;
  }
  base = 10;
  goto printunsigned;
  
printunsigned:
  /*
   * Prereq:  - base is set to the number base (10 or 8)
   *          - uival is set to the value to print
   * Uses:    i, intbuffer
   */
  i = 0;
  if (uival == 0) {
    intbuffer[i++] = '0';
  } else {
    while (uival > 0 && i < MAX_ULONG_CHARS) {
      j = uival % base;
      if ( j>9 ) {
	intbuffer[i++] = (capX ? 'A' : 'a') + (j-10);
      } else {
	intbuffer[i++] = '0' + j;
      }
      uival /= base;
    }
  }
  --i;
  while (i>=0) {
    PRINTCHAR(intbuffer[i]);
    --i;
  }
  intbuffer[i]='\0';
  goto nextchar;

}
#undef PRINTCHAR

#pragma optimize 78
#pragma debug 0

char *
sprintmt (char *buffer, size_t blen, const char *format, ...) {
	va_list ap;
	char *result;

	va_start(ap, format);
	result = vsprintmt(buffer, blen, format, ap);
	va_end(ap);
	return result;
}

#if 0	/* testing, only */

#pragma debug 25
#pragma optimize 0

#include <stdio.h>

#define BUFFERSIZE 32
#define I   -1234
#define IL  -12345678L
#define U    5678
#define UL   12345678L
#define X    5432
#define XL   98765432L
#define SHORTSTRING "a short test string"
#define LONGSTRING  "123456789 123456789 123456789 123456789 123456789 123456789"

int main (int argc, char **argv) {

  static char buffer[BUFFERSIZE];
  int i;
  
  sprintmt(buffer, BUFFERSIZE, "-->%d<--", I);
  printf("1\t%d:\t%s\n", I, buffer);

  sprintmt(buffer, BUFFERSIZE, "-->%u<--", U);
  printf("2\t%u:\t%s\n", U, buffer);

  sprintmt(buffer, BUFFERSIZE, "-->%x<--", X);
  printf("3\t%x:\t%s\n", X, buffer);

  sprintmt(buffer, BUFFERSIZE, "-->%X<--", X);
  printf("4\t%x:\t%s\n", X, buffer);

  sprintmt(buffer, BUFFERSIZE, "-->%ld<--", IL);
  printf("5\t%ld:\t%s\n", IL, buffer);

  sprintmt(buffer, BUFFERSIZE, "-->%lu<--", UL);
  printf("6\t%lu:\t%s\n", UL, buffer);

  sprintmt(buffer, BUFFERSIZE, "-->%lx<--", XL);
  printf("7\t%lx:\t%s\n", XL, buffer);

  sprintmt(buffer, BUFFERSIZE, "-->%lX<--", XL);
  printf("8\t%lX:\t%s\n", XL, buffer);

  sprintmt(buffer, BUFFERSIZE, "-->%s<--", SHORTSTRING);
  printf("9\tshort:\t\"%s\"\n", buffer);

  sprintmt(buffer, BUFFERSIZE, "-->%s<--", LONGSTRING);
  printf("10\tlong:\t\"%s\"\n", buffer);

  for (i=0; i<sys_nerr; i++) {
    errno = i;
    sprintmt(buffer, BUFFERSIZE, "-->%m<--");
    printf("%d\terrno %d:\t\"%s\"\n", 11+i, i, buffer);
  }
  return 0;
}

#endif	/* 0 */
