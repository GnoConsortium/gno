/*
    more.cc

    Revision history:

    v2.0  - now sets tty to cbreak mode, since TTY's cooked mode has
    	    changed.
    v1.31 - uses fstat to check whether output is a tty, instead of
		    _Direction
    v1.3  - now prints name of file in block if multiple files specified
*/

#pragma optimize -1
#pragma stacksize 1024

#include <stdio.h>
#include <stdlib.h>
#include <types.h>
#include <sys/signal.h>
#include <gno/gno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <shell.h>
#include <gsos.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <texttool.h>
#include <orca.h>

#pragma lint -1

#define MAX_LINE 23
#define MAX_COL 80

void PrintFileName(char *s)
{
char *r;
    for (r = s; *r != 0; r++)
        *r = tolower(*r);

    if ((r = strrchr(s,'/')) != NULL)
        printf("%s\n",r+1);
    else printf("%s\n",s);
}

char getcharacter(int fd)
{
char c;

   if (fd == -1) c = ReadChar(0);
   else read(fd,&c,1);
   return c;
}

struct sgttyb sg;
FILE *tty;
int oldsg_flags;

void inthndl(int sig, int code)
{
    sg.sg_flags = oldsg_flags;
    ioctl(tty->_file,TIOCSETP,&sg);
    exit(sig);
}

char i_buf[1024];
word b_ind = 0;
word b_size = 0;
int b_ref;
int b_pushback = EOF;
word b_eof;
longword b_mark;

int buf_open(char *name)
{
int fd;
    fd = open(name,O_RDONLY);
    b_ind = 0;
    b_size = 0;
    b_mark = 0l;
    b_eof = 0;
    return (b_ref = fd);
}

void buf_fd_open(int fd)
{
    b_ind = 0;
    b_size = 0;
    b_mark = 0l;
    b_ref = fd;
    b_eof = 0;
}

int buf_getc(void)
{
char c;

    if (b_pushback != EOF) { c = b_pushback; b_pushback = EOF; return c; }
    if (b_ind >= b_size) {
        if (b_eof) return EOF;
        b_size = read(b_ref,i_buf,1024);
        if (!b_size) b_eof = 1;
        b_ind = 0;
        return buf_getc();
    }
    else {
        c = i_buf[b_ind++];
        b_mark = b_mark + 1l;
        return c;
    }
}

#define OBUFSIZE 1024
char o_buf[OBUFSIZE];
word b_oind = 0;

void buf_flush(void)
{ write(STDOUT_FILENO,o_buf,b_oind); b_oind = 0; }

void buf_putc(char c)
{
    if (b_oind == OBUFSIZE) buf_flush();
    o_buf[b_oind++] = c;
}

int main(int argc,char *argv[])
{
int file;
word line,col;
int i;
int c,quit,abort;
int pipeFlag; /* 1 means input is piped in or redirected */
int standardOut;
EOFRecGS eofs;
char expanded[65];
char *truncated;
static struct stat sb;
extern int _INITGNOSTDIO(void);

    /*if (!_INITGNOSTDIO()) {
    	fprintf(stderr,"'more' requires GNO/ME.\n");
	    exit(1);
    }*/
    setvbuf(stdout,NULL,_IOLBF,256);
    tty = fopen(".tty","r");
    /* turn off echo mode */
    ioctl(tty->_file, TIOCGETP, &sg);
    signal(SIGINT, inthndl);
    buf_putc(6);

    fstat(STDOUT_FILENO,&sb);
    if (sb.st_mode & S_IFCHR) standardOut = 1;
    else standardOut = 0;

    pipeFlag = (argc == 1) ? 1 : 0;
    if (argc == 1) argc = 2;
    quit = 0;
    abort = 0;

    oldsg_flags = sg.sg_flags;
    sg.sg_flags &= ~ECHO;
    sg.sg_flags |= CBREAK;
    ioctl(tty->_file, TIOCSETP, &sg);
       
    for (i = 1; (i < argc && !abort); i++)
    {
       quit = 0;
       if (pipeFlag) {
           file = STDIN_FILENO;
           buf_fd_open(file);
       }
       else {
           truncated = strrchr(argv[i],'/');
           if (truncated == NULL) truncated = argv[i];
               else truncated++;
    	   stat(argv[i],&sb);
           if (sb.st_mode & S_IFDIR) {
               fprintf(stderr,"more: %s is a directory\n",argv[i]);
               fflush(stderr);
               continue;
           }
           file = buf_open(argv[i]);
           if (file < 0) {
               perror(argv[i]);
               buf_putc(5); buf_flush(); exit(1);
           }
           eofs.pCount = 2;
           eofs.refNum = file;
           GetEOFGS(&eofs);
           if (toolerror()) {
               printf("GS/OS Error $%X\n",toolerror());
               buf_putc(5); buf_flush(); exit(-1);
           }
       }
       if (argc > 2) {
            printf("::::::::::::::::\n");
            PrintFileName(argv[i]);
            printf("::::::::::::::::\n");
            line = 3; col = 1;
       }
       else {line = 1; col = 1;}

       c = buf_getc(); /* wierdness fix */
       if (c == EOF) quit = 1;
    /*   c &= 0xFF; */
       if (c == 0x04) quit = 1; 
       while (!quit && !abort)

       {
       int k;

           if (c == 0x0c) { buf_flush(); printf("^L\n");
                            line = MAX_LINE; col = 1; }
           else if (c == '\n') {
           	buf_putc('\r');
	        col = 1; line++;
	   }
           else if (c == '\r') {
               if ((k = buf_getc()) == '\n') /*  IBM silly CR & LF EOL */
                   buf_putc('\r');
               else {
                   b_pushback = k;
                   b_mark = b_mark-1;
                   buf_putc('\r');
               }
               col = 1; line++;
           }
           else if (c == '\n')
           {
               buf_putc('\n');
               col = 1; line++;
           }
           else
           {
               buf_putc(c);
               if (c == 8) col--;
               else if (c > 32) col++;
               if (col > MAX_COL)
               {
                   col = 1; line++;
               }
           }
           if ((line == MAX_LINE) && standardOut)
           {   long percent;
               buf_flush();
               putchar(15);
               if (!pipeFlag)
               {
                   percent = (b_mark * 100) / eofs.eof;
                   printf(" - %s (%2ld%%) - ",truncated,percent);
               }
               else
                   printf(" - (more) - ");
               putchar(14);
               fflush(stdout);
               c = getcharacter(tty->_file);
               c = c & 0x7f;

               if (c == 'q') quit = 1;
               if (c == 27) abort = 1;
               if (c == 13) line--;
               else line = 1;
               for (c = 0; c < (12 + ((percent == 100) ? 1 : 0) +
                       (pipeFlag ? 0 : strlen(truncated))); c++)
               {
                   putchar(8);
                   putchar(' ');
                   putchar(8);
               }
               fflush(stdout);
           }
           c = buf_getc();
           if (c == EOF) quit = 1;
           /*c &= 0xFF; */
           if (c == 0x04) quit = 1;
       }
       if (!pipeFlag) close(file);
       else abort = 1; /* we are DONE if this was a pipe */
       if (!abort && (argc > 2) && (i != argc) && (line < MAX_LINE)
           && standardOut)
       {
           putchar(15);
           printf("hit a key for next file");
           putchar(14);
           fflush(stdout);
           c = getcharacter(tty->_file);
           for (c = 0; c < 23; c++)
           {
               putchar(8);
               putchar(' ');
               putchar(8);
           }
           c = c & 0x7f;
           if (c == 27) abort = 1;
       }
    }
    buf_flush();
    putchar(5);
    sg.sg_flags = oldsg_flags;
    ioctl(tty->_file,TIOCSETP,&sg);
    exit(0);
}
