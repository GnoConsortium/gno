#	$Id: Makefile,v 1.1 1998/02/02 08:18:19 taubert Exp $

.INCLUDE: /src/gno/paths.mk

PROG=	kern
OBJS=	main.o patch.o kern.o sys.o signal.o ctool.o sem.o \
	queue.o data.o diag.o resource.o tty.o select.o gsos.o \
	p16.o pipe.o shellcall.o fastfile.o texttool.o driver.o \
	util.o var.o ep.o err.root regexp.root stat.o ports.o \
	sleep.o pty.o net.o \
	inout.o console.o box.root conpatch.o
CFLAGS+=-DKERNEL
.SOURCE: ../drivers

$(PROG): $(OBJS)
	gsh -c ${mktmp prefix 13 /lang/orca/libraries; $(LD) $(LDFLAGS) -o $@ $<}
	compile +w $(PROG).rez keep=$@
	chtyp -t s16 $@
$(PROG): $(PROG).rez

# dependencies

data.o: conf.h kernel.h proc.h q.h sem.h
diag.o: proc.h sys.h
ep.o: proc.h sys.h
fastfile.o: proc.h sys.h
main.o: proc.h gno.h sys.h sem.h tty.h
net.o: gno.h proc.h sys.h kernel.h net.h
patch.o: proc.h gno.h sys.h sem.h conf.h
ports.o: conf.h kernel.h proc.h gno.h
queue.o: q.h conf.h proc.h kernel.h sys.h
sem.o: sys.h sem.h proc.h q.h conf.h kernel.h gno.h
signal.o: conf.h kernel.h proc.h sys.h gno.h
sleep.o: proc.h sys.h kernel.h
stat.o: proc.h sys.h kernel.h
sys.o: conf.h proc.h kernel.h kvm.h gno.h sys.h
driver.o: m/driver.mac global.equates inc/tty.inc
err.o: m/err.mac
gsos.o: m/gsos.mac global.equates inc/gsos.inc inc/tty.inc
p16.o: m/p16.mac global.equates inc/gsos.inc inc/tty.inc
pipe.o: m/pipe.mac inc/kern.inc inc/gsos.inc
pty.o: inc/tty.inc
regexp.o: m/regexp.mac
resource.o: inc/gsos.inc
select.o: m/select.mac inc/tty.inc inc/gsos.inc inc/kern.inc global.equates
shellcall.o: m/shellcall.mac global.equates
texttool.o: m/texttool.mac global.equates
tty.o: m/tty.mac inc/tty.inc
util.o: m/util.mac
var.o: m/var.mac

