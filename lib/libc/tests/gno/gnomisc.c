/*
 * This file tests routines from gno/gnomisc.c and gno/stack.asm
 * Written by Devin Reade
 *
 * $Id: gnomisc.c,v 1.1 1997/02/28 05:12:54 gdr Exp $
 */

#include <stdio.h>
#include <gno/gno.h>

/* early debugging */
#if 0
char *optarg, *suboptarg;
int optopt, optind, opterr, __mb_cur_max;
#endif

void cleanup (void) {
  printf("stack usage: %d bytes\n", _endStackCheck());
}

int main (int argc, char **argv) {
	char *p;

	printf("starting stack check\n");
  _beginStackCheck();
	atexit(cleanup);

  printf("checking for GNO\n");
	if (needsgno() == 0) {
		printf("GNO is NOT active\n");
	} else {
		printf("GNO is active\n");
	}
	p = __prognameGS();
	printf("program name is %s\n", p);
	return 0;
}
