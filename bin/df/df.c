/*
 * Copyright (c) 1980, 1990, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	$Id: df.c,v 1.1 1997/10/03 05:42:18 gdr Exp $
 */

/*
 * df for GNO/ME v2.0.6
 * $ID$
 */

#ifndef __GNO__
#ifndef lint
static char const copyright[] =
"@(#) Copyright (c) 1980, 1990, 1993, 1994\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */
#endif

#ifndef __GNO__
#ifndef lint
static char const sccsid[] = "@(#)df.c	8.9 (Berkeley) 5/8/95";
#endif /* not lint */
#endif

#include <sys/param.h>
#include <sys/stat.h>
#ifndef __GNO__
#include <sys/mount.h>
#include <ufs/ufs/ufsmount.h>
#else
#include "df.h"
#endif

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef __GNO__
int	  checkvfsname __P((const char *, char **));
char	**makevfslist __P((char *));
#else
void	  makevfslist __P((char *));
#endif
#ifndef __GNO__
long	  regetmntinfo __P((struct statfs **, long, char **));
int	  bread __P((off_t, void *, int));
char	 *getmntpt __P((char *));
#endif
void	  prtstat __P((struct statfs *, int));
#ifndef __GNO__
void	  ufs_df __P((char *, int));
#endif
void	  usage __P((void));

int	iflag, nflag;
struct	ufs_args mdev;

#ifdef __STACK_CHECK__
#include <gno/gno.h>

static void cleanup(void)
{
    (void) fprintf(stderr, "Stack Usage: %d\n", _endStackCheck());
}
#endif

/*
   The #ifdef's get quite messy, as there are certain filesystem things that
   GNO/ME doesn't handle the same as UNIX.  A lot of code is #ifdef'd out, but
   left here with the thought that in the future some of it may be useful as
   new libraries or FSTs are created.

   The files gnodf.c contains a couple of UNIX library functions I've recreated
   well enough to work for this applications, but I wouldn't even think of 
   adding to a library...
  
   In particular, I muck with the statfs structure.  DO NOT CALL statfs() or
   fstatfs() with this structure.  It may work and may cause a crash.  This
   structure is strictly for use with df(1) and the hacked functions in gnodf.c
   which fill in fields with GS/OS calls, not kernel calls.
 */

int
#ifndef __STDC__
main(argc, argv)
	int argc;
	char *argv[];
#else
main(int argc, char *argv[])
#endif
{
	struct stat stbuf;
	struct statfs statfsbuf, *mntbuf;
	long mntsize;
	int ch, err, i, maxwidth, width;
#ifndef __GNO__
	char *mntpt, **vfslist;

	vfslist = NULL;
#else
	char *mntpt;
#endif

#ifdef __STACK_CHECK__
	atexit(cleanup);
	_beginStackCheck();
#endif

#ifndef __GNO__
	while ((ch = getopt(argc, argv, "iknt:")) != -1)
#else
	while ((ch = getopt(argc, argv, "kt:")) != -1)
#endif
		switch (ch) {
#ifndef __GNO__
		case 'i':
			iflag = 1;
			break;
#endif
		case 'k':
			putenv("BLOCKSIZE=1k");
			break;
#ifndef __GNO__
		case 'n':
			nflag = 1;
			break;
#endif
		case 't':
#ifndef __GNO__
			if (vfslist != NULL)
#else
			if (vfslist[14])
#endif
				errx(1, "only one -t option may be specified.");
#ifndef __GNO__
			vfslist = makevfslist(optarg);
#else
			makevfslist(optarg);
#endif
			break;
		case '?':
		default:
			usage();
		}
	argc -= optind;
#ifndef __ORCAC__
	argv += optind;
#else
	argv = argv + optind;
#endif

	mntsize = getmntinfo(&mntbuf, MNT_NOWAIT);
	maxwidth = 0;
	for (i = 0; i < mntsize; i++) {
		width = strlen(mntbuf[i].f_mntfromname);
		if (width > maxwidth)
			maxwidth = width;
	}

	if (!*argv) {
#ifndef __GNO__
		mntsize = regetmntinfo(&mntbuf, mntsize, vfslist);
		if (vfslist != NULL) {
			maxwidth = 0;
			for (i = 0; i < mntsize; i++) {
				width = strlen(mntbuf[i].f_mntfromname);
				if (width > maxwidth)
					maxwidth = width;
			}
		}
#endif
		for (i = 0; i < mntsize; i++)
			prtstat(&mntbuf[i], maxwidth);
		exit(0);
	}

	for (; *argv; argv++) {
		if (stat(*argv, &stbuf) < 0) {
			err = errno;
#ifndef __GNO__
			if ((mntpt = getmntpt(*argv)) == 0) {
				warn("%s", *argv);
				continue;
			}
#else
			warn("%s", *argv);
			continue;
#endif
#ifndef __GNO__
		} else if ((stbuf.st_mode & S_IFMT) == S_IFCHR) {
			ufs_df(*argv, maxwidth);
			continue;
		} else if ((stbuf.st_mode & S_IFMT) == S_IFBLK) {
			if ((mntpt = getmntpt(*argv)) == 0) {
				mntpt = mktemp(strdup("/tmp/df.XXXXXX"));
				mdev.fspec = *argv;
				if (mkdir(mntpt, DEFFILEMODE) != 0) {
					warn("%s", mntpt);
					continue;
				}
				if (mount("ufs", mntpt, MNT_RDONLY,
				    &mdev) != 0) {
					ufs_df(*argv, maxwidth);
					(void)rmdir(mntpt);
					continue;
				} else if (statfs(mntpt, &statfsbuf) == 0) {
					statfsbuf.f_mntonname[0] = '\0';
					prtstat(&statfsbuf, maxwidth);
				} else
					warn("%s", *argv);
				(void)unmount(mntpt, 0);
				(void)rmdir(mntpt);
				continue;
			}
#endif
		} else
			mntpt = *argv;
		/*
		 * Statfs does not take a `wait' flag, so we cannot
		 * implement nflag here.
		 */

#ifndef __GNO__
		if (statfs(mntpt, &statfsbuf) < 0) {
			warn("%s", mntpt);
			continue;
		}
#else
		strcpy(statfsbuf.f_mntonname, mntpt);
		if (nameinfo(&statfsbuf) < 0) {
			warn("%s", statfsbuf.f_mntonname);
			continue;
		}
#endif

		if (argc == 1)
			maxwidth = strlen(statfsbuf.f_mntfromname) + 1;
		prtstat(&statfsbuf, maxwidth);
	}
	return (0);
}

#ifndef __GNO__
char *
#ifndef __STDC__
getmntpt(name)
	char *name;
#else
getmntpt(char *name)
#endif
{
	long mntsize, i;
	struct statfs *mntbuf;

	mntsize = getmntinfo(&mntbuf, MNT_NOWAIT);
	for (i = 0; i < mntsize; i++) {
		if (!strcmp(mntbuf[i].f_mntfromname, name))
			return (mntbuf[i].f_mntonname);
	}
	return (0);
}
#endif

#ifndef __GNO__
/*
 * Make a pass over the filesystem info in ``mntbuf'' filtering out
 * filesystem types not in vfslist and possibly re-stating to get
 * current (not cached) info.  Returns the new count of valid statfs bufs.
 */
long
#ifndef __STDC__
regetmntinfo(mntbufp, mntsize, vfslist)
	struct statfs **mntbufp;
	long mntsize;
	char **vfslist;
#else
regetmntinfo(struct fsinfo **mntbufp, long mntsize, char **vfslist)
#endif
{
	int i, j;
	struct statfs *mntbuf;

	if (vfslist == NULL)
		return (nflag ? mntsize : getmntinfo(mntbufp, MNT_WAIT));

	mntbuf = *mntbufp;
	for (j = 0, i = 0; i < mntsize; i++) {
		if (checkvfsname(mntbuf[i].f_fstypename, vfslist))
			continue;
		if (!nflag)
			(void)statfs(mntbuf[i].f_mntonname,&mntbuf[j]);
		else if (i != j)
			mntbuf[j] = mntbuf[i];
		j++;
	}
	return (j);
}
#endif

/*
 * Convert statfs returned filesystem size into BLOCKSIZE units.
 * Attempts to avoid overflow for large filesystems.
 */
#define fsbtoblk(num, fsbs, bs) \
	(((fsbs) != 0 && (fsbs) < (bs)) ? \
		(num) / ((bs) / (fsbs)) : (num) * ((fsbs) / (bs)))

/*
 * Print out status about a filesystem.
 */
void
#ifndef __STDC__
prtstat(sfsp, maxwidth)
	struct statfs *sfsp;
	int maxwidth;
#else
prtstat(struct statfs *sfsp, int maxwidth)
#endif
{
	static long blocksize;
	static int headerlen, timesthrough;
	static char *header;
	long used, availblks, inodes;

	if (maxwidth < 11)
		maxwidth = 11;
	if (++timesthrough == 1) {
		header = getbsize(&headerlen, &blocksize);
#ifdef __GNO__
		(void)printf("Dev# ");
#endif
		(void)printf("%-*.*s %s    Used   Avail Capacity",
		    maxwidth, maxwidth, "Filesystem", header);
		if (iflag)
			(void)printf(" iused   ifree  %%iused");
#ifdef __GNO__
		(void)printf("%12s", "FST");
#endif
		(void)printf(" Mounted on\n");
	}
#ifdef __GNO__
	if(!vfslist[sfsp->f_type])
		return;
	(void)printf(".d%-2ld ", (sfsp->f_fsid).lo);
#endif
	(void)printf("%-*.*s", maxwidth, maxwidth, sfsp->f_mntfromname);
	used = sfsp->f_blocks - sfsp->f_bfree;
	availblks = sfsp->f_bavail + used;
	(void)printf(" %*ld %7ld %7ld", headerlen,
	    fsbtoblk(sfsp->f_blocks, sfsp->f_bsize, blocksize),
	    fsbtoblk(used, sfsp->f_bsize, blocksize),
	    fsbtoblk(sfsp->f_bavail, sfsp->f_bsize, blocksize));
	(void)printf(" %5.0f%%",
	    availblks == 0 ? 100.0 : (double)used / (double)availblks * 100.0);
	if (iflag) {
		inodes = sfsp->f_files;
		used = inodes - sfsp->f_ffree;
		(void)printf(" %7ld %7ld %5.0f%% ", used, sfsp->f_ffree,
		   inodes == 0 ? 100.0 : (double)used / (double)inodes * 100.0);
	} else
		(void)printf("  ");
#ifdef __GNO__
	(void)printf("%12s", fs_name[sfsp->f_type]);
#endif
	(void)printf(" %s\n", sfsp->f_mntonname);
}

#ifndef __GNO__
/*
 * This code constitutes the pre-system call Berkeley df code for extracting
 * information from filesystem superblocks.
 */
#include <ufs/ufs/dinode.h>
#include <ufs/ffs/fs.h>
#include <errno.h>
#include <fstab.h>

union {
	struct fs iu_fs;
	char dummy[SBSIZE];
} sb;
#define sblock sb.iu_fs

int	rfd;

void
#ifndef __STDC__
ufs_df(file, maxwidth)
	char *file;
	int maxwidth;
#else
ufs_df(char *file, int maxwidth)
#endif
{
	struct statfs statfsbuf;
	struct statfs *sfsp;
	char *mntpt;
	static int synced;

	if (synced++ == 0)
		sync();

	if ((rfd = open(file, O_RDONLY)) < 0) {
		warn("%s", file);
		return;
	}
	if (bread((off_t)SBOFF, &sblock, SBSIZE) == 0) {
		(void)close(rfd);
		return;
	}
	sfsp = &statfsbuf;
	sfsp->f_type = 1;
	strcpy(sfsp->f_fstypename, "ufs");
	sfsp->f_flags = 0;
	sfsp->f_bsize = sblock.fs_fsize;
	sfsp->f_iosize = sblock.fs_bsize;
	sfsp->f_blocks = sblock.fs_dsize;
	sfsp->f_bfree = sblock.fs_cstotal.cs_nbfree * sblock.fs_frag +
		sblock.fs_cstotal.cs_nffree;
	sfsp->f_bavail = freespace(&sblock, sblock.fs_minfree);
	sfsp->f_files =  sblock.fs_ncg * sblock.fs_ipg;
	sfsp->f_ffree = sblock.fs_cstotal.cs_nifree;
	sfsp->f_fsid.val[0] = 0;
	sfsp->f_fsid.val[1] = 0;
	if ((mntpt = getmntpt(file)) == 0)
		mntpt = "";
	memmove(&sfsp->f_mntonname[0], mntpt, MNAMELEN);
	memmove(&sfsp->f_mntfromname[0], file, MNAMELEN);
	prtstat(sfsp, maxwidth);
	(void)close(rfd);
}

int
#ifndef __STDC__
bread(off, buf, cnt)
	off_t off;
	void *buf;
	int cnt;
#else
bread(off_t off, void *buf, int cnt)
#endif
{
	int nr;

	(void)lseek(rfd, off, SEEK_SET);
	if ((nr = read(rfd, buf, cnt)) != cnt) {
		/* Probably a dismounted disk if errno == EIO. */
		if (errno != EIO)
			(void)fprintf(stderr, "\ndf: %qd: %s\n",
			    off, strerror(nr > 0 ? EIO : errno));
		return (0);
	}
	return (1);
}
#endif

void
#ifndef __STDC__
usage()
#else
usage(void)
#endif
{
	(void)fprintf(stderr,
#ifndef __GNO__
	    "usage: df [-ikn] [-t type] [file | filesystem ...]\n");
#else
	    "usage: df [-k] [-t type] [devnum | volume | file ...]\n");
#endif
	exit(1);
}
