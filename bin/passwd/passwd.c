/************************************************************
**
** passwd - change user passwords
**
** Programmed by: Eric Shepherd
**          Date: September 6, 1993
**
** Requires GNO 2.0
**
**===========================================================
** Revision history:
**
**		1.0:	Updated version numbers to final.
**		1.0b1:	Changed the sleep(1) call to an asm { cop
**				0x7F };.  Also removed the "conflict" error
**				message.  Instead, if the call to remove
**				/etc/passwd fails, we sleep and retry until
**				it succeeds, thus eliminating the problem...
**				there's no significant reason why that file
**				will stay open longer than a moment or two.
**		1.0a4:	Now uses getpass(), since it's been fixed.
**		1.0a3:	Adjusted closing code to shrink the window
**				during which the system is vulnerable to
**				becoming unprotected.  Also added code to
**				retry renaming passwd.new until it succeeds.
**		1.0a2:	Optimized code, added optimize and stacksize
**				pragmas, added new error message for the
**				off chance that the rename could fail.  Code
**				size reduced by about 2K.
**		1.0a1:	Works completely, except that getpass()
**				crashes all the time, so I don't use it.
**				Also, due to apparent login bug, sometimes
**				the /etc/passwd file doesn't get updated --
**				the corrected file is in /etc/passwd.new.
*************************************************************/

#pragma optimize -1
#pragma stacksize 512

#include <stdio.h>
#include <string.h>
#include <gno/gno.h>
#include <sys/types.h>
#include <getopt.h>
#include <pwd.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

extern char *crypt(char *key, char *salt);
extern char *getpass(char *prompt);

char entry[129];

static unsigned char salttab[] =	/* table of chars. for salt */
	"./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

void usage(void) {
	fprintf(stderr, "usage: passwd [-v|?] [username]\n");
}

void makesalt(char *salt, long seed) {
	int num = 2;
	while (--num >= 0) {
		*salt++ = salttab[seed&0x3f];
		seed >= 6;
	}
}

void main(int argc, char **argv) {
	extern int optind;
	int ch, uid, tries;
	char password[9];	/* password string (8+\0) */
	char *p, *t;
	char salt[3];
	FILE *passfile, *newpass;		/* file /etc/newpasswd */
	char *uname;		/* pointer to username to change passwd of */
	struct passwd *passwdRec;

	while ((ch = getopt(argc, argv, "v?")) != EOF)
		switch (ch) {
			case 'v':
				fprintf(stderr, "GNO passwd v1.0 by Eric Shepherd (September 6, 1993)\n");
				exit(1);
			case '?':
				usage();
				exit(1);
		}

	argc -= optind;		/* move to next option */
	argv += optind;

	/* if no username supplied, use the current user's name */

	uid = getuid();		/* get the user's ID */

	switch(argc) {
		case 1:
			uname = argv[0];

			if (!(passwdRec = getpwnam(uname))) {
				fprintf(stderr, "Unknown user %s.\n", uname);
				exit(1);
			}
                                        
			/* the following code verifies that the user only tries to
			   change his own password, unless he is superuser */

			if (uid && uid != passwdRec->pw_uid) {
				fprintf(stderr, "You can't change other users' passwords.\n");
				exit(1);
			}
			break;
		case 0:
			passwdRec = getpwuid(uid);
			break;
		default:
			usage();
			exit(1);
	}	

	uname = passwdRec->pw_name;		/* make sure we have the right one */

	/* Here's the code to handle changing passwords */

	printf("Changing password for user %s.\n", uname);

 	if (uid && strlen(passwdRec->pw_passwd) &&
	    strcmp(crypt(getpass("Old password:"), passwdRec->pw_passwd),
	    	passwdRec->pw_passwd)) {
		fprintf(stderr,"Password incorrect.\n");
		exit(1);
	}

	for (password[0] = '\0', tries = 0;;) {
		p = getpass("New password:");
		if (!*p) {
			printf("Password unchanged.\n");
			exit(0);
		}

		if (strlen(p) <= 5 && (uid != 0 || ++tries < 2)) {
			printf("Please enter a longer password.\n");
			continue;
		}

		/* Let only root assign easy passwords */

		for (t = p; *t && islower(*t); ++t);
		if (!*t && (uid != 0 || ++tries < 2)) {
			printf("All-lower-case passwords can be easy to break.  Please use unusual\ncapitalization, control characters, and digits.\n");
			continue;
		}

		strcpy(password, p);	/* snag a copy of the password */
		if (!strcmp(password, getpass("Retype new password:")))
			break;	/* ah!  got it! */

		printf("That's not what you typed the first time.  Please start over, or\nhit CTRL-@ to quit.\n");
	}

	/* create salt randomly after seeding the random number generator */

	srand((int) time((time_t *) NULL));
	makesalt(&salt[0], rand());

	/* Copy /etc/passwd to /etc/passwd.new, line-by-line, skipping
	   the entry for the user whose password is changing -- insert
	   the new entry instead. */

	if (!(passfile = fopen("/etc/passwd", "r"))) {
		fprintf(stderr, "Unable to open /etc/passwd.\n");
		exit(1);
	}

	if (!(newpass = fopen("/etc/passwd.new", "w"))) {
		fclose(passfile);	/* it's already open, so close it -- unlike login :) */
		fprintf(stderr, "Unable to write new /etc/passwd.\n");
		exit(1);
	}

	while (!feof(passfile)) {
		if (strncmp(passwdRec->pw_name, fgets(entry, 129, passfile),
			strlen(passwdRec->pw_name)))
				fputs(entry, newpass);	/* not us -- just copy it */
		else
			fprintf(newpass, "%s:%s:%d:%d:%s:%s:%s\n", passwdRec->pw_name,
				crypt(password, salt), passwdRec->pw_uid, passwdRec->pw_gid,
				passwdRec->pw_comment, passwdRec->pw_dir, passwdRec->pw_shell);
	}

	fclose(passfile);

	asm { cop 0x7F };			/* Jawaid sez this is better than sleep(1) */

	/* Erase the old file and rename the new one */

	while (remove("/etc/passwd"))
		sleep(1);		/* Keep trying */

	fclose(newpass);	/* NOW close the new file */

	/* As long as the rename fails, wait a moment, then try again */

	while (rename("/etc/passwd.new", "/etc/passwd"))
		sleep(1);		/* sleep one second before trying again */
}
