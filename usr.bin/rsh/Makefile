#	@(#)Makefile	8.1 (Berkeley) 7/19/93
#	$Id: Makefile,v 1.2 1998/02/09 08:32:56 taubert Exp $

PROG=	rsh
SRCS=	rsh.c

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
.SOURCE: ../rlogin

LDADD=  -lnetdb
OPTIMIZE=79
DEBUG=	25

.INCLUDE: /src/gno/prog.mk
