/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)types.h	8.3 (Berkeley) 1/5/94
 * $Id: types.h,v 1.1 1997/02/28 04:42:07 gdr Exp $
 */

#ifndef	_MACHTYPES_H_
#define	_MACHTYPES_H_

#if !defined(_ANSI_SOURCE) && !defined(_POSIX_SOURCE)
typedef struct _physadr {
	int r[1];
} *physadr;

#if 1	/* for GNO */
typedef  int			label_t[13];
#else	/* for BSD */
typedef struct label_t {
	int val[6];
} label_t;
#endif
#endif

typedef	unsigned long	vm_offset_t;
typedef	unsigned long	vm_size_t;

#if (__STDC__ != 1) || defined(_IN_KERNEL)	/* GNO-specific */
typedef  unsigned short		iord_t;
typedef  unsigned short		pgadr_t;
typedef  char			swck_t;
typedef  unsigned_char		use_t;
typedef  long                   ubadr_t;
#define  MAXUSE			255
#endif /* (__STDC__ != 1) || defined(_IN_KERNEL) */

/*
 * Basic integral types.  Omit the typedef if
 * not possible for a machine/compiler combination.
 */
#ifdef __STDC__
typedef signed char		   int8_t;
#else
typedef signed char		   int8_t;
#endif
typedef	unsigned char		 u_int8_t;
typedef	short			  int16_t;
typedef	unsigned short		u_int16_t;
typedef	long			  int32_t;
typedef	unsigned long		u_int32_t;

#if 0
typedef	long long		  int64_t;
typedef	unsigned long long	u_int64_t;
#endif

#endif	/* _MACHTYPES_H_ */
