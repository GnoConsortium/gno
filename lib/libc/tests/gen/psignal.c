/*
 * $Id: psignal.c,v 1.1 1997/09/05 06:46:20 gdr Exp $
 */

#include <stdio.h>
#include <sys/signal.h>

int main(int argc, char **argv) {
	int i;

	printf("starting psignal test:\n\n");
	for (i=0; i<NSIG + 1; i++) {
		psignal(i, "signal test");
	}
	return 0;
}
