/*
 * Test by Devin Reade.
 *
 * $Id: getgrent.c,v 1.1 1997/02/28 05:12:52 gdr Exp $
 */

#include <sys/types.h>
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WHEEL "wheel"

int main(int argc, char **argv) {
	struct group *gp;
	int id, i, j;
	char **pp;

	for (j=0; j<2; j++) {
		switch (j) {
		case 0:
			printf("printing all entries\n");
			break;
		case 1:
			printf("resetting file pointer\n");
			setgrent();
			break;
		}
		while((gp = getgrent()) != NULL) {
			printf("entry: %s %s %d ", gp->gr_name,
				gp->gr_passwd, gp->gr_gid);
			pp = gp->gr_mem;
			for (i=0; pp[i] != NULL; i++) {
				printf ("%s ", pp[i]);
			}
			printf("\n");
		}
	}

	if (argc > 1) {
		if (!strcmp(argv[1], WHEEL)) {
			if ((gp = getgrnam(argv[1])) == NULL) {
				printf("%s not present in database\n",
					argv[1]);
			} else {
				printf("group number for %s is %d\n",
					argv[1], gp->gr_gid);
			}
		} else {
			id = atoi(argv[1]);
			if ((gp = getgrgid(id)) == NULL) {
				printf("group %d not present in database\n",
					id);
			} else {
				printf("group name for %d is %s\n",
					id, gp->gr_name);
			}
		}
	}                                                 

	endgrent();
	return 0;
}
