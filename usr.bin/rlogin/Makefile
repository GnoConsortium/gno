#	@(#)Makefile	8.1 (Berkeley) 7/19/93
#	$Id: Makefile,v 1.3 1998/04/17 01:02:22 gdr-ftp Exp $

PROG=	rlogin
SRCS=	rlogin.c
LDADD=	-lnetdb
OPTIMIZE=79
BINDIR	= /usr/bin
HAS_BSD_MANPAGE	= true

#.if exists(${DESTDIR}/usr/lib/libkrb.a) && (defined(MAKE_KERBEROS) \
#	|| defined(MAKE_EBONES))
#SRCS+=	krcmd.c kcmd.c
#DPADD=	${LIBKRB} ${LIBDES}
#CFLAGS+=-DKERBEROS -DCRYPT
#LDADD=	-lkrb -ldes
#DISTRIBUTION=	krb
#.endif

BINOWN=	root
BINMODE=4555
INSTALLFLAGS=-fschg

.INCLUDE: /src/gno/prog.mk
