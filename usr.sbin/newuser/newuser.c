/*                                                                          */
/* newuser - add a new user to the system, v1.1 -- James Brookes            */
/*                                                                          */
/* Changes from 1.0                                                         */
/*                                                                          */
/*    * Removed code to add "set $home/$user" stuff to new user's gshrc     */
/*      file, per request of Phil Vandry.                                   */
/*    * Added restriction that the new password entered must be greater     */
/*      than four characters.                                               */
/*    * Newuser will now try multiple times to open up the /etc/passwd      */
/*      file, just as passwd itself does.                                   */
/*                                                                          */
/*    Some code borrowed from Eric Shepard's passwd source.                 */
/*                                                                          */
/* files: /var/adm/newuser/newid                                            */
/*        /var/adm/newuser/newusers                                         */
/*        /var/adm/newuser/gshrc                                            */
/*        /user/                                                            */
/*                                                                          */

/*#pragma optimize -1 */
#pragma stacksize 512

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <sgtty.h>
#include <gno/gno.h>
#include <pwd.h>
#include <unistd.h>

#pragma lint -1

char *getpass(char *prompt);
char *crypt(char *key, char *salt);

void  time_out(void);
void  makesalt(char *salt, long seed);
int   get_next_uid(void);
void  getpassword(char *password, char *salt, char *passstring);
int   bad_name(char *acct_name);
void  myfgets(char *string, int maxchar, FILE *FilePtr);
FILE *smartopen(char *file, char *mode);

#define NEWID_FILE    "/var/adm/newuser/newid"
#define NEWUSERS_FILE "/var/adm/newuser/newusers"
#define NEWGSHRC_FILE "/var/adm/newuser/gshrc"

#define DEFAULT_GID   2         /* default group ID assigned to new user   */
#define ACCT_NAME_LEN 8         /* max # of chars in account name          */
#define REAL_NAME_LEN 30        /* max # of chars in real name             */
#define TERM_TYPE_LEN 8         /* max # of chars in terminal type         */
#define MAX_TRIES     3         /* max # of trials for opening passwd file */

#ifndef FALSE
#define FALSE 0
#define TRUE  1
#endif

static char acct_name[ACCT_NAME_LEN+1],
            buffer[80],
            name[REAL_NAME_LEN+1],
            pass1[14], pass2[14],
            salt[3],
            scratch[256],
            term_type[TERM_TYPE_LEN+1];

static unsigned char salttab[] =	/* table of chars for salt */
	"./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

void time_out(void)

   {
   printf("\nnewuser timed out after 60 seconds.\n");
   exit(1);
   }

/* Borrowed from Eric's passwd.cc */

void makesalt(char *salt, long seed)

   {
   int num = 2;
   while (--num >= 0)

      {
      *salt++ = salttab[seed&0x3f];
      seed >= 6;
      }

   }

int get_next_uid(void)

   {
   FILE *FPtr;
   int uid;

   FPtr = fopen(NEWID_FILE,"r+");
   fscanf(FPtr,"%d\n",&uid);
   rewind(FPtr);
   fprintf(FPtr,"%d\n",uid+1);
   fclose(FPtr);

   return(uid); 
   }

void getpassword(char *password, char *salt, char *passstring)

   {
   char *pass, *passcode;

   pass = getpass(passstring);
   passcode = crypt(pass,salt);
   strcpy(password,passcode);
   }

int bad_name(char *acct_name)

   {
   if (!isalpha(*acct_name++))
      return(TRUE);
 
   while (*acct_name != '\0')
      if (!isalnum(*acct_name++))
         return(TRUE);

   return(FALSE);
   }

void myfgets(char *string, int maxchar, FILE *FilePtr)

   {
   int last_char;
   char *tmp_buf;

   tmp_buf = (char *) malloc (256);

   maxchar++;
   fgets(tmp_buf,maxchar,FilePtr);
   if (*tmp_buf == 0x00)                   /* ^D */
      exit(1);
   last_char = strlen(tmp_buf)-1; 

      /* remove terminating \n if necessary */

   if ((tmp_buf[last_char] == '\n') || (tmp_buf[last_char] == '\r'))
      tmp_buf[last_char] = '\0';

   strcpy(string,tmp_buf);
   fflush(stdin);
   free(tmp_buf);
   }

/* try multiple times to open /etc/passwd file */

FILE *smartopen(char *file, char *mode)

   { 
   FILE *FOutPtr;
   int i;

   for (i = 0; i < MAX_TRIES; i++)   
   
      {
      FOutPtr = fopen(file,mode);
      if (FOutPtr == NULL)
         sleep(1);
      else
         break;
      }

   return(FOutPtr);
   }

int main (int argc, char **argv)

   {
   int validate, uid;
   FILE *FInPtr, *FOutPtr;
   
   struct sgttyb *s;
   s = (struct sgttyb *) malloc (sizeof(struct sgttyb));

   signal(SIGINT,SIG_IGN);
   signal(SIGHUP,SIG_IGN);
   signal(SIGQUIT,SIG_IGN);
   signal(SIGTSTP,SIG_IGN);
   signal(SIGALRM,time_out);

      /* Set proper erase character */

   gtty(STDIN_FILENO,s);
   s->sg_erase = 0x7f;
   stty(STDIN_FILENO,s);
   free(s); 
   
   validate = FALSE;

   if (argc == 2 && !strcmp(argv[1],"-v"))
      validate = TRUE;
   else if (argc == 1);
   else
      exit(1);
   
     /* Make sure all required files exist before going any further */

   if ((FInPtr = fopen(NEWID_FILE,"r+")) == NULL)
   
      {
      fprintf(stderr,"unable to open %s; exiting.\n",NEWID_FILE);
      exit(1);
      }

   fclose(FInPtr);

   if ((FInPtr = fopen(NEWGSHRC_FILE,"r+")) == NULL)
   
      {
      fprintf(stderr,"unable to open %s; exiting.\n",NEWGSHRC_FILE);
      exit(1);
      }

   fclose(FInPtr);

      /* Get information */

   printf("\nReal Name: ");
   myfgets(name,REAL_NAME_LEN,stdin);
 
      /* Get login name.  If the login name is duplicate, prompt for  */
      /* a new login name.  If the login name would call for the      */
      /* creation of a bogus directory, prompt for a new login name.  */
      /* Note that I'm using the restrictions of the ProDOS FST,      */
      /* since HFS is less restrictive.  In other words, the username */
      /* must start with a character and may only contain letters     */
      /* and numbers.                                                 */

   while(1)

      {
      printf("Login Name: ");
      myfgets(acct_name,ACCT_NAME_LEN,stdin);
      printf("\n\n(login name: '%s')\n\n",acct_name);
      if (getpwnam(acct_name) != NULL)
         printf("Duplicate username: please choose another.\n");
      else if (bad_name(acct_name))

         {
         printf("\n** Invalid username: please choose a name comprised of\n");
         printf("   alphanumeric characters which starts with an alphabetic\n");
         printf("   character.\n");
         }
         
      else      
         break;
      }

   printf("Terminal Type: ");
   myfgets(term_type,TERM_TYPE_LEN,stdin);

      /* Get password of > 4 chars, with verification */

   makesalt(salt, rand());
   while(1) 

      {
      getpassword(pass1,salt,"Password: ");
      getpassword(pass2,salt,"Verify: ");
      if (!strcmp(pass1,pass2) && (strlen(pass1) > 4))
         break;
      else
         printf("*** Failed verification.\n");
      }

   uid = get_next_uid();        /* get and update next free ID# */
  
      /* make home directory */

   sprintf(scratch,"mkdir -s /user/%s",acct_name);
   exec("/bin/mkdir",scratch);
   sleep(2);

      /* and copy default gshrc to it */

   sprintf(scratch,"/user/%s/gshrc",acct_name);
   FOutPtr = fopen(scratch,"w");

   FInPtr = fopen(NEWGSHRC_FILE,"r");

   while(fgets(buffer,80,FInPtr) != NULL)
      fputs(buffer,FOutPtr);
   fclose(FInPtr);   

      /* update default gshrc to have correct $home, $user, and $term */

/*   Phil asked that this be removed, so... :)
   fprintf(FOutPtr,"set home=/user/%s\n",acct_name);
   fprintf(FOutPtr,"set user=%s\n",acct_name);
*/

   fprintf(FOutPtr,"set term=%s\n",term_type);

/*
   fprintf(FOutPtr,"export home user term\n");
*/
   fprintf(FOutPtr,"export term\n");
   fclose(FOutPtr);

   if (!validate)   /* no validation, so append new entry to /etc/passwd */

      {
      FOutPtr = smartopen("/etc/passwd","a");
      if (FOutPtr == NULL)
         
         {
         fprintf(stderr,"Trouble opening /etc/passwd file.\nExiting\n"); 
         exit(1);
         }

      fprintf(FOutPtr,"%s:%s:%d:%d:%s:/user/%s:/bin/gsh\n",acct_name,pass1,
           uid,DEFAULT_GID,name,acct_name);
      fclose(FOutPtr);
      printf("You may now log in.\n"); 
      }

   else  /* validation selected -- so append new entry to NEWUSERS_FILE */

      {
      FOutPtr = fopen(NEWUSERS_FILE,"a");
      fprintf(FOutPtr,"%s:%s:%d:0:%s:/user/%s:/bin/gsh\n",acct_name,pass1,
           uid,name,acct_name);
      fclose(FOutPtr);
      printf("Try back in 24 hours.\n");
      }

   exit(0);
   }
