#	@(#)Makefile	8.1 (Berkeley) 7/19/93
#	$Id: Makefile,v 1.1 1998/01/24 08:30:54 taubert Exp $

PROG=	rlogin
SRCS=	rlogin.c
LDADD=	-lnetdb
OPTIMIZE=79

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
