/* write.cc  - UNIX-like write utility.  Send message to terminal. */
/* Copyright 1993 by Leslie M. Barstow III */
/* Placed in the Public Domain with the following condition:
      Please retain original Copyright in source, program, and manpage. */

#pragma optimize -1
#pragma debug 0
#pragma stacksize 1024

#define O_WRONLY  0x0002

#include <time.h>
#include <types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <orca.h>
#include <utmp.h>
#include <fcntl.h>

void Usage(void)
{fputs("usage: write username [message]\n",stdout);
 fputs("GS write: Copyright 1993 by Leslie M. Barstow III.\n", stdout);
}

int userTTY(char *user, int **list)
{int fd, tmp;
 struct utmp uentry;
 int count = 0;
 int *newlist;
 fd=open(_PATH_UTMP, 257);
 if (fd < 1)
  {fputs("write - unable to retrieve current users.\n", stdout);
   exit(-1);
  }
 do
   {tmp = read(fd, (void *)(&uentry), sizeof(uentry));
    if (tmp)
      {if (!strncmp(uentry.ut_name,user,strlen(user)))
	       {count++;
          newlist = (int *)realloc(*list, count *2);
          if (newlist == (int *)0L)
	          {for (tmp = 1; tmp < count; tmp++)
	             close((*list)[tmp-1]);
             free(*list);
             close(fd);
             fputs("write - unable to allocate memory.\n", stdout);
             exit(-1);
            }
          *list = newlist;
          (*list)[count-1] = open(uentry.ut_line, 0x0002);
          if ((*list)[count-1]  < 1)
	          {fputs("write - unable to open terminal ",stdout);
             fputs(uentry.ut_line, stdout);
             fputs(".\n", stdout);
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
 char *line, *uname;
 int *fdlist = (int *)0L;

if (argc > 3)
  {Usage();
   exit(-1);
  }
if (argc == 3)
  {count = userTTY(argv[1], &fdlist);
   if (count < 1)
     {fputs("write - no active terminals for user.\n", stdout);
      exit(-1);
     }

   uname = getenv("user");
   for (i = 0; i < count; i++)
     {write(fdlist[i], "\n\aMessage from ", 15);
      write(fdlist[i], uname, strlen(uname));
      write(fdlist[i], ".\r\n", 3);
      write(fdlist[i], argv[2], strlen(argv[2]));
      write(fdlist[i], "\r\n", 2);
     }
   for (i = 0; i < count; i++)
      close(fdlist[i]);
   exit(0);
  }
if ((argc == 2) && (argv[1][0] != '-'))  /* enter read loop */
  {line = (char *)malloc(256);
   if (line == (char *)0L)
     {fputs("write - error allocating buffer.\n", stdout);
      exit(-1);
     }
   count = userTTY(argv[1], &fdlist);
   if (count < 1)
  	{fputs("write - no active terminals for user.\n", stdout);
      free(line);
      exit(-1);
  	}
   uname = getenv("user");
   for(i = 0; i < count; i++)
     {write(fdlist[i], "\n\aMessage from ", 15);
      write(fdlist[i], uname, strlen(uname));
      write(fdlist[i], ".\r\n", 3);
     }
   while (j > 1)
     {fgets(line, 256, stdin);
      j = strlen(line);
      for (i = 0; i< count; i++)
  	    write(fdlist[i], line, j);
         
  	}
   free(line);
   for (i = 0; i < count; i++)
      close(fdlist[i]);
   exit(0);
  }
else
  {Usage();
   exit(-1);
  }
}
