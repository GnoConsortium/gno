/*
 * uname(3) - get system identification
 *
 * For GNO/ME (Apple IIgs) by Steve Reeves, January 1998
 *
 * $Id: uname.c,v 1.1 1998/02/16 22:44:03 gdr-ftp Exp $
 */

#ifdef __ORCAC__
segment "libc_gen__";
#endif

#include <sys/utsname.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <gno/gno.h>

#define SYSNAME	"GNO/ME"
#define DEFNODENAME	"unknown"
#define MACHINE	"IIgs/ROM"
#define GSROMID	(*(unsigned char *)0x00FFFB59)

int
uname(struct utsname *name)
{
	static char host[sizeof(name->nodename)];
	unsigned int ver;

	strncpy(name->sysname, SYSNAME, sizeof(name->sysname) - 1);
	name->sysname[sizeof(name->sysname) - 1] = '\0';

	if (gethostname(name->nodename, sizeof(name->nodename) - 1) == -1) {
		/* Assume init isn't running, so no hostname was set */
		strncpy(name->nodename, DEFNODENAME,
			sizeof(name->nodename) - 1);
	}
	name->nodename[sizeof(name->nodename) - 1] = '\0';

	ver = kernVersion();
	if (_toolErr)
		ver = 0;

	/*
	 * Kernel version 0x0IJK translates to I.J.K, where I.J is the version
	 * number and K is the release number.
	 *
	 * Note that this does NOT match the description of Apple IIgs toolset
	 * versions in IIgs Tech Note 100 (toolsets don't have release numbers).
	 *
	 * If the most significant bit is set, then its a prototype, indicated
	 * by a 'p' after the release number.
	 */

	snprintf(name->release, sizeof(name->release) - 1, "%u%s",
		    ver & 0x000F, (ver & 0x8000) ? "p" : "");
	name->release[sizeof(name->release) - 1] = '\0';

	snprintf(name->version, sizeof(name->version) - 1, "%u.%u",
		    (ver & 0x0F00) >> 8, (ver & 0x00F0) >> 4);
	name->version[sizeof(name->version) - 1] = '\0';

	snprintf(name->machine, sizeof(name->machine) - 1, MACHINE "%c",
		    GSROMID + '0');
	name->machine[sizeof(name->machine) - 1] = '\0';

	return 0;
}
