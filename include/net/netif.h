/* 
 * Copyright (C) 1990 by NeXT, Inc., All Rights Reserved
 *
 */

/*
 * Network Interface API
 *
 * HISTORY
 * 09-Apr-90  Bradley Taylor (btaylor) at NeXT, Inc.
 *	Created. Some parts originally part of <net/if.h>.
 */
#ifndef _NETIF_
#define _NETIF_

#ifndef _NETBUF_
#include <net/netbuf.h>
#endif

/*
 * BEGIN: BSD copyrighted material
 */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */
#define	IFF_UP		0x1		/* interface is up */
#define	IFF_BROADCAST	0x2		/* broadcast address valid */
#define	IFF_DEBUG	0x4		/* turn on debugging */
#define	IFF_LOOPBACK	0x8		/* is a loopback net */
#define	IFF_POINTTOPOINT	0x10		/* interface is point-to-point link */
#define	IFF_POINTOPOINT	0x10		/* interface is point-to-point link */
#define	IFF_NOTRAILERS	0x20		/* avoid use of trailers */
#define	IFF_RUNNING	0x40		/* resources allocated */
#define	IFF_NOARP	0x80		/* no address resolution protocol */
/* next two not supported now, but reserved: */
#define	IFF_PROMISC	0x100		/* receive all packets */
#define	IFF_ALLMULTI	0x200		/* receive all multicast packets */
#define IFF_D1		0x400		/* For CSLIP */
#define IFF_D2		0x800		/* For CSLIP */	
#define IFF_D3		0x1000		/* For CSLIP */	
#define IFF_COMPAC	IFF_D1		/* For PPP */
#define IFF_COMPPROT	0x2000		/* For PPP */
/*
 * END: BSD copyrighted material
 */

#define	IFF_AUTOCONF	0x4000		/* Network is autoconfiguring */
#define IFF_AUTODONE	0x8000		/* Network has autoconfigured */

typedef enum netif_class {
	NETIFCLASS_REAL = 0,
	NETIFCLASS_VIRTUAL = 0x1000,
	NETIFCLASS_SNIFFER = 0x2000
} netif_class_t;

#ifdef KERNEL

extern const char IFCONTROL_SETFLAGS[];
extern const char IFCONTROL_SETADDR[];
extern const char IFCONTROL_GETADDR[];
extern const char IFCONTROL_AUTOADDR[];
extern const char IFCONTROL_UNIXIOCTL[];


typedef struct if_ioctl {
	unsigned ioctl_command;
	void *ioctl_data;
} if_ioctl_t;
	
typedef struct { char opaque[1]; } *netif_t;


typedef int (*if_input_func_t)(netif_t netif, netif_t realnetif,
				netbuf_t nb, void *extra);
typedef int (*if_init_func_t)(netif_t netif);

typedef int (*if_output_func_t)(netif_t netif, netbuf_t nb, void *address);
typedef netbuf_t (*if_getbuf_func_t)(netif_t netif);
typedef int (*if_control_func_t)(netif_t netif, const char *command, 
				 void *data);

typedef void (*if_attach_func_t)(void *private, netif_t realif);

extern int if_output(netif_t netif, netbuf_t packet, void *addr);
extern int if_init(netif_t netif);
extern int if_control(netif_t netif, const char *command, void *data);


extern int if_ioctl(netif_t netif, unsigned command, void *data);

extern netbuf_t if_getbuf(netif_t netif);
extern int if_handle_input(netif_t netif, netbuf_t nb, void *extra);



extern netif_t if_attach(if_init_func_t init_func, 
			 if_input_func_t input_func,
			 if_output_func_t output_func,
			 if_getbuf_func_t getbuf_func,
			 if_control_func_t control_func,
			 const char *name,
			 unsigned unit,
			 const char *type,
			 unsigned mtu,
			 unsigned flags,
			 netif_class_t class,
			 void *private);

extern void if_registervirtual(if_attach_func_t attach_func, void *private);

extern void if_detach(netif_t);

extern netif_t iflist_first(void);
extern netif_t iflist_next(netif_t);
		  
/*
 * Readable attributes
 */
extern const char *if_type(netif_t netif);
extern const char *if_name(netif_t netif);
extern unsigned if_unit(netif_t netif);
extern unsigned if_mtu(netif_t netif);
extern netif_class_t if_class(netif_t netif);
extern void *if_private(netif_t netif);

/*
 * Writable attributes 
 */
extern unsigned if_flags(netif_t netif);
extern void if_flags_set(netif_t netif, unsigned flags);

extern unsigned if_ipackets(netif_t netif);
extern void if_ipackets_set(netif_t netif, unsigned ipackets);

extern unsigned if_opackets(netif_t netif);
extern void if_opackets_set(netif_t netif, unsigned opackets);

extern unsigned if_oerrors(netif_t netif);
extern void if_oerrors_set(netif_t netif, unsigned oerrors);

extern unsigned if_ierrors(netif_t netif);
extern void if_ierrors_set(netif_t netif, unsigned ierrors);

extern unsigned if_collisions(netif_t netif);
extern void if_collisions_set(netif_t netif, unsigned collisions);

#endif /* KERNEL */

#endif /* _NETIF_ */
