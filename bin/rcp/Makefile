#	@(#)Makefile	8.1 (Berkeley) 7/19/93
#	$Id: Makefile,v 1.1 1998/01/24 07:35:16 taubert Exp $

PROG=	rcp
SRCS=	rcp.c util.c
LDADD=	-lnetdb
OPTIMIZE=79
DEBUG=	25


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
