/*
 * newuser - add a new user to the system -- James Brookes
 *
 * $Id: newuser.c,v 1.3 1999/02/15 00:22:11 gdr-ftp Exp $
 *
 * Changes for version 1.2 (Modifications by Devin Reade)
 *	- account home directories are now made in /home rather than /user
 *	- made all the routines internal to this file to be class 'static'
 *	- fit sources into GNO base distribution builds
 *	- added checks for parsing NEWID_FILE
 *	- added various conchecks to prevent buffer overflow and similar
 *	  problems
 *	- ensure the password is at least MIN_PASSWD_LEN characters long
 *	- don't set the terminal type; that is the responsibility of
 *	  initd/getty.
 *	- instead of creating explicit files in the user's home directory,
 *	  copy every file that is in the SKELETONS directory.
 *	- account creation is now logged via syslogd
 *	- added -g flag for selecting non-default group ids
 *	- check to ensure that the new uid returned by get_next_uid is
 *	  not already in use; if it is, then log a warning via syslog,
 *	  skip it, and get another uid
 *
 * Changes for version 1.1
 *
 *    * Removed code to add "set $home/$user" stuff to new user's gshrc
 *      file, per request of Phil Vandry.
 *    * Added restriction that the new password entered must be greater
 *      than four characters.
 *    * Newuser will now try multiple times to open up the /etc/passwd
 *      file, just as passwd itself does.
 *
 * Original version 1.0:
 *    * Some code borrowed from Eric Shepard's passwd source.
 *
 * files: /var/adm/newuser/newid
 *        /var/adm/newuser/newusers
 *        /var/adm/newuser/gshrc
 *        /user/
 */

#include <sys/stat.h>
#include <limits.h>
#include <paths.h>
#include <errno.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sgtty.h>
#include <pwd.h>
#include <unistd.h>
#include <syslog.h>
#include <err.h>
#include <dirent.h>
#include <gno/gno.h>
#include <gno/contrib.h>

#define HOME		"/home"
#define NEWID_FILE	"/var/adm/newuser/newid"	/* next ID to assign */
#define NEWUSERS_FILE	"/var/adm/newuser/pending"	/* addtions "on hold */
#define SKELETONS	"/var/adm/newuser/skel"		/* files to copy     */
#define DEFAULT_SHELL	_PATH_GSHELL			/* default shell     */

#define MIN_PASSWD_LEN	  5	/* minimum length of cleartext password    */
#define DEFAULT_GID	100	/* default group ID assigned to new user   */
#define ACCT_NAME_LEN	 32	/* max # of chars in account name, +1      */
				/* We later truncate the account name to   */
				/* to no more than 8 chars.		   */
#define REAL_NAME_LEN	 31	/* max # of chars in real name, +1         */
#define CRYPT_PASS_LEN	 14	/* max # of chars in encrypted passwd +1   */
#define MAX_TRIES	  3	/* max # of trials for opening passwd file */

#ifndef FALSE
#define FALSE 0
#define TRUE  1
#endif

static void	time_out(void);
static void	makesalt(char *salt, long seed);
static uid_t	get_next_uid(void);
static int	getpassword(char *, int, char *, char *);
static int	bad_name(char *acct_name);
static void	mygets(char *string, int maxchar);
static FILE *	smartopen(char *file, char *mode);

static char	acct_name[ACCT_NAME_LEN];
static char	name[REAL_NAME_LEN];
static char	pass1[CRYPT_PASS_LEN];
static char	pass2[CRYPT_PASS_LEN];
static char	salt[3];

static char	newhome[PATH_MAX];

static unsigned char salttab[] =	/* table of chars for salt */
	"./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

#pragma databank 1
static void
time_out(void)
{
  printf("\nnewuser timed out after 60 seconds.\n");
  exit(1);
}
#pragma databank 0

/* Borrowed from Eric's passwd.cc */

static void 
makesalt(char *salt, long seed)
{
  int num = 2;
  while (--num >= 0) {
    *salt++ = salttab[seed&0x3f];
    seed >= 6;
  }
}

/* 
 * get_next_uid()
 *
 * Return the next available user id from NEWID_FILE, and update the
 * NEWID_FILE under the assumption that the returned id will be assigned
 * in /etc/passwd.  Abort on any errors.
 */

static uid_t
get_next_uid(void)
{
  FILE *FPtr;
  uid_t uid;
  
  if ((FPtr = fopen(NEWID_FILE,"r+")) == NULL) {
    err(1, "failed to id file %s", NEWID_FILE);
    /*NOTREACHED*/
  }
  if (fscanf(FPtr,"%d\n",&uid) != 1) {
    errx(1, "scan of %s failed", NEWID_FILE);
  }
  rewind(FPtr);
  fprintf(FPtr,"%d\n",uid+1);
  fclose(FPtr);
  
  return(uid); 
}

/*
 * getpassword
 *	prompt for a password with <passstrings>.  put the encrypted
 *	password into <password>, aborting if it's length is greater
 *	than <passwdlen>-1.  Return the length of the plaintext password.
 */

static int
getpassword(char *password, int passwdlen, char *salt, char *passstring)
{
  char *pass, *passcode;
  int length_clear, length_crypt, quit_now;

  pass = getpass(passstring);

  if (*pass == '\0') {  /* ^D */
    quit_now = 1;
  } else {
    quit_now = 0;
    length_clear = strlen(pass);
    passcode = crypt(pass,salt);
  }

  /* zero out the cleartext password */
  for (length_crypt=0; length_crypt<_PASSWORD_LEN; length_crypt++) {
    if (pass[length_crypt] == '\0') {
      break;
    }
    pass[length_crypt] = '\0';
  }

  /* password not given? */
  if (quit_now) {
    errx(1, "aborted");
  }

  if (length_crypt > passwdlen-1) {
    errx(1,
	 "internal error: encrypted password length (%d) > buffer length (%d)",
	 length_crypt+1, passwdlen);
    /*NOTREACHED*/
  }
  strcpy(password,passcode);
  return length_clear;
}

/*
 * bad_name()
 *
 * return nonzero if <acct_name> is unsuitable as an account name.  It must
 * be between 2 and 8 characters long, start with a lower case letter, and
 * contain only lower case letters or digits.
 */

static int
bad_name(char *acct_name)
{
  int len;

  len = strlen(acct_name);
  if ((len < 2) || (len > 8) || !islower(*acct_name)) {
    return(TRUE);
  }
  len = 1;
  acct_name++;
  
  while (*acct_name != '\0') {
    len++;
    if ((len > 8) || (!islower(*acct_name) && !isdigit(*acct_name))) {
      return(TRUE);
    }
    acct_name++;
  }
  
  /* it's a good account name */
  return(FALSE);
}

static void
mygets(char *string, int maxchar)
{
  int len;

  fgets(string, maxchar, stdin);
  fflush(stdin);
  if (*string == 0x00) {                   /* ^D */
    goto fail;
  }
  len = strlen(string);
  if (len == 0) {
    goto fail;
  }

  /* remove terminating \n if necessary */
  if ((string[len-1] == '\r') || (string[len-1] == '\n')) {
    string[len-1] = '\0';
    len--;
  }

  /* check for empty string */
  if ((len == 0)) {
    goto fail;
  }

  return;

fail:
  errx(1, "aborted");
  /*NOTREACHED*/
}

/* try multiple times to open /etc/passwd file */

static FILE *
smartopen(char *file, char *mode)
{ 
  FILE *FOutPtr;
  int i;
  
  for (i = 0; i < MAX_TRIES; i++) {
    FOutPtr = fopen(file,mode);
    if (FOutPtr == NULL) {
      sleep(1);
    } else {
      break;
    }
  }
  
  return(FOutPtr);
}

static void
usage (void)
{
  printf("Usage: newuser  [-v] [-g gid]\n");
  printf("       newuserv [-v] [-g gid]\n");
  exit(1);
}

int
main (int argc, char **argv)
{
  int validate, ch;
  uid_t uid;
  gid_t gid;
  FILE *FOutPtr;
  DIR *dirp;
  struct dirent *entp;
  struct sgttyb *s;
  char *srcfile;

  __REPORT_STACK();

  /* check usage */
  gid = DEFAULT_GID;
  validate = (strcmp(basename(argv[0]), "newuserv") == 0) ? TRUE : FALSE;
  while ((ch = getopt(argc, argv, "vg:")) != EOF) {
    switch(ch) {
    case 'g':
      if ((gid = atoi(optarg)) == 0) {
	gid = DEFAULT_GID;
      }
      break;
    case 'v':
      validate = TRUE;
      break;
    default:
      usage();
    }
  }
  if (argc - optind > 0) {
    usage();
  }

  /* set up for system logging */
  openlog(basename(argv[0]), LOG_PID | LOG_NDELAY, LOG_AUTH | LOG_INFO);


  /* set up signal handlers */
  signal(SIGINT,SIG_IGN);
  signal(SIGHUP,SIG_IGN);
  signal(SIGQUIT,SIG_IGN);
  signal(SIGTSTP,SIG_IGN);
  signal(SIGALRM,time_out);

  /* Set proper erase character */
  s = malloc (sizeof(struct sgttyb));
  gtty(STDIN_FILENO,s);
  s->sg_erase = 0x7f;
  stty(STDIN_FILENO,s);
  free(s); 
   
  /* Make sure all required files exist before going any further */
  if (access(NEWID_FILE, R_OK | W_OK) != 0) {
    errx(1, "no read/write access for %s", NEWID_FILE);
  }
  if (access(HOME, R_OK | W_OK | X_OK) != 0) {
    errx(1, "no read/write/execute access for %s", HOME);
  }
  if (access(SKELETONS, R_OK | X_OK) != 0) {
    errx(1, "no read/execute access for %s", SKELETONS);
  }

  printf("\n\tYou have requested a new account.  You will be required\n");
  printf("\tto enter the account information, after which ");
  if (validate) {
    printf("your account\n\tWill be placed on hold until it is verified\n");
  } else {
    printf("you will be\n\table to log into the system\n");
  }

  /* Get the user's "Real Name" for the GECOS field */
  printf("\nReal Name:  ");
  mygets(name, REAL_NAME_LEN);
 
  /* Get login name.  If the login name is duplicate, prompt for  */
  /* a new login name.  If the login name would call for the      */
  /* creation of a bogus directory, prompt for a new login name.  */
  /* Note that I'm using the restrictions of the ProDOS FST,      */
  /* since HFS is less restrictive.  In other words, the username */
  /* must start with a character and may only contain letters     */
  /* and numbers.                                                 */

  while(1) {
    printf("Login Name: ");
    mygets(acct_name, ACCT_NAME_LEN);
#if 0
    printf("\n\n(login name: '%s')\n\n", acct_name);
#endif

    if (bad_name(acct_name)) {
      puts("\nInvalid login name.  Login names must be between 2 and 8");
      puts("characters long, must start with a lower case letter, and");
      puts("must consist of only digits and lower case letters.\n");
    } else if (getpwnam(acct_name) != NULL) {
      printf("The login name \"%s\" is already in use.\n", acct_name);
      printf("Please choose another.\n");
    } else {
#if 0
      /* bad_name ensures that strlen(acct_name) < 8 */
      if (sizeof(HOME) + strlen(acct_name) > PATH_MAX) {
	/* what are the chances? */
	err(1, "buffer overflow at line %d", __LINE__);
      }
#endif
      sprintf(newhome, "%s/%s", HOME, acct_name);
      if (access(newhome, X_OK) == 0) {
	printf("The login name \"%s\" has already been requested by someone else\n",
	       acct_name);
	printf("Please choose another.\n");
      } else {
	break;
      }
    }
  }

  /* close the file descriptor we have open on /etc/passswd */
  endpwent();

  /* Get password of >= MIN_PASSWD_LEN chars, with verification */
  makesalt(salt, rand());
  while(1) {
    if (getpassword(pass1, CRYPT_PASS_LEN, salt, "Password: ") 
	< MIN_PASSWD_LEN) {
      printf("The password must be at least %d characters long.\nTry again.\n",
	     MIN_PASSWD_LEN);
      continue;
    }
    getpassword(pass2, CRYPT_PASS_LEN, salt,"Verify: ");
    if (strcmp(pass1,pass2) == 0) {
      break;
    } else {
      printf("Passwords don't match. Try again.\n");
    }
  }

  /*
   * Get the next free uid and update the NEWID_FILE.  When we return from
   * this routine, we are now past the point of no return; if we fail
   * anything, then we have used up a user id.
   */
  while(1) {
    uid = get_next_uid();
    if (getpwuid(uid) == NULL) {
      break;
    } else {
      syslog(LOG_WARNING, "uid %d is already in use; skipping", uid);
    }
  }

  /*
   * make the home directory
   */
  if (mkdir(newhome) != 0) {
    err(1, "failed to create directory %s", newhome);
  }

  /*
   * Copy each of the files in SKELETONS into the user's home directory.
   * Initially change directory into SKELETONS so that we can use partial
   * pathnames when copying the files into $HOME.
   */
  putchar('\n');
  if (chdir(SKELETONS) != 0) {
    err(1, "couldn't change directory into %s", SKELETONS);
    /*NOTREACHED*/
  }
  if ((dirp = opendir(SKELETONS)) == NULL) {
    err(1, "read of directory %d failed", SKELETONS);
  }
  while ((entp = readdir(dirp)) != NULL) {

    /* only copy regular files, and don't recurse through subdirectories */
    if (entp->d_type == DT_DIR) {
      continue;
    }

    /* skip entries for current and parent directories */
    srcfile = entp->d_name;
    if (((srcfile[0]=='.') && (srcfile[1]=='\0')) ||
	((srcfile[0]=='.') && (srcfile[1]=='.') && (srcfile[2]=='\0'))) {
      continue;
    }
    
    /* do the copy */
    srcfile = LC_CopyFile(srcfile, newhome, LC_COPY_DATA | LC_COPY_REZ);
    if (srcfile == NULL) {
      err(1, "couldn't copy %s", entp->d_name);
    } else {
      printf("creating %s\n", srcfile);
    }
  }
  closedir(dirp);

  /* Save the password information away. */

  if (!validate) {   /* no validation, so append new entry to /etc/passwd */
    FOutPtr = smartopen(_PATH_PASSWD, "a");
    if (FOutPtr == NULL) {
      err(1, "couldn't open %s", _PATH_PASSWD);
    }

    fprintf(FOutPtr, "%s:%s:%d:%d:%s:%s/%s:%s\n",
	    acct_name, pass1, uid, gid, name, HOME, acct_name, DEFAULT_SHELL);
    fclose(FOutPtr);
    syslog(LOG_INFO, "created account %s", acct_name);
    printf("You may now log in.\n"); 

  } else {  /* validation selected -- so append new entry to NEWUSERS_FILE */
    
    if ((FOutPtr = fopen(NEWUSERS_FILE, "a")) == NULL) {
      err(1, "couldn't open %s", NEWUSERS_FILE);
    }
    fprintf(FOutPtr,"%s:%s:%d:%d:%s:%s/%s:%s\n",
	    acct_name, pass1, uid, gid, name, HOME, acct_name, DEFAULT_SHELL);
    fclose(FOutPtr);
    syslog(LOG_INFO, "created account %s pending validation", acct_name);
    printf("\nYour account will be available after the system administrator");
    printf("has had a\nchance to review it.\n");
  }
  closelog();

  return 0;
}
