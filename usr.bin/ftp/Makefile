#	$Id: Makefile,v 1.3 1998/02/15 00:31:00 gdr-ftp Exp $

PROG=	ftp
MAIN=	main
SRCS=	main.c cmdtab.c domacro.c ruserpass.c ftp.c glob.c cmds.c ftp_var.c

# We explicitly set the object files here so that we can avoid a
# non-ProDOS name on the /obj partition.  Consequently, we also have
# to give an explicit rule for making the ftp.var.o file.
FAKESRCS = $(SRCS:s,_,.,g)
OBJS	 = $(OBJ_DIR){$(FAKESRCS:b)}.o

HAS_BSD_MANPAGE	= true		# use ftp.1G rather than ftp.1

STACK=	2048
LDADD=	-lnetdb
DEBUG=	25

.INCLUDE: /src/gno/prog.mk

$(OBJ_DIR)ftp.var.o: ftp_var.c
	$(CC) -o $@ $(CFLAGS) -c $<

#%.o : %.c
#	$(CC) -o $@ $(CFLAGS) $(__OFLAG) -c $<
#	@gsh -c ${mktmp cd $(OBJ_DIR); nameobj -p $@}
