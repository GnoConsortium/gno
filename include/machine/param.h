/*
 * machine/param.h
 *
 * Copyright 1991-1996, Procyon, Inc.
 * All Rights Reserved
 *
 * $Id: param.h,v 1.1 1997/02/28 04:42:07 gdr Exp $
 */

#ifndef _MACHINE_PARAM_H_
#define _MACHINE_PARAM_H_

#define MACHINE "APPLEIIGS"

#ifdef KERNEL			/* non-BSD.  Obsolete? */
#define DAYSPERNYEAR 365
#define SECSPERDAY 86400L
#endif

/*
 * Round p (pointer or byte index) up to a correctly-aligned value
 * for all data types (int, long, ...).   The result is unsigned int
 * and must be cast to any desired pointer type.
 */
#define ALIGNBYTES	0
#define ALIGN(p)	p

#endif
