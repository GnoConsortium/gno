#	$Id: Makefile,v 1.4 1999/03/19 07:18:00 gdr-ftp Exp $

.INCLUDE: /src/gno/paths.mk

TARGETS = $(OBJ_DIR)modem $(OBJ_DIR)printer $(OBJ_DIR)null

build: all

all: $(OBJ_DIR) $(TARGETS)

$(OBJ_DIR):
	install -d $(OBJ_DIR)

$(OBJ_DIR)modem: port.o msccf.o
	$(LD) $(LDFLAGS) -l /usr/lib/libsim -o $@ $<
	chtyp -t 187 -a 32257 $@

$(OBJ_DIR)printer: port.o psccf.o
	$(LD) $(LDFLAGS) -l /usr/lib/libsim -o $@ $<
	chtyp -t 187 -a 32257 $@

$(OBJ_DIR)null: null.o
	$(LD) $(LDFLAGS) -o $@ $<
	chtyp -t 187 -a 32257 $@

$(OBJ_DIR)console: console.o inout.o box.o conpatch.o
	$(LD) $(LDFLAGS) -o $@ $<
	chtyp -t 187 -a 32257 $@

$(OBJ_DIR)modem::   modem.r;	$(CATREZ) -d $@ $<
$(OBJ_DIR)printer:: printer.r;	$(CATREZ) -d $@ $<
$(OBJ_DIR)null::    null.r;	$(CATREZ) -d $@ $<
$(OBJ_DIR)console:: console.r;	$(CATREZ) -d $@ $<

release: all
	$(INSTALL) -d $(RELEASE_DIR)/dev 
	$(INSTALL) $(TARGETS) $(RELEASE_DIR)/dev

install:
	@echo You cannot install the device drivers into a live system.  Target ignored.

# Implicit rule to handle Rez source on case sensitive Appleshare servers
.IF $(APPLESHARE_CASE_SENSITIVE) != $(NULL)
%.r : %.rez
	$(INSTALL) $< $(TMPDIR)/$<
	$(REZ) -o $@ -c $(REZFLAGS) $(TMPDIR)/$<
	$(RM) -f $(TMPDIR)/$<
.END

### Dependencies ###

box.o: box.mac console.equates
conpatch.asm: conpatch.mac
console.root: port.mac console.equates kern.equates ../gno/inc/tty.inc
inout.root: inout.mac console.equates kern.equates ../gno/inc/tty.inc
msccf.root: sccf.mac equates md.equates sccf.asm ../gno/inc/tty.inc
null.root: port.mac ../gno/inc/tty.inc
port.root: port.mac equates ../gno/inc/tty.inc
psccf.root: sccf.mac equates pr.equates sccf.asm ../gno/inc/tty.inc

