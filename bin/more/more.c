/*
    more.cc

    Revision history:

    v1.4  - Termcap support has been added and tested.  Look for a soon to be
                    faster version coming soon to a ~/bin directory near you!

    v1.32 - uses isatty(x) instead of fstat.  more portable (opinion).
                    soon to be added, TERMCAP support, won't that be nice?

    v1.31 - uses fstat to check whether output is a tty, instead of
		    _Direction
    v1.3  - now prints name of file in block if multiple files specified
*/

#pragma optimize 8
#pragma stacksize 3072

#include <stdio.h>
#include <stdlib.h>
#include <types.h>
#include <signal.h>
#include <gno/gno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ioctl.h>
#include <shell.h>
#include <gsos.h>
#include <string.h>
#include <ctype.h>
#include <texttool.h>
#include <orca.h>
#include <termcap.h>

#pragma lint -1

#define MAX_LINE 23
#define MAX_COL 80

int tprchar(char c);

void PrintFileName(char *s)
{
char *r;
    for (r = s; *r != 0; r++)
        *r = tolower(*r);

    if ((r = strrchr(s,'/')) != NULL)
        printf("%s\n",r+1);
    else
        printf("%s\n",s);
}

char getcharacter(int fd)
{
char c;
IORecGS r = {4, fd, (void *)&c, 1l};

   if (fd == -1)
       c = ReadChar(0);
   else
       ReadGS(&r);

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

char inv[20], nor[20];
char *pterm, *pcap, *ps;
char termcap[1030], capability[100];

int main(int argc,char *argv[])
{
FILE *file;
int line,col;
int i;
int c,quit,abort;
int pipeFlag; /* 1 means input is piped in or redirected */
int standardOut;
EOFRecGS eofs;
char expanded[65];
char *truncated;
static struct stat sb;
extern int _INITGNOSTDIO(void);

    if (!_INITGNOSTDIO())
    {
    	fprintf(stderr,"'more' requires GNO/ME.\n");
	    exit(1);
    }
    if ((pterm = getenv ("TERM"))
    && (tgetent (termcap, pterm) == 1))
    {
	pcap = capability;
	if (ps = tgetstr ("so", &pcap))
	{
		/*
		 *   sun has padding in here. this is NOT portable.
		 *   better to use tputs() to strip it...
		 */
		strcpy (inv, ps);
	}
               else { fprintf(stderr,"couldn't get standout mode\n");
                      exit(1); }
	if (ps = tgetstr ("se", &pcap))
	{
		strcpy (nor, ps);
	}
    }


    setvbuf(stdout,NULL,_IOLBF,256);
    tty = fopen(".tty","r");
	/* turn off echo mode */
    ioctl(tty->_file, TIOCGETP, &sg);
    signal(SIGINT, inthndl);
    /*putchar(6);*/

    if (!isatty(2))
	standardOut = 0;
    else
	standardOut = 1;

    pipeFlag = (argc == 1) ? 1 : 0;
    if (argc == 1)
        argc = 2;
    quit = 0;
    abort = 0;

    oldsg_flags = sg.sg_flags;
    sg.sg_flags &= ~ECHO;
    ioctl(tty->_file, TIOCSETP, &sg);
       
    for (i = 1; (i < argc && !abort); i++)
    {
       quit = 0;
       if (pipeFlag)
       {
           file = stdin;
       }
       else
       {
           truncated = strrchr(argv[i],'/');
           if (truncated == NULL) truncated = argv[i];
               else truncated++;
           file = fopen(argv[i],"rb");
           if (file == NULL)
           {
               perror(argv[i]);
               /*putchar(5);*/ exit(1);
           }
           eofs.pCount = 2;
           eofs.refNum = file->_file;
           GetEOFGS(&eofs);
           if (toolerror())
           {
               printf("GS/OS Error $%X\n",toolerror());
               /*putchar(5);*/ exit(-1);
           }
       }
       if (argc > 2)
       {
            printf("::::::::::::::::\n");
            PrintFileName(argv[i]);
            printf("::::::::::::::::\n");
            line = 3; col = 1;
       }
       else
       {
            line = 1; col = 1;
       }

       c = getc(file); /* wierdness fix */
       if (c == EOF) quit = 1;
       c &= 0xFF;
       if (c == 0x04) quit = 1;
       while (!quit && !abort)

       {
       int k;

           if (c == 0x0c)
           {
                printf("^L\n");
                line = MAX_LINE; col = 1;
           }
           else if (c == '\r')
           {
               if ((k = getc(file)) == '\n') /*  IBM silly CR & LF EOL */
                   putchar('\n');
               else
               {
                   ungetc(k,file); putchar('\n');
               }
               col = 1; line++;
           }
           else if (c == '\n')
           {
               putchar('\n');
               col = 1; line++;
           }
           else
           {
               putchar(c);
               col++;
               if (col > MAX_COL)
               {
                   col = 1; line++;
               }
           }
           if ((line == MAX_LINE) && standardOut)
           {   long percent;
               tputs(inv,1,tprchar);
               if (!pipeFlag)
               {
                   percent = (ftell(file) * 100) / eofs.eof;
                   printf(" - %s (%2ld%%) - ",truncated,percent);
               }
               else
                   printf(" - (more) - ");
               tputs(nor,1,tprchar);
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
           }
           c = getc(file);
           if (c == EOF) quit = 1;
           c &= 0xFF;
           if (c == 0x04) quit = 1;
       }
       if (!pipeFlag)
           fclose(file);
       else
           abort = 1; /* we are DONE if this was a pipe */
       if (!abort && (argc > 2) && (i != argc) && (line < MAX_LINE)
           && standardOut)
       {
           tputs(inv,1,tprchar);
           printf("hit a key for next file");
           tputs(nor,1,tprchar);
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
    /*putchar(5);*/
    sg.sg_flags = oldsg_flags;
    ioctl(tty->_file,TIOCSETP,&sg);
    exit(0);
}

int tprchar(char c)
{
    putchar( c );
}
