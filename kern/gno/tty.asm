*	$Id: tty.asm,v 1.1 1998/02/02 08:19:54 taubert Exp $
*
*      tty.asm
*      New generic standard UNIX Line Discipline
*
	mcopy	m/tty.mac
	case	on
	copy	inc/tty.inc
	copy	inc/gsos.inc
	copy	inc/kern.inc

udispatch	gequ   $E10008
IncBusyFlag	gequ   $E10064
DecBusyFlag	gequ   $E10068

DebugNames	gequ   1

;#include <gno/gno.h>
;#include <types.h>
;#include <sgtty.h>
;#include <string.h>
;#include <texttool.h>
;#include <ctype.h>
;#include <signal.h>

;#define MIN(x,y) (x > y ? y : x)
;#define CTRL(x) (x-'@')
;#define DEV 0

* device control subsystem (ioctl & friends)

SIGTSTP	gequ	18

; Control character definitions
BS	gequ	8	; ^H
LF	gequ	10	; ^J
NL	gequ	10	; ^J
CR	gequ	13	; ^M


;   NOTE: Whenever we're not in RAW mode and we get a signal character
;   (^Z ^C ^\ ^Y) we must clear all pending input/output on the terminal
;   with the equivalent of a TIOCFLUSH.  In addition to clearing the
;   actual interrupt queues, we need to set editInd = editBegin = 0


* dispatches to a vectored TTY function handling routine.  Various
* data is implicit : the pointer to the tty structure (location 0 on
* the DP) and thus the device number).

ttyDispatch	START KERN2
ttyDispatch	name
ttyPtr	equ	0

	short	m
	iny
	iny
	lda	[ttyPtr],y
	pha
	long	m
	dey
	dey
	lda	[ttyPtr],y
	dec	a
	pha
	rtl
	END	

ttwrite	START KERN2
ttwrite	name
ttyPtr	equ	0
c	equ	4
xfer	equ	6
retval	equ	8

	subroutine (2:length,4:buf,2:devNum),10

	lda	devNum
	jsr	fetchDevPtr	
	sta	ttyPtr
	stx	ttyPtr+2

	lda	length
	sta	xfer	
	stz	retval
	ldx	devNum
	ldy	#mutex
	jsl	ttyDispatch

* if we're clobbering an input line, set the retype flag
	ldy	#sg_flags	See what mode we're in. If not in
	lda	[ttyPtr],y	cooked mode, then we don't need
	bit	#CBREAK+RAW	to do this check (no editing)
	bne	x1
	ldy	#st_flags
	lda	[ttyPtr],y
	beq	x1	
	ldy	#local
	lda	[ttyPtr],y
	ora	#LPENDIN
	sta	[ttyPtr],y	

x1	anop		we were in raw or cbreak mode

	lda	length
	jeq	w1
	dea
	sta	length
	lda	[buf]
	and	#$00FF
	sta	c
	inc	buf
	bne	x2
	inc	buf+2
x2	anop
	ldy	#sg_flags	if RAW mode do no output processing
	lda	[ttyPtr],y
	bit	#RAW
	bne	x4

	ldy	#sg_flags	if (tty->sg_flags & CRMOD) {
	lda	[ttyPtr],y
	bit	#CRMOD
	beq	x3
	lda	c	  if (c == LF) {
	cmp	#CR
	bne	x3
	
	pea	CR	       (*tty->out_enq)(DEV,CR);
	ldx	devNum
	ldy	#out_enq
	jsl	ttyDispatch
	pea	LF	       (*tty->out_enq)(DEV,LF);
	ldx	devNum
	ldy	#out_enq
	jsl	ttyDispatch
	bra	x1	       continue
x3	anop
	ldy	#local	if (tty->local & LTILDE)
	lda	[ttyPtr],y
	bit	#LTILDE
	beq	x4
	lda	c
	cmp	#'~'
	bne	x4
	lda	#'`'
	sta	c
x4	anop
	pei	(c)
	ldx	devNum
	ldy	#out_enq
	jsl	ttyDispatch
	jmp	x1
w1	ldx	devNum
	ldy	#demutex
	jsl	ttyDispatch

	pea	0	; 0 means 'write occurred'
	ldx	devNum
	ldy	#t_signalIO
	jsl	ttyDispatch

	return 4:xfer
	END

;char editbuf[256];

;void echoCtlChar(char c)
;{
;    if (tty->local & LCTLECH) {
;        if (c < 32) {
;            if ((c != CR) && (c != LF)) {
;                (*tty->out_enq)(DEV,'^');
;                (*tty->out_enq)(DEV,c+64);
;                return;
;            }
;x2:     } else if (c == 0x7f) {
;            (*tty->out_enq)(DEV,'^');
;            (*tty->out_enq)(DEV,'?');
;            return;
;        }
;    }
;x1:
;    (*tty->out_enq)(DEV,c);
;x3:
;}
* Remember, DP is set to ttwrite/ttread local vars

echoCtlChar	START KERN2
echoCtlChar	name  
ttyPtr	equ	0
tmpC	equ	6
loc_dev	equ	30

	sta	tmpC
	ldy	#local
	lda	[ttyPtr],y
	bit	#LCTLECH	
	jeq	x1
	lda	tmpC
	cmp	#' '
	bcs	x2
	cmp	#CR
	beq	x3
	cmp	#LF
	beq	x1
	pea	'^'
	ldx	loc_dev
	ldy	#out_enq
	jsl	ttyDispatch
	lda	tmpC
	clc
	adc	#'@'
	pha
	ldx	loc_dev
	ldy	#out_enq
	jsl	ttyDispatch
	pea	0	; 0 means 'write occurred'
	ldx	loc_dev
	ldy	#t_signalIO
	jsl	ttyDispatch
	rts
x1	pei	(tmpC)
	ldx	loc_dev
	ldy	#out_enq
	jsl	ttyDispatch
	pea	0	; 0 means 'write occurred'
	ldx	loc_dev
	ldy	#t_signalIO
	jsl	ttyDispatch
	rts
x3	ldy	#sg_flags	; it's a CR.  Check to see if the
	lda	[ttyPtr],y
	bit	#CRMOD	; CRMOD flag is set. If it is, we
	beq	x1	; need to do a CR->CRLF translation
	pea	CR
	ldx	loc_dev
	ldy	#out_enq
	jsl	ttyDispatch
	pea	LF
	ldx	loc_dev
	ldy	#out_enq
	jsl	ttyDispatch
	pea	0	; 0 means 'write occurred'
	ldx	loc_dev
	ldy	#t_signalIO
	jsl	ttyDispatch
	rts
x2	cmp	#$007F	; DELETE?
	bne	x1
	pea	'^'
	ldx	loc_dev
	ldy	#out_enq
	jsl	ttyDispatch
	pea	'?'
	ldx	loc_dev
	ldy	#out_enq
	jsl	ttyDispatch
	pea	0	; 0 means 'write occurred'
	ldx	loc_dev
	ldy	#t_signalIO
	jsl	ttyDispatch
	rts
	END

;echoCtlErase(char c)
;{
;    if (tty->local & LCRTBS)
;        (*tty->out_enq)(DEV,BS);
;x1:
;   else if (tty->local & LCRTERA) {
;        (*tty->out_enq)(DEV,BS);
;        (*tty->out_enq)(DEV,' ');
;        (*tty->out_enq)(DEV,BS);
;    }
;x2:
;    else (*tty->out_enq)(DEV,c);
;}
echoCtlErase	START KERN2
echoCtlErase	name
ttyPtr	equ	0
tmpC	equ	6
loc_dev	equ	30
	
	sta	tmpC
	ldy	#local
	lda	[ttyPtr],y
	bit	#LCRTBS
	beq	x1
	pea	BS
	ldx	loc_dev
	ldy	#out_enq
	jsl	ttyDispatch
	pea	0	; 0 means 'write occurred'
	ldx	loc_dev
	ldy	#t_signalIO
	jsl	ttyDispatch
	rts
x1	bit	#LCRTERA
	beq	x2	
	pea	BS
	ldx	loc_dev
	ldy	#out_enq
	jsl	ttyDispatch
	pea	' '
	ldx	loc_dev
	ldy	#out_enq
	jsl	ttyDispatch
	pea	BS
	ldx	loc_dev
	ldy	#out_enq
	jsl	ttyDispatch
	pea	0	; 0 means 'write occurred'
	ldx	loc_dev
	ldy	#t_signalIO
	jsl	ttyDispatch
	rts
x2	pei	(tmpC)
	ldx	loc_dev
	ldy	#out_enq
	jsl	ttyDispatch
	pea	0	; 0 means 'write occurred'
	ldx	loc_dev
	ldy	#t_signalIO
	jsl	ttyDispatch
	rts
	END

;void retype(int _index)
;{
;int x;
;    ttwrite("\r#",2);
;    for (x = 0; x < _index; x++) echoCtlChar(editbuf[x]);
;    tty->local &= ~LPENDIN; /* clear the flag */
;}

; retypes the current input buffer onto the screen (happens when
; user hits ^R or input is overwritten with asynchronous output)

retype	START KERN2
retype	name
ttyPtr	equ	0
tmpC	equ	6
_editBuf	equ	24

	pha
	lda	#CR
	jsr	echoCtlChar

	pea	0
	
x1	lda	1,s
	cmp	3,s
	bcs	x2
	tay
	lda	[_editBuf],y	; local copy of the pointer
	jsr	echoCtlChar
	lda	1,s
	inc	a
	sta	1,s
	bra	x1
x2	anop
	ldy	#local
	lda	[ttyPtr],y
; damned ORCA doesn't work right
;	and	#(LPENDIN.EOR.$FFFF)
	and	#%1101111111111111
	sta	[ttyPtr],y
	pla
	pla
	rts
	END

fetchDevPtr	START KERN2
fetchDevPtr	name
	asl	a
	asl	a
	tax
	lda	>DeviceBlock+2,x
	tay
	lda	>DeviceBlock,x
	tyx
	rts	
	END
	
ttread	START KERN2
ttread	name
ttyPtr	equ	0
c	equ	4
tmpC	equ	6
origLength	equ   8
tmp	equ	10
xfer	equ	12
retval	equ	14
inQueue	equ	16
_flags	equ	18
_index	equ	20
_begin	equ	22
_editBuf	equ	24
_x	equ	28
_zero	equ	28
loc_dev	equ	30

	subroutine (2:length,4:buf,2:devNum),32
* zero byte transfer? return immediately
	lda	devNum
	sta	loc_dev

	stz	xfer
	stz	retval
	lda	devNum
	jsr	fetchDevPtr	
	sta	ttyPtr
	stx	ttyPtr+2
	lda	length
	jeq	x1

dobgcheck	anop
	ldx	devNum	; get the driver's number
	lda   #21
	jsl   BGCheck
	cmp   #0
	beq   isfg
	cmp   #1
	beq   dobgcheck          ; we were suspended, try again

	lda   #$0027             ; the signal was blocked, so return
	sta	retval	; an I/O error
	jmp	doReturn

isfg	ldy	#editBuf
	lda	[ttyPtr],y
	sta	_editBuf
	ldy	#editBuf+2
	lda	[ttyPtr],y
	sta	_editBuf+2

	lda	#0
	ldy	#st_flags
	sta	[ttyPtr],y
	
	ldy	#sg_flags
	lda	[ttyPtr],y
	sta	_flags
	ldy	#editInd
	lda	[ttyPtr],y
	sta	_index
	ldy	#editBegin
	lda	[ttyPtr],y
	sta	_begin	
	
;   (*tty->mutex)(DEV);
	ldx	devNum
	ldy	#mutex
	jsl	ttyDispatch

;    if (_begin != 0) { /* if there is data still in the edit buffer */
	lda	_begin	
	beq	x2

*  number of unread bytes in the edit buffer.  These should
*  be read out whether we're in RAW/CBREAK or COOKED mode.
;       int x;
;       x = _index - _begin;
	lda	_index	
	sec
	sbc	_begin
	sta	_x

;       /*  see how many we should transfer  */
;       tmp = MIN(x,length);

	cmp	length
	bcc	x3
	lda	length
x3	sta	tmp
	
;       /*  copy them  */
;       memcpy(buf,editbuf+_begin,(size_t)tmp);
	pea	0
	pha
	ldx	_editBuf+2
	lda	_editBuf	
	clc	
	adc	_begin
	bcc	x4
	inx
x4	phx
	pha
	pei	(buf+2)
	pei	(buf)
	jsl	memcpy

;       /*  if we got all the data the caller wanted then update our
;           counts and return */
;       if (tmp == length) {
	lda	tmp
	cmp	length
	bne	x5

;           _begin += tmp;
	clc
	adc	_begin
	sta	_begin

;           if (_begin == _index) _begin = 0;
	cmp	_index
	bne	x6
	stz	_begin
;x6         xfer = tmp;
x6	lda	tmp
	sta	xfer
;           goto doReturn;
	jmp	doReturn
;       }
;x5:
;       /*  Otherwise, we've returned what we had in the buffer, and
;           we return to the caller (i.e., always return at a NL) */
;       _begin = _index = 0;
x5	stz	_begin
	stz	_index
	lda	tmp
	sta	xfer
	jmp	doReturn
;       xfer = tmp;
;       goto doReturn;
;   }

x2	anop
;   _index = 0;
	stz	_index
;   origLength = length;
	lda	length
	sta	origLength
;   inQueue = (*tty->size_inq)(DEV);
	ldx	loc_dev
	ldy	#size_inq
	jsl	ttyDispatch
	sta	inQueue

;   /* The next statement is reponsible for choosing between EWOULDBLOCK
;      for the fcntl nonblocking I/O mode and sleeping anyway. */
;   if (!inQueue) inQueue = 1; /* no data? read at least one! */
	cmp	#0
	bne	y1
	lda	#1	
	sta	inQueue
	ldy	#_flags
	bit	#RAW+CBREAK
	beq	y1	
; after the read, check to see if we got more data than just one in
; to improve performance
	lda	#1	
	sta	length
	sta	origLength

;y1:    while (1) { /* pretty much an infinite loop, bub! */
y1	anop

;       if (tty->local & LPENDIN) retype(_index);
;       tty->editInd = _index;
	ldy	#local
	lda	[ttyPtr],y
	bit	#LPENDIN
	beq	y2
	lda	_index
	jsr	retype

;y2:    c = (*tty->in_deq)(DEV);
y2	anop
	ldy	#editInd
	lda	_index
	sta	[ttyPtr],y
	ldy	#editBegin
	lda	_begin
	sta	[ttyPtr],y

	ldx	devNum
	ldy	#in_deq
	jsl	ttyDispatch
;       /* if C is a value that indicates we were sleeping and interrupted,
;          then restart the read operation, return an error, or other
;          stuff, depending on fcntl settings */
	cmp	#$7E43
	bne	strip_mod
	lda	#$430e	; EINTR
	sta	retval
	jmp	doReturn
strip_mod	anop
	and	#$00FF	; strip out modifier flags
	sta	c

;       /* we skip _all_ processing in RAW mode */
;       if (_flags & RAW) {
_checkRaw	name
	lda	_flags
	bit	#RAW
	beq	y3
	
;           *buf++ = c; xfer++;
	short	m
	lda	c
	sta	[buf]
	long	m
	inc	buf
	bne	y4
	inc	buf+2
y4	inc	xfer
;           if ((--inQueue == 0) || (--length == 0)) break;
	dec	inQueue
	jeq	doReturn
	dec	length
	jeq	doReturn
;           continue;
	bra	y1

;       }
; recache this because it can be changed by an interrupt routine
;y3:    /* Check for dsuspc in CBREAK & COOKED modes */
;       if (c == tty->t_dsuspc) {
	
y3	anop
;       _index = tty->editInd; _begin = tty->editBegin;
	ldy	#editInd
	lda	[ttyPtr],y
	sta	_index
	ldy	#editBegin
	lda	[ttyPtr],y
	sta	_begin	

	ldy	#t_dsuspc
	lda	[ttyPtr],y
	and	#$00FF
	cmp	c
	bne	z1

;       int zero = 0;
	stz	_zero

;           if (_flags & ECHO) echoCtlChar(c);
	lda	_flags
	bit	#ECHO
	beq	z77
	lda	c
	jsr	echoCtlChar

;z77        if (!(tty->local & LNOFLSH)) {
z77	ldy	#local
	lda	[ttyPtr],y
	bit	#LNOFLSH
	bne	z3
	
;               _index = _begin = 0;
	lda	#0
	sta	_index	
	sta	_begin
	ldy	#editBegin
	sta	[ttyPtr],y
	ldy	#editInd
	sta	[ttyPtr],y
	ldy	#st_flags
	sta	[ttyPtr],y

;               ioctl(1,TIOCFLUSH,(ampersand)zero);
;           }
;z3         kill(getpid(),SIGTSTP);
z3	anop
	pha
	pha
	jsl	KERNgetpid	; this is #pragma toolparms 1
	pea	SIGTSTP
	ph4	#errno
	jsl	KERNkill	; this is #pragma toolparms 1
	pla
;                       continue; /* restart the read operation */
	jmp	x2	; reset & restart the read
;       }

;z1:    if (_flags & CRMOD)
z1	lda	_flags
	bit	#CRMOD
	beq	z2

;           if (c == LF) c = CR;
	lda	c
	cmp	#LF
	bne	z2
	lda	#CR
	sta	c
;       /* did we receive an ERASE character? */
;       if (!(_flags & CBREAK))
z2	lda	_flags
	bit	#CBREAK
	jne	w1

;           tty->st_flags = 1; /* we're in the middle of an input edit */
	lda	#1
	ldy	#st_flags
	sta	[ttyPtr],y

;           if (c == tty->sg_erase) {
	ldy	#sg_erase
	lda	[ttyPtr],y
	and	#$00FF
	cmp	c
	bne	w1

;               if (_index) {
	lda	_index
	beq	w2

;                   _index--;
	dea
	sta	_index
	
;                   c = editbuf[_index];
	tay
	lda	[_editBuf],y
	and	#$00FF
	sta	c
;                   if (tty->local & LPRTERA) {
	ldy	#local
	lda	[ttyPtr],y
	bit	#LPRTERA
	beq	z9

;                       (*tty->out_enq)(DEV,'\\');
;                       (*tty->out_enq)(DEV,c);
	pea	'\'
	ldx	loc_dev
	ldy	#out_enq	
	jsl	ttyDispatch
	pei	(c)
	ldx	loc_dev
	ldy	#out_enq	
	jsl	ttyDispatch
	pea	'/'
	ldx	loc_dev
	ldy	#out_enq
	jsl	ttyDispatch
	pea	0	; 0 means 'write occurred'
	ldx	loc_dev
	ldy	#t_signalIO
	jsl	ttyDispatch
	bra	w1
;z9:                } else {
;                       if ((tty->local & LCTLECH) && (iscntrl(c)))
z9	anop
	ldy	#local
	lda	[ttyPtr],y
	bit	#LCTLECH
	beq	z7
	lda	c
	cmp	#$7F
	beq	z8	
	cmp	#' '
	bcs	z7
;z8                         echoCtlErase(c);
z8	lda	c
	jsr	echoCtlErase
	
;z7                      echoCtlErase(c);
z7	lda	c
	jsr	echoCtlErase
;                   }
;                   continue;
;               }
	jmp	y1
;w2:             else { /* nothing to erase! */
;                   tty->st_flags = 0;

w2	lda	#0
	ldy	#st_flags
	sta	[ttyPtr],y
;                   continue; /* ignore the backspace */
	jmp	y1
;               }
;           }

;w1:    /* Echo input characters? */
;       if (_flags & ECHO)
w1	lda	_flags
	bit	#ECHO
	beq	v1
;           echoCtlChar(c);
	lda	c
	jsr	echoCtlChar

;v1     /* Check for and handle the Reprint character */
;       if ((c == tty->t_rprntc) && (_flags & ECHO)) {
v1	ldy	#t_rprntc
	lda	[ttyPtr],y
	and	#$00FF
	cmp	c
	bne	v2
	lda	_flags
	bit	#ECHO
	beq	v2
;           tty->local |= LPENDIN;
	ldy	#local
	lda	[ttyPtr],y
	ora	#LPENDIN
	sta	[ttyPtr],y
;           continue;
	jmp	y1
;       }

;v2     /* Put character into COOKED mode buffer & return if we got CR */
;       if (!(_flags & CBREAK)) {
v2	lda	_flags
	bit	#CBREAK
	bne	u2

;           if (c != tty->t_eofc)
	ldy	#t_eofc
	lda	[ttyPtr],y
	and	#$00FF
	cmp	c
	beq	v3
;               editbuf[_index++] = c;
	ldy	_index
	short	m
	lda	c
	sta	[_editBuf],y
	long	m
	iny
	sty	_index

;v3         /* end of line? */
;           if ((c == NL) || (c == CR) || (c == tty->t_eofc)) {
v3	anop
	lda	c
	cmp	#NL
	beq	v4
	cmp	#CR
	beq	v4
	ldy	#t_eofc
	lda	[ttyPtr],y
	and	#$00FF
	cmp	c
	bne	u1

;v4             tty->st_flags = 0; /* done editing this line */
v4	anop
	lda	#0
	ldy	#st_flags
	sta	[ttyPtr],y

;               tmp = MIN(_index,origLength);
	lda	_index
	cmp	origLength	
	bcc	v5
	lda	origLength
	
;                memcpy(buf,editbuf,(size_t)tmp);
v5	sta	tmp
	pea	0
	pha
	pei	(_editBuf+2)
	pei	(_editBuf)
	pei	(buf+2)
	pei	(buf)
	jsl	memcpy

;               /* insert code to set up state for a partial read _here_ */
;               _begin += tmp;
	lda	tmp
	clc
	adc	_begin
	sta	_begin

;               if (_begin == _index) _begin = 0;
	cmp	_index
	bne	v6
	stz	_begin
;v6             xfer += tmp;
v6	lda	tmp
	clc
	adc	xfer
	sta	xfer
; if (c == tty->t_eofc) { retval = 0x4C; goto doReturn; }
	ldy	#t_eofc
	lda	[ttyPtr],y
	and	#$00FF
	cmp	c
	bne	doReturn
	lda	#$4C
	sta	retval
	bra	doReturn
;               break;
;	goto	doReturn
;           }
;       }
u1	jmp	y1
;   }
u2	anop
;   if (flags & Cbreak)
	lda	_flags
	bit	#CBREAK
	beq	u1
	short	m
	lda	c
	sta	[buf]
	long	m
	inc	buf
	bne	yy4
	inc	buf+2
yy4	inc	xfer
;           if ((--inQueue == 0) || (--length == 0)) break;
	dec	inQueue
	beq	doReturn
	dec	length
	beq	doReturn
;           continue;
	bra	u1

doReturn	anop
;   (*tty->demutex)();
	ldx	devNum
	ldy	#demutex
	jsl	ttyDispatch
;   tty->editInd = _index;
	lda	_index
	ldy	#editInd
	sta	[ttyPtr],y
;   tty->editBegin = _begin;
	lda	_begin
	ldy	#editBegin
	sta	[ttyPtr],y
;   (*tty->t_signalIO)(1); /* signal a read operation occurred */
	pea	1
	ldy	#t_signalIO
	ldx	devNum
	jsl	ttyDispatch

;   return xfer;
x1	anop
	return 4:xfer
;}
	END

* An array of 38 pointers to device entry blocks.  A NULL pointer
* indicates that no device is active in that slot.

DeviceBlock	START KERN2
	dc	38i4'0'
	END

* Dispatches to one of the line discipline routines (open,close,r/w,ioctl)
* A is the device number, Y is the index of the routine to call

LineDiscDispatch	START KERN2
LineDiscDispatch	name
rtlAdr	equ   7
headerPtr	equ   1

	tax
	phb                      ; 1 + 
	pha                      ; 2 = 3 byte for fake rtl 
	phd
	pha                      ; some dp space
	pha
	tsc
	tcd
	txa
	asl   a                  ; calculate index into table
	asl   a
	tax
	lda   >DeviceBlock,x
	sta   headerPtr
	lda   >DeviceBlock+2,x
	sta   headerPtr+2
	lda   [headerPtr],y
	dec   a
	sta   rtlAdr
	iny
	iny
	short m
	lda   [headerPtr],y
	sta   rtlAdr+2
	long  m
	pla
	pla
	pld
	rtl
	END

;Device Information
;
;o	Device Name
;o	Device Number
;o	Device pointer points to:
;      -	tty linedisc info
;      -	misc, device-dependent info (Q pointers, semaphores, etc)


* ioctl(int fd, longword tioc, void *dataptr)
* ioctl's are encoded in the following format
* 33222222 22221111 11111100 00000000
* 10987654 32109876 54321098 76543210
* ii        sssssss tttttttt nnnnnnnn          

* ii = in/out (01 = copy out, 10 = copy in)
* s  = size of parameter to copy in/out
* t  = 't' : tty, 's' : socket, 'i' : inet
* n  = base ioctl code

TIioctl	START KERN2
TIioctl	name
	using pipeRecord
	using	KernelStruct

space	equ   16

retval	equ   24+space
fd	equ   22+space
tioc	equ   18+space
dataPtr	equ   14+space
ERRNOptr	equ   10+space

rtl1	equ   7+space
rtl2	equ   4+space
dsp	equ   2+space
bsp	equ   1+space

bitTmp	equ   13
tInd	equ   15
dInd	equ   13
ttyPtr	equ   9
fdPtr	equ   5
tmpPtr	equ   1

	phd
	phb

	phk
	plb
	tsc
	sec
	sbc   #space
	tcd
	tcs

	stz   retval
	ph2   fd
	jsl   getFDptr
	sta   fdPtr
	stx   fdPtr+2
	ora	fdPtr+2
	beq	refIsBad

	ldy   #FDrefNum
	lda   [fdPtr],y
	bne   validFD
refIsBad	lda   #EBADF
	sta   [ERRNOptr]
	lda   #-1
	sta   retval
	jmp   byeIoctl

* the file descriptor is valid, if it is a socket then re-route control to
* network software

validFD	anop
	ldy   #FDrefType
	lda   [fdPtr],y
	cmp   #FDsocket
	bne   notsocket
	ldy	#FDrefNum
	lda	[fdPtr],y
	pha
	ph4	tioc
	ph4	dataPtr
	jsl	SOCKioctl
	beq	sockgroovy
	sta	[ERRNOptr]
	lda	#-1
sockgroovy	anop
	sta	retval
	jmp	byeIoctl

* here we should check to see what type
* of operation the user wants to perform (file or tty).

notsocket	anop
	lda	tioc
	and	#$FF00
	xba
	cmp	#'t'	; tty style?
	beq	doTTYioctl
	cmp	#'f'
	beq	doFioctl
	jmp	invalid

doTTYioctl	anop
	ldy   #FDrefType
	lda   [fdPtr],y
	cmp   #FDtty
	beq   isatty
	lda   #ENOTTY
	sta   [ERRNOptr]
	lda   #-1
	sta   retval
	jmp   byeIoctl

isatty	anop
	lda   [fdPtr]
	dec   a
	sta	bitTmp	; driver number, for null hack
	asl   a
	asl   a
	tax
	lda	>DeviceBlock,x
	sta	ttyPtr
	lda	>DeviceBlock+2,x
	sta	ttyPtr+2

	lda   tioc
	and   #$7F
	cmp   #25
	bcs   jmpNew
	asl   a
	asl   a
	inc   a
	inc   a
	tax
	jmp   (tOTable,X)
jmpNew	eor   #$7F
	cmp   #32
	bcs   invalid
	asl   a
	asl   a
	inc   a
	inc   a
	tax
	jmp   (tNTable,X)

doFioctl	lda	tioc
	and	#$7F	
	eor   #$7F
	cmp   #1
	bcs   invalid
	asl   a
	asl   a
	inc   a
	inc   a
	tax
	jmp   (fTable,X)
* Table for OLD ioctls

invalid	anop
	lda   #EINVAL
	sta   [ERRNOptr]
	lda   #-1
	sta   retval
	jmp   byeIoctl

tOTable	anop
	dc    i2'0',a2'TIOCGETD'
	dc    i2'1',a2'TIOCSETD'
	dc    i2'2',a2'TIOCHPCL'
	dc    i2'3',a2'TIOCMODG'
	dc    i2'4',a2'TIOCMODS'
	dc    i2'-1',a2'invalid'
	dc    i2'-1',a2'invalid'
	dc    i2'-1',a2'invalid'
	dc    i2'8',a2'TIOCGETP'
	dc    i2'9',a2'TIOCSETP'
	dc    i2'10',a2'TIOCSETN'
	dc    i2'-1',a2'invalid'
	dc    i2'-1',a2'invalid'
	dc    i2'13',a2'TIOCEXCL'
	dc    i2'14',a2'TIOCNXCL'
	dc    i2'-1',a2'invalid'
	dc    i2'16',a2'TIOCFLUSH'
	dc    i2'17',a2'TIOCSETC'
	dc    i2'18',a2'TIOCGETC'
	dc	i2'19',a2'TIOCSETK'
	dc	i2'20',a2'TIOCGETK'
	dc	i2'21',a2'TIOCSHUP'
	dc	i2'22',a2'TIOCGHUP'
	dc	i2'23',a2'TIOCSVECT'
	dc	i2'24',a2'TIOCGVECT'

* Table for new ioctls
tNTable	anop
	dc    i2'127',a2'TIOCLBIS'
	dc    i2'126',a2'TIOCLBIC'
	dc    i2'125',a2'TIOCLSET'
	dc    i2'124',a2'TIOCLGET'

	dc    i2'123',a2'TIOCSBRK'
	dc    i2'122',a2'TIOCCBRK'
	dc    i2'121',a2'TIOCSDTR'
	dc    i2'120',a2'TIOCCDTR'

	dc    i2'119',a2'TIOCGPGRP'
	dc    i2'118',a2'TIOCSPGRP'
	dc    i2'117',a2'TIOCSLTC'
	dc    i2'116',a2'TIOCGLTC'
	dc    i2'115',a2'TIOCOUTQ'
	dc    i2'114',a2'TIOCSTI'
	dc    i2'113',a2'TIOCNOTTY'
	dc    i2'-1',a2'invalid'
	dc    i2'111',a2'TIOCSTOP'
	dc    i2'110',a2'TIOCSTART'

	dc    i2'109',a2'TIOCMSET'
	dc    i2'108',a2'TIOCMBIS'
	dc    i2'107',a2'TIOCMBIC'
	dc    i2'106',a2'TIOCMGET'
	dc    i2'-1',a2'invalid'
	dc    i2'104',a2'TIOCGWINSZ'
	dc    i2'103',a2'TIOCSWINSZ'
	dc    i2'-1',a2'invalid'		102
	dc    i2'-1',a2'invalid'         101
	dc    i2'-1',a2'invalid'         100
	dc    i2'-1',a2'invalid'         99
	dc    i2'-1',a2'invalid'         98
	dc	i2'97',a2'TIOCSCTTY'	97

fTable	anop
	dc	i2'127',a2'FIONREAD'

FIONREAD	anop
* check to see what type of FD this is, files aren't supported yet
	ldy   #FDrefType
	lda   [fdPtr],y
	cmp   #FDtty
	beq   frTTY
	cmp	#FDpipe
	beq	frPIPE
	lda   #ENOTTY
	sta   [ERRNOptr]
	lda   #-1
	sta   retval
	jmp   byeIoctl
frTTY	anop
	ph4   tioc
	ph4   dataPtr
	lda   [fdPtr]
	dec   a
	pha

	asl   a
	asl   a
	tax
	lda	>DeviceBlock,x
	sta	ttyPtr
	lda	>DeviceBlock+2,x
	sta	ttyPtr+2

	jsl   DoIoctl        ; let the device know what happened
	jmp	byeIoctl
* another last minute addition/hack
frPIPE	anop
	ldy	#FDrefNum
	lda	[fdPtr],y	; get the pipe ID code
	jsl	calcPipeInd	; get index
	sta	tInd
	tax	
	lda   >accessSem,x
	pha
	jsl   asmWait
	ldx   tInd
	lda	>readLeft,x
	sta	[dataPtr]
	lda   >accessSem,x
	pha
	jsl   asmSignal
	jmp	byeIoctl

********************************************************
* Set a process' controlling TTY to the input FD
* We've already been validated as a TTY file descriptor
TIOCSCTTY	anop
	lda	>curProcInd
	tax
	lda   [fdPtr]
	dec   a
	sta   >ttyID,x
	jmp	byeIoctl

********************************************************
TIOCGPGRP	anop
TIOCSPGRP	anop
	jmp	invalid

* remove process' associate with controlling terminal
* may need to futz with process groups too, maybe not
* Currently sets controlling terminal to .NULL
TIOCNOTTY	anop
	lda	>curProcInd
	tax
	lda	#0
	sta	>ttyID,x
	jmp	byeIoctl
********************************************************
TIOCGETD	ldy   #t_linedisc
	lda   [ttyPtr],y
	sta   [dataPtr]
	jmp   byeIoctl
********************************************************
TIOCSETD	lda   [dataPtr]
	beq   okaySETD
	cmp   #2
	beq   okaySETD
	lda   #EINVAL
	sta   [ERRNOptr]
	lda   #-1
	sta   retval
	jmp   byeIoctl
okaySETD	ldy   #t_linedisc
	sta   [ttyPtr],y
	jmp   byeIoctl
********************************************************
TIOCHPCL	ldy   #privFlags
	lda   [ttyPtr],y
	ora   #HUPCL
	sta   [ttyPtr],y
	jmp   byeIoctl
********************************************************
* These Ioctl's must be handled by the driver
TIOCSVECT	anop
TIOCGVECT	anop
TIOCSHUP	anop
TIOCGHUP	anop
TIOCMODG	anop
TIOCMODS	anop
TIOCFLUSH	anop
	ph4   tioc
	ph4   dataPtr
	lda   [fdPtr]
	dec   a
	tax
	pha
	jsl   DoIoctl        ; let the device know what happened

********************************************************
* SETP should wait for output buffer to empty before copying info
* that's a device IOCTL call

TIOCSETP	anop
* wait for output to drain
* Flush input buffer

TIOCSETN	anop                     ; same as SETP, but no flush
TIOCGETP	ldy   #sg_ispeed
	jsr   CopyInfo
	ph4   tioc
	ph4   dataPtr
	lda   [fdPtr]
	dec   a
	pha
	jsl   DoIoctl        ; let the device know what happened
	cmp   #0
	beq   okayETP
	sta   [ERRNOptr]
	lda   #-1
	sta   retval
okayETP	jmp   byeIoctl
********************************************************
TIOCEXCL	lda	bitTmp	; don't allow exclusive acess of .null
	beq	isNullDev
	ldy   #privFlags
	lda   [ttyPtr],y
	ora   #EXCL
	sta   [ttyPtr],y
isNullDev	jmp   byeIoctl
TIOCNXCL	ldy   #privFlags
	lda   [ttyPtr],y
	and   #EXCL.EOR.$FFFF
	sta   [ttyPtr],y
	jmp   byeIoctl
********************************************************
TIOCSETC	anop
TIOCGETC	anop
	ldy   #t_intrc
	jsr   CopyInfo
	jmp   byeIoctl
********************************************************
TIOCSETK	anop
TIOCGETK	anop
	ph4   tioc
	ph4   dataPtr
	lda   [fdPtr]
	dec   a
	pha
	jsl   DoIoctl
	jmp   byeIoctl
********************************************************
TIOCLGET	ldy   #local
	lda   [ttyPtr],y
	sta   [dataPtr]
	jmp   byeIoctl
TIOCLSET	ldy   #local
	lda   [dataPtr]
	sta   [ttyPtr],y
	jmp   byeIoctl
TIOCLBIS	ldy   #local
	lda   [ttyPtr],y
	ora   [dataPtr]
	sta   [ttyPtr],y
	jmp   byeIoctl
TIOCLBIC	ldy   #local
	lda   [ttyPtr],y
	sta   bitTmp
	lda   [dataPtr]
	trb   bitTmp
	sta   [ttyPtr],y
	jmp   byeIoctl
********************************************************
TIOCSBRK	anop
TIOCCBRK	anop
TIOCSDTR	anop
TIOCCDTR	anop
TIOCOUTQ	anop
TIOCSTI	anop
TIOCSTOP	anop
TIOCSTART	anop
TIOCMSET	anop
TIOCMBIS	anop
TIOCMBIC	anop
TIOCMGET	anop
	ph4   tioc
	ph4   dataPtr
	lda   [fdPtr]
	dec   a
	pha
	jsl   DoIoctl
	jmp   byeIoctl
********************************************************
TIOCSLTC	anop
TIOCGLTC	anop
	ldy   #t_suspc
	jsr   CopyInfo
	jmp   byeIoctl
********************************************************
TIOCGWINSZ	anop
* for SWINSZ, if new information is different from old,
* send a SIGWINCH signal to the members of the TTY's process group

TIOCSWINSZ	anop
	ldy   #ws_row
	jsr   CopyInfo
	jmp   byeIoctl
********************************************************

byeIoctl	tsc
	clc
	adc   #space
	tcs
	plb
	pld
	ldx   #0
	ldy   #14
	jmp   >tool_exit

* This does the work for many of the IOCTLs, which mainly copy data
* in and out.
* The in mode copies data TO the tty record (dataPtr -> ttyPtr)
* The out mode copies data FROM the tty record (ttyPtr -> dataPtr)
* no registers are valid after the call.
* Provide in Y the position in ttyPtr to start the transfer

CopyInfo	anop
	lda   tioc+2
	and   #%01111111
	tax

	sty   tInd               ; save it on the stack  3,s
	stz   dInd               ; the other index       1,s
	lda   tioc+2             ; get the type of transfer
	and   #%1100000000000000 ; mask out in/out flags
	cmp   #$8000             ; copy 'in'
	beq   copyIn
	short m
outLoop	cpx   #0
	beq   doneCopy
	ldy   tInd
	lda   [ttyPtr],y
	iny
	sty   tInd
	ldy   dInd
	sta   [dataPtr],y
	iny
	sty   dInd
	dex
	bra   outLoop
copyIn	short m
inLoop	cpx   #0
	beq   doneCopy
	ldy   dInd
	lda   [dataPtr],y
	iny
	sty   dInd
	ldy   tInd
	sta   [ttyPtr],y
	iny
	sty   tInd
	dex
	bra   inLoop
doneCopy	long  m
	rts

; vector to the TTY's ioctl handler
DoIoctl	anop
	ldy	#t_ioctl+2
	short	m
	lda	[ttyPtr],y	
	pha
	long	m
	ldy	#t_ioctl
	lda	[ttyPtr],y
	dea
	pha
	rtl
	END

* Must be in same data segment main() is expecting, or we have to
* use pointers to access these variables.

vecs	START
OldGetNextEvent	ENTRY
	jmp	>$000000
OldOSEventAvail	ENTRY
	jmp	>$000000
OldGetOSEvent	ENTRY
	jmp	>$000000
OldEventAvail	ENTRY
	jmp	>$000000

mouseMode	ENTRY
	dc	i1'0'
	dc	i1'0'
	dc	i2'0'
	dc	i2'0'
	END

GetDaMouseMod	START
GetDaMouseMod	name

	pha
	pha
	pha
	_ReadMouse
	pla
	and	#$000F
	sta	>mouseMode
	pla
	pla
	rtl
	END

toolOffset	gequ   8

NewEMStartUp	START KERN2
NewEMStartUp	name
userID	equ	0+toolOffset
ymax	equ	userID+2
ymin	equ	ymax+2
xmax	equ	ymin+2
xmin	equ	xmax+2

	phb
	phk
	plb
	tsc
	phd
	tcd

	pei	(xmin)
	lda	xmax
	dec	a
	pha
	pei	(ymin)
	lda	ymax
	dec	a
	pha
	_ClampMouse

	pei	(xmin)
	lda	xmax
	dec	a
	pha
	pei	(ymin)
	lda	ymax
	dec	a
	pha
	_SetAbsClamp

	ph4	#bufr
	ph4	#$40067408
	pea	1
	jsl	ioctl
	lda	#$20
	tsb	bufr+4
	ph4	#bufr
	ph4	#$80067409
	pea	1
	jsl	ioctl

	lda	>mouseMode
	pha
	_SetMouse

	_HomeMouse

	pld
	plb

	ldx #0
	ldy #14
	jmp >tool_exit

NewEMShutDown	ENTRY

	phb
	phk
	plb
	ph4	#bufr
	ph4	#$40067408
	pea	1
	jsl	ioctl
	lda	#$20
	trb	bufr+4
	ph4	#bufr
	ph4	#$80067409
	pea	1
	jsl	ioctl

	plb

	ldx	#0
	ldy	#0
	jmp	>tool_exit
bufr	ds	6
	END

NewGetNextEvent	START
NewGetNextEvent	name

	phy
	phx
	pha
	phb
	phk
	plb
	jsr	CheckGNOCON
	plb
	pla
	plx
	ply
	jmp	>OldGetNextEvent
	END

NewEventAvail	START
NewEventAvail	name

	phy
	phx
	pha
	phb
	phk
	plb
	jsr	CheckGNOCON
	plb
	pla
	plx
	ply
	jmp	>OldEventAvail
	END

NewOSEventAvail	START
NewOSEventAvail	name

	phy
	phx
	pha
	phb
	phk
	plb
	jsr	CheckGNOCON
	plb
	pla
	plx
	ply
	jmp	>OldOSEventAvail
	END

NewGetOSEvent	START
NewGetOSEvent	name

	phy
	phx
	pha
	phb
	phk
	plb
	jsr	CheckGNOCON
	plb
	pla
	plx
	ply
	jmp	>OldGetOSEvent
	END

CheckGNOCON	START
CheckGNOCON	name
	using	InOutData
	using	ADBData

* If this is the actual nullProcess, don't fetch keyboard input.  If this
* is, however, the CDA panel, let events get caught.  Then check for
* a process in the background (and ignore if so).
* We check for nullprocess by a bogus event mask because we can't
* simply check the PID - it could be the CDA panel running as the
* nullprocess (or init or something wierd like that).

	lda	20,s
	cmp	#$0400
	bne	notNullProc
inbg	rts		don't go away eventless, just go away

notNullProc	anop
	lda	>bufState
	bne	doGetKey	we're in the CDA panel

	jsr	isBGProcess
	bcc	doGetKey
	rts

doGetKey	php
	sei		; temporarily shut off interrupts
again	lda	>bufState
	bne	bufferOff

	lda	>head	; if head == tail then there
	cmp	>tail	;  is no data in the input buffer
	beq	noKeyWaiting

	lda   >tail	; get the character & modifiers
	tax                      ; from the buffer, indexed by
	lda   >keybuf,x
	and   #$00FF	; tail.
	tay

	phy		; preserve the Y register thru this, moron
	lda	>modbuf,x           ; portions out
	and	#$00FF
	jsl	ConvKMR2EM
	pha
	short m	remove the keystroke from the
	lda   >tail	key buffer
	inc	a
	sta   >tail
	long  m
	pla
	ply

doThePost	pha
	bit	#$08
	bne	autoKeyIt
	pea	3
	bra	noak
autoKeyIt	pea   5
noak	pha
	phy
	jsr	DoKeyTranslation
	bcs	noPostEvent
	_PostEvent
	pla
	bra   again

noKeyWaiting	plp	                   ; restore interrupts
	rts
noPostEvent	pla
	pla
	pla
	pla
	bra	noKeyWaiting	go ahead and zap the keypress

bufferOff	short m
	lda	>$E0C000	key available?
	bpl	noKeyWaiting
	tay
	lda	>$E0C025
	sta	>$E0C010
	long	m
	and	#$00FF	map the modifiers
	phy		preserve the Y register thru this
	jsl	ConvKMR2EM
	ply
	pha
	tya
	and	#$7F
	tay
	pla
	bra	doThePost

DoKeyTranslation	anop
	phd
	lda   >$E103CA	Get the WAP for the Event Manager
	pha
	lda   >$E103C8
	pha
	tsc
	tcd		And set the DP to it
	ldy   #$0018	Tool set 6 * 4 bytes each
	lda   [$01],y
	ply
	ply
	cmp   #$0000
	beq   doNothing	Event Manager not started??!?!!
	tcd
	lda   $68	Translation table loaded?
	bne   okay	yes
	clc                      no, so don't translate the keypress
	bra   doNothing
okay	pei   $68	Save the translation handle on the stack
	pei   $66
	ldy   #$0002
	lda   [$66],y
	tax
	lda   [$66]	Deref the key translation
	sta   $66                table handle directly into
	stx   $68                the same pointer
	lda   $0B,s
	and   #$0800             option Key pressed?
	beq   noOption
	bra   OptionKey
doneXlate	pla
	sta   $66
	pla
	sta   $68
doNothing	pld
	rts
OptionKey	lda   $09,s	set the hi bit of
	ora   #$0080	the character if
	bra   yucky1	option was pressed
noOption	lda   $09,s
yucky1	tay
	lda   [$66],y
	and   #$00FF
	cmp   #$00FF	if the entry is FF
	bne   l1A79	$$$
	tya		set the hi bit
	ora   #$0080
	tay
	lda	[$66],y	and load in the
	and   #$00FF	option-key version
l1A79	tax		and stuff in X
	lda   $6A	last keystroke a dead key?
	bne   validateDead	if so, go to 'validate'
	tya
	and   #$0080
	beq   notDead
	stx   $6A                      set the dead key
	ldy   #$0100
scanDeadKey	lda   [$66],y
	and   #$00FF
	beq   notDead                  last entry in array?
	cmp   $6A
	beq   isDeadKey                dead key found?
	iny
	iny
	bra   scanDeadKey
notDead	txa
	stz   $6A                      clear dead key flag/value
	sta   $09,s
	clc
	bra   doneXlate
isDeadKey	sec
	bra   doneXlate
validateDead	ldy   #$0100
lp2	lda   [$66],y                  scan the dead key
	and   #$00FF                   table for the offset
	beq   noDeadMatch              in the replacement
	cmp   $6A                      table
	beq   gotIt                    got it
	iny
	iny
	bra   lp2
gotIt	lda   [$66],y
	and   #$FF00
	xba
	asl   a
	clc
	adc   #$0100             deadKey+replaceTab
	tay                      is 256 bytes at most
	phx
scanReplace	lda   [$66],y            now find the keycode
	and   #$00FF             they just hit
	beq   noReplMatch        end of list, no match
	cmp   $01,s
	beq   matched            matched a key
	iny
	iny
	bra   scanReplace
noDeadMatch	txa
	sta   $09,s
	stz   $6A
	clc
	bra   l1234
matched	plx
	lda   [$66],y
	and   #$FF00
	xba
	tax
	bra   noDeadMatch	replaced, clc means
noReplMatch	pla                      regular key
	sta   $09,s
	lda   $0B,s
	pha
	pea   $0003
	ora   #$0800
	pha
	lda   $6A
	pha
	_PostEvent
	pla
	stz   $6A
	clc
l1234	brl   doneXlate
	END

checkExclTTY	START KERN2
checkExclTTY	name
ttyPtr	equ	1

	tay
	pha
	pha
	tsc
	phd
	tcd

	dey
	tya
	asl   a
	asl   a
	tax
	lda	>DeviceBlock,x
	sta	ttyPtr
	lda	>DeviceBlock+2,x
	sta	ttyPtr+2

	ldy	#privFlags
	lda	[ttyPtr],y
	ldy	#0
	bit	#EXCL
	beq	not_excl
	ldy	#1
not_excl	anop
	pld
	pla
	pla
	rtl
	END

* FIXME: change calls that use this to call TIioctl directly
* int ioctl (int fd, unsigned long request, void *argp);
ioctl	START KERN2
ioctl	name
retval	equ	1
	sub (4:argp,4:request,2:d),2

	pha
	ph2	d
	ph4	request
	ph4	argp
	ph4	#errno
	ldx	#$2603
	jsl	udispatch
	pla
	sta	retval
	ret 2:retval
	END

