#
# gno/lib/libc/stdio/Makefile
#
# $Id: Makefile,v 1.10 1998/02/09 08:44:46 taubert Exp $
#

LIB=	c
LIBPFX=	$(OBJ_DIR)../
OBJS=	clrerr.o fclose.o fdopen.o feof.o ferror.o fflush.o fgetc.o \
	fgetln.o fgetpos.o fgets.o fileno.o findfp.o flags.o fopen.o \
	fprintf.o fpurge.o fputc.o fputs.o fread.o freopen.o fscanf.o \
	fseek.o fsetpos.o ftell.o funopen.o fvwrite.o fwalk.o fwrite.o \
	getc.o getchar.o gets.o getw.o makebuf.o mktemp.o perror.o printf.o \
	putc.o putchar.o puts.o putw.o refill.o remove.o rewind.o rget.o \
	scanf.o setbuf.o setbuffer.o setvbuf.o snprintf.o sprintf.o sscanf.o \
	stdio.o tempnam.o tmpfile.o tmpnam.o ungetc.o vfscanf.o \
	vprintf.o vscanf.o vsnprintf.o vsprintf.o vsscanf.o wbuf.o wsetup.o \
	vfprintf1.o vfprintf2.o

#
# There is a problem with ORCA/C v2.1.1b2 (and likely earlier versions)
# that keeps it from compiling vfprintf.c.  Instead the machine crashes.
# (Perhaps buffer overflow?)
# We can avoid the problem by preprocessing this file with a stand-alone
# cpp(1).  This problem has been reported to Byte Works.
#
# It is *critical* that the directories specified here match those defined
# by the "path" pragma in 13/orcacdefs/defaults.h.
#
JINCL	= -I/usr/include -I/lang/orca/libraries/orcacdefs
JDEFS	= -D__ORCAC__ -D__appleiigs__ -D__GNO__ -DSPLIT_FILE_2

.INCLUDE: /src/gno/lib/lib.mk

junk.c:		vfprintf.c
	gsh -c ${mktmp cpp $(JINCL) $(JDEFS) $< > 14/junk.c; $(CP) 14/junk.c $@}

vfprintf1.o:	vfprintf.c
	$(CC) -c -o $@ $(CFLAGS) -DSPLIT_FILE_1 $<

vfprintf2.o:	junk.c
	$(CC) -c -o $@ $(CFLAGS) -DSPLIT_FILE_2 $<

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
