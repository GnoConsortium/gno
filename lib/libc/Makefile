#
# gno/lib/libc/Makefile
#
# This is the top-level makefile for libc, GNO v2.0.6
#
# dmake is too bloated right now to due recursive makes well, so before
# you run make in this directory, ensure you run make in the following
# directories:
#		gen, gno, locale, stdio, stdlib, string, sys
#
# Devin Reade, 1997.
#
# $Id: Makefile,v 1.4 1997/09/05 06:50:30 gdr Exp $
#

.INCLUDE: ../../paths.mk
.INCLUDE: ../const.mk

LIBC_OBJ	= \
	sys/exec.o \
	sys/syscall.o \
	sys/trap.o \
	gno/gnocmd.o \
	gno/gnomisc.o \
	gno/gsstring.o \
	gno/map.o \
	gno/parsearg.o \
	gno/stack.o \
	gen/basename.o \
	gen/bmem.o \
	gen/compat.o \
	gen/crypt.o \
	gen/crypta.o \
	gen/dirent.o \
	gen/err.o \
	gen/fnmatch.o \
	gen/fts.o \
	gen/getcwd.o \
	gen/getgrent.o \
	gen/getpass.o \
	gen/getpwent.o \
	gen/getttyent.o \
	gen/hostname.o \
	gen/oldlog.o \
	gen/popen.o \
	gen/psignal.o \
	gen/pwcache.o \
	gen/scandir.o \
	gen/setjmp.o \
	gen/siglist.o \
	gen/sleep.o \
	gen/syslog.o \
	gen/tty.o \
	gen/utime.o \
	locale/table.o \
	stdio/clrerr.o \
	stdio/fclose.o \
	stdio/fdopen.o \
	stdio/feof.o \
	stdio/ferror.o \
	stdio/fflush.o \
	stdio/fgetc.o \
	stdio/fgetln.o \
	stdio/fgetpos.o \
	stdio/fgets.o \
	stdio/fileno.o \
	stdio/findfp.o \
	stdio/flags.o \
	stdio/fopen.o \
	stdio/fprintf.o \
	stdio/fpurge.o \
	stdio/fputc.o \
	stdio/fputs.o \
	stdio/fread.o \
	stdio/freopen.o \
	stdio/fscanf.o \
	stdio/fseek.o \
	stdio/fsetpos.o \
	stdio/ftell.o \
	stdio/funopen.o \
	stdio/fvwrite.o \
	stdio/fwalk.o \
	stdio/fwrite.o \
	stdio/getc.o \
	stdio/getchar.o \
	stdio/gets.o \
	stdio/getw.o \
	stdio/makebuf.o \
	stdio/mktemp.o \
	stdio/perror.o \
	stdio/printf.o \
	stdio/putc.o \
	stdio/putchar.o \
	stdio/puts.o \
	stdio/putw.o \
	stdio/refill.o \
	stdio/remove.o \
	stdio/rewind.o \
	stdio/rget.o \
	stdio/scanf.o \
	stdio/setbuf.o \
	stdio/setbuffer.o \
	stdio/setvbuf.o \
	stdio/snprintf.o \
	stdio/sprintf.o \
	stdio/sscanf.o \
	stdio/stdio.o \
	stdio/tempnam.o \
	stdio/tmpfile.o \
	stdio/tmpnam.o \
	stdio/ungetc.o \
	stdio/vfprintf.o \
	stdio/vfscanf.o \
	stdio/vprintf.o \
	stdio/vscanf.o \
	stdio/vsnprintf.o \
	stdio/vsprintf.o \
	stdio/vsscanf.o \
	stdio/wbuf.o \
	stdio/wsetup.o \
	stdlib/cvt.o \
	stdlib/environ.o \
	stdlib/fpspecnum.o \
	stdlib/getopt.o \
	stdlib/getsubopt.o \
	string/case.o \
	string/str.o \
	string/strerror.o

libc:	$(LIBC_OBJ)
	$(RM) -f $@
	$(MAKELIB) $(MAKELIBFLAGS) -l $@ $(LIBC_OBJ)
