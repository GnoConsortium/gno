#include <stdio.h>
#include <gno/gno.h>
#include <gsos.h>
#include <sys/signal.h>

void usage(void)
{
    fprintf(stderr,"usage: launch [-n] pathname\n");
    fprintf(stderr,"  -n\tDo not return to GNO after launched program exits\n");
    exit(1);
}

int main(int argc, char *argv[])
{
GSString255Ptr gs;
extern GSString255Ptr __C2GSMALLOC(char *);
unsigned int quitFlag;
int patharg = 1;

    quitFlag = 0x8000;
    if (argc == 3) {
        if (strcmp("-n",argv[1])) usage();
        else {
            quitFlag = 0;
            patharg = 2;
        }
    }

    gs = __C2GSMALLOC(argv[patharg]);
    SetGNOQuitRec(2,gs,quitFlag);
    /* eventually will need to signal 'init' to shutdown the system;
       for now, simply make the user ^D the shell */
}
