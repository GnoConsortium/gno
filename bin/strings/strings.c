#pragma stacksize 1024

#include <types.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

const long bufSize = 512l;
int all;

int myprint(char c)
{
    if (all) {
        if (isprint(c) || (c==9)) return 1;
    } else {
        if ( ((c>64) && (c<91)) || ((c>96) && (c<123)) || ((c>47) && (c<58))
           || (c==32) || (c==9) ) return 1;
    }
    return 0;
}

main(int argc,char **argv)
{
FILE *duh;
int concur,c,errflg = 0,size,pos;
int inRow = 4,keepOff = FALSE;
unsigned char *buffer,*string;
unsigned long stringSize = 256l,offSet;
extern char *optarg;
extern int optind;
extern int getopt(int,char **,char*);

/*  optarg = NULL; optind = 0; */ /* this makes getopt restartable */
    all = FALSE;

    while ((c = getopt(argc,argv, "c:na")) != EOF) {
        switch (c) {
            case 'c' :
                inRow = atoi(optarg);
                break;
            case 'n' :
                keepOff = TRUE;
                break;
            case 'a' :
                all = TRUE;
                break;
            default : errflg++;
        }
    }
    if (errflg) {
        fprintf(stderr,"usage: strings [-a] [-c #chars] [-n] files...\n");
        exit(2);
    }
    if (inRow < 4) inRow = 4;
    printf("optind: %d argc: %d\n",optind,argc);
    argv += optind;
    buffer = (unsigned char *)malloc(bufSize);
    string = (unsigned char *)malloc(stringSize);
    if (optind == argc) {
        duh = stdin;
        goto action;
    }
    for (;optind<argc;optind++) {
        if ((duh = fopen(*argv,"rb")) == NULL) {
            fprintf(stderr,"error opening %s\n",*argv);
            exit(1);
        }
        argv++;
action: concur = 0;
        offSet = 0l;
        while (!feof(duh)) {
            size = fread(buffer,sizeof(unsigned char),bufSize,duh);
            for (pos = 0;pos < size;pos++,offSet++) {
                if (myprint(c=toascii(buffer[pos]))) {
                    string[concur] = c;
                    if (concur++ > stringSize) {
                        stringSize += 256l;
                        if ((string = realloc(string,stringSize)) == NULL)
                           exit(2);
                    }
                } else if ((isspace(c) || !c) && (concur >= inRow)) {
                    string[concur] = 0;
                    if (keepOff) printf("%6.6lX : ",offSet);
                    printf("%s\n",string);
                    concur = 0;
                } else {
                    concur = 0;
                }
            }
        }
        fclose(duh);
    }
    free(buffer);
    free(string);
    exit(0);
}
