/* write.cc  - UNIX-like write utility.  Send message to terminal. */
/* Copyright 1993, 1994 by Leslie M. Barstow III */
/* This program is Freeware:  Distribute and modify as you like, but
      please retain original Copyright in source, program, and manpage. */

/* v1.0 - original release */
/* v1.1 - 2/2/94 updated for new Multi-User package using wtmp */
/* v2.0 - 2/21/94  fixed several bugs, added date to message. */
/* v2.1 - 3/8/94   changed errors to stderr, fixed some errors, added TZ.
                   changes taken in part from Phil Vandry's sample code -
                   Thanks, Phil :-) */

#pragma optimize -1
#pragma debug 0
#pragma stacksize 768

#define O_WRONLY  0x0002

#include <time.h>
#include <types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <orca.h>
#include <utmp.h>
#include <fcntl.h>
#include <pwd.h>
#include <unistd.h>
#include <texttool.h>
#include <sys/ioctl.h>

void Usage(void)
{fputs("usage: write username [message]\n",stdout);
 fputs("GS write v2.1: Copyright 1994 by Leslie M. Barstow III.\n", stdout);
}

int userTTY(char *user, int **list)
{int fd, tmp;
 static struct utmp uentry;
 static char tmpline[UT_LINESIZE+1];
 int count = 0;
 int *newlist;
 fd=open(_PATH_UTMP, 257);
 if (fd < 1)
  {fputs("write - unable to retrieve current users.\n", stderr);
   exit(-1);
  }
 do
   {tmp = read(fd, (void *)(&uentry), sizeof(uentry));
    if (tmp && uentry.ut_type && !(uentry.ut_type & UT_INVISIBLE))
      {if (!strncmp(uentry.ut_name,user,UT_NAMESIZE))
	       {count++;
          newlist = (int *)realloc(*list, count *2);
          if (newlist == (int *)0L)
	          {for (tmp = 1; tmp < count; tmp++)
	             close((*list)[tmp-1]);
             free(*list);
             close(fd);
             fputs("write - unable to allocate memory.\n", stderr);
             exit(-1);
            }
          *list = newlist;
          (*list)[count-1] = open(uentry.ut_line, 0x0002);
          if ((*list)[count-1]  < 1)
	          {fputs("write - unable to open terminal ",stderr);
             strncpy(tmpline, uentry.ut_line, UT_LINESIZE);
             tmpline[UT_LINESIZE] = '\0';
             fputs(tmpline, stderr);
             fputs(".\n", stderr);
             count--;
            }
         }
      }
   }
 while (tmp);
 close(fd);
 return(count);
}


int main(int argc, char **argv)
{int count;
 int i;
 int j = 2;
 time_t Time;
 char *cTime;
 char *uname;
 int *fdlist = (int *)0L;
 uid_t uid;
 int doEcho;
 struct passwd *pwent;
 static char line [512];


if ((argc < 2) || (argc > 3) || (argv[1][0] == '-'))
  {Usage();
   exit(-1);
  }

uid = getuid();
pwent = getpwuid(uid);
uname = pwent->pw_name;
Time = time(0L);
cTime = ctime(&Time);

count = userTTY(argv[1], &fdlist);
if (count < 1)
  {fputs("write - no active terminals for user.\n", stderr);
   exit(-1);
  }
for (i = 0; i < count; i++)
  {write(fdlist[i], "\r\aMessage from ", 15);
   write(fdlist[i], uname, strlen(uname));
   write(fdlist[i], " at ", 4);
   write(fdlist[i], cTime, strlen(cTime));
   write(fdlist[i], "\r", 1);
  }

/* message on command line - write it and exit */

if (argc == 3)
  {for (i = 0; i < count; i++)
     {write(fdlist[i], argv[2], strlen(argv[2]));
      write(fdlist[i], "\r", 1);
      close(fdlist[i]);
     }
   exit(0);
  }

/* No message on command line - read from stdin */

 while (j = ReadLine(line, 512, '\r', 0))
   {for (i = 0; i< count; i++)
  	 {write(fdlist[i], line, j);
       write(fdlist[i], "\r", 1);
      }
   }
 for (i = 0; i < count; i++)
    write(fdlist[i], "\r<EOT>\r", 7);
 free(line);
 for (i = 0; i < count; i++)
    close(fdlist[i]);
 exit(0);
}
