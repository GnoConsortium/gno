/*
 * Copyright (c) 1988, 1989, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)termios.h	8.3 (Berkeley) 3/28/94
 * $Id: termios.h,v 1.1 1997/02/28 04:42:15 gdr Exp $
 */

#ifndef _SYS_TERMIOS_H_
#define _SYS_TERMIOS_H_

/*
 * Special Control Characters
 *
 * Index into c_cc[] character array.
 *
 *	Name	     Subscript	Enabled by
 */
#define	VEOF		0	/* ICANON */
#define	VEOL		1	/* ICANON */
#define VINTR		2	/* ISIG */
#define VQUIT		3	/* ISIG */
#define VSTART		4	/* IXON, IXOFF */
#define VSTOP		5	/* IXON, IXOFF */
#define VSUSP		6	/* ISIG */
#define	VERASE		7	/* ICANON */
#define VKILL		8	/* ICANON */

#define VMIN		1	/* !ICANON */
#define VTIME		2	/* !ICANON */

#define	NCCS		9

#if 0				/* not used by GNO v2.0.6 */
#ifndef _POSIX_SOURCE
#define	VEOL2		9	/* ICANON */
#define VWERASE 	10	/* ICANON */
#define	VREPRINT 	11	/* ICANON */
#define VDSUSP		12	/* ISIG */
#define	VLNEXT		13	/* IEXTEN */
#define	VDISCARD	14	/* IEXTEN */
#define VSTATUS		15	/* ICANON */
#define	NCCS		15
#endif
#endif	/* 0 */

#define	_POSIX_VDISABLE	0xff

#ifndef _POSIX_SOURCE
#define	CCEQ(val, c)	((c) == (val) ? (val) != _POSIX_VDISABLE : 0)
#endif

/*
 * Input flags - software input processing
 */
#define	BRKINT		0x0001	/* map BREAK to SIGINTR */
#define	IGNBRK		0x0002	/* ignore BREAK condition */
#define	IGNPAR		0x0004	/* ignore (discard) parity errors */
#define	PARMRK		0x0008	/* mark parity and framing errors */
#define	INPCK		0x0010	/* enable checking of parity errors */
#define	ISTRIP		0x0020	/* strip 8th bit off chars */
#define	INLCR		0x0040	/* map NL into CR */
#define	IGNCR		0x0080	/* ignore CR */
#define	ICRNL		0x0100	/* map CR to NL (ala CRMOD) */
#define	IXON		0x0200	/* enable output flow control */
#define	IXOFF		0x0400	/* enable input flow control */
#if 0 && !defined(_POSIX_SOURCE)	/* not used by GNO v2.0.6 */
#define	IXANY		0x0800	/* any char will restart after stop */
#define IMAXBEL		0x2000	/* ring bell on input queue full */
#endif  /*_POSIX_SOURCE */

/*
 * Output flags - software output processing
 */
#define	OPOST		0x0001	/* enable following output processing */
#ifndef _POSIX_SOURCE
#define ONLCR		0x0002	/* map NL to CR-NL (ala CRMOD) */
#define OXTABS		0x0004	/* expand tabs to spaces */
#if 0				/* not used by GNO v2.0.6 */
#define ONOEOT		0x0008	/* discard EOT's (^D) on output) */
#endif
#endif  /*_POSIX_SOURCE */

/*
 * Control flags - hardware control of terminal
 */
#define CSIZE		0x000C		/* character size mask */
#define     CS5		    0x0000	    /* 5 bits (pseudo) */
#define     CS6		    0x0004	    /* 6 bits */
#define     CS7		    0x0008	    /* 7 bits */
#define     CS8		    0x000C	    /* 8 bits */
#define CLOCAL		0x0001		/* ignore modem status lines */
#define CREAD		0x0002		/* enable receiver */
#define CSTOPB		0x0010		/* send 2 stop bits iff set */
#define HUPCL		0x0020		/* hang up on last close */
#define PARENB		0x0040		/* parity enable */
#define PARODD		0x0080		/* odd parity iff set, else even */

#if 0					/* not used by GNO v2.0.6 */
#ifndef _POSIX_SOURCE
#define CCTS_OFLOW	0x0100		/* CTS flow control of output */
#define CRTSCTS		(CCTS_OFLOW | CRTS_IFLOW)
#define CRTS_IFLOW	0x0200		/* RTS flow control of input */
#define	CDTR_IFLOW	0x0400		/* DTR flow control of input */
#define CDSR_OFLOW	0x0800		/* DSR flow control of output */
#define	CCAR_OFLOW	0x1000		/* DCD flow control of output */
#define	MDMBUF		0x2000		/* old name for CCAR_OFLOW */
#define	CIGNORE		0x4000		/* ignore control flags */
#endif	/* _POSIX_SOURCE */
#endif	/* 0 */

/*
 * "Local" flags - dumping ground for other state
 *
 * Warning: some flags in this structure begin with
 * the letter "I" and look like they belong in the
 * input flag.
 */
#undef	ECHO				/* from <sys/ioctl_compat.h> */
#define ECHO		0x0001		/* enable echoing */
#define	ECHOE		0x0002		/* visually erase chars */
#define	ECHOK		0x0004		/* echo NL after line kill */
#define	ECHONL		0x0008		/* echo NL even if ECHO is off */
#define	ICANON		0x0010		/* canonicalize input lines */
#define	ISIG		0x0020		/* enable signals INTR, QUIT, [D]SUSP */
#undef	NOFLSH
#define	NOFLSH		0x0040		/* don't flush after interrupt */
#undef	TOSTOP
#define TOSTOP		0x0080		/* stop background jobs from output */
#define	IEXTEN		0x0100		/* enable DISCARD and LNEXT */

#if 0					/* not used for GNO v2.0.6 */
#ifndef _POSIX_SOURCE
#define EXTPROC         0x0200		/* external processing */
#define	ECHOKE		0x0400		/* visual erase for line kill */
#define	ECHOPRT		0x0800		/* visual erase mode for hardcopy */
#define ECHOCTL  	0x1000		/* echo control chars as ^(Char) */
#define FLUSHO		0x2000		/* output being flushed (state) */
#define	NOKERNINFO	0x4000		/* no kernel output from VSTATUS */
#define PENDIN		0x8000		/* XXX retype pending input (state) */
#define ALTWERASE   Oops. No more bits	/* use alternate WERASE algorithm */
#endif  /*_POSIX_SOURCE */
#endif	/* 0 */

typedef unsigned short	tcflag_t;	/* non-BSD type (as u_long) */
typedef unsigned char	cc_t;
typedef unsigned char	speed_t;	/* XXX should be unsigned long */

struct termios {		/* non-BSD; sequence of fields changed */
	speed_t		c_ispeed;	/* input speed */
	speed_t		c_ospeed;	/* output speed */
	cc_t		c_cc[NCCS];	/* control chars */
	tcflag_t	c_iflag;	/* input flags */
	tcflag_t	c_oflag;	/* output flags */
	tcflag_t	c_cflag;	/* control flags */
	tcflag_t	c_lflag;	/* local flags */
};

/*
 * Commands passed to tcsetattr() for setting the termios structure.
 */
#define	TCSANOW		0		/* make change immediate */
#define	TCSADRAIN	1		/* drain output, then change */
#define	TCSAFLUSH	2		/* drain output, flush input */
#ifndef _POSIX_SOURCE
#define TCSASOFT	0x10		/* flag - don't alter h.w. state */
#endif

/*
 * Standard speeds
 */
#define B0	0
#define B50	1
#define B75	2
#define B110	3
#define B134	4
#define B150	5
#define B200	6	/* This is really 57600 baud!!! */
#define B300	7
#define B600	8
#define B1200	9
#define	B1800	10
#define B2400	11
#define B4800	12
#define B9600	13
#define B19200	14
#define B38400	15
#ifndef _POSIX_SOURCE
#define EXTA	14
#define EXTB	15
#define B57600	B200
#endif

#if 0					/* not used by GNO v2.0.6 */
#ifndef _POSIX_SOURCE
#define B7200	16
#define B14400	17
#define B28800	18
#define B76800	19
#define B115200	20
#define B230400	21
#endif  /* !_POSIX_SOURCE */
#endif	/* 0 */

#ifndef KERNEL

#define	TCIFLUSH	1
#define	TCOFLUSH	2
#define TCIOFLUSH	3
#define	TCOOFF		1
#define	TCOON		2
#define TCIOFF		3
#define TCION		4

#ifndef _SYS_CDEFS_H_
#include <sys/cdefs.h>
#endif

__BEGIN_DECLS
speed_t	cfgetispeed __P((const struct termios *));
speed_t	cfgetospeed __P((const struct termios *));
int	cfsetispeed __P((struct termios *, speed_t));
int	cfsetospeed __P((struct termios *, speed_t));
int	tcgetattr __P((int, struct termios *));
int	tcsetattr __P((int, int, const struct termios *));
int	tcdrain __P((int));
int	tcflow __P((int, int));
int	tcflush __P((int, int));
int	tcsendbreak __P((int, int));

#ifndef _POSIX_SOURCE
void	cfmakeraw __P((struct termios *));
int	cfsetspeed __P((struct termios *, speed_t));
#endif /* !_POSIX_SOURCE */
__END_DECLS

#endif /* !KERNEL */


/*
 * Include tty ioctl's that aren't just for backwards compatibility
 * with the old tty driver.  These ioctl definitions were previously
 * in <sys/ioctl.h>.
 */
#if !defined(_POSIX_SOURCE) && !defined(_SYS_TTYCOM_H_)
#include <sys/ttycom.h>
#endif

/*
 * END OF PROTECTED INCLUDE.
 */
#endif /* !_SYS_TERMIOS_H_ */

#if !defined(_POSIX_SOURCE) && !defined(_SYS_TTYDEFAULTS_H_)
#include <sys/ttydefaults.h>
#endif
