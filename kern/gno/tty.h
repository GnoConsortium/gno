/*	$Id: tty.h,v 1.1 1998/02/02 08:19:12 taubert Exp $ */

struct tty_s {
	/*q outQ;
	q inQ; */
        char    sg_ispeed;              /* input speed */
        char    sg_ospeed;              /* output speed */
        char    sg_erase;               /* erase character */
        char    sg_kill;                /* kill character */
        unsigned sg_flags;               /* mode flags */
        
        unsigned local;

        char    t_intrc;        /* interrupt */
        char    t_quitc;        /* quit */
        char    t_startc;       /* start output */
        char    t_stopc;        /* stop output */
        char    t_eofc;         /* end-of-file */
        char    t_brkc;         /* input delimiter (like nl) */

        char    t_suspc;        /* stop process signal */
        char    t_dsuspc;       /* delayed stop process signal */
        char    t_rprntc;       /* reprint line */
        char    t_flushc;       /* flush output (toggles) */
        char    t_werasc;       /* word erase */
        char    t_lnextc;       /* literal next character */

        unsigned short  ws_row;                 /* rows, in characters */
        unsigned short  ws_col;                 /* columns, in characters */
        unsigned short  ws_xpixel;              /* horizontal size, pixels */
        unsigned short  ws_ypixel;              /* vertical size, pixels */

  	void (*t_open)(int devnum);
        void (*t_close)(int devnum);
        int (*t_ioctl)(int devNum, void *dataptr, unsigned long tioc);
        int (*t_read)(int devNum, void *dataptr, unsigned count);
        int (*t_write)(int devNum, void *dataptr, unsigned count);
        void (*mutex)();
	void (*demutex)();
        void (*out_enq)(char c);
	void (*in_enq)(char c);
	int  (*out_deq)();
	int  (*in_deq)();
	int  (*size_inq)();
	int  (*size_outq)();
        unsigned editInd,editBegin;
        unsigned st_flags;
	char	*editBuf;
};
typedef struct tty_s ttyb;
