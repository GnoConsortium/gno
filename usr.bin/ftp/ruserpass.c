#ifdef __ORCAC__
segment "ftpmain   ";
#endif

static char sccsid[] = "@(#)ruserpass.c	5.3 (Berkeley) 3/1/91";

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <utmp.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/stat.h>
#include <errno.h>
#include <netdb.h>

#include "ftp.var.h"
#include "cmds.h"
#include "main.h"
#include "domacro.h"
#include "ruserpass.h"

static	FILE *cfile;

#define	DEFAULT	1
#define	LOGIN	2
#define	PASSWD	3
#define	ACCOUNT 4
#define MACDEF  5
#define	ID	10
#define	MACH	11

static char tokval[100];

static struct toktab {
	char *tokstr;
	int tval;
} toktab[]= {
	"default",	DEFAULT,
	"login",	LOGIN,
	"password",	PASSWD,
	"passwd",	PASSWD,
	"account",	ACCOUNT,
	"machine",	MACH,
	"macdef",	MACDEF,
	0,		0
};

static int token (void)
{
char *cp;
int c;
struct toktab *t;

	if (feof(cfile))
		return (0);
	while ((c = getc(cfile)) != EOF &&
	    (c == '\n' || c == '\t' || c == ' ' || c == ','))
		continue;
	if (c == EOF)
		return (0);
	cp = tokval;
	if (c == '"') {
		while ((c = getc(cfile)) != EOF && c != '"') {
			if (c == '\\')
				c = getc(cfile);
			*cp++ = c;
		}
	} else {
		*cp++ = c;
		while ((c = getc(cfile)) != EOF
		    && c != '\n' && c != '\t' && c != ' ' && c != ',') {
			if (c == '\\')
				c = getc(cfile);
			*cp++ = c;
		}
	}
	*cp = 0;
	if (tokval[0] == 0)
		return (0);
	for (t = toktab; t->tokstr; t++)
		if (!strcmp(t->tokstr, tokval))
			return (t->tval);
	return (ID);
}

int ruserpass (char *host, char **aname, char **apass, char **aacct)
{
char *hdir, *tmp;
static char buf[BUFSIZ];
static char myname[MAXHOSTNAMELEN];
char *mydomain;
int t, i, c, usedefault = 0;
static struct stat stb;

	hdir = getenv("HOME");
	if (hdir == NULL)
		hdir = ".";
	sprintf(buf, "%s/.netrc", hdir);
	cfile = fopen(buf, "r");
	if (cfile == NULL) {
		if (errno != ENOENT)
			perror(buf);
		return(0);
	}
	if (gethostname(myname, sizeof(myname)) < 0)
		myname[0] = '\0';
	if ((mydomain = index(myname, '.')) == NULL)
		mydomain = "";
next:
	while ((t = token())) switch(t) {

	case DEFAULT:
		usedefault = 1;
		/* FALL THROUGH */

	case MACH:
		if (!usedefault) {
			if (token() != ID)
				continue;
			/*
			 * Allow match either for user's input host name
			 * or official hostname.  Also allow match of 
			 * incompletely-specified host in local domain.
			 */
			if (strcasecmp(host, tokval) == 0)
				goto match;
			if (strcasecmp(hostname, tokval) == 0)
				goto match;
			if ((tmp = index(hostname, '.')) != NULL &&
			    strcasecmp(tmp, mydomain) == 0 &&
			    strncasecmp(hostname, tokval, tmp-hostname) == 0 &&
			    tokval[tmp - hostname] == '\0')
				goto match;
			if ((tmp = index(host, '.')) != NULL &&
			    strcasecmp(tmp, mydomain) == 0 &&
			    strncasecmp(host, tokval, tmp - host) == 0 &&
			    tokval[tmp - host] == '\0')
				goto match;
			continue;
		}
	match:
		while ((t = token()) && t != MACH && t != DEFAULT) switch(t) {

		case LOGIN:
			if (token())
				if (*aname == 0) { 
					*aname = malloc((unsigned) strlen(tokval) + 1);
					strcpy(*aname, tokval);
				} else {
					if (strcmp(*aname, tokval))
						goto next;
				}
			break;
		case PASSWD:
			if (strcmp(*aname, "anonymous") &&
			    fstat(fileno(cfile), &stb) >= 0 &&
			    (stb.st_mode & 077) != 0) {
	fprintf(stderr, "Error - .netrc file not correct mode.\n");
	fprintf(stderr, "Remove password or correct mode.\n");
				goto bad;
			}
			if (token() && *apass == 0) {
				*apass = malloc((unsigned) strlen(tokval) + 1);
				strcpy(*apass, tokval);
			}
			break;
		case ACCOUNT:
			if (fstat(fileno(cfile), &stb) >= 0
			    && (stb.st_mode & 077) != 0) {
	fprintf(stderr, "Error - .netrc file not correct mode.\n");
	fprintf(stderr, "Remove account or correct mode.\n");
				goto bad;
			}
			if (token() && *aacct == 0) {
				*aacct = malloc((unsigned) strlen(tokval) + 1);
				strcpy(*aacct, tokval);
			}
			break;
		case MACDEF:
			if (proxy) {
				fclose(cfile);
				return(0);
			}
			while ((c=getc(cfile)) != EOF && c == ' ' || c == '\t');
			if (c == EOF || c == '\n') {
				printf("Missing macdef name argument.\n");
				goto bad;
			}
			if (macnum == 16) {
				printf("Limit of 16 macros have already been defined\n");
				goto bad;
			}
			tmp = macros[macnum].mac_name;
			*tmp++ = c;
			for (i=0; i < 8 && (c=getc(cfile)) != EOF &&
			    !isspace(c); ++i) {
				*tmp++ = c;
			}
			if (c == EOF) {
				printf("Macro definition missing null line terminator.\n");
				goto bad;
			}
			*tmp = '\0';
			if (c != '\n') {
				while ((c=getc(cfile)) != EOF && c != '\n');
			}
			if (c == EOF) {
				printf("Macro definition missing null line terminator.\n");
				goto bad;
			}
			if (macnum == 0) {
				macros[macnum].mac_start = macbuf;
			}
			else {
				macros[macnum].mac_start = macros[macnum-1].mac_end + 1;
			}
			tmp = macros[macnum].mac_start;
			while (tmp != macbuf + 4096) {
				if ((c=getc(cfile)) == EOF) {
				printf("Macro definition missing null line terminator.\n");
					goto bad;
				}
				*tmp = c;
				if (*tmp == '\n') {
					if (*(tmp-1) == '\0') {
					   macros[macnum++].mac_end = tmp - 1;
					   break;
					}
					*tmp = '\0';
				}
				tmp++;
			}
			if (tmp == macbuf + 4096) {
				printf("4K macro buffer exceeded\n");
				goto bad;
			}
			break;
		default:
	fprintf(stderr, "Unknown .netrc keyword %s\n", tokval);
			break;
		}
		goto done;
	}
done:
	fclose(cfile);
	return(0);
bad:
	fclose(cfile);
	return(-1);
}
