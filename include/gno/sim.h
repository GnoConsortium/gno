/*
 *	Serial Interrupt Manager
 *	C Interface File
 *	Copyright 1993-1996, Procyon Enterprises Incorporated
 *
 *	Free distribution is hereby granted for use with the SIM tool.
 *
 *	$Id: sim.h,v 1.1 1997/02/28 04:42:06 gdr Exp $
 */

#ifndef _GNO_SIM_H_
#define _GNO_SIM_H_

#ifndef _SYS_CDEFS_H_
#include <sys/cdefs.h>
#endif

extern int SIMVERSION __P((int *versionPtr));
extern int INSTALLINTVECT __P((int port, unsigned long address));
extern int REMOVEINTVECT __P((int port, unsigned long address));

#define SIMVersion	SIMVERSION
#define InstallIntVect	INSTALLINTVECT
#define RemoveIntVect	REMOVEINTVECT

#define SIMPrinterPort	1
#define SIMModemPort	2
#define SIMIRQ0		3
#define SIMIRQ1		4
#define SIMIRQ2		5
#define SIMIRQ3		6
#define SIMIRQ4		7
#define SIMIRQ5		8
#define SIMIRQ6		9
#define SIMIRQ7		10

#define SIMNoError	0
#define SIMAlreadyInst	1
#define SIMInvalidAddr	2
#define SIMATalkActive	3
#define SIMNotInstalled 4
#define SIMInvalidPort	5
#define SIMNotFound	6
#define SIMNO8259	7

#endif /* _GNO_SIM_H_ */
