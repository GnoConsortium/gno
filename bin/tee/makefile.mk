# CFLAGS	= -w -i -G25 -v -DCHECK_STACK=1
# LDFLAGS	= -l/usr/lib/gnulib -l/usr/lib/stack

CFLAGS	= -w -i -O -s768
LDFLAGS	= -l/usr/lib/gnulib -s768

all: tee

install:
	cp -f tee /bin
	cp -f tee.1 /usr/man/man1
