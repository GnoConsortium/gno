#	@(#)Makefile	8.1 (Berkeley) 7/19/93
#	$Id: Makefile,v 1.1 1998/01/24 08:13:58 taubert Exp $

PROG=	rsh
SRCS=	rsh.c
LDADD=  -lnetdb
OPTIMIZE=79
DEBUG=	25

#.if exists(${DESTDIR}/usr/lib/libkrb.a) && (defined(MAKE_KERBEROS) \
#	|| defined(MAKE_EBONES))
#CFLAGS+=-DKERBEROS -DCRYPT
#SRCS+=	krcmd.c kcmd.c
#DPADD=	${LIBKRB} ${LIBDES}
#LDADD=	-lkrb -ldes
#DISTRIBUTION=	krb
#.endif

BINOWN=	root
BINMODE=4555
INSTALLFLAGS=-fschg
#.PATH:	${.CURDIR}/../rlogin

.INCLUDE: /src/gno/prog.mk
