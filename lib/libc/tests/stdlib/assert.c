/*
 * This (indirectly) tests that raise(3) (through abort(3)) is implemented
 * and does not require a backward linking reference.
 *
 * $Id: assert.c,v 1.1 1997/09/21 17:46:05 gdr Exp $
 *
 * Devin Reade, 1997
 */

#include <assert.h>

int
main(int argc, char **argv) {
	int i = 1;

	assert(i == 0);
	return 1;
}
