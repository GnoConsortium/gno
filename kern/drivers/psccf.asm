*	$Id: psccf.asm,v 1.1 1998/02/02 08:18:03 taubert Exp $
* Note that there's not much in this file- just a copy for macros,
* a copy for defining the port memory locations & values, and
* a copy for the generic low level port driver code itself

	mcopy sccf.mac
	copy	equates
	copy	../gno/inc/tty.inc
	copy	pr.equates
	copy	sccf.asm
