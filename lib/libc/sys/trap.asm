*
*        trap.asm
*        Toolbox Interface library for
*
*        GNO Kernel
*        v2.0.6
*        Copyright 1991-1997, Procyon Inc.
*
*	$Id: trap.asm,v 1.3 1998/05/30 15:36:15 gdr-ftp Exp $
*
	case  on
	mcopy trap.mac

udispatch gequ	$E10008

dummy	start		; ends up in .root
	end

* int getpid(void) inline (0x0903, udispatch);
getpid	START	libc_sys__
retval	equ	1
	sub	(0:foo),2
	assertVersion $0204	; check for minimum version
	pha		; push result space
	anop		; doesn't take errno
	ldx	#$0903
	jsl	udispatch
	pla
	sta	retval
	ret	2:retval
	END

* int   getppid(void) inline (0x4003, udispatch);
getppid	START	libc_sys__
retval	equ	1
	sub	(0:foo),2
	assertVersion $0204	; check for minimum version
	pha		; push result space
	ph4	#errno
	ldx	#$4003
	jsl	udispatch
	pla
	sta	retval
	ret	2:retval
	END

getuid	START	libc_sys__
retval	equ	1
	sub	(0:foo),2
	assertVersion $0204	; check for minimum version
	pha		; push result space
	ph4	#errno
	ldx	#$2A03
	jsl	udispatch
	pla
	sta	retval
	ret	2:retval
	END

getgid	START	libc_sys__
retval	equ	1
	sub	(0:foo),2
	assertVersion $0204	; check for minimum version
	pha		; push result space
	ph4	#errno
	ldx	#$2B03
	jsl	udispatch
	pla
	sta	retval
	ret	2:retval
	END

geteuid	START	libc_sys__
retval	equ	1
	sub	(0:foo),2
	assertVersion $0204	; check for minimum version
	pha		; push result space
	ph4	#errno
	ldx	#$2C03       
	jsl	udispatch
	pla
	sta	retval
	ret	2:retval
	END

getegid	START	libc_sys__
retval	equ	1
	sub	(0:foo),2
	assertVersion $0204	; check for minimum version
	pha		; push result space
	ph4	#errno
	ldx	#$2D03
	jsl	udispatch
	pla
	sta	retval
	ret	2:retval
	END

setuid	START	libc_sys__
retval	equ	1
	sub	(2:uid),2
	assertVersion $0204	; check for minimum version
	pha		; push result space
	ph2	uid
	ph4	#errno
	ldx	#$2E03
	jsl	udispatch
	pla
	sta	retval
	ret	2:retval
	END

setruid	START	libc_sys__
retval	equ	1
	sub	(2:ruid),2
	assertVersion $0206	; check for minimum version
	pha		; push result space
	pea	$FFFF
	ph2	ruid
	ph4	#errno
	ldx	#$5303	; setreuid
	jsl	udispatch
	pla
	sta	retval
	ret	2:retval
	END

seteuid	START	libc_sys__
retval	equ	1
	sub	(2:euid),2
	assertVersion $0206	; check for minimum version
	pha		; push result space
	ph2	euid
	pea	$FFFF
	ph4	#errno
	ldx	#$5303	; setreuid
	jsl	udispatch
	pla
	sta	retval
	ret	2:retval
	END

setreuid	START	libc_sys__
retval	equ	1
	sub	(2:euid,2:ruid),2
	assertVersion $0206	; check for minimum version
	pha		; push result space
	ph2	euid
	ph2	ruid
	ph4	#errno
	ldx	#$5303	;	setreuid
	jsl	udispatch
	pla
	sta	retval
	ret	2:retval
	END

setgid	START	libc_sys__
retval	equ	1
	sub	(2:gid),2
	assertVersion $0204	; check for minimum version
	pha		; push result space
	ph2	gid	
	ph4	#errno
	ldx	#$2F03
	jsl	udispatch
	pla
	sta	retval
	ret	2:retval
	END

setrgid	START	libc_sys__
retval	equ	1
	sub	(2:rgid),2
	assertVersion $0206	; check for minimum version
	pha		; push result space
	pea	$FFFF
	ph2	rgid
	ph4	#errno
	ldx	#$5403	; setregid
	jsl	udispatch
	pla
	sta	retval
	ret	2:retval
	END

setegid	START	libc_sys__
retval	equ	1
	sub	(2:egid),2
	assertVersion $0206	; check for minimum version
	pha		; push result space
	ph2	egid
	pea	$FFFF
	ph4	#errno
	ldx	#$5403	; setregid
	jsl	udispatch
	pla
	sta	retval
	ret	2:retval
	END

setregid	START	libc_sys__
retval	equ	1
	sub	(2:egid,2:rgid),2
	assertVersion $0206	; check for minimum version
	pha		; push result space
	ph2	egid
	ph2	rgid
	ph4	#errno
	ldx	#$5403	; setregid
	jsl	udispatch
	pla
	sta	retval
	ret	2:retval
	END

* int   kill(int pid, int sig) inline (0x0A03, udispatch);
kill	START	libc_sys__
retval	equ	1
	sub	(2:sig,2:pid),2
	assertVersion $0204	; check for minimum version
	pha
	ph2	pid
	ph2	sig
	ph4	#errno	; tell the library routine where it is
	ldx	#$0A03
	jsl	udispatch
	pla
	sta	retval
	ret	2:retval
	END

* int   fork(void *subr) inline(0x0B03, udispatch);
fork	START	libc_sys__
vfork	ENTRY
retval	equ	1
	sub	(4:subr),2
	assertVersion $0204	; check for minimum version
	pha
	ph4	subr
	ph4	#errno
	ldx	#$0B03
	jsl	udispatch
	pla
	sta	retval
	ret	2:retval
	END

* int fork2(void *subr, int stack, int prio, char *name, word argc, ...)
fork2	START	libc_sys__
subr	equ	7
stack	equ	subr+4
prio	equ	stack+2
name	equ	prio+2
argc	equ	name+4

*	set up the variable frame
	phb
	phk
	plb
	phd
	tsc
	tcd

	assertVersion $0204	; check for minimum version

	pha		; temp space for result
	pei	(subr+2)
	pei	(subr)
	pei	(stack)
	pei	(prio)
	pei	(name+2)
	pei	(name)
	pea	0
	tdc
	clc
	adc	#argc
	pha
	ph4	#errno

	ldx	#$3F03
	jsl	udispatch

	pla
	tay		; temp store result in Y
	lda	argc
	asl	a
	clc
	adc	#argc
	tax
	dex
	lda	5
	sta	1,x
	lda	4
	sta	0,x
	pld
	plb
	dex
	dex
	phx
	tsc
	clc
	adc	1,s
	tcs

	tya
	rtl
	END

* int   exec(char *filename,char *cmdline) inline(0x0C03, udispatch);
*
*	This function is provided in exec.c
*
*exec	START	libc_sys__
*retval	equ	1
*	sub	(4:cmdline,4:filename),2
*
*	pha
*	ph4	filename
*	ph4	cmdline
*	ph4	#errno
*	ldx	#$0C03
*	jsl	udispatch
*	pla
*	sta	retval
*	ret	2:retval
*	END

* int   swait(int sem) inline(0x0D03, udispatch);
swait	START	libc_sys__
retval	equ	1
	sub	(2:sem),2
	assertVersion $0204	; check for minimum version
	pha
	ph2	sem
	ph4	#errno
	ldx	#$0D03
	jsl	udispatch
	pla
	sta	retval
	ret	2:retval
	END

* int ssignal(int sem) inline(0x0E03, udispatch);
ssignal 	START	libc_sys__
retval	equ	1
	sub	(2:sem),2
	assertVersion $0204	; check for minimum version
	pha
	ph2	sem
	ph4	#errno
	ldx	#$0E03
	jsl	udispatch
	pla
	sta	retval
	ret	2:retval
	END

* int   screate(int count) inline(0x0F03, udispatch);
screate	START	libc_sys__
retval	equ	1
	sub	(2:sem),2
	assertVersion $0204	; check for minimum version
	pha
	ph2	sem
	ph4	#errno
	ldx	#$0F03
	jsl	udispatch
	pla
	sta	retval
	ret	2:retval
	END

* int   sdelete(int sem) inline(0x1003, udispatch);
sdelete	START	libc_sys__
retval	equ	1
	sub	(2:sem),2
	assertVersion $0204	; check for minimum version
	pha
	ph2	sem
	ph4	#errno
	ldx	#$1003
	jsl	udispatch
	pla
	sta	retval
	ret	2:retval
	END

* void  *signal(int sig, void (*func)()) inline(0x1603, udispatch);
signal 	START	libc_sys__
retval	equ	1
	sub	(4:func,2:sig),4
	assertVersion $0204	; check for minimum version
	pha
	pha
	ph2	sig
	ph4	func
	ph4	#errno
	ldx	#$1603
	jsl	udispatch
	pla
	sta	retval	
	pla
	sta	retval+2
	ret	4:retval
	END

* int   wait(union wait *status) inline(0x1703, udispatch);
wait	START	libc_sys__
retval	equ	1
	sub	(4:status),2
	assertVersion $0204	; check for minimum version
	pha
	ph4	status
	ph4	#errno
	ldx	#$1703
	jsl	udispatch
	pla
	sta	retval
	ret	2:retval
	END

*	int tcnewpgrp(int fdtty) inline(0x1803, udispatch);
tcnewpgrp	START libc_sys__
retval	equ	1
	sub	(2:fdtty),2
	assertVersion $0204	; check for minimum version
	pha
	ph2	fdtty
	ph4	#errno
	ldx	#$1803
	jsl	udispatch
	pla
	sta	retval
	ret	2:retval
	END

*	int   settpgrp(int fdtty) inline(0x1903, udispatch);
settpgrp	START	libc_sys__
retval	equ	1
	sub	(2:fdtty),2
	assertVersion $0204	; check for minimum version
	pha
	ph2	fdtty
	ph4	#errno
	ldx	#$1903
	jsl	udispatch
	pla
	sta	retval
	ret	2:retval
	END

* int   tctpgrp(int fdtty, int pid) inline(0x1A03, udispatch);
tctpgrp	START	libc_sys__
retval	equ	1
	sub	(2:pid,2:fdtty),2
	assertVersion $0204	; check for minimum version
	pha
	ph2	fdtty
	ph2	pid
	ph4	#errno
	ldx	#$1A03
	jsl	udispatch
	pla
	sta	retval
	ret	2:retval
	END

* longword sigsetmask(longword mask) inline(0x1B03, udispatch);
sigsetmask	START libc_sys__
retval	equ	1
	sub	(4:mask),4
	assertVersion $0204	; check for minimum version
	pha
	pha
	ph4	mask
	ph4	#errno
	ldx	#$1B03
	jsl	udispatch
	pla
	sta	retval	
	pla
	sta	retval+2
	ret	4:retval
	END

* longword sigblock(longword mask) inline(0x1C03, udispatch);
sigblock	START	libc_sys__
retval	equ	1
	sub	(4:mask),4
	assertVersion $0204	; check for minimum version
	pha
	pha
	ph4	mask
	ph4	#errno
	ldx	#$1C03
	jsl	udispatch
	pla
	sta	retval	
	pla
	sta	retval+2
	ret	4:retval
	END

* int _execve(char *filename,char *cmdline) inline(0x1D03,udispatch);
_execve	START	libc_sys__
retval	equ	1
	sub	(4:cmdline,4:filename),2
	assertVersion $0204	; check for minimum version
	pha
	ph4	filename
	ph4	cmdline
	ph4	#errno
	ldx	#$1D03
	jsl	udispatch
	pla
	sta	retval
	ret	2:retval
	END

* longword alarm(longword seconds) inline(0x1E03,udispatch);
alarm	START	libc_sys__
retval	equ	1
	sub	(4:seconds),4
	assertVersion $0204	; check for minimum version
	pha
	pha
	ph4	seconds
	ph4	#errno
	ldx	#$1E03
	jsl	udispatch
	pla
	sta	retval	
	pla
	sta	retval+2
	ret	4:retval
	END

* longword alarm10(longword seconds) inline(0x4203,udispatch);
alarm10	START	libc_sys__
retval	equ	1
	sub	(4:seconds),4
	assertVersion $0204	; check for minimum version
	pha
	pha
	ph4	seconds
	ph4	#errno
	ldx	#$4203
	jsl	udispatch
	pla
	sta	retval	
	pla
	sta	retval+2
	ret	4:retval
	END

* int   setdebug(int code) inline(0x1F03,udispatch);
setdebug	START libc_sys__
retval	equ	1
	sub	(2:code),2
	assertVersion $0204	; check for minimum version
	pha
	ph2	code
	anop		; doesn't take errno
	ldx	#$1F03
	jsl	udispatch
	pla
	sta	retval
	ret	2:retval
	END

* void *setsystemvector(void *vect) inline(0x2003,udispatch);
setsystemvector START libc_sys__
retval	equ	1
	sub	(4:vect),4
	assertVersion $0204	; check for minimum version
	pha
	pha
	ph4	vect
	anop		; doesn't take errno
	ldx	#$2003
	jsl	udispatch
	pla
	sta	retval	
	pla
	sta	retval+2
	ret	4:retval
	END

* int   sigpause(longword mask) inline(0x2103,udispatch);
sigpause	START	libc_sys__
retval	equ	1
	sub	(4:mask),2
	assertVersion $0204	; check for minimum version
	pha
	ph4	mask
	ph4	#errno
	ldx	#$2103
	jsl	udispatch
	pla
	sta	retval
	ret	2:retval
	END

* kvmt *kvm_open(void) inline(0x1103, udispatch);
kvm_open	START	libc_sys__
retval	equ	1
	sub	(0:foo),4
	assertVersion $0204	; check for minimum version
	pha
	pha
	ph4	#errno
	ldx	#$1103
	jsl	udispatch
	pla
	sta	retval
	pla
	sta	retval+2
	ret	4:retval
	END
* int kvm_close(kvmt *k) inline(0x1203, udispatch);
kvm_close	START libc_sys__
retval	equ	1
	sub	(4:k),2
	assertVersion $0204	; check for minimum version
	pha
	ph4	k
	ph4	#errno
	ldx	#$1203
	jsl	udispatch
	pla
	sta	retval
	ret	2:retval
	END

* struct pentry *kvm_getproc(kvmt *kd, int pid) inline(0x1303, udispatch);
kvm_getproc	START libc_sys__
kvmgetproc	ENTRY
retval	equ	1
	sub	(2:pid,4:kd),4
	assertVersion $0204	; check for minimum version
	pha
	pha
	ph4	kd
	ph2	pid
	ph4	#errno
	ldx	#$1303
	jsl	udispatch
	pla
	sta	retval
	pla
	sta	retval+2
	ret	4:retval
	END

* struct pentry *kvm_nextproc(kvmt *kd) inline(0x1403, udispatch);
kvm_nextproc	START libc_sys__
kvmnextproc	ENTRY
retval	equ	1
	sub	(4:kd),4
	assertVersion $0204	; check for minimum version
	pha
	pha
	ph4	kd
	ph4	#errno
	ldx	#$1403
	jsl	udispatch
	pla
	sta	retval
	pla
	sta	retval+2
	ret	4:retval
	END

* int kvm_setproc(kvmt *kd) inline(0x1503, udispatch);
kvm_setproc	START libc_sys__
kvmsetproc	ENTRY
retval	equ	1
	sub	(4:kd),4
	assertVersion $0204	; check for minimum version
	pha
	ph4	kd
	ph4	#errno
	ldx	#$1503
	jsl	udispatch
	pla
	sta	retval
	ret	2:retval
	END

dup	START	libc_sys__
retval	equ	1
	sub	(2:fd),2
	assertVersion $0204	; check for minimum version
	pha
	ph2	fd
	ph4	#errno
	ldx	#$2203
	jsl	udispatch
	pla
	sta	retval
	ret	2:retval
	END

dup2	START	libc_sys__
retval	equ	1
	sub	(2:fd2,2:fd1),2
	assertVersion $0204	; check for minimum version
	pha
	ph2	fd1
	ph2	fd2
	ph4	#errno
	ldx	#$2303
	jsl	udispatch
	pla
	sta	retval
	ret	2:retval
	END

pipe	START	libc_sys__
retval	equ	1
	sub	(4:intptr),2
	assertVersion $0204	; check for minimum version
	pha
	ph4	intptr
	ph4	#errno
	ldx	#$2403
	jsl	udispatch
	pla
	sta	retval
	ret	2:retval
	END

* int   _getpgrp(int pid) inline(0x2503, udispatch);
_getpgrp	START	libc_sys__
retval	equ	1
	sub	(2:pid),2
	assertVersion $0204	; check for minimum version
	pha
	ph2	pid
	ph4	#errno
	ldx	#$2503
	jsl	udispatch
	pla
	sta	retval
	ret	2:retval
	END

* int   setpgrp(int pid, int pgrp) inline(0x2503, udispatch);
setpgid	START	libc_sys__
setpgrp	ENTRY
retval	equ	1
	sub	(2:pgrp,2:pid),2
	assertVersion $0204	; check for minimum version
	pha
	ph2	pid
	ph2	pgrp
	ph4	#errno
	ldx	#$3403
	jsl	udispatch
	pla
	sta	retval
	ret	2:retval
	END

* int ioctl (int fd, unsigned long request, void *argp);
ioctl	START	libc_sys__
retval	equ	1
	sub	(4:argp,4:request,2:d),2
	assertVersion $0204	; check for minimum version
	pha
	ph2	d
	ph4	request
	ph4	argp
	ph4	#errno
	ldx	#$2603
	jsl	udispatch
	pla
	sta	retval
	ret	2:retval
	END

* int stat (const char *filename, struct stat *s_buf);
stat	START	libc_sys__
retval	equ	1
	sub	(4:buf,4:name),2
	assertVersion $0204	; check for minimum version
	pha
	ph4	name
	ph4	buf
	ph4	#errno	
	ldx	#$2703
	jsl	udispatch
	pla
	sta	retval
	ret	2:retval
	END

* int fstat (int fd, struct stat *s_buf);
fstat	START	libc_sys__
retval	equ	1
	sub	(4:buf,2:fd),2
	assertVersion $0204	; check for minimum version
	pha
	ph2	fd
	ph4	buf
	ph4	#errno	
	ldx	#$2803
	jsl	udispatch
	pla
	sta	retval
	ret	2:retval
	END

* int lstat (const char *filename, struct stat *s_buf);
lstat	START	libc_sys__
retval	equ	1
	sub	(4:buf,4:name),2
	assertVersion $0204	; check for minimum version
	pha
	ph4	name
	ph4	buf
	ph4	#errno	
	ldx	#$2903
	jsl	udispatch
	pla
	sta	retval
	ret	2:retval
	END

procsend	START	libc_sys__
retval	equ	1
	sub	(4:msg,2:pid),2
	assertVersion $0204	; check for minimum version
	pha
	ph2	pid
	ph4	msg
	ph4	#errno
	ldx	#$3003
	jsl	udispatch
	pla
	sta	retval
	ret	2:retval
	END

procreceive	START libc_sys__
retval	equ	1
	sub	(0:foo),4
	assertVersion $0204	; check for minimum version
	pha
	pha
	ph4	#errno
	ldx	#$3103
	jsl	udispatch
	pla
	sta	retval
	pla
	sta	retval+2
	ret	4:retval
	END

procrecvclr	START libc_sys__
retval	equ	1
	sub	(0:foo),4
	assertVersion $0204	; check for minimum version
	pha
	pha
	ph4	#errno
	ldx	#$3203
	jsl	udispatch
	pla
	sta	retval
	pla
	sta	retval+2
	ret	4:retval
	END
	
procrecvtim	START libc_sys__
retval	equ	1
	sub	(2:timeout),4
	assertVersion $0204	; check for minimum version
	pha
	pha
	ph2	timeout
	ph4	#errno
	ldx	#$3303
	jsl	udispatch
	pla
	sta	retval
	pla
	sta	retval+2
	ret	4:retval
	END

times	START	libc_sys__
retval	equ	1
	sub	(4:buffer),2
	assertVersion $0204	; check for minimum version
	pha
	ph4	buffer
	ph4	#errno
	ldx	#$3503
	jsl	udispatch
	pla
	sta	retval
	ret	2:retval
	END

* int pcreate(int count);
pcreate	START	libc_sys__
	subroutine (2:count),0
	assertVersion $0204	; check for minimum version
	pha
	pei	(count)
	ph4	#errno
	ldx	#$3603
	jsl	udispatch
	ply
	return2
	END

* int psend(int portid, long int msg);
psend	START	libc_sys__
	subroutine (4:msg,2:portid),0
	assertVersion $0204	; check for minimum version
	pha
	pei	(portid)
	pei	(msg+2)
	pei	(msg)
	ph4	#errno
	ldx	#$3703
	jsl	udispatch
	ply
	return2
	END

* long int preceive(int portid);
preceive	START	libc_sys__
	subroutine (2:portid),0
	assertVersion $0204	; check for minimum version
	pha
	pha
	pei	(portid)
	ph4	#errno
	ldx	#$3803
	jsl	udispatch
	ply
	plx
	return2
	END

* int pdelete(int portid, int (*dispose)());
pdelete	START	libc_sys__
	subroutine (4:dispose,2:portid),0
	assertVersion $0204	; check for minimum version
	pha
	pei	(portid)
	pei	(dispose+2)
	pei	(dispose)
	ph4	#errno
	ldx	#$3903
	jsl	udispatch
	ply
	return2
	END

*	int preset(int portid, int (*dispose)());
preset	START	libc_sys__
	subroutine (4:dispose,2:portid),0
	assertVersion $0204	; check for minimum version
	pha
	pei	(portid)
	pei	(dispose+2)
	pei	(dispose)
	ph4	#errno
	ldx	#$3A03
	jsl	udispatch
	ply
	return2
	END

* int pbind(int portid, char *name);
pbind	START	libc_sys__
	subroutine (4:name,2:portid),0
	assertVersion $0204	; check for minimum version
	pha
	pei	(portid)
	pei	(name+2)
	pei	(name)
	ph4	#errno
	ldx	#$3B03
	jsl	udispatch
	ply
	return2
	END

* int pgetport(char *name);
pgetport	START	libc_sys__
	subroutine (4:name),0
	assertVersion $0204	; check for minimum version
	pha
	pei	(name+2)
	pei	(name)
	ph4	#errno
	ldx	#$3C03
	jsl	udispatch
	ply
	return2
	END

* int pgetcount(int portnum);
pgetcount	START libc_sys__
	subroutine (2:portnum),0
	assertVersion $0204	; check for minimum version
	pha
	pei	(portnum)
	ph4	#errno
	ldx	#$3D03
	jsl	udispatch
	ply
	return2
	END
	   
* int pgetport(char *name);
scount	START	libc_sys__
	subroutine (2:sem),0
	assertVersion $0204	; check for minimum version
	pha
	pei	(sem)
	ph4	#errno
	ldx	#$3E03
	jsl	udispatch
	ply
	return2
	END

* void SetGNOQuitRec(word pCount,GSString255Ptr pathname, word flags)
SetGNOQuitRec START libc_sys__
retval	equ	1
	sub	(2:flags,4:pathname,2:pCount),0
	assertVersion $0204	; check for minimum version
	pha
	ph2	pCount
	ph4	pathname
	ph2	flags
	ph4	#errno	; tell the library routine where it is
	ldx	#$4103
	jsl	udispatch
	pla
	ret
	END

* int select (int width, fd_set *readfds, fd_set *writefds,
*             fd_set *exceptfds, struct timeval *timeout);
select	START	libc_sys__
res	equ	1
	sub	(4:toptr,4:exc,4:wr,4:rd,2:nfd),2

	assertVersion $0206	; check for minimum version

	pha
	pei	(nfd)
	pei	(rd+2)
	pei	(rd)
	pei	(wr+2)
	pei	(wr)
	pei	(exc+2)
	pei	(exc)
	pei	(toptr+2)
	pei	(toptr)
	ph4	#errno
	ldx	#$4303
	jsl	udispatch
	pla
	sta	res
	ret	2:res
	END

InstallNetDriver START libc_sys__
res	equ	1
	sub	(2:domain,4:netcore),2
	assertVersion $0206	; check for minimum version
	pha
	pei	(netcore+2)
	pei	(netcore)
	pei	(domain)
	ph4	#errno
	ldx	#$4403
	jsl	udispatch
	pla
	sta	res
	ret	2:res
	END

socket	START	libc_sys__
res	equ	1
	sub	(2:protocol,2:type,2:domain),2
	assertVersion $0206	; check for minimum version
	pha
	pei	(domain)
	pei	(type)
	pei	(protocol)
	ph4	#errno
	ldx	#$4503
	jsl	udispatch
	pla
	sta	res
	ret	2:res
	END

bind	START	libc_sys__
res	equ	1
	sub	(2:addrlen,4:myaddr,2:fd),2
	assertVersion $0206	; check for minimum version
	pha
	pei	(fd)
	pei	(myaddr+2)
	pei	(myaddr)
	pei	(addrlen)
	ph4	#errno
	ldx	#$4603
	jsl	udispatch
	pla
	sta	res
	ret	2:res
	END

connect	START	libc_sys__
res	equ	1
	sub	(2:addrlen,4:servaddr,2:fd),2
	assertVersion $0206	; check for minimum version
	pha
	pei	(fd)
	pei	(servaddr+2)
	pei	(servaddr)
	pei	(addrlen)
	ph4	#errno
	ldx	#$4703
	jsl	udispatch
	pla
	sta	res
	ret	2:res
	END

listen	START	libc_sys__
res	equ	1
	sub	(2:backlog,2:fd),2
	assertVersion $0206	; check for minimum version
	pha
	pei	(fd)
	pei	(backlog)
	ph4	#errno
	ldx	#$4803
	jsl	udispatch
	pla
	sta	res
	ret	2:res
	END

accept	START	libc_sys__
res	equ	1
	sub	(4:addrlen,4:remaddr,2:fd),2
	assertVersion $0206	; check for minimum version
	pha
	pei	(fd)
	pei	(remaddr+2)
	pei	(remaddr)
	pei	(addrlen+2)
	pei	(addrlen)
	ph4	#errno
	ldx	#$4903
	jsl	udispatch
	pla
	sta	res
	ret	2:res
	END

recvfrom	START	libc_sys__
res	equ	1
	sub	(4:addrlen,4:remaddr,2:flags,4:len,4:buf,2:fd),2

	assertVersion $0206	; check for minimum version
	pha
	pei	(fd)
	pei	(buf+2)
	pei	(buf)
	pei	(len+2)
	pei	(len)
	pei	(flags)
	pei	(remaddr+2)
	pei	(remaddr)
	pei	(addrlen+2)
	pei	(addrlen)
	ph4	#errno
	ldx	#$4A03
	jsl	udispatch
	pla
	sta	res
	ret	2:res
	END

sendto	START	libc_sys__
res	equ	1
	sub	(2:addrlen,4:remaddr,2:flags,4:len,4:buf,2:fd),2

	assertVersion $0206	; check for minimum version
	pha
	pei	(fd)
	pei	(buf+2)
	pei	(buf)
	pei	(len+2)
	pei	(len)
	pei	(flags)
	pei	(remaddr+2)
	pei	(remaddr)
	pei	(addrlen)
	ph4	#errno
	ldx	#$4B03
	jsl	udispatch
	pla
	sta	res
	ret	2:res
	END

recv	START	libc_sys__
res	equ	1
	sub	(2:flags,4:len,4:buf,2:fd),2

	assertVersion $0206	; check for minimum version
	pha
	pei	(fd)
	pei	(buf+2)
	pei	(buf)
	pei	(len+2)
	pei	(len)
	pei	(flags)
	ph4	#errno
	ldx	#$4C03
	jsl	udispatch
	pla
	sta	res
	ret	2:res
	END

send	START	libc_sys__
res	equ	1
	sub	(2:flags,4:len,4:buf,2:fd),2

	assertVersion $0206	; check for minimum version
	pha
	pei	(fd)
	pei	(buf+2)
	pei	(buf)
	pei	(len+2)
	pei	(len)
	pei	(flags)
	ph4	#errno
	ldx	#$4D03
	jsl	udispatch
	pla
	sta	res
	ret	2:res
	END

getpeername	START libc_sys__
res	equ	1
	sub	(4:addrlen,4:addr,2:s),2
	assertVersion $0206	; check for minimum version
	pha
	pei	(s)
	pei	(addr+2)
	pei	(addr)
	pei	(addrlen+2)
	pei	(addrlen)
	ph4	#errno
	ldx	#$4E03
	jsl	udispatch
	pla
	sta	res
	ret	2:res
	END

getsockname	START libc_sys__
res	equ	1
	sub	(4:addrlen,4:addr,2:s),2
	assertVersion $0206	; check for minimum version
	pha
	pei	(s)
	pei	(addr+2)
	pei	(addr)
	pei	(addrlen+2)
	pei	(addrlen)
	ph4	#errno
	ldx	#$4F03
	jsl	udispatch
	pla
	sta	res
	ret	2:res
	END

getsockopt	START libc_sys__
res	equ	1
	sub	(4:optlen,4:optval,2:optname,2:level,2:s),2
	assertVersion $0206	; check for minimum version
	pha
	pei	(s)
	pei	(level)
	pei	(optname)
	pei	(optval+2)
	pei	(optval)
	pei	(optlen+2)
	pei	(optlen)
	ph4	#errno
	ldx	#$5003
	jsl	udispatch
	pla
	sta	res
	ret	2:res
	END

setsockopt	START libc_sys__
res	equ	1
	sub	(2:optlen,4:optval,2:optname,2:level,2:s),2
	assertVersion $0206	; check for minimum version
	pha
	pei	(s)
	pei	(level)
	pei	(optname)
	pei	(optval+2)
	pei	(optval)
	pei	(optlen)
	ph4	#errno
	ldx	#$5103
	jsl	udispatch
	pla
	sta	res
	ret	2:res
	END

shutdown	START	libc_sys__
res	equ	1
	sub	(2:how,2:s),2
	assertVersion $0206	; check for minimum version
	pha
	pei	(s)
	pei	(how)
	ph4	#errno
	ldx	#$5203
	jsl	udispatch
	pla
	sta	res
	ret	2:res
	END
