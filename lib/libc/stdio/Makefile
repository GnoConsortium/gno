#
# gno/lib/libc/stdio/Makefile
#
# Devin Reade, 1997
#
# $Id: Makefile,v 1.4 1997/09/05 06:18:46 gdr Exp $
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
	stdio.o tempnam.o tmpfile.o tmpnam.o ungetc.o vfprintf.o vfscanf.o \
	vprintf.o vscanf.o vsnprintf.o vsprintf.o vsscanf.o wbuf.o wsetup.o

default:	$(OBJS)

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
vfscanf.o::	local.h floatio.h
wbuf.o::	local.h
wsetup.o::	local.h
