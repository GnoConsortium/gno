/*
 * Test by Devin Reade.
 *
 * $Id: getpwent.c,v 1.1 1997/02/28 05:12:52 gdr Exp $
 */

#pragma debug 25

#include <sys/types.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WHOAMI "glyn"

int main(int argc, char **argv) {
	struct passwd *pw;
	int id, i, j;
	char **pp;

	for (j=0; j<2; j++) {
		switch (j) {
		case 0:
			printf("printing all entries\n");
			break;
		case 1:
			printf("resetting file pointer\n");
			setpwent();
			break;
		}
		while((pw = getpwent()) != NULL) {
			printf("entry: \"%s\" \"%s\" %u %u\n",
				pw->pw_name, pw->pw_passwd,
				pw->pw_uid, pw->pw_gid);
		}
	}

	if (argc > 1) {
		if (!strcmp(argv[1], WHOAMI)) {
			if ((pw = getpwnam(argv[1])) == NULL) {
				printf("%s not present in database\n",
					argv[1]);
			} else {
				printf("uid for %s is %u\n",
					argv[1], pw->pw_uid);
			}
		} else {
			id = atoi(argv[1]);
			if ((pw = getpwuid(id)) == NULL) {
				printf("uid %u not present in database\n",
					id);
			} else {
				printf("id for %u is %s using shell %s\n",
					id, pw->pw_name, pw->pw_shell ?
					pw->pw_shell : "(null)");
			}
		}
	}                                                 

	endpwent();
	return 0;
}
