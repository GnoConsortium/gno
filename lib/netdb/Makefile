#
# Makefile for netdb
#
# $Id: Makefile,v 1.3 1998/02/15 19:27:19 gdr-ftp Exp $
#

LIB=	netdb
SRCS=	rcmd.c getnetbyname.c getnetbyaddr.c getnetent.c getprotoname.c \
	getproto.c getprotoent.c getservbyname.c getservbyport.c \
	getservent.c gethostnamadr.c sethostent.c inet_addr.c inet_lnaof.c \
	inet_makeaddr.c inet_netof.c inet_network.c inet_ntoa.c res_comp.c \
	res_debug.c res_init.c res_mkquery.c res_query.c res_send.c \
	herror.c writev.c

# Put object files in /obj hierarchy and replace underscores with periods.
FAKESRC	= $(SRCS:s/_/./g)
OBJS	= $(OBJ_DIR){$(FAKESRC:b)}.o

OPTIMIZE=79

.INCLUDE: /src/gno/lib/lib.mk

# We have to give all these explicit rules because we have renamed our
# object files such that they can be placed on a ProDOS volume.
$(OBJ_DIR)inet.addr.o:		inet_addr.c;	$(CC) -o $@ $(CFLAGS) -c $<
$(OBJ_DIR)inet.lnaof.o:		inet_lnaof.c;	$(CC) -o $@ $(CFLAGS) -c $<
$(OBJ_DIR)inet.makeaddr.o:	inet_makeaddr.c;$(CC) -o $@ $(CFLAGS) -c $<
$(OBJ_DIR)inet.netof.o:		inet_netof.c;	$(CC) -o $@ $(CFLAGS) -c $<
$(OBJ_DIR)inet.network.o:	inet_network.c;	$(CC) -o $@ $(CFLAGS) -c $<
$(OBJ_DIR)inet.ntoa.o:		inet_ntoa.c;	$(CC) -o $@ $(CFLAGS) -c $<
$(OBJ_DIR)iso.addr.o:		iso_addr.c;	$(CC) -o $@ $(CFLAGS) -c $<
$(OBJ_DIR)ns.addr.o:		ns_addr.c;	$(CC) -o $@ $(CFLAGS) -c $<
$(OBJ_DIR)ns.ntoa.o:		ns_ntoa.c;	$(CC) -o $@ $(CFLAGS) -c $<
$(OBJ_DIR)res.comp.o:	 	res_comp.c;	$(CC) -o $@ $(CFLAGS) -c $<
$(OBJ_DIR)res.debug.o:		res_debug.c;	$(CC) -o $@ $(CFLAGS) -c $<
$(OBJ_DIR)res.init.o:		res_init.c;	$(CC) -o $@ $(CFLAGS) -c $<
$(OBJ_DIR)res.mkquery.o:	res_mkquery.c;	$(CC) -o $@ $(CFLAGS) -c $<
$(OBJ_DIR)res.query.o:		res_query.c;	$(CC) -o $@ $(CFLAGS) -c $<
$(OBJ_DIR)res.send.o:		res_send.c;	$(CC) -o $@ $(CFLAGS) -c $<

