#if	CMU
/* 
 **********************************************************************
 * Mach Operating System
 * Copyright (c) 1986 Carnegie-Mellon University
 *  
 * This software was developed by the Mach operating system
 * project at Carnegie-Mellon University's Department of Computer
 * Science. Software contributors as of May 1986 include Mike Accetta, 
 * Robert Baron, William Bolosky, Jonathan Chew, David Golub, 
 * Glenn Marcy, Richard Rashid, Avie Tevanian and Michael Young. 
 * 
 * Some software in these files are derived from sources other
 * than CMU.  Previous copyright and other source notices are
 * preserved below and permission to use such software is
 * dependent on licenses from those institutions.
 * 
 * Permission to use the CMU portion of this software for 
 * any non-commercial research and development purpose is
 * granted with the understanding that appropriate credit
 * will be given to CMU, the Mach project and its authors.
 * The Mach project would appreciate being notified of any
 * modifications and of redistribution of this software so that
 * bug fixes and enhancements may be distributed to users.
 *
 * All other rights are reserved to Carnegie-Mellon University.
 **********************************************************************
 */ 
 
#endif	/* CMU */
/*
 * Copyright (c) 1980, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)af.h	7.1 (Berkeley) 6/4/86
 */

/*
 * Address family routines,
 * used in handling generic sockaddr structures.
 *
 * Hash routine is called
 *	af_hash(addr, h);
 *	struct sockaddr *addr; struct afhash *h;
 * producing an afhash structure for addr.
 *
 * Netmatch routine is called
 *	af_netmatch(addr1, addr2);
 * where addr1 and addr2 are sockaddr *.  Returns 1 if network
 * values match, 0 otherwise.
 */
struct afswitch {
	int	(*af_hash)();
	int	(*af_netmatch)();
};

struct afhash {
	u_int	afh_hosthash;
	u_int	afh_nethash;
};

#ifdef KERNEL
struct	afswitch afswitch[];
#endif
