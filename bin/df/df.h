/* 
 * df.h
 *
 * Bastardized sys/mount.h with local constants and functions to provide
 * functions not available in the gno libraries.
 *
 * $Id: df.h,v 1.1 1997/10/03 05:42:18 gdr Exp $
 */

#ifndef _SYS_TYPES_H_
#include <sys/types.h>
#endif

typedef quad_t fsid_t;		/* formerly an equivalent struct */

struct statfs {
        long	f_type; 	/* FST type (see below) */
        long	f_bsize;	/* fundamental file system block size */
        long	f_blocks;	/* total blocks in file system */
        long	f_bfree;	/* free blocks */
        long	f_bavail;	/* free blocks available to non-superuser */
        long	f_files;	/* total file nodes in file system */
        long	f_ffree;	/* free file nodes in fs */
        fsid_t	f_fsid; 	/* file system id (GS/OS device number) */
        long	f_spare[7];	/* reserved */
	char 	f_mntfromname[32];	/* device name */
	char	f_mntonname[32];	/* volume name */
};

#define	MOUNT_PRODOS	0x0001	/* ProDOS or SOS */
#define	MOUNT_DOS_33	0x0002	/* DOS 3.3 */
#define	MOUNT_DOS_32	0x0003	/* DOS 3.1 or 3.2 */
#define	MOUNT_PASCAL	0x0004	/* Apple II Pascal */
#define	MOUNT_MFS	0x0005	/* Macintosh (MFS) */
#define	MOUNT_HFS	0x0006	/* Macintosh (HFS) */
#define	MOUNT_LISA	0x0007	/* Lisa */
#define	MOUNT_CPM	0x0008	/* Apple CP/M */
#define	MOUNT_MSDOS	0x000A	/* MS/DOS */
#define	MOUNT_HISHS	0x000B	/* High Sierra */
#define	MOUNT_CD9660	0x000C	/* ISO 9660 (CD-ROM) */
#define	MOUNT_APLSHAR	0x000D	/* AppleShare */

#ifndef _SYS_CDEFS_H_
#include <sys/cdefs.h>
#endif

/* This constant is used in the one non-#ifdef'd getmntinfo() call */

#define MNT_NOWAIT	0

/* "Helper" functions to fill in the statfs structure where statfs() fails */

int nameinfo(struct statfs *);
void devinfo(struct statfs *);
long getmntinfo(struct statfs **, char);
char *getbsize(int *, long *);

/* Data structures */

extern char *fs_name[];
extern char vfslist[];
