/*

        tar.c   - a GS version of the venerable Unix tape archive 
                  utility.

	7/12/93 - changed to use read/write instead of fread/fwrite.
	          Some performance improvement.
        Copyright 1991-1993 Procyon Enterprises, Inc.
        This code and the executable derived from it are hereby
        put in the public domain.

*/

#pragma stacksize 1024
#pragma optimize 9

#include <types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include <gsos.h>
#include <fcntl.h>
#include <errno.h>

#define TBLOCK 512
#define NAMSIZ 100

union hblock {
	char dummy[TBLOCK];
	struct header {
		char name[NAMSIZ];
		char mode[8];
		char uid[8];
		char gid[8];
		char size[12];
		char mtime[12];
		char chksum[8];
		char linkflag;
		char linkname[NAMSIZ];
	        FileInfoRecGS fInfo;
        } dbuf;
};

byte buffer[1024];
char filename[255];
int tarfile;

int optVerbose, optFile, optExtract,
    optTest;

/* pull a file out of the archive one block at a time */

GSString255Ptr MakeGSString1(char *s)
{
GSString255Ptr n;
    n = malloc(sizeof(GSString255));
    strcpy((char *) n->text,s);
    n->length = strlen(s);
    return n;
}

static char fixed[NAMSIZ];
char *fixName(char *n)
{
int i,j;

    for (i = 0, j = 0; n[i] != 0;) {
	if (n[i] == '/') {
            fixed[j++] = '/';
            while (n[i] == '/') i++;
        }
        else fixed[j++] = n[i++];
    }
    fixed[j] = 0;
    return fixed;
}

int extractFile(char *name, longword blocks, longword length)
{
word excess;
int got,get,i,e;
int output;
char *d,dirName[256];
CreateRecGS c;
FileInfoRecGS inf;

    if (optVerbose) printf("extracting %s (%ld blocks)\n",name, blocks);
    blocks = length / 1024;
    excess = length % 1024;
    if (excess) blocks++;

    for (i = 0; i < strlen(name); i++)
        if (!( isalnum(name[i]) || (name[i] == '.') || (name[i] == '/') ))
         name[i] = '.';

    if (name[0] == '/') {
        fprintf(stderr, "Can't extract to a volume name!\n");
        exit(1);
    }
    d = name;
    while ((d = strchr(d, '/')) != NULL) {

        strncpy(dirName, name, d-name);
        dirName[(int) (d-name)] = '\0';
 
        inf.pCount = 3;
        inf.pathname = MakeGSString1(dirName);
        GetFileInfoGS(&inf);
        if (e = toolerror()) {
          switch (e) {
            case 0x46:
            case 0x44: break;
            default:   fprintf(stderr, "error statting file %s (%x)\n",
                           dirName,e);
                       exit(1); break;
          }
        }
        else if (inf.fileType != 0x0F) {
            fprintf(stderr, "can't overwrite file %s\n", dirName);
            exit(1);
        }

        if (e) {
            c.pCount = 3;
            c.pathname = inf.pathname;
            c.access = 0xC3;
            c.fileType = 0x0F;
            CreateGS(&c);
            if (e = toolerror()) {
                fprintf(stderr, "fatal GS/OS error %x\n",e);
                exit(1);
            }
        }
        free(inf.pathname);
        while (*d == '/') d++;
    }

    if (!blocks) return 0;
    name=fixName(name);
    output = open(name, O_WRONLY|O_CREAT,0666);
    for (i = 0; i < blocks; i++) {
        if ((i == blocks-1) && excess) get = excess;
        else get = 1024;
        got = read(tarfile, buffer, (size_t) get);
        if (got != get) { fprintf(stderr, "read error\n"); exit(1); }

        if (write(output, buffer, (size_t) got) < get)
          { fprintf(stderr, "write error\n"); exit(1); }
    }
    close(output);
}

int testFile(char *name, longword blocks, longword length)
{
    printf("%s (%ld blocks)\n",name, blocks);
}

void usage(void)
{
    fprintf(stderr,"Usage: tar -options [archive]\n"
           "  options:  t  - list files in archive (test)\n"
           "            x  - extract files from archive\n"
           "            f  - use file [archive] instead of tape\n"
           "            v  - verbose mode\n");
    exit(1);
}

void parseOpts(char *opts)
{
char *i = opts;

    while (*i != '\0') {
        switch (*i) {
            case 'x': if (optTest) usage();
                      optExtract = 1; break;
            case 't': if (optExtract) usage();
                      optTest = 1; break;
            case 'f': optFile = 1; break;
            case 'v': optVerbose = 1; break;
            default: usage();
        }
        i++;
    }
}

int main(int argc, char *argv[])
{
longword block;
longword size;
longword fileBlocks;
word got;
int SessionPB = 0;
int err;

    block = 0;
    optVerbose = optFile = optExtract = optTest = 0;

    if (argc == 1) usage();
    if (argv[1][0] == '-') parseOpts(&argv[1][1]);
    else parseOpts(argv[1]);

    if (optFile) tarfile = open(argv[2], O_RDONLY);
    else { fprintf(stderr, "no SCSI tape found\n"); exit(1); }

    if (tarfile == 0) {
    	perror("Couldn't open archive");
	exit(1);
    }

    if (!(optExtract || optTest)) usage();
    BeginSession(&SessionPB);

    do {
        if (lseek(tarfile, (long) block*512, SEEK_SET) == -1) {
            err = errno;
            fprintf(stderr, "Seek error(%s)- aborting\n",strerror(err));
            exit(1);
        }
        got = read(tarfile, buffer, (size_t) 512);
        if (!buffer[0]) break;
        if (got == 0) { fprintf(stderr, "Read error- aborting\n"); exit(1); }
        if (got == 512) {
            sscanf(buffer+0174, "%lo", &size);

            fileBlocks = (size / 512);
            if (size % 512) fileBlocks++;

            block += fileBlocks + 1;
            strcpy(filename, (char *) buffer); /* copy the filename for future
                                         reeference */
            if (optExtract) extractFile(filename, fileBlocks, size);
            else if (optTest) testFile(filename, fileBlocks, size);

            buffer[0] = 0;
        }
    } while (got == 512);
    close(tarfile);
    EndSession(&SessionPB);
    return 0;
}
