/*      time.c
 *	return process execution time of command
 *	1.1 5/7/92 modified to use times() for higher accuracy
 *	1.0 jb	
 */

#pragma optimize -1
#pragma stacksize 1024

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/times.h>
#include <orca.h>

int main(int argc, char *argv[])
{
struct tms t;
time_t start,end;

    if (argc < 2) {
        fprintf(stderr,"usage: time [command]\n\r");
        exit(1);
    }

    start = time(NULL);
    system(commandline()+strlen(argv[0])+1);
    end = time(NULL);
    times(&t);
    fprintf(stderr," %8ld real %7.2f user %7.2f sys\n",
	end-start,t.tms_cutime/60.0,t.tms_cstime/60.0);
}
