*************************************************************************
*
* GNO/ME system call macros for Orca/M.
* Copyright (C) 1991-1998 by Procyon, Inc.
*
* $Id: m16.gno,v 1.1 1998/03/08 17:18:11 gdr-ftp Exp $
*
*************************************************************************

;
; assembly mutual exclusion macros
;
	MACRO
&lab	lock	&a1
&lab	lda	#1
	tsb	&a1
	beq	*+6
	cop	$7F
	bra	*-7	
	MEND

	MACRO
&lab	unlock	&a1
&lab	stz	&a1
	MEND

	MACRO
&lab	key
&lab	dc	i2'0'
	MEND
;
; GNO system calls
;

	MACRO
&lab	kernVersion &a1
&lab	pha
	ldx	#$403
	jsl	$E10008
	pl2	&a1
	MEND

	MACRO
&lab	kernStatus &a1
&lab	pha
	ldx	#$603
	jsl	$E10008
	pl2	&a1
	MEND

	MACRO
&lab	kill	&a1
&lab	ph2	&a1(2)
	ph2	&a1(1)
	case	on
	jsl	kill
	case	off
	MEND

	MACRO
&lab	fork	&a1
&lab	ph4	&a1
	case	on
	jsl	fork
	case	off
	MEND

	MACRO
&lab	exec	&a1
&lab	ph4	&a1(2)
	ph4	&a1(1)
	case	on
	jsl	exec
	case	off
	MEND

	MACRO
&lab	swait	&a1
&lab	ph2	&a1
	case	on
	jsl	swait
	case	off
	MEND

	MACRO
&lab	ssignal &a1
&lab	ph2	&a1
	case	on
	jsl	ssignal
	case	off
	MEND

	MACRO
&lab	screate &a1
&lab	ph2	&a1
	case	on
	jsl	screate
	case	off
	MEND

	MACRO
&lab	sdelete &a1
&lab	ph2	&a1
	case	on
	jsl	sdelete
	case	off
	MEND

	MACRO
&lab	signal &a1
&lab	ph4	&a1(2)
	ph2	&a1(1)
	case	on
	jsl	signal
	case	off
	MEND

	MACRO
&lab	wait &a1
&lab	ph4	&a1
	case	on
	jsl	wait
	case	off
	MEND

	MACRO
&lab	tcnewpgrp &a1
&lab	ph2	&a1
	case	on
	jsl	tcnewpgrp
	case	off
	MEND

	MACRO
&lab	settpgrp &a1
&lab	ph2	&a1
	case	on
	jsl	settpgrp
	case	off
	MEND

	MACRO
&lab	tctpgrp &a1
&lab	ph2	&a1(2)
	ph2	&a1(1)
	case	on
	jsl	tctpgrp
	case	off
	MEND

	MACRO
&lab	sigsetmask &a1
&lab	ph4	&a1
	case	on
	jsl	sigsetmask
	case	off
	MEND

	MACRO
&lab	sigblock &a1
&lab	ph4	&a1
	case	on
	jsl	sigblock
	case	off
	MEND

	MACRO
&lab	execve &a1
&lab	ph4	&a1(2)
	ph4	&a1(1)
	case	on
	jsl	execve
	case	off
	MEND

	MACRO
&lab	alarm &a1
&lab	ph4	&a1
	case	on
	jsl	alarm
	case	off
	MEND

	MACRO
&lab	setdebug &a1
&lab	ph2	&a1
	case	on
	jsl	setdebug
	case	off
	MEND

	MACRO
&lab	setsystemvector &a1
&lab	ph4	&a1
	case	on
	jsl	setsystemvector
	case	off
	MEND

	MACRO           
&lab	sigpause &a1
&lab	ph4	&a1
	case	on
	jsl	sigpause
	case	off
	MEND

	MACRO
&lab	getpid
&lab	case	on
	jsl	getpid
	case	off
	MEND

	MACRO
&lab	kvm_open
&lab	case	on
	jsl	kvm_open
	case	off
	MEND

	MACRO           
&lab	kvm_close &a1
&lab	ph4	&a1
	case	on
	jsl	kvm_close
	case	off
	MEND

	MACRO           
&lab	kvmgetproc &a1
&lab	ph2	&a1(2)
	ph4	&a1(1)	
	case	on
	jsl	kvmgetproc
	case	off
	MEND

	MACRO           
&lab	kvmnextproc &a1
&lab	ph4	&a1
	case	on
	jsl	kvmnextproc
	case	off
	MEND

	MACRO           
&lab	kvmsetproc &a1
&lab	ph4	&a1
	case	on
	jsl	kvmsetproc
	case	off
	MEND

	MACRO           
&lab	getpgrp &a1
&lab	ph2	&a1
	case	on
	jsl	getpgrp
	case	off
	MEND

	MACRO           
&lab	pipe 	&a1
&lab	ph4	&a1
	case	on
	jsl	pipe
	case	off
	MEND

	MACRO           
&lab	dup2 	&a1
&lab	ph2	&a1(2)
	ph2	&a1(1)	
	case	on
	jsl	dup2
	case	off
	MEND

	MACRO           
&lab	ioctl 	&a1
&lab	ph4	&a1(3)
	ph4	&a1(2)
	ph2	&a1(1)	
	case	on
	jsl	ioctl
	case	off
	MEND

	MACRO           
&lab	tgetent &a1
&lab	ph4	&a1(2)
	ph4	&a1(1)	
	case	on
	jsl	tgetent
	case	off
	MEND

	MACRO           
&lab	tgetstr &a1
&lab	ph4	&a1(2)
	ph4	&a1(1)	
	case	on
	jsl	tgetstr
	case	off
	MEND

	MACRO           
&lab	tgetnum &a1
&lab	ph4	&a1
	case	on
	jsl	tgetnum
	case	off
	MEND

	MACRO           
&lab	tgetflag &a1
&lab	ph4	&a1
	case	on
	jsl	tgetflag
	case	off
	MEND

	MACRO           
&lab	tgoto 	&a1
&lab	ph2	&a1(3)
	ph2	&a1(2)
	ph4	&a1(1)	
	case	on
	jsl	tgoto
	case	off
	MEND

	MACRO           
&lab	tputs 	&a1
&lab	ph4	&a1(3)
	ph2	&a1(2)
	ph4	&a1(1)	
	case	on
	jsl	tputs
	case	off
	MEND

        	MACRO             
&lab	getuid
&lab	case	on
	jsl	getuid
	case	off
        	MEND
