/* 
    BINPRINT.C

    Displays files in hex-dump format (with ascii subdisplay if desired)

    v1.2  Optimized and fixed input from terminal bug (pv)
    v1.1  Added stacksize directive (jb)
    v1.0  Original version by Derek Taubert

*/

#pragma optimize -1
#pragma stacksize 1024

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>

extern FILE *fdopen(int,char *);

unsigned int doline(char *dest, char *source,
    unsigned int actual, unsigned int cols);

unsigned char *buffer2;

main(argc,argv)
int argc;
char **argv;
{
int duh;
int a;
int c,errflg = 0,columns = 16;
size_t pos = 0;
unsigned char *buffer;
extern char *optarg;
extern int optind;
extern int getopt(int,char **,char*);

    while ((c = getopt(argc,argv, "Vc:")) != EOF)
        switch (c) {
            case 'c' :
                columns = atoi(optarg);
                break;
	    case 'V' :
		fprintf(stdout, "binprint v1.2 for GNO/ME\n");
		exit(0);
            default : errflg++;
        }
    if (errflg) {
        fprintf(stderr,"usage: binprint [-c<columns>] files...\n");
        exit(2);
    }
    argv += optind;
    if (columns < 8) columns = 8;
    if ((buffer2 = (unsigned char *)malloc((size_t)(columns*4)+1)) == NULL) {
    	fprintf(stderr,"Cannot allocate buffer space\n");
    	exit(1);
    }
    buffer = (unsigned char *)(buffer2+(columns*3));
    if (optind == argc) {
        duh = STDIN_FILENO;
        goto action;
    }
    for (;optind<argc;optind++) {
        if ((duh = open(*argv,O_RDONLY|O_BINARY)) == -1) {
            fprintf(stderr,"error opening %s\n",*argv);
            exit(1);
        }
        printf("\n%s\n",*argv);
action:     while ((a = (int)read(duh, buffer,
		(size_t)(columns * sizeof(unsigned char)))) != 0) {
            printGood(pos,buffer,a,columns);
            pos += a;
        }
        close(duh);
    }
    free(buffer);
    free(buffer2);
    exit(0);
}

printGood(off,buf,real,form)
long off;
unsigned char *buf;
int real;
int form;
{
    if (!real) return;
/*
    printf("%8lX: ",off);
*/

/* The following is a hack required because of buffering by the stdio
   libraries. I wish it was not necesary... */

	{
		static char puthere[11];
		int howmany;
		
		howmany = sprintf(puthere,"%8lX: ",off);
		write(STDOUT_FILENO, puthere, (size_t)howmany);
	}

    write(STDOUT_FILENO, buffer2, (size_t)doline(buffer2,buf,form,real));
}
