/*
 * Tests getlogin(2) and setlogin(2).
 * Devin Reade, 1997.
 *
 * $Id: getlogin.c,v 1.1 1997/09/21 16:32:15 gdr Exp $
 */

#include <unistd.h>
#include <stdio.h>

int
main (int argc, char **argv) {
	char *p;

	p = getlogin();
	printf("first getlogin() returns \"%s\"\n", (p == NULL)? "(null)" : p);

	if (setlogin("bork") < 0) {
		perror("setlogin failed");
	} else {
		printf("setlogin passed\n");
	}

	p = getlogin();
	printf("second getlogin() returns \"%s\"\n", (p == NULL)? "(null)" : p);
         
        return 0;
}
