#	$Id: Makefile,v 1.1 1998/02/02 08:17:50 taubert Exp $

.INCLUDE: /src/gno/paths.mk

all: modem printer null

modem: port.o msccf.o
	$(LD) $(LDFLAGS) -l libsim -o $@ $<
	chtyp -t 187 -a 32257 $@
modem: libsim

printer: port.o psccf.o
	$(LD) $(LDFLAGS) -l libsim -o $@ $<
	chtyp -t 187 -a 32257 $@
printer: libsim

null: null.o
	$(LD) $(LDFLAGS) -o $@ $<
	chtyp -t 187 -a 32257 $@

console: console.o inout.o box.root conpatch.o
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

