/*	$Id: net.h,v 1.1 1998/02/02 08:18:39 taubert Exp $ */

/*
 *	GNO/ME Network Support
 *
 *	Copyright 1994-1998, Procyon Enterprises Inc.
 *
 *	Written by Derek Taubert and Jawaid Bazyar
 *
 */

/* Request types for pr_usrreq() */

#define PRU_ABORT	0
#define PRU_ACCEPT	1
#define PRU_ATTACH	2
#define PRU_BIND	3
#define PRU_CONNECT	4
#define PRU_CONNECT2	5
#define PRU_CONTROL	6
#define PRU_DETACH	7
#define PRU_DISCONNECT	8
#define PRU_LISTEN	9
#define PRU_PEERADDR	10
#define PRU_RCVD	11
#define PRU_RCVOOB	12
#define PRU_SEND	13
#define PRU_SENDOOB	14
#define PRU_SENSE	15
#define PRU_SHUTDOWN	16
#define PRU_SOCKADDR	17
#define PRU_CO_GETOPT	18
#define PRU_CO_SETOPT	19
#define PRU_SELECT	20

