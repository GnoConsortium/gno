/*
 *	Serial Interrupt Manager
 *	C Interface File
 *	Copyright 1993, Procyon Enterprises Incorporated
 *	Free distribution is hereby granted for use with the
 *	SIM tool
 */

#ifdef __ORCAC__
/*
 *	ORCA/C Prototypes
 */
extern int SIMVERSION(int *versionPtr);
extern int INSTALLINTVECT(int port, unsigned long address);
extern int REMOVEINTVECT(int port, unsigned long address);
#else
/*
 *	APW C/other prototypes
 */
int SIMVERSION();
int INSTALLINTVECT();
int REMOVEINTVECT();
#endif

#define SIMVersion SIMVERSION
#define InstallIntVect INSTALLINTVECT
#define RemoveIntVect REMOVEINTVECT

#define SIMPrinterPort	1
#define SIMModemPort	2

#define SIMNoError	0
#define SIMAlreadyInst	1
#define SIMInvalidAddr	2
#define SIMATalkActive	3
#define SIMNotInstalled 4
#define SIMInvalidPort	5
#define SIMNotFound	6
