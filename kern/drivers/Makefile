#	$Id: Makefile,v 1.3 1999/02/23 04:27:07 gdr-ftp Exp $

.INCLUDE: /src/gno/paths.mk

build: all

all: $(OBJ_DIR) $(OBJ_DIR)modem $(OBJ_DIR)printer $(OBJ_DIR)null

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

$(OBJ_DIR)console: console.o inout.o box.root conpatch.o
	$(LD) $(LDFLAGS) -o $@ $<
	chtyp -t 187 -a 32257 $@

### Dependencies ###

box.root: box.mac console.equates
conpatch.asm: conpatch.mac
console.root: port.mac console.equates kern.equates ../gno/inc/tty.inc
inout.root: inout.mac console.equates kern.equates ../gno/inc/tty.inc
msccf.root: sccf.mac equates md.equates sccf.asm ../gno/inc/tty.inc
null.root: port.mac ../gno/inc/tty.inc
port.root: port.mac equates ../gno/inc/tty.inc
psccf.root: sccf.mac equates pr.equates sccf.asm ../gno/inc/tty.inc

