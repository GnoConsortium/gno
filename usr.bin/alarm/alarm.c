/*  Alarm v1.0. Copyright 1994 by Christopher Neufeld              */

#include <stdio.h>
#include <gno/gno.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include <ctype.h>

#pragma stacksize 512

#define BEEP putchar((char)0x07)
#define TBEEPS 2              /* Time between beeps in seconds */

#define MAXBEEPS 5

#define SECSPERDAY (60 * 60 * 24)

#define DIGTOI(x) ((int) ((x) - '0'))

void usage(char *exename)
{
  fprintf(stderr, "Alarm program. Copyright 1994 by Christopher Neufeld\n");
  fprintf(stderr, "Usage: %s [+]#### [<delay> [\"<message>\"]]\n", exename);
  fprintf(stderr, "Where #### is a time in hhmm format, ie. 0010 is ten past midnight\n");
  fprintf(stderr, "      +#### indicates that number of hhmm after the current time.\n");
  fprintf(stderr, "      delay is the time delay between beeps or messages, if any.\n");
  fprintf(stderr, "      message is delivered to stdout at alarm condition\n\n");
  fprintf(stderr, "This executable contains linked runtime libraries copyrighted\n");
  fprintf(stderr, "by The Byte Works. Used with Permission.\n\n");
  exit(1);
}


int main(int argc, char **argv)
{
  char *ptr1, *tptr, *msg;
  static struct tm timenow, timethen;
  static time_t curtime, alarmtime;
  static int i, mins, hrs, offset, message, deltat;
  unsigned long initwait;
  
  message = 0;
  deltat = TBEEPS;
  if (argc < 2 || argc > 4) usage(argv[0]);
  if (argc > 2) {
    deltat = (int) strtoul(argv[2], &ptr1, 0);
    if (*ptr1 != 0) usage(argv[0]);   /* Unable to parse delay time */
    if (argc == 4) {
      msg = argv[3];
      message = 1;
    }
  }
  tptr = argv[1] + (offset = (argv[1][0] == '+'));
  if (strlen(tptr) != 4) usage(argv[0]);
  for (i=0;i<4;i++)
    if (!isdigit(tptr[i])) usage(argv[0]);
  hrs = 10 * DIGTOI(tptr[0]) + DIGTOI(tptr[1]);
  mins = atoi(tptr + 2);
  if (mins > 59) {
    if (offset) hrs++;
    else usage(argv[0]);
  }
  if (hrs > 23 && !offset) usage(argv[0]);
  if (!offset) {
    curtime = time(NULL);
    timethen = *localtime(&curtime);
    timethen.tm_sec = 0;
    timethen.tm_min = mins;
    timethen.tm_hour = hrs;
    alarmtime = mktime(&timethen);
    initwait = difftime(alarmtime, curtime);
    if (initwait <= 0) initwait += SECSPERDAY;
  } else initwait = 60 * (mins + 60 * hrs);
  sleep(initwait);
  for (i=0;i<MAXBEEPS;i++) {
    BEEP;
    fflush(stdout);
    if (message) printf("%s\n", msg);
    sleep(deltat);
  }
  exit(0);
}
