/*
 *  GNO/ME 2.0.6
 *  mount.h -	File system interface calls.
 *		This file used to be called <sys/vfs.h>
 *
 * $Id: mount.h,v 1.1 1997/02/28 04:42:14 gdr Exp $
 */

#ifndef _SYS_MOUNT_H_
#define _SYS_MOUNT_H_

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

int statfs __P((const char *path, struct statfs *buf));
int fstatfs __P((int fd, struct statfs *buf));

#endif	/* _SYS_MOUNT_H_ */

