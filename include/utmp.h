/*
 *      GNO/ME utmp/wtmp/lastlog definitions.  These structures and _PATH*
 *	locations aren't even close to being BSD-ish.
 *
 *	$Id: utmp.h,v 1.1 1997/02/28 04:42:04 gdr Exp $
 */

#ifndef	_UTMP_H_
#define	_UTMP_H_

#ifndef _SYS_CDEFS_H_
#include <sys/cdefs.h>
#endif

#ifndef _TIME_H_
#include <time.h>
#endif

#ifndef _SYS_TYPES_H_
#include <sys/types.h>
#endif

#define	_PATH_UTMP	"/var/adm/utmp"
#define	_PATH_WTMP	"/var/adm/wtmp"
#define	_PATH_LASTLOG	"/var/adm/lastlog"

#define	UT_NAMESIZE	8
#define	UT_LINESIZE	8
#define	UT_HOSTSIZE	16

/* In the hopes that this will help keep old programs from breaking if the
   format is expanded, these values are actually bitmapped. The masks which
   can be used to obtain certain characteristics about a process are listed
   below, after the types themselves   */

#define USER_PROCESS 1
#define LOCK_PROCESS 32
#define DEAD_PROCESS 128
#define CRASHED_PROCESS 129
#define WTMP_EXTENSION 256
#define GETTY_PROCESS 96
#define NOT_DEAD_YET 257

#define UT_SPECIAL 256           /* special                              */
#define UT_HAS_NO_WTMPENT 64     /* lock which is not recorded in wtmp   */
#define UT_INVISIBLE 32          /* does not apply to wtmp, only to utmp */
#define UT_LOGOUT_PROC 128       /* anything valid as ut_offtype         */
#define WTMP_CONTINUED 32768     /* continued in the next entry          */

struct wtmp {
	unsigned int	ut_type;
	time_t		ut_time;
	unsigned int	ut_offtype;
	time_t		ut_offtime;
	char		ut_name[UT_NAMESIZE];
	char		ut_line[UT_LINESIZE];
	char		ut_host[UT_HOSTSIZE];
};

struct utmp {
	unsigned int	ut_type;
	time_t		ut_time;
	pid_t		ut_pid;
	unsigned long	ut_wtmpent;
	char		ut_name[UT_NAMESIZE];
	char		ut_line[UT_LINESIZE];
	char		ut_host[UT_HOSTSIZE];
};

struct lastlog {
	time_t		ll_time;
	char		ll_line[UT_LINESIZE];
	char		ll_host[UT_HOSTSIZE];
	unsigned long	ll_wtmpent;
};

#ifndef _POSIX_SOURCE
int	locktty __P((unsigned ut_type, pid_t ut_pid, char *name, char *line,
		    char *host, int slot));
int	unlocktty __P((int slot, pid_t ut_pid));
#endif

#endif	/* _UTMP_H_ */
