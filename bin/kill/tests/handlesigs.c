/*
 *  handlesigs: a test program for GNO that waits for signals.
 *  It is designed to be run in the background and takes two optional
 *  parameters: location of kill command and name of test script to be
 *  created. From the shell do something like this:
 *
 *       handlesigs ../kill /tmp/testsigs > /tmp/siglist &
 *       sleep 10
 *       chtyp -l exec /tmp/testsigs
 *       /tmp/testsigs
 *       cat /tmp/siglist
 *
 *  (See the script /src/gno/bin/kill/tests/dotests.)
 *
 *  Written by Dave Tribby beginning January 20, 1998
 *  $Id: handlesigs.c,v 1.1 1998/02/08 23:47:13 tribby Exp $
 */

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <gno/gno.h>
#include <unistd.h>

/* Program name */
char		*prog_name;
/* GNO process ID */
pid_t		process_id;
/* Received signal */
int		received_sig=0;


/*
 * GNO Signal handler routine
 */
#pragma databank 1
void SigHandler(int sig, int code)
{
   /* Keep it simple: just set global variable */
   received_sig = sig;
}
#pragma databank 0


int main (int argc, char **argv)
{
/* Command-line parameters and their default values */
char	*def_cmdname="../kill",       *cmdname=def_cmdname;
char	*def_cmdfile="/tmp/killtest", *cmdfile=def_cmdfile;

int	i;
sig_t	*sig_rtnval;
FILE	*cf;

/* Get parameters: program name, location of kill command, */
/* and name of command script to be created.               */
prog_name = argv[0];
if (argc > 1)
   cmdname = argv[1];
if (argc > 2)
   cmdfile = argv[2];

/* Are we running under the GNO kernel? */
if ( !needsgno() )   {
   fprintf(stderr, "ERROR: %s can only run under the GNO kernel!\n", prog_name);
   return 1;
   }

/* Install the GNO signal handler for all possible signals */
fprintf(stderr,"Installing GNO signal handler routine for all interrupts...\n");
for (i = 1; i < NSIG; i++) {
   sig_rtnval = signal(i,  SigHandler);
   }                           

/* Get process ID */
process_id = getpid();

/* Create file to send test signals */
fprintf(stderr, "Creating test command file %s\n", cmdfile);
if ((cf = fopen(cmdfile, "w")) == NULL)
   err(1, "%s", cmdfile);

/* Send the default signal (15) */
fprintf(cf, "%s %d\n", cmdname,process_id);
/* Send all the legal signals, using alternate formats for first two */
for (i = 1; i < NSIG; i++) {
   switch (i) {
      case 1:
         fprintf(cf, "%s -s %s %d\n", cmdname,sys_signame[i],process_id);
         break;
      case 2:
         fprintf(cf, "%s -%s %d\n", cmdname,sys_signame[i],process_id);
      /* Leave out the signals that mess up the tests! */
      case 9:
      case 17:
      case 18:
      case 19:
         break;
      default:
         fprintf(cf, "%s -%d %d\n", cmdname,i,process_id);
      }
   }                           
/* Make KILL (9) the last signal sent */
fprintf(cf, "%s -s KILL  %d\n", cmdname,process_id);
fclose(cf);

/* Hang around waiting for a signal */
fprintf(stderr, "Waiting for a signal to occur. To terminate, use\n");
fprintf(stderr, "   kill -s KILL  %d\n", process_id);
while (1)  {
   procreceive();
   printf("Program %s received signal %2d", prog_name, received_sig);
   if (received_sig <= NSIG)  printf(" (%s)", sys_signame[received_sig]);
   printf("\n");
   received_sig = 0;
   fflush(stdout);
   }

return 0;
}
