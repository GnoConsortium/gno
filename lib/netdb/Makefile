#
# Makefile for netdb
#
# $Id: Makefile,v 1.1 1998/01/24 07:05:09 taubert Exp $
#

.INCLUDE:	../../paths.mk
.INCLUDE: 	../const.mk

OBJS=	rcmd.o getnetbyname.o getnetbyaddr.o getnetent.o getprotoname.o \
	getproto.o getprotoent.o getservbyname.o getservbyport.o \
	getservent.o gethostnamadr.o sethostent.o inet_addr.o inet_lnaof.o \
	inet_makeaddr.o inet_netof.o inet_network.o inet_ntoa.o res_comp.o \
	res_debug.o res_init.o res_mkquery.o res_query.o res_send.o \
	herror.o writev.o

CFLAGS	+=-O79

libnetdb .PHONY: $(OBJS)
	$(RM) $@
	$(MAKELIB) $(MAKELIBFLAGS) -l $@ $(OBJS)
              
obj:	$(OBJS)

%.o: %.c
	$(CC) -o $@ -c $(__OFLAG) $(CFLAGS) $<

