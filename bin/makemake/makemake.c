/*
 *	I got tired of making new makefiles for the IIgs make that's
 *	available, so here's a bloody program to write them for us
 *
 *	To create the dependency lists, the source files are searched for
 *	the following information:
 *
 *  .c		#include "filename"
 *  
 *	Please note that this program isn't particularly smart.
 */

#pragma stacksize 2048
#include <stdio.h>
#include <string.h>
#include <ctype.h>

int ndepend;
char dep[32][32]; /* 32 files, 32 characters long */
char line[1024];
char linenws[1024];

void removews(char *line, char *line2)
{
int n = strlen(line);
int i,j;

    j = 0;
    for (i = 0; i < n; i++)
	if (!isspace(line[i])) line2[j++] = line[i];
    line2[j] = 0;
}

finddepend(char *top, char *fname)
{
FILE *s;
char *p,*q;
int n;

    printf("scanning '%s'",fname);
    fflush(stdout);
    s = fopen(fname,"r");
    if (s == NULL) { fprintf(stderr,"Couldn't open %s\n",fname); exit(1); }
    while (!feof(s)) {
	fgets(line,1023,s);
        n = strlen(line);
	removews(line,linenws);
        if (!strncmp(linenws,"#include",8)) {
            putchar('.'); fflush(stdout);
            if (p = strchr(line+8,'<')) continue;
            p = strchr(line+8,'"');
            if (p == NULL) continue;
	    q = strchr(p+1,'"');
	    strncpy(dep[ndepend],p+1,(q-p)-1);
            dep[ndepend++][q-p-1] = 0;
	    finddepend(top,dep[ndepend-1]);
        }
    }
    fclose(s);
    putchar('\n');
}

int main(int argc, char *argv[])
{
FILE *f;
char nbuf[80];
int i,j;

    f = fopen("makefile","w");
    for (i = 1; i < argc; i++) {
	strcpy(nbuf,argv[i]);
	*strrchr(nbuf,'.') = 0;
        ndepend = 0;
        finddepend("#include",argv[i]);
        fprintf(f,"o/%s.a: %s",nbuf,argv[i]);
        for (j = 0; j < ndepend; j++)
	    fprintf(f," %s",dep[j]);
        fprintf(f,"\n");
        fprintf(f,"  compile %s keep=o/%s\n\n",argv[i],nbuf);
    }
    fclose(f);
}
