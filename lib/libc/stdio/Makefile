#
# gno/lib/libc/stdio/Makefile
#
# Devin Reade, 1997
#
# $Id: Makefile,v 1.7 1997/10/30 04:52:44 gdr Exp $
#

.INCLUDE:	../../../paths.mk
.INCLUDE: 	../../const.mk

OBJS	= clrerr.o fclose.o fdopen.o feof.o ferror.o fflush.o fgetc.o \
	fgetln.o fgetpos.o fgets.o fileno.o findfp.o flags.o fopen.o \
	fprintf.o fpurge.o fputc.o fputs.o fread.o freopen.o fscanf.o \
	fseek.o fsetpos.o ftell.o funopen.o fvwrite.o fwalk.o fwrite.o \
	getc.o getchar.o gets.o getw.o makebuf.o mktemp.o perror.o printf.o \
	putc.o putchar.o puts.o putw.o refill.o remove.o rewind.o rget.o \
	scanf.o setbuf.o setbuffer.o setvbuf.o snprintf.o sprintf.o sscanf.o \
	stdio.o tempnam.o tmpfile.o tmpnam.o ungetc.o vfscanf.o \
	vprintf.o vscanf.o vsnprintf.o vsprintf.o vsscanf.o wbuf.o wsetup.o \
	vfprintf1.o vfprintf2.o

CFLAGS	+= -O78

build .PHONY:	../libc
obj:	$(OBJS)

#
# There is a problem with ORCA/C v2.1.1b2 (and likely earlier versions)
# that keeps it from compiling vfprintf.c.  Instead the machine crashes.
# (Perhaps buffer overflow?)
# We can avoid the problem by preprocessing this file with a stand-alone
# cpp(1).  This problem has been reported to Byte Works.
#
JINCL	= -I/src/gno/include -I/lang/orca/include
JDEFS	= -D__ORCAC__ -D__appleiigs__ -D__GNO__ -DSPLIT_FILE_2

junk.c:		vfprintf.c
	cpp $(JINCL) $(JDEFS) $< > $@

vfprintf1.o:	vfprintf.c
	$(CC) -c -o $@ $(CFLAGS) -DSPLIT_FILE_1 $<

vfprintf2.o:	junk.c
	$(CC) -c -o $@ $(CFLAGS) -DSPLIT_FILE_2 $<

.INCLUDE:	../rules.mk

# Additional dependancies:
fclose.o::	local.h
fdopen.o::	local.h
fflush.o::	local.h
fgetln.o::	local.h
fgets.o::	local.h
findfp.o::	local.h glue.h
fopen.o::	local.h
fpurge.o::	local.h
fputs.o::	fvwrite.h
fread.o::	local.h
freopen.o::	local.h
fseek.o::	local.h
ftell.o::	local.h
funopen.o::	local.h
fvwrite.o::	local.h fvwrite.h
fwalk.o::	local.h glue.h
fwrite.o::	local.h fvwrite.h
makebuf.o::	local.h
puts.o::	fvwrite.h
putw.o::	fvwrite.h
refill.o::	local.h          
rget.o::	local.h
setbuf.o::	local.h
setvbuf.o::	local.h
sprintf.o::	local.h
sscanf.o::	local.h
stdio.o::	local.h
ungetc.o::	local.h
vfprintf.o::	local.h fvwrite.h floatio.h
vfprintf1.o::	local.h fvwrite.h floatio.h
vfprintf2.o::	local.h fvwrite.h floatio.h
vfscanf.o::	local.h floatio.h
wbuf.o::	local.h
wsetup.o::	local.h
