/*                                                                        */
/* extended ps v1.0 for GNO/ME v1.1                                       */
/*                                                                        */
/*    6/17/93 -- initial coding, this version does everything gsh's 'ps'  */
/*               and also shows the Parent's PID, as well as giving a     */
/*               larger time field.  -u, -e and -n options implemented    */
/*                                                                        */
/*    6/18/93 -- fixed time calculations, which were off for minutes and  */
/*               hours by a power of 10.                                  */
/*                                                                        */
/*    6/20/93 -- added 'sleeping' category to process state listing.      */
/*                                                                        */
/*    6/22/93 -- fixed display for forked child                           */
/*               added -t option flag                                     */
/*               added rexit() call, and getopt_restart()                 */
/*                                                                        */
/*    6/23/93 -- fixed -t option to work with ttyID of 00                 */
/*                                                                        */
/*    6/26/93 -- fixed miscellaneous problems with memory allocation      */
/*                                                                        */
/*    6/27/93 -- added -w, -ww, -l flags.                                 */
/*                                                                        */
/*    Author: James Brookes                                               */
/*            jamesb@cscihp.ecst.csuchico.edu                             */
/*                                                                        */

#pragma optimize -1
#pragma stacksize 512 

#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <misctool.h>
#include <limits.h>
#include <gno/kvm.h>
#include <getopt.h>

#define FALSE 0
#define TRUE  1

void  print_time(long ticks);
int   getuid(void);
char *ttyname(int fd);
extern int getopt_restart(void);

/* boolean global options */

int   username;        /* username to show   */
int   allprocs;        /* show all processes */
int   showname;        /* show username      */
int   tty;             /* tty to show        */
int   showtty;         /* show only ttyID    */
int   w_counter;       /* width option       */
int   long_list;       /* long listing       */

static char *status[] = { "unused", "running", "ready", "blocked", "new",
                          "suspended", "wait", "waitsigch", "paused",
                          "sleeping", "unknown" };

void usage(char *callname)

   {
   fprintf(stderr,"usage: %s -anlwt <tty> u <username>\n",callname);
   rexit(0);
   }

/* Process option flags */

void getopts(int argc, char **argv)

   {
   int c, username_select, tty_select;
   struct passwd *pw_s;

   username_select = FALSE;
   tty_select = FALSE;

   pw_s = (struct passwd *) malloc (sizeof(struct passwd));

   username  = getuid();
   allprocs  = FALSE;
   showname  = FALSE;
   showtty   = FALSE;
   long_list = FALSE;
   w_counter = 0;

   if (argc == 1)
      return;

   while((c = getopt(argc,argv,"anwlt:u:")) != EOF)

      {
      switch((unsigned char)c)

         {
         case 'a':

            allprocs = TRUE;
            break;
 
         case 'n':
 
            showname = TRUE;
            break;

         case 't':

            tty_select = TRUE;
            showtty = TRUE;
            tty = atoi(optarg);
            break;

         case 'u':

            username_select = TRUE;
            pw_s = getpwnam(optarg);
            username = pw_s->pw_uid;
            break;
           
         case 'w':

            w_counter++;
            break;

         case 'l':

            long_list = TRUE;
            break;

         default:

            usage(argv[0]);
         }

      }

   if ((allprocs) && ((username_select) || (tty_select)))
      usage(argv[0]);
   }

void print_time(long ticks)

   {
   int secs, mins, hous, days, shortmins;

   secs = (ticks/60)      %  60;
   mins = (ticks/3600)    %  60;
   hous = (ticks/216000)  %  24;
   days = (ticks/5184000) % 365;

   if (!long_list) 
      shortmins = (ticks/3600) % 999;

   if (long_list)

      {
      if (hous)
         printf("%2d:",hous);
      else
         printf("   ");

      printf("%02d:",mins);
      printf("%02d ",secs);
      }

   else

      {
      printf("%03d:",shortmins);
      printf("%02d ",secs);
      } 
      
   }

int main(int argc, char **argv)
 
   {
   kvmt *proc_kvm;
   struct pentry *proc_entry;
   struct passwd *pw_s;
   int i, pstate, p_uid, columns, num_chars;
   char **name;

   getopt_restart();
   getopts(argc,argv);       /* process command line options */

   if (w_counter == 0)       /* w option - width of screen */
      w_counter = 80;
   else if (w_counter == 1)
      w_counter = 132;
   else
      w_counter = INT_MAX;
         
   name = calloc (1024l,sizeof (char *));       /* up to 1024 usernames */
   pw_s = (struct passwd *) malloc (sizeof(struct passwd));

   proc_kvm = kvm_open();

     /* Print header */

   printf("   ID");
   columns += 5;

   if (long_list)

      {
      printf("  PPID");
      columns += 6;
      }

   printf(" STATE     TT");
   columns += 13;

   if (long_list)
    
      {
      printf(" MMID");
      columns += 5;
      }

   printf(" USER");
   columns+=5;

   if (showname)
     
      {
      if (long_list)

         {
         printf("         ");
         columns += 10;
         }
 
      else

         {
         printf("       ");
         columns += 7;
         }
      
      }

   else   /* !showname */

      {
      if (long_list)

         {
         printf("     ");
         columns += 5;
         }
      
      else

         {
         printf("   ");
         columns += 3;
         }

      }

   printf("TIME COMMAND\n");
   columns+=6;

   do                                           /* while proc_entry != NULL */
 
      {
      i = 0;
      proc_entry = kvmnextproc(proc_kvm);       /* get next proc entry */ 
      
      if (proc_entry == NULL)                   /* no more processes */
         continue;
         
      p_uid = proc_entry->p_uid;
      
      if ((tty != proc_entry->ttyID) && (showtty));

      else if ((username == p_uid) || (allprocs) || (showtty))

         {
         printf("%5d ",proc_kvm->pid);          /* pid */
         if (long_list)
            printf("%5d ",proc_entry->parentpid); /* pid of parent process */
   
            /* if processState is screwed, call it 'unknown' */

         if ((proc_entry->processState < 0) || (proc_entry->processState > 9))
            pstate = 10;
         else
            pstate = proc_entry->processState;

         printf("%-9s ",status[pstate]);        /* process state */
         
         if (proc_entry->ttyID == 3)            /* ttyID   */
            printf("co ");                      /* console */
         else
            printf("%02d ",proc_entry->ttyID);

         if (long_list)
            printf("%4X ",proc_entry->userID);  /* MMID */
      
         if (showname)                          /* -n */
        
            {
            if (name[p_uid] == NULL)

               {
               pw_s = getpwuid(p_uid);          /* username */
               name[p_uid] = (char *) malloc (strlen(pw_s->pw_name) + 1);
               strcpy(name[p_uid],pw_s->pw_name);
               }

            printf("%-8s ",name[p_uid]);
            }

         else                                   /* default--no name, just id */
            printf("%04d ",p_uid);

         print_time(proc_entry->ticks);         /* time process has run */

         num_chars = (w_counter - columns) + 5;
         for (i = 8; i < num_chars; i++)        /* commandline */

            {
            if (proc_entry->args == NULL)

               {
               printf("forked child of process %d",proc_entry->parentpid);
               break;
               }

            else if (proc_entry->args[i] == '\0')
               break;
            else
               putchar(proc_entry->args[i]);
            }
 
         if (i == num_chars)
            printf("...");                 
         printf("\n");
         }
	
      }

   while(proc_entry != NULL);                   /* handle all proc entries */

   kvm_close(proc_kvm);
   free(pw_s);
   rexit(0);
   }
