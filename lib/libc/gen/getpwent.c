/*
 * Copyright (c) 1988 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * This is an old implementation and needs to be replaced; just not quite
 * yet -- gdr
 *
 * $Id: getpwent.c,v 1.3 2012/08/26 02:54:59 gdr Exp $
 *
 * This file is formatted with tab stops every 8 characters.
 */

#ifdef __ORCAC__
segment "libc_gen__";
#endif

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)getpwent.c	5.21 (Berkeley) 3/14/91";
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <err.h>

#define LENGTH 128

static struct passwd	_pw_passwd;		/* password structure */
static char		pwline[LENGTH];		/* line in /etc/passwd */
static FILE *		f_passwd = NULL;
#if 0
static int		_pw_stayopen = 0;	/* keep fd's open */
#endif

static FILE *
_getpwfp (void) {
	if (f_passwd == NULL) {
		f_passwd = fopen(_PATH_PASSWD, "r");
		if (f_passwd == NULL) {
			/* this should really be going out via syslog */
			errx(1, "couldn't open %s", _PATH_PASSWD);
		}
	}
	return f_passwd;
}

static struct passwd *
_parsepw (struct passwd *_pw, char *line)
{
	char *q, *r;

	_pw->pw_fields = 0L;	/* no fields filled in yet */
	if ((q = strchr(line, '\n')) != NULL) {
		*q = '\0';
	}

	/* user name */
	_pw->pw_name = line;
        if ((q = strchr(line,':')) == NULL) {
        	return NULL;
        }
        *q++ = '\0';
	_pw->pw_fields |= _PWF_NAME;

	/* password */
        _pw->pw_passwd = q;
        if ((q = strchr(q,':')) == NULL) {
        	return NULL;
        }
        *q++ = '\0';
        _pw->pw_fields |= _PWF_PASSWD;

        /* user id */
        errno = 0;
        _pw->pw_uid = (int) strtol(q, &r, 10);
        if (errno == ERANGE || q == r) {
        	return NULL;
        }
	q = r;
	*q++ = '\0';
        _pw->pw_fields |= _PWF_UID;

	/* group id */
        errno = 0;
        _pw->pw_gid = (int) strtol(q, &r, 10);
        if (errno == ERANGE || q == r) {
        	return NULL;
        }
	q = r;
	*q++ = '\0';
        _pw->pw_fields |= _PWF_GID;

	/* real name */
        _pw->pw_gecos = q;
        if ((q = strchr(q,':')) == NULL) {
        	return NULL;
        }
        *q++ = '\0';
        _pw->pw_fields |= _PWF_GECOS;

	/* home directory */
        _pw->pw_dir = q;
        if ((q = strchr(q,':')) == NULL) {
        	return NULL;
        }
        *q++ = '\0';
        _pw->pw_fields |= _PWF_DIR;

	/* shell -- last one, so handle it differently */
        _pw->pw_shell = q;
        if ((q = strchr(q,':')) != NULL) {
        	*q = '\0';
        }
        _pw->pw_fields |= _PWF_SHELL;

	/* "not filled in":
	 *	password change time
	 *	user access class
	 *	account expiry time
	 */
	_pw->pw_change = _pw->pw_expire = 0L;
	_pw->pw_class = NULL;

	return _pw;
}

struct passwd *
getpwent(void)
{
	FILE *fp;

	fp = _getpwfp();
	if (fgets(pwline, LENGTH, fp) == NULL) {
		return NULL;
	}
	return _parsepw (&_pw_passwd, pwline);
}

struct passwd *
getpwnam(char *name) {
	FILE *fp;
	struct passwd *result;
	
	fp = _getpwfp();
	rewind(fp);
	while (fgets(pwline, LENGTH, fp) != NULL) {
		result = _parsepw (&_pw_passwd, pwline);
		if (result == NULL || !strcmp(name, result->pw_name)) {
			return result;
		}
	}
	return NULL;
}
		
struct passwd *
getpwuid(uid_t uid)
{
	FILE *fp;
	struct passwd *result;
	
	fp = _getpwfp();
	rewind(fp);
	while (fgets(pwline, LENGTH, fp) != NULL) {
		result = _parsepw (&_pw_passwd, pwline);
		if (result == NULL || uid == result->pw_uid) {
			return result;
		}
	}
	return NULL;
}

int
setpwent(void)
{
	if (f_passwd != NULL) {
		rewind(f_passwd);
	}
	return 1;
}

void
endpwent(void)
{
	if (f_passwd != NULL) {
		fclose(f_passwd);
		f_passwd = NULL;
	}
}

#ifdef NOTDEFINED
int
setpassent(stayopen)
	int stayopen;
{
	_pw_keynum = 0;
	_pw_stayopen = stayopen;
	return(1);
}
#endif
