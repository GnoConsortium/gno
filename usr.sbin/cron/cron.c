/* Converted to use syslog() by Phillip Vandry
   #define NO_SYSLOG to get the old version     */

#pragma stacksize 768
#pragma optimize -1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gno/gno.h>
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <sys/syslog.h>
#include "pathnames.h"

#define DEBUG(x)
/* #define DEBUG(x) x */

#define MAX_INTLIST 10

#define IT_ITEM		0
#define IT_RANGE	1
#define IT_LIST		2
#define IT_ALL		3

#ifdef __ORCAC__
#define NL 		13
#else
#define NL		10
#endif

typedef struct intlist {
    int	itype;
    int count;
    int intarr[MAX_INTLIST];
} intlist;

typedef struct ctentry {
    struct ctentry *next;
    intlist minute;
    intlist hour;
    intlist day;
    intlist month;
    intlist weekday;
    char *user;
    char *cmd;
} ctentry;

struct ctentry *crontab = NULL;

#ifdef NO_SYSLOG
FILE *cl;

void cronlog(char *s)
{
time_t t;
char *s1;

    time(&t);
    s1 = ctime(&t);
    s1[strlen(s1)-1] = 0;
    fprintf(cl,"%s: %s\n",s1,s);
}
#endif

void disposecrontab(void)
{
struct ctentry *x = crontab,*x2;

    while (x) {
	free(x->user);
        free(x->cmd);
        x2 = x->next;
        free(x);
        x = x2;
    }
    crontab = NULL;
}

char rcbuf[256];

int scanws(int i)
{
    while (!(isspace(rcbuf[i]))) i++;
    while (isspace(rcbuf[i])) i++;
    return i;
}

int scan2ws(int i)
{
    while ( !( isspace(rcbuf[i++]) ) );
    return i;
}

int scandig(int i)
{
    while ( isdigit(rcbuf[i])) i++;
    return i;
}

int parseItem(int ind, struct intlist *i)
{
int nind;
int count;
	
    if (rcbuf[ind] == '*')
	i->itype = IT_ALL;
    else {
	i->itype = IT_ITEM;
        sscanf(rcbuf+ind,"%d",&i->intarr[0]);
	    nind = scandig(ind);
        if (rcbuf[nind] == ',') {
	    ind = nind;
	    count = 1;
            while (rcbuf[ind] == ',') {
	        sscanf(rcbuf+ind+1,"%d",&i->intarr[count]);
                ind = scandig(ind+1);
		count++;
            }
            i->itype = IT_LIST;
            i->count = count;
        }
        else if (rcbuf[nind] == '-') {
	    sscanf(rcbuf+nind+1,"%d",&i->intarr[1]);
            ind = nind;
            i->itype = IT_RANGE;
        }
    }
    return scanws(ind);
}

void readcrontab(void)
{
FILE *ct;
ctentry *n;
int ind,nind;

    if (crontab) disposecrontab();
retry:
    ct = fopen(PATH_crontab,"r");
    if (ct == NULL) {
#ifndef NO_SYSLOG
    syslog(LOG_ERR, "couldn't open crontab");
#else
    perror("couldn't open crontab");
        cronlog("couldn't open crontab");
#endif
        sleep(30);
        goto retry;
    }
    while (!feof(ct)) {
    	if (fgets(rcbuf,255,ct) == NULL) break;
	DEBUG(fprintf(stderr,"gotline %s",rcbuf));
    	if (rcbuf[0] == '#') continue;
        if (strlen(rcbuf) < 2) continue;
        rcbuf[strlen(rcbuf)-1] = 0;
        n = malloc(sizeof(ctentry));
    	if (crontab) n->next = crontab;
    	else n->next = NULL;
    	crontab = n;

    	ind = parseItem(0,&n->minute);
	ind = parseItem(ind,&n->hour);
    	ind = parseItem(ind,&n->day);
    	ind = parseItem(ind,&n->month);
    	ind = parseItem(ind,&n->weekday);
    	nind = scan2ws(ind);
    	n->user = malloc(nind-ind+1);
    	memcpy(n->user,rcbuf+ind,nind-ind);
    	n->user[nind-ind] = 0;
	ind = scanws(ind);
    	n->cmd = malloc(strlen(rcbuf)-ind+1);
    	strcpy(n->cmd,rcbuf+ind);
	for (ind = 0; ind < strlen(n->cmd); ind++)
	    if (n->cmd[ind] == '%') n->cmd[ind] = NL;
        rcbuf[0] = rcbuf[1] = 0;
    }
	fclose(ct);
}

int matchItem(struct intlist *i,int t)
{
int p;

    if (i->itype == IT_ALL) return 1;
    else if (i->itype == IT_ITEM) {
	if (t == i->intarr[0]) return 1;
        else return 0;
    }
    else if (i->itype == IT_RANGE) {
	if ((t >= i->intarr[0]) && (t <= i->intarr[1]))
	    return 1;
        else return 0;
    }
    else if (i->itype == IT_LIST) {
	for (p = 0; p < i->count; p++)
	    if (i->intarr[p] == t) return 1;
        return 0;
    }
}

int matchTime(ctentry *x,struct tm *t)
{
    if (matchItem(&x->minute,t->tm_min) &&
        matchItem(&x->hour,t->tm_hour) &&
        matchItem(&x->day,t->tm_mday) &&
        matchItem(&x->month,t->tm_mon) /* &&
        matchItem(&x->weekday,t->tm_wday) */ )
        return 1;
    return 0;
}

void printItem(struct intlist *i)
{
int p;
    if (i->itype == IT_ALL) fprintf(stderr,"* ");
    else if (i->itype == IT_ITEM)
	fprintf(stderr,"%d ",i->intarr[0]);
    else if (i->itype == IT_RANGE)
	fprintf(stderr,"%d-%d",i->intarr[0],i->intarr[1]);
    else if (i->itype == IT_LIST) {
	fprintf(stderr,"%d",i->intarr[0]);
        for (p = 1; p < i->count; p++)
	    fprintf(stderr,",%d",i->intarr[p]);
    }
}

void docrontab(void)
{
struct tm *t;
time_t t1;
ctentry *x = crontab;
char *cmdpath;
word ind = 0;

    t1 = time(NULL);
    t = localtime(&t1);
    if (x == NULL) { /* syslog(LOG_NOTICE, "crontab empty");*/ return; }
    while (x) {
        DEBUG(printItem(&x->minute));
        DEBUG(printItem(&x->hour));
        DEBUG(printItem(&x->day));
        DEBUG(printItem(&x->month));
        DEBUG(printItem(&x->weekday));
        DEBUG(fprintf(stderr,":%s \n",x->cmd));
	if (matchTime(x,t)) {
            DEBUG(fprintf(stderr,"matched!\n"));
	    ind = strpos(x->cmd,' ');
	    if (ind == -1) ind = strlen(x->cmd);
            cmdpath = malloc(ind+1);
	    memcpy(cmdpath,x->cmd,ind);
	    cmdpath[ind] = 0;
            exec(cmdpath,x->cmd);
        }
        x = x->next;
    }
}

int main(int argc, char *argv[])
{
time_t ct_mod,start_time;
unsigned int i;
struct tm *exptime;
static struct stat sb;
int sl;

/* Previously this was a for. ORCA/C didn't compile that right, though.
   This generates more efficient code anyway */
	i = 3;
	while (++i < 20) close(i);
#ifndef NO_SYSLOG
    openlog(NULL, 0, LOG_CRON);
#else
    cl = fopen(PATH_cronlog,"a");
    if (!cl) {
	    perror("cron: couldn't open cronlog file");
        exit(1);
    }
#endif

    /* turn off buffering so cronlog is up-to-date in the event of a
     * system crash
     */
#ifdef NO_SYSLOG
    setvbuf(cl,NULL,_IONBF,0l);
    cronlog("cron started up");
#else
    syslog(LOG_INFO, "cron started up");
#endif

    /*
     * start cron sessions on or near an exact minute
     * (giving 2 seconds leeway for 'bounce')
     */
    start_time = time(NULL);
    exptime = localtime(&start_time);
    sleep(62 - exptime->tm_sec); 

#pragma optimize 0  /* ORCA/C has problems... */
    ct_mod = 0l;
    while (1) {
		DEBUG(fprintf(stderr,"loop\n"));

        stat(PATH_crontab,&sb);
	    if (sb.st_mtime != ct_mod) {
	        ct_mod = sb.st_mtime;
            readcrontab();
        }
#pragma optimize 1
        docrontab();

        /*
         * account for the time we spent loading/parsing/executing
         */
        start_time = time(NULL);
        exptime = localtime(&start_time);
        sl = 61-exptime->tm_sec;
        /*while (sl < 0) sl+=60;*/
        DEBUG(fprintf(stderr,"sleep: %d\n",sl));
        sleep(sl);
    }
}
