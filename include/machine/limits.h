/*
 * $Id: limits.h,v 1.1 1997/02/28 04:42:07 gdr Exp $
 */

#ifndef _MACHINE_LIMITS_H_
#define	_MACHINE_LIMITS_H_

#define	CHAR_BIT	8		/* number of bits in a char */
#define	MB_LEN_MAX	1		/* Allow 31 bit UTF2 */

/*
 * According to ANSI (section 2.2.4.2), the values below must be usable by
 * #if preprocessing directives.  Additionally, the expression must have the
 * same type as would an expression that is an object of the corresponding
 * type converted according to the integral promotions.
 */

#define	SCHAR_MAX	127		/* min value for a signed char */
#define	SCHAR_MIN	(-128)		/* max value for a signed char */

#define CHAR_MAX        255u		/* max value for a char */
#define CHAR_MIN        0		/* min value for a char */
#define	UCHAR_MAX	255u		/* max value for an unsigned char */

#define	USHRT_MAX	65535u		/* max value for an unsigned short */
#define	SHRT_MAX	32767		/* max value for a short */
#define	SHRT_MIN	(-32767-1)	/* min value for a short */

#define	UINT_MAX	65535u		/* max value for an unsigned int */
#define	INT_MAX		32767		/* max value for an int */
#define	INT_MIN		(-32767-1)	/* min value for an int */

#define	ULONG_MAX	4294967295u	/* max value for an unsigned long */
#define	LONG_MAX	2147483647	/* max value for a long */
#define	LONG_MIN	(-2147483647-1)	/* min value for a long */

#if !defined(_ANSI_SOURCE)
#define	SSIZE_MAX	LONG_MAX	/* max value for a ssize_t */

#if !defined(_POSIX_SOURCE)

#define LONG_BIT		32
#define WORD_BIT		16

#define	SIZE_T_MAX	ULONG_MAX	/* max value for a size_t */

#endif /* !_POSIX_SOURCE */
#endif /* !_ANSI_SOURCE */

#endif /* !_MACHINE_LIMITS_H_ */
