/* Calendar file v1.0. Copyright 1994 by Christopher Neufeld       */
/* The executable for this file contains linked runtime libraries
   copyrighted by The Byte Works. Used with permission.	           */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include <ctype.h>

#pragma stacksize 512

#define NMONTHS 12
#define NDAYS 31
#define MAXLINELEN 255

#define CALFILE "calendar"

#define SECSPERDAY (24 * 60 * 60)

const char months[NMONTHS][] = {"jan", "feb", "mar", "apr", "may", "jun",
                                "jul", "aug", "sep", "oct", "nov", "dec"};
              
int main(void)
{
  FILE *ifile;
  time_t t1, t2;
  struct tm st1, st2;
  int monthnum, daynum, dayschk, i, j;
  char *ptr1, *ptr2, holdmnth[4];
  static char thislin[MAXLINELEN+1];
  long deltat;

  if ((ifile = fopen(CALFILE, "r")) == NULL) exit(0);
  t1 = time(NULL);
  st1 = *localtime(&t1);
  st1.tm_sec = st1.tm_min = st1.tm_hour = 0;
  t1 = mktime(&st1);
  dayschk = (st1.tm_wday >= 5) ? 8 - st1.tm_wday : 1;
  thislin[MAXLINELEN] = 0;
  while (!feof(ifile)) {
    fgets(thislin, MAXLINELEN, ifile);
    ptr1 = thislin;
    while (isspace(*ptr1) && *ptr1 != 0) ptr1++;
    if (*ptr1 == 0) continue;                        /* Blank line */
    monthnum = -1;
    if (isdigit(*ptr1)) {   /* month/day format */
      monthnum = strtoul(ptr1, &ptr2, 10) - 1;
      daynum = strtoul(ptr2+1, NULL, 10);
      if (monthnum < 0 || monthnum >= NMONTHS || daynum < 0 || daynum > NDAYS)
        continue;
    } else {
      for (i=0; i<3; i++) holdmnth[i] = tolower(ptr1[i]);
      holdmnth[3] = 0;
      for (i = 0; i < NMONTHS; i++)
        if (!(strcmp(holdmnth, months[i]))) {
          monthnum = i;
          break;
        }
      if (monthnum == -1) continue;
      while (!isspace(*ptr1) && *ptr1 != 0) ptr1++;
      if (*ptr1 == 0) continue;
      while (isspace(*ptr1) && *ptr1 != 0) ptr1++;
      if (*ptr1 == 0) continue;
      daynum = atoi(ptr1);
      if (daynum < 1 || daynum > NDAYS) continue;
    }
    st2.tm_sec = st2.tm_min = st2.tm_hour = st2.tm_isdst = 0;
    st2.tm_mday = daynum;
    st2.tm_mon = monthnum;
    st2.tm_year = st1.tm_year;
    t2 = mktime(&st2);
    if ((deltat = difftime(t2, t1)) < 0) {  /* Next year */
      st2.tm_year++;
      t2 = mktime(&st2);
      deltat = difftime(t2, t1);
    }
    if (deltat <= dayschk * SECSPERDAY) printf(thislin);
  }
  fclose(ifile);
  exit(0);
}
