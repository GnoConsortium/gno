#
# gno/lib/libc/stdio/Makefile
#
# $Id: Makefile,v 1.11 1998/02/19 01:07:33 gdr-ftp Exp $
#

LIB	= c
LIBPFX	= $(OBJ_DIR)../
NO_REZ	= true
SRCS	= clrerr.c fclose.c fdopen.c feof.c ferror.c fflush.c fgetc.c \
	  fgetln.c fgetpos.c fgets.c fileno.c findfp.c flags.c fopen.c \
	  fprintf.c fpurge.c fputc.c fputs.c fread.c freopen.c fscanf.c \
	  fseek.c fsetpos.c ftell.c funopen.c fvwrite.c fwalk.c fwrite.c \
	  getc.c getchar.c gets.c getw.c makebuf.c mktemp.c perror.c printf.c \
	  putc.c putchar.c puts.c putw.c refill.c remove.c rewind.c rget.c \
	  scanf.c setbuf.c setbuffer.c setvbuf.c snprintf.c sprintf.c sscanf.c \
	  stdio.c tempnam.c tmpfile.c tmpnam.c ungetc.c vfscanf.c \
	  vprintf.c vscanf.c vsnprintf.c vsprintf.c vsscanf.c wbuf.c wsetup.c \
	  vfprintf1.c vfprintf2.c

.INCLUDE: /src/gno/lib/lib.mk

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

junk:	$(OBJ_DIR) $(OBJ_DIR)junk.c

$(OBJ_DIR)junk.c:		vfprintf.c
	gsh -c ${mktmp cpp $(JINCL) $(JDEFS) $< > $@}

vfprintf1.o:	vfprintf.c
	$(CC) -c -o $@ $(CFLAGS) -DSPLIT_FILE_1 $<

vfprintf2.o:	$(OBJ_DIR)junk.c
	$(CC) -c -o $@ $(CFLAGS) -DSPLIT_FILE_2 $<

# Additional dependancies:
fclose.o:	local.h
fdopen.o:	local.h
fflush.o:	local.h
fgetln.o:	local.h
fgets.o:	local.h
findfp.o:	local.h glue.h
fopen.o:	local.h
fpurge.o:	local.h
fputs.o:	fvwrite.h
fread.o:	local.h
freopen.o:	local.h
fseek.o:	local.h
ftell.o:	local.h
funopen.o:	local.h
fvwrite.o:	local.h fvwrite.h
fwalk.o:	local.h glue.h
fwrite.o:	local.h fvwrite.h
makebuf.o:	local.h
puts.o:		fvwrite.h
putw.o:		fvwrite.h
refill.o:	local.h          
rget.o:		local.h
setbuf.o:	local.h
setvbuf.o:	local.h
sprintf.o:	local.h
sscanf.o:	local.h
stdio.o:	local.h
ungetc.o:	local.h
vfprintf.o:	local.h fvwrite.h floatio.h
vfprintf1.o:	local.h fvwrite.h floatio.h
vfprintf2.o:	local.h fvwrite.h floatio.h
vfscanf.o:	local.h floatio.h
wbuf.o:		local.h
wsetup.o:	local.h
