/****************************************************************
*
*  ctype.h - character types
*
*  February 1989
*  Mike Westerfield
*
*  Copyright 1989
*  Byte Works, Inc.
*
****************************************************************/

#ifndef __ctype__
#define __ctype__

extern char __ctype[],__ctype2[];

#define __digit         0x01
#define __upper         0x02
#define __lower         0x04
#define __control       0x08
#define __punctuation   0x10
#define __space         0x20
#define __hex           0x40
#define __print         0x80

#define __csym          0x01
#define __csymf         0x02
#define __octal         0x04
#define	__blank		0x08

#define _isalnum(c)      ((__ctype)[(c)+1] & (__upper|__lower|__digit))
#define _isalpha(c)      ((__ctype)[(c)+1] & (__upper|__lower))
#define _isascii(c)      ((unsigned)(c) < 0x0080)
#define _isblank(c)      ((__ctype2)[(c)+1] & __blank)
#define _iscntrl(c)      ((__ctype)[(c)+1] & __control)
#define _iscsym(c)       ((__ctype2)[(c)+1] & __csym)
#define _iscsymf(c)      ((__ctype2)[(c)+1] & __csymf)
#define _isdigit(c)      ((__ctype)[(c)+1] & __digit)
#define _isgraph(c)      ((__ctype)[(c)+1] & (__upper|__lower|__digit|__punctuation))
#define _islower(c)      ((__ctype)[(c)+1] & __lower)
#define _isodigit(c)     ((__ctype2)[(c)+1] & __octal)
#define _isprint(c)      ((__ctype)[(c)+1] & __print)
#define _ispunct(c)      ((__ctype)[(c)+1] & __punctuation)
#define _isspace(c)      ((__ctype)[(c)+1] & __space)
#define _isupper(c)      ((__ctype)[(c)+1] & __upper)
#define _isxdigit(c)     ((__ctype)[(c)+1] & __hex)
#define _toascii(c)      ((c) & 0x7F)
#define _tolower(c)      ((c) | 0x20)
#define _toupper(c)      ((c) & 0x5F)

/*
 * If one #includes this file, the macro will be used.  Otherwise, the
 * function will be used.
 */
int	isalnum(int);
int	isalpha(int);
int	iscntrl(int);
int	isdigit(int);
int	isgraph(int);
int	islower(int);
int	isprint(int);
int	ispunct(int);
int	isspace(int);
int	isupper(int);
int	isxdigit(int);
int	tolower(int);		/* no macro version (but see _tolower()) */
int	toupper(int);		/* no macro version (but see _toupper()) */
#ifndef __KeepNamespacePure__
int	isascii(int);
int	isblank(int);
int	iscsym(int);
int	iscsymf(int);
int	isodigit(int);
int	toascii(int);
int	toint(char);		/* No macro version */
#endif

#define isalnum(c)	_isalnum(c)
#define isalpha(c)	_isalpha(c)
#define iscntrl(c)	_iscntrl(c)
#define isdigit(c)	_isdigit(c)
#define isgraph(c)	_isgraph(c)
#define islower(c)	_islower(c)
#define isprint(c)	_isprint(c)
#define ispunct(c)	_ispunct(c)
#define isspace(c)	_isspace(c)
#define isupper(c)	_isupper(c)
#define isxdigit(c)	_isxdigit(c)
#ifndef __KeepNamespacePure__
#   define isascii(c)	_isascii(c)
#   define isblank(c)	_isblank(c)
#   define iscsym(c)	_iscsym(c)
#   define iscsymf(c)	_iscsymf(c)
#   define isodigit(c)	_isodigit(c)
#   define toascii(c)	_toascii(c)
#endif

#endif
