/*
 * yes program: based on FreeBSD code, but significantly
 * rewritten for GNO (Apple IIGS) by Dave Tribby, September 1997
 *
 * $Id: yes.c,v 1.1 1997/10/03 04:04:53 gdr Exp $
 */

#include <stdio.h>

int main(int argc, char *argv[])
{
	char *s, *y="y";
	if (argc > 1)
		s = argv[1];
	else
		s = y;
	for (;;)
		puts(s);
}
