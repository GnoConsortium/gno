*	$Id: tty.inc,v 1.1 1998/02/02 08:20:59 taubert Exp $

* Select codes
SEL_READ	gequ   0
SEL_WRITE	gequ   1
SEL_EXCEPT	gequ   2

* Error codes
EINVAL	gequ  6                  ; Invalid argument
EBADF	gequ  7                  ; bad file descriptor
EINTR	gequ  14                 ; Interrupted system call
ENOTTY	gequ  20                 ; Not a terminal

* privFlags:
HUPCL	gequ	1	; hang up modem on last close
EXCL	gequ	2	; don't allow any more opens
TS_RCOLL	gequ	4	; select collision flag

* tty mode flags
CBREAK	gequ	$0002	/* half-cooked mode */
LCASE	gequ	$0004    	/* simulate lower case */
ECHO	gequ	$0008	/* echo input */
CRMOD	gequ	$0010	/* map \r to \r\n on output */
RAW	gequ	$0020	/* no i/o processing */
ODDP	gequ	$0040	/* get/send odd parity */
EVENP	gequ	$0080	/* get/send even parity */

LCRTBS	gequ	$0001     /* do backspacing for crt */
LPRTERA	gequ	$0002     /* \ ... / erase */
LCRTERA	gequ	$0004     /* " \b " to wipe out char */
LTILDE	gequ  $0008     /* hazeltine tilde kludge */
LMDMBUF	gequ  $0010     /* start/stop output on carrier intr */
LLITOUT	gequ  $0020     /* literal output */
LTOSTOP	gequ  $0040     /* SIGSTOP on background output */
LFLUSHO	gequ  $0080     /* flush output to terminal */
LNOHANG	gequ  $0100     /* no SIGHUP on carrier drop */
LPASS8OUT	gequ  $0200     /* 8 bit no parity for cooked output */
LCRTKIL	gequ  $0400     /* kill line with " \b " */
LPASS8	gequ  $0800     /* 8 bit no parity for cooked input */
LCTLECH	gequ  $1000     /* echo control chars as ^X */
LPENDIN	gequ  $2000     /* tp->t_rawq needs reread */
LDECCTQ	gequ  $4000     /* only ^Q starts after ^S */
LNOFLSH	gequ  $8000     /* no output flush on signal */

B0	gequ	0
B50	gequ	1
B75	gequ	2
B110	gequ	3
B134	gequ	4
B150	gequ	5
B200	gequ	6
B300	gequ	7
B600	gequ	8
B1200	gequ	9
B1800	gequ	10
B2400	gequ	11
B4800	gequ	12
B9600	gequ	13
B19200	gequ	14
EXTA	gequ	14
B38400	gequ	15
EXTB	gequ	15

* Definition of the tty structure

sg_ispeed	gequ   0	input speed
sg_ospeed	gequ   1	output speed
sg_erase	gequ   2	erase character
sg_kill	gequ	3	kill character
sg_flags	gequ   4	mode flags
local	gequ	6	local mode flags (new driver)

t_intrc	gequ	8
t_quitc	gequ	9
t_startc	gequ	10
t_stopc	gequ	11
t_eofc	gequ	12
t_brkc	gequ	13

t_suspc	gequ	14
t_dsuspc	gequ	15
t_rprntc	gequ	16
t_flushc	gequ	17
t_werasc	gequ	18
t_lnextc	gequ	19

ws_row	gequ	20
ws_col	gequ	22
ws_xpixel	gequ   24
ws_ypixel	gequ   26

t_open	gequ	28
t_close	gequ	32
t_ioctl	gequ	36
t_read	gequ	t_ioctl+4
t_write	gequ	t_read+4
mutex	gequ	t_write+4
demutex	gequ	mutex+4
out_enq	gequ	demutex+4
in_enq	gequ	out_enq+4
out_deq	gequ	in_enq+4
in_deq	gequ	out_deq+4
size_inq	gequ   in_deq+4
size_outq	gequ   size_inq+4
editInd	gequ	size_outq+4
editBegin	gequ   editInd+2
st_flags	gequ   editBegin+2
editBuf	gequ	st_flags+2	; 4K input editing buffer
t_linedisc	gequ   editBuf+4
t_devNum	gequ   t_linedisc+2
privFlags	gequ   t_devNum+2	; 
t_sendSignal	gequ   privFlags+2        ; a jml statement
t_BGCheck	gequ   t_sendSignal+4
t_GetProcInd	gequ   t_BGCheck+4
t_userid	gequ   t_GetProcInd+4
t_signalIO	gequ   t_userid+2
t_select_proc	gequ   t_signalIO+4
t_select	gequ   t_select_proc+2	; the select function pointer
t_selwakeup	gequ   t_select+4	; set during install
ttyRecSize	gequ   t_selwakeup+4

* pty structure additions - these are actually maintained in bank 0
p_bufAptr	gequ   0
p_bufBptr	gequ   p_bufAptr+4
p_Aleft	gequ	p_bufBptr+4
p_Bleft	gequ	p_Aleft+2
p_sem	gequ	p_Bleft+2
p_Ahead	gequ	p_sem+2
p_Atail	gequ	p_Ahead+2
p_Bhead	gequ	p_Atail+2
p_Btail	gequ	p_Bhead+2
ptyRecSize	gequ   p_Btail+2
