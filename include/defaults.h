/*************************************************
 * Start of GNO v2.0.6 definitions.
 *
 * $Id: defaults.h,v 1.1 1997/02/28 04:42:01 gdr Exp $
 */

#define	__appleiigs__
#define	__GNO__

/*
 * __ORCAC_VERSION should be:
 *	undefined	for Orca/C earlier than v2.1.0
 *	210		for Orca/C v2.1.0 - v2.1.1b1
 *	211		for Orca/C v2.1.1b2 and higher
 * Note that if you have Orca/C v2.0.x or earlier, it doesn't even
 * include this file by default.
 */
#define __ORCAC_VERSION	211

/*
 * These should be identically equal.  Orca/C headers use one, BSD headers
 * use the other.  Unfortunately, this construct will miss definitions
 * given in source files -- it is only caught on the occ command line.
 */
#if defined(__KeepNamespacePure__) && !defined(_ANSI_SOURCE)
#define _ANSI_SOURCE
#endif
#if defined(_ANSI_SOURCE) && !defined(__KeepNamespacePure__)
#define __KeepNamespacePure__
#endif

/*
 * End of GNO v2.0.6 definitions.
 *************************************************/
