/* 
 * Mach Operating System
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * HISTORY
 * 26-Sep-89  Morris Meyer (mmeyer) at NeXT
 *	NFS 4.0 Changes: Back out statistics gathering support.
 *
 * 13-Aug-87  Peter King (king) at NeXT
 *	SUN_RPC: Added rcb_cc and rcb_mbcnt for statistics gathering.
 *		    Added RAW_TALLY flag to turn on statistics gathering
 */ 
 
/*
 * Copyright (c) 1980, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)raw_cb.h	7.1 (Berkeley) 6/4/86
 */

/*	@(#)raw_cb.h	2.1 88/05/30 4.0NFSSRC SMI;	from UCB 7.1 6/5/86	*/

/*
 * Raw protocol interface control block.  Used
 * to tie a socket to the generic raw interface.
 */
struct rawcb {
	struct	rawcb *rcb_next;	/* doubly linked list */
	struct	rawcb *rcb_prev;
	struct	socket *rcb_socket;	/* back pointer to socket */
	struct	sockaddr rcb_faddr;	/* destination address */
	struct	sockaddr rcb_laddr;	/* socket's address */
	struct	sockproto rcb_proto;	/* protocol family, protocol */
	caddr_t	rcb_pcb;		/* protocol specific stuff */
	struct	mbuf *rcb_options;	/* protocol specific options */
	struct	route rcb_route;	/* routing information */
	short	rcb_flags;
/* SUN_RPC */
        int     rcb_cc;                 /* bytes of rawintr queued data */
        int     rcb_mbcnt;              /* bytes of rawintr queued mbufs */
/* SUN_RPC */
};

/*
 * Since we can't interpret canonical addresses,
 * we mark an address present in the flags field.
 */
#define	RAW_LADDR	01
#define	RAW_FADDR	02
#define	RAW_DONTROUTE	04		/* no routing, default */
#define RAW_TALLY       0x08            /* tally delivered packets (SUN_RPC) */

#define	sotorawcb(so)		((struct rawcb *)(so)->so_pcb)

/*
 * Nominal space allocated to a raw socket.
 */
#define	RAWSNDQ		2048
#define	RAWRCVQ		2048

/*
 * Format of raw interface header prepended by
 * raw_input after call from protocol specific
 * input routine.
 */
struct raw_header {
	struct	sockproto raw_proto;	/* format of packet */
	struct	sockaddr raw_dst;	/* dst address for rawintr */
	struct	sockaddr raw_src;	/* src address for sbappendaddr */
};

#ifdef KERNEL
struct rawcb rawcb;			/* head of list */
#endif
