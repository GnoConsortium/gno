/*
 * The sleep(3) code is from BSD 4.3-reno, heavily modified (shorted)
 * for use with GNO.
 *
 * $Id: sleep.c,v 1.2 1997/09/21 06:05:01 gdr Exp $
 *
 * This file has been formatted for tabs every 8 columns
 */

#ifdef __ORCAC__
segment "libc_gen__";
#endif

#include <signal.h>
#include <unistd.h>

static int _ringring;

#pragma databank 1
static void
_sleephandler(int sig, int code) {
	_ringring = 1;
}
#pragma databank 0

unsigned int
sleep(unsigned int seconds) {
	sig_t ovec;
	long omask;

	if (seconds == 0) {
		return 0;
	}
	ovec = signal(SIGALRM, _sleephandler);
	omask = sigblock(sigmask(SIGALRM));
	_ringring = 0;
        alarm(seconds);
	while (!_ringring) {
		sigpause(omask &~ sigmask(SIGALRM));
	}
	signal(SIGALRM, ovec);
	sigsetmask(omask);
	return 0;
}

int pause(void)
{
	sigpause(0L);
	return -1;
}
