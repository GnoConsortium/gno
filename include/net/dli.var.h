/* 
 * Mach Operating System
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 *
 **********************************************************************
 * HISTORY
 * 06-May-87  Mike Accetta (mja) at Carnegie-Mellon University
 *	Created.
 *	[ V5.1(F10) ]
 *
 **********************************************************************
 */



/*
 *  Data-link interface address structure.
 *
 *  The standard interface address header plus a pointer to the DLI protocol
 *  specific block.
 */
struct dli_ifaddr {
    struct ifaddr    da_ifa;
#define	da_addr	     da_ifa.ifa_addr
#define	da_broadaddr da_ifa.ifa_broadaddr
#define	da_ifp       da_ifa.ifa_ifp
    struct dli_var  *da_dlv;
};


/*
 *  Per-interface data block
 *
 *  Each network device driver which supports this protocol will allocate
 *  one of these blocks for each network interface with which it is
 *  configured.  The driver will call dli_init() with the appropriate
 *  parameters to initialize the block and DLI protocol for the interface
 *  every time that it is initialized.
 */
struct dli_var {
    struct dli_ifaddr dlv_da;	  /* interface address (linked into the */
				  /*  master address list for the interface) */ 
    struct sockproto  dlv_rproto; /* protocol of current input packet */
    struct sockaddr   dlv_rsrc;   /* src addr of current input packet */
    u_char            dlv_hln;    /* hardware address length (bytes) */
    u_char            dlv_lhl;	  /* local interface header length (bytes) */
};
