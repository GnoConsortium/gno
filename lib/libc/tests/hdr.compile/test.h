/*
 * $Id: test.h,v 1.1 1997/02/28 05:12:56 gdr Exp $
 */

#ifdef KERNEL

#ifdef __STDC__
#define __P(a) a
#else
#define __P(a) ()
#endif

typedef void (*__SIG_FUNC__) __P((int, int));

#ifndef _POSIX_SOURCE
typedef __SIG_FUNC__ sig_t;
#endif

#endif	/* KERNEL */
