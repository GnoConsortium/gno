#	$Id: Makefile,v 1.1 1998/01/24 08:21:08 taubert Exp $

PROG=	ftp
MAIN=	main
SRCS=	main.c cmdtab.c domacro.c ruserpass.c ftp.c glob.c cmds.c ftp_var.c
STACK=	2048
LDADD=	-lnetdb
DEBUG=	25

.INCLUDE: /src/gno/prog.mk

#%.o : %.c
#	$(CC) -o $@ $(CFLAGS) $(__OFLAG) -c $<
#	nameobj -p $@
