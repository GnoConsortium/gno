/*
 * shellenv: print information on the shell environment
 *
 * Written by Dave Tribby to test gsh  *  June 1998
 * $Id: shellenv.c,v 1.1 1999/11/30 20:28:24 tribby Exp $
 */

#include <stdlib.h>
#include <stdio.h>
#include <shell.h>
#include <types.h>

/* VersionGS parameter block */
VersionGSPB	vers_pb = {1};

/* GetLInfoGS parameter block */
ResultBuf255Ptr		sFilePtr = NULL;
ResultBuf255Ptr		dFilePtr = NULL;
ResultBuf255Ptr		paramsPtr = NULL;
ResultBuf255Ptr		iStringPtr = NULL;
GetLInfoGSPB	get_li_pb = {11};

/* GetLangGS paramater block */
GetLangGSPB	get_l_pb = {1};

/* DirectionGS parameter block */
DirectionGSPB	dir_pb = {2};
char		*devnm[3]={"stdin ", "stdout", "stderr"};
char		*devdir[3]={"Console", "Printer", "Disk file"};


/* Cleanup routine at end: deallocate buffers */
void cleanup(void)
{
	if (get_li_pb.sFile) {
        	free(get_li_pb.sFile);
                get_li_pb.sFile = NULL;
                }
	if (get_li_pb.dFile) {
        	free(get_li_pb.dFile);
                get_li_pb.dFile = NULL;
                }
	if (get_li_pb.parms) {
        	free(get_li_pb.parms);
                get_li_pb.parms = NULL;
                }
	if (get_li_pb.iString) {
        	free(get_li_pb.iString);
                get_li_pb.iString = NULL;
                }
}                

/* Allocate memory for a GS/OS result buffer */
ResultBuf255Ptr allocate_buf(Word numbytes)
{
	ResultBuf255Ptr	hold_ptr;
	if ( (hold_ptr = (ResultBuf255Ptr) malloc(numbytes+1)) == 0) {
		printf("Error allocating %u bytes of memory\n");
		exit (1);
	}
	hold_ptr->bufSize = 1024;
	return hold_ptr;
}

/* Print information about a buffer */
void print_buf_info(char *title, ResultBuf255Ptr buf_ptr)
{
	char	*end_str;
	/* Calculate address beyond end of string */
        end_str = buf_ptr->bufString.text + buf_ptr->bufString.length;
	/* Terminate the string with a null character */
        *end_str = '\0';
	printf("Buffer for %s: size = %u\n%s\n\n",
           title, buf_ptr->bufString.length, buf_ptr->bufString.text);
}

/* Print information about option flags */
void print_flag_info(unsigned long flags)
{
	char		ch='A';
        unsigned long	testpos=0x80000000;

	printf("%08lX:", flags);
        while (ch <= 'Z') {
                if (testpos & flags) printf(" %c",ch);
	        ch++;
                testpos = testpos >> 1;
        }
}


int main(int argc, char *argv[])
{
	int i;

	printf("=< S H E L L   E N V I R O N M E N T   R E P O R T >=\n");

	atexit(cleanup);

	VersionGS(&vers_pb);
	printf("Shell version: ");
        for (i=0; i<4; i++) putchar(vers_pb.version[i]);
	putchar('\n');

	printf("Edit/Compile/Link environment:\n");
	get_li_pb.sFile = allocate_buf(1024);
	get_li_pb.dFile = allocate_buf(255);
	get_li_pb.parms = allocate_buf(255);
	get_li_pb.iString = allocate_buf(1024);
	GetLInfoGS(&get_li_pb);
                
	print_buf_info("Source file",      (ResultBuf255Ptr) get_li_pb.sFile); 
	print_buf_info("Destination file", (ResultBuf255Ptr) get_li_pb.dFile);
	print_buf_info("Parameter List",   (ResultBuf255Ptr) get_li_pb.parms);
	print_buf_info("Command specific", (ResultBuf255Ptr) get_li_pb.iString);
	printf("Error level:  Maximum = %d,  Found = %d\n",
					get_li_pb.merr, get_li_pb.merrf);
	printf("Operations flag = %08X     Keep flag = %d\n",
					get_li_pb.lops, get_li_pb.kflag);
	printf("Flags: -");
	print_flag_info(get_li_pb.mFlags);
	printf("   +");
	print_flag_info(get_li_pb.pFlags);
	putchar('\n');
	printf("Displacement into file: %ld\n", get_li_pb.org);

        GetLangGS(&get_l_pb);
        printf("Current language number = %d (", get_l_pb.lang);
        switch(get_l_pb.lang) {
		case 0:		printf("ProDOS");
				break;
		case 1:		printf("text");
				break;
		case 3:		printf("ASM65816");
				break;
		case 5:		printf("ORCA/Pascal");
				break;
		case 6:		printf("EXEC");
				break;
		case 8:		printf("ORCA/C");
				break;
	        default:	printf("?");
		}
        printf(")\n");

	printf("I/O Redirection:\n");
	for (i=0; i<3; i++) {
		dir_pb.device = i;
                DirectionGS(&dir_pb);
	        printf(" %d: %s  direction = %d", i,devnm[i],dir_pb.direct);
                if (dir_pb.direct < 3) printf(" (%s)", devdir[dir_pb.direct]);
		putchar('\n');
		} 
	return 0;
}
