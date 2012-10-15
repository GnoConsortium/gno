/*-
 * Copyright (c) 2002 Mike Barcroft <mike@FreeBSD.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#ifndef _STRINGS_H_
#define	_STRINGS_H_

//#include <sys/cdefs.h>
//#include <sys/_types.h>

#ifndef _MACHINE_ANSI_H_
#include <machine/ansi.h>
#endif

#if defined(_BSD_SIZE_T_) && !defined(__size_t__)
#define __size_t__ 1
typedef	_BSD_SIZE_T_	size_t;
#undef	_BSD_SIZE_T_
#endif

#ifndef	NULL
#define NULL  (void *) 0L
#endif

#ifndef _SYS_CDEFS_H_
#include <sys/cdefs.h>
#endif

__BEGIN_DECLS
int	 bcmp(const void *, const void *, size_t) __pure;	/* LEGACY */
void	 bcopy(const void *, void *, size_t);			/* LEGACY */
void	 bzero(void *, size_t);					/* LEGACY */
int	 ffs(int) __pure2;

char	*index(const char *, int) __pure;			/* LEGACY */
char	*rindex(const char *, int) __pure;			/* LEGACY */
int	 strcasecmp(const char *, const char *) __pure;
int	 strncasecmp(const char *, const char *, size_t) __pure;

#ifndef _ANSI_SOURCE

int	 ffsl(long) __pure2;
//int	 ffsll(long long) __pure2;
int	 fls(int) __pure2;
int	 flsl(long) __pure2;
//int	 flsll(long long) __pure2;

#endif
__END_DECLS

#endif /* _STRINGS_H_ */
