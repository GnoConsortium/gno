/* 
 * gnodf.c
 *
 * Provide missing library functions to make df(1) work right
 * $ID$
 */

#include <gsos.h>
#include <stdlib.h>
#include <string.h>
#include "df.h"

/* Array of FST names */

char *fs_name[] = {
	"",
	"ProDOS",
	"DOS 3.3",
	"DOS 3.2",
	"Pascal",
	"MFS",
	"HFS",
	"LISA",
	"CP/M",
	"",
	"MS-DOS",
	"High Sierra",
	"ISO 9660",
	"AppleShare"
};

/* vfsname is an array of fsts accepted by the -t flag */

static char *vfsname[15] =  {
	"",
	"prodos",
	"dos33",
	"dos32",
	"pascal",
	"mfs",
	"hfs",
	"lisa",
	"cpm",
	"",
	"msdos",
	"highsierra",
	"iso9660",
	"appleshare"
};

/* Each member of the vfslist array reflects if the FST is included in the
 * output (as affected by the -t flag).
 * 	1 = display
 *	0 = don't display
 */

char vfslist[15] = {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0};

/* getbsize()
 *
 * in: pointer to int, pointer to long
 * out: pointer to char
 *
 * getbsize() returns a string indicative of the current value of the
 * BLOCKSIZE environment variable.  If BLOCKSIZE is 1k or 1024, " kbytes"
 * is returned.  Otherwise, " blocks" is returned.  If BLOCKSIZE is NULL,
 * a standard 512 byte blocksize is returned.  len is the strlen of the
 * returned string, always 7 in this case.
 *
 * BLOCKSIZE may be a plain number, in which case the number represents the
 * number of bytes in a block.  It may also be #k, where # represents the
 * number of kilobytes in a block.
 */
 
char *getbsize(int *len, long *blocksize)
{
	char *size;
	char *p;

	size = getenv("BLOCKSIZE");
	if(size == NULL)
		*blocksize = 512;
	else {
		*blocksize = strtol(size, &p, 0);
		if(*p == 'k')
			*blocksize *= 1024;
	}

	*len = 7;

	if(*blocksize != 1024) 
		return(" blocks");
	else
		return(" kbytes");
}

/* makevfslist()
 *
 * in: string containing a comma separated list of FSTs.
 *	(see manpage for the exact list of recognized strings)
 *
 * global: char vfslist[14]
 *
 * vfslist is a character array.  Each entry determines if the FST number
 * is to be displayed in the df list or not.  
 *
 * makevfslist() parses the input string for valid FST names.  If the name
 * is specified, the FST is either included or excluded from the list.
 * If the string starts with "no", all FSTs are included except those present
 * in the string.  Otherwise, only the FSTs specified are included.
 */

void makevfslist(char *list)
{
	int def, include, i;
	char *p, *str;

	def = 0;	
	include = 1;
	p = list;	

	if(*list == 'n') {
		p += 2;
		include = 0;
		def = 1;
	}

	for(i = 1; i < 14; i++) 
		vfslist[i] = def;

	str = strtok(p, ",");
	do {
		for(i=1; i < 14; i++) {
			if(!strcmp(str, vfsname[i])) {
				vfslist[i] = include;
				break;
			}
		}
	} while((str = strtok(NULL, NULL)) != NULL);

	/* Mark that makevfslist() has been run */

	vfslist[14] = 0xFF;
}

/* nameinfo()
 *
 * in: customize statfs structure pointer
 *
 * out:  0 = success, -1 = failure
 *
 * nameinfo() check the f_mntonname field of the statfs structure and calls
 * the GetDevNumber and DInfo GS/OS calls to fill in the f_mntfromname field
 * and the f_type field of the statfs structure.
 */

static ResultBuf32 devName = {35};
static DInfoRecGS dInfo = {3, 0, &devName};

int nameinfo(struct statfs *sbuf)
{
	GSString255 dName = {strlen(sbuf->f_mntonname)};
	DevNumRecGS dNum = {2, (GSString32 *) &dName};
	
	strcpy(&dName.text[0], sbuf->f_mntonname);
	GetDevNumberGS(&dNum);
	if(_toolErr)
		return(-1);

	dInfo.devNum = dNum.devNum;
	DInfoGS(&dInfo);
	if(_toolErr)
		return(-1);
	
	devName.bufString.text[devName.bufString.length] = '\0';
	strcpy(sbuf->f_mntfromname, devName.bufString.text);
	(sbuf->f_fsid).lo = (long) dNum.devNum;
	devinfo(sbuf);

	return(0);
}

/* devinfo()
 *
 * in: customized statfs structure pointer
 *
 * devinfo() takes the f_mntfromname field of the statfs structure and fills
 * in the fields used by df with the results of the GSOS Volume call.
 * Devices with no valid media (i.e. a 3.5" drive with no disk) are filled
 * with blank values and excluded from the resulting output.
 */

static ResultBuf255 vRes = {259};

void devinfo(struct statfs *sbuf)
{
	GSString32 dname = {strlen(sbuf->f_mntfromname)};
	VolumeRecGS vInfo = {6, &dname, &vRes};

	strcpy(dname.text, sbuf->f_mntfromname);
	VolumeGS(&vInfo);
	if(_toolErr) {
		strcpy(sbuf->f_mntonname, "");
		sbuf->f_bsize = 0;
		sbuf->f_blocks = 0;
		sbuf->f_bfree = sbuf->f_bavail = 0;
		sbuf->f_type = 0;
	} else {
		vRes.bufString.text[vRes.bufString.length] = '\0';
		strcpy(sbuf->f_mntonname, vRes.bufString.text);
		sbuf->f_bsize = vInfo.blockSize;
		sbuf->f_blocks = vInfo.totalBlocks;
		sbuf->f_bfree = sbuf->f_bavail = vInfo.freeBlocks;
		sbuf->f_type = vInfo.fileSysID;
	}
}

/* getmntinfo()
 *
 * in: struct statfs **, char
 *
 * out: number of block devices currently online.
 *
 * getmntinfo() first finds out how many devices are online and how many of
 * them are block devices by repeated calls to DInfo.  Then it dynamically
 * creates and array of statfs structures and for each block devices fills in
 * an entry in the array, returning the total number of block devices present.
 */

long getmntinfo(struct statfs **mntlist, char flags)
{
	int n, b, i, j;

	/* First find out how many devices exist, and how many are block
	   devices */

	b = n = 0;

	while(1) {
		dInfo.devNum = n+1;
		DInfoGS(&dInfo);
		if(_toolErr)
			break;
		if(dInfo.characteristics & 0x80)
			b++;
		n++;
	}

	/* Prepare the mntlist array */

	if((*mntlist = (struct statfs *) malloc(sizeof(struct statfs)*b)) == NULL)
		return(0L);

	/* Copy the device names of each block device to the array */
	
	for(j=0,i=1;i<=n;i++) {
		dInfo.devNum = i;
		DInfoGS(&dInfo);
		if(dInfo.characteristics & 0x80) {
			devName.bufString.text[devName.bufString.length] = '\0';
			strcpy((*mntlist)[j].f_mntfromname, devName.bufString.text);
			(*mntlist)[j].f_fsid.lo = (long) dInfo.devNum;
			devinfo(&(*mntlist)[j++]);
		}
	}

	return((long) b);
}
