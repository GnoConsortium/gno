#
# $Id: Makefile,v 1.2 1998/12/13 02:50:33 gdr-ftp Exp $
#

LIB	= libdvr
ODIR	= obj
OBJS	= \
	$(ODIR)/AddPath.o \
	$(ODIR)/ChangeAux.o \
	$(ODIR)/Close.o \
	$(ODIR)/Create.o \
	$(ODIR)/Data.o \
	$(ODIR)/Delete.o \
	$(ODIR)/GetBufSize.o \
	$(ODIR)/GetVolume.o \
	$(ODIR)/Gnfn.o \
	$(ODIR)/MSGReceive.o \
	$(ODIR)/MSGSend.o \
	$(ODIR)/Open.o \
	$(ODIR)/Read.o \
	$(ODIR)/Rename.o \
	$(ODIR)/SetBufSize.o \
	$(ODIR)/SetDir.o \
	$(ODIR)/SetEOF.o \
	$(ODIR)/Space.o \
	$(ODIR)/Write.o

INCL2	= ../includes
HDRS	= ks.fileio.h $(INCL2)/ks.fileio.e $(INCL2)/ks.memory.h \
	  $(INCL2)/ks.defines.h $(INCL2)/KS.CDEV.h $(INCL2)/KS.Routines.h \
	  $(INCL2)/Portable.C.h $(INCL2)/SharedData.h 

SEGMENT	= -Slibdvr____

CFLAGS	= $(SEGMENT) -I$(INCL2) -r -w -v -O78

build:	$(LIB)       

$(ODIR):
	install -d $(ODIR)

$(LIB):	$(OBJS)
	$(RM) -f $@
	makelib.apw -w -r -p -l $@ $(OBJS)

$(ODIR)/%.o:	%.c
	$(CC) -o $@ -c $(CFLAGS) $<

$(ODIR)/AddPath.o::	$(HDRS)
$(ODIR)/Write.o::	$(HDRS)
$(ODIR)/Space.o::	$(HDRS)
$(ODIR)/SetEOF.o::	$(HDRS)
$(ODIR)/SetDir.o::	$(HDRS)
$(ODIR)/SetBufSize.o::	$(HDRS)
$(ODIR)/Rename.o::	$(HDRS)
$(ODIR)/Read.o::	$(HDRS)
$(ODIR)/Open.o::	$(HDRS)
$(ODIR)/msgsend.o::	$(HDRS)
$(ODIR)/msgreceive.o::	$(HDRS)
$(ODIR)/gnfn.o::	$(HDRS)
$(ODIR)/GetVolume.o::	$(HDRS)
$(ODIR)/GetBufSize.o::	$(HDRS)
$(ODIR)/Delete.o::	$(HDRS)
$(ODIR)/Data.o::	$(HDRS)
$(ODIR)/Create.o::	$(HDRS)
$(ODIR)/Close.o::	$(HDRS)
$(ODIR)/ChangeAux.o::	$(HDRS)
