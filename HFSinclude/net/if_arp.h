/*
 * Too bad we don't have symlinks for GNO ...
 *	This file allows a program to use the *proper* name for this
 *	header file, while allowing the "real" header file to reside on
 *	a ProDOS volume.  This is good for speed and disk integrity,
 *	and gives a work-around for those sites which don't use HFS.
 *
 * $Id: if_arp.h,v 1.1 1997/02/28 04:41:53 gdr Exp $
 */

#ifndef _NET_IF_ARP_H_
#include <net/if.arp.h>
#endif
