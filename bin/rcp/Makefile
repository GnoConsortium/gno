#	@(#)Makefile	8.1 (Berkeley) 7/19/93
#	$Id: Makefile,v 1.2 1998/02/09 04:00:36 gdr-ftp Exp $

PROG=	rcp
SRCS=	rcp.c util.c
LDADD=	-lnetdb
OPTIMIZE=79
DEBUG=	25

HAS_BSD_MANPAGE	= true		# We want rcp.1G, not rcp.1

#.if exists(${DESTDIR}/usr/lib/libkrb.a) && (defined(MAKE_KERBEROS)	\
#	|| defined(MAKE_EBONES))
#SRCS+=	kcmd.c krcmd.c
#CFLAGS+=-DKERBEROS -DCRYPT
#DPADD=	${LIBKRB} ${LIBDES}
#LDADD=	-lkrb -ldes
#DISTRIBUTION=	krb
#.endif

BINOWN=	root
BINMODE=4555
INSTALLFLAGS=-fschg

#.PATH:	${.CURDIR}/../../usr.bin/rlogin

.INCLUDE: /src/gno/prog.mk
