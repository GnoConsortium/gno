#	$Id: Makefile,v 1.4 1998/02/17 02:44:05 gdr-ftp Exp $

PROG=	ftp
MAIN=	main
SRCS=	main.c cmdtab.c domacro.c ruserpass.c ftp.c glob.c cmds.c ftp_var.c

HAS_BSD_MANPAGE	= true		# use ftp.1G rather than ftp.1

STACK=	2048
LDADD=	-lnetdb
DEBUG=	25

.INCLUDE: /src/gno/prog.mk

#%.o : %.c
#	$(CC) -o $@ $(CFLAGS) $(__OFLAG) -c $<
#	@gsh -c ${mktmp cd $(OBJ_DIR); nameobj -p $@}
