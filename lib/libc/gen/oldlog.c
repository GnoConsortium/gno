/*
 * old_syslog library routine. Used if syslogd is not present. This routine
 * has a subset of the functions of syslogd. It always outputs to
 * /etc/syslog, and it accepts only one string, and does no error checking.
 * It also prints the facility/log level as a number.
 *
 * If you're sure syslogd will be available when your program is run (or you
 * want syslog() to do nothing if it isn't), create a dummy old_syslog() that
 * simply returns. This will make your program smaller.
 *
 * Phillip Vandry, August 1993.
 *
 * $Id: oldlog.c,v 1.1 1997/02/28 05:12:44 gdr Exp $
 *
 * This file is formatted with tab stops every 8 columns.
 */

#ifdef __ORCAC__
segment "libc_gen__";
#endif

#pragma optimize 0
#pragma debug 0
#pragma memorymodel 0

#include <stdio.h>
#include <fcntl.h>
#include <memory.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/syslog.h>

#define _PATH_SYSLOG "/etc/syslog"

/* Perhaps this struct decl belongs in <sys/syslog.h> ? */

struct syslogd {
	int version;              /* == 0 */
	int prio;                 /* priority and facility */
	int numstrings;           /* number of strings sent to syslogd */
	int string1;              /* offset of string #1 */
	/* offset of more strings. This routine handles only one string */
	int string1_len;          /* length of string #1 (GS/OS string) */
	char string1_text;        /* variable length */
};

void
old_syslog (Handle datahand)
{
	struct syslogd *data;
	static time_t trec;
	int where;
	static char number[32];

	HLock(datahand);
	data = (struct syslogd *)*datahand;
	if ((where = open(_PATH_SYSLOG, O_APPEND|O_WRONLY|O_CREAT)) == -1) {
		return;
	}
	trec = time(NULL);
	sprintf(number,"%s: <%d> ", ctime(&trec), data->prio);
	write(where,number,(size_t)strlen(number));
	write(where,&(data->string1_text),(size_t)data->string1_len);
	write(where,"\r",1l);
	close(where);
	DisposeHandle(datahand);
}
