/*
 * Calendar file v1.0. Copyright 1994-1998 by Christopher Neufeld
 *
 * The executable for this file contains linked runtime libraries
 * copyrighted by The Byte Works. Used with permission.
 *
 * Change Log:
 *
 * 1.0 -- [Christopher Neufeld, 1994]  Initial version.
 *
 * 1.1 -- [Marlin Allred, 1 Jul 1998]
 *        + the date can now appear anywhere on the line
 *        + on the weekend, Monday events are also listed
 *        + man page was converted from preformatted ASCII to nroff source
 *        + binary linked with the v2.0.6 libraries
 *
 * 1.2 -- [Devin Reade, 6 Jul 1998]
 *        + fixed a bug introduced in 1.1 where getting an error while
 *          reading the calandar file could cause an infinite loop
 *        + added the describe entry and resource fork
 *        + added these "ChangeLog" comments for all versions
 *        + added this program to the GNO base distribution
 *        + eliminated "pragma stacksize" from the source file; it was too
 *	    small (was 512 bytes, currently using 862 bytes) and overriding
 *	    the value given on the command line during the GNO base build
 *          process
 *
 * $Id: calendar.c,v 1.3 1998/07/07 02:14:30 gdr-ftp Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <err.h>
#ifdef __GNO__
#include <gno/gno.h>
#endif

#define NMONTHS 12
#define NDAYS 31
#define MAXLINELEN 255

#define CALFILE "calendar"

#define SECSPERDAY (24 * 60 * (long)60)

const char months[NMONTHS][4] = {"jan", "feb", "mar", "apr", "may", "jun",
                                "jul", "aug", "sep", "oct", "nov", "dec"};
              
#ifdef NODIFFTIME
#define difftime(x1,x2) (double)((x1) - (x2))
#endif        /* Note that this is not necessarily portable, but works with */
              /* some systems which don't have 'difftime'                   */

int main(void)
{
  FILE *ifile;
  time_t t1, t2;
  static struct tm st1, st2;
  int monthnum, daynum, dayschk, i, j;
  char *ptr1, *ptr2, holdmnth[4];
  static char thislin[MAXLINELEN+1];
  long deltat;
  long ldayschk;

#ifdef __GNO__
  __REPORT_STACK();
#endif

  if ((ifile = fopen(CALFILE, "r")) == NULL) exit(0); /* Open calendar file
                       in CWD. If there is none, exit successfully */
  t1 = time(NULL);  /* Get the current time */
  st1 = *localtime(&t1);  /* Convert to formatted date/time */
  st1.tm_sec = st1.tm_min = st1.tm_hour = 0;  /* Pretend it's midnight, it
                       makes the checking later much easier   */
  t1 = mktime(&st1);   /* Make an internal representation for that midnight
                       (the one which heralded today's date)  */
  dayschk = (st1.tm_wday >= 5) ? 8 - st1.tm_wday : 2;  /* Check today and
                       tomorrow, unless tomorrow is on the weekend, in
                       which case we check up to and including Monday  */
  thislin[MAXLINELEN] = 0;

  /* Get each line from the calendar file */
  while (fgets(thislin, MAXLINELEN, ifile) != NULL) {
    ptr1 = thislin;
    while (isspace(*ptr1) && *ptr1 != 0) ptr1++;  /* Flush initial whitespace */
    if (*ptr1 == 0) continue;                        /* Blank line */
    monthnum = -1;
    while (*ptr1) {
    if (isdigit(*ptr1)) {   /* month/day format */
      monthnum = strtoul(ptr1, &ptr2, 10) - 1;
      daynum = strtoul(ptr2+1, NULL, 10);
                       /* We've now parsed a month/day format line */
      if (monthnum && monthnum < NMONTHS && daynum && daynum <= NDAYS)
          break;  /* Valid date, print line of the file */
    } else {
      for (i=0; i<3; i++) holdmnth[i] = tolower(ptr1[i]); /* make the search
                       case-insensitive */
      holdmnth[3] = 0;
      for (i = 0; i < NMONTHS; i++)
        if (!(strcmp(holdmnth, months[i]))) {
          monthnum = i;  /* look for "jan", "feb", etc. */
          break;
        }
      if (monthnum != -1) {  /* found a valid month, get day */
          while (!isspace(*ptr1) && *ptr1 != 0) ptr1++;  /* flush text */
          if (*ptr1 == 0) break;
          while (isspace(*ptr1) && *ptr1 != 0) ptr1++; /* flush whitespace */
          if (*ptr1 == 0) break;    /* No day number, go to next line in file */
          daynum = atoi(ptr1);   /* get day number */
          if (daynum >= 1 && daynum <= NDAYS) break;  /* Valid, go print line */
      }
    }
    ptr1++;
    }
    st2.tm_sec = st2.tm_min = st2.tm_hour = st2.tm_isdst = 0;
    st2.tm_mday = daynum;
    st2.tm_mon = monthnum;
    st2.tm_year = st1.tm_year;
                       /* We've now set up the time for midnight at the
                          beginning of the day represented by the line we
                          found in the calendar file */
    t2 = mktime(&st2);  /* Change to internal format */
    if ((deltat = difftime(t2, t1)) < 0) {  /* The day was in the past, check
                       the same date next year   */
      st2.tm_year++;
      t2 = mktime(&st2);
      deltat = difftime(t2, t1);
    }
    if (deltat <= dayschk * SECSPERDAY) printf(thislin);
      /* print the entire line if it is inside our acceptance window */
  }
  if (ferror(ifile)) {
    err(1, "error while reading %s", CALFILE);
  }
  fclose(ifile);
  exit(0);
}
