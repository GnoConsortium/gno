#pragma stacksize 2048

/* IIGS History:
 *
 * v1.0		(Summer sometime)
 *			Original port by Derek Taubert
 * v1.1		(11/30/91 jb)
 *			Applied stacksize directive
 * v1.2		(1/21/92 jb)
 *			Recompiled under C 1.3 and with new libraries
 *			ANSI-fied the source
 * v1.4		(6/16/93 jb)  Recompiled under C 2.0.1a3 & with new libs
 *			Modified '/' to ':' to support HFS
 *			Fully optimized
 *
 * $Id: du.c,v 1.1 1998/04/10 18:13:27 gdr-ftp Exp $
 */

/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Newcomb.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1989 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)du.c        5.6 (Berkeley) 6/1/90";
#endif /* not lint */

#define _SYSV_SOURCE
#include <sys/param.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>

#pragma lint -1

typedef struct _ID {
        dev_t   dev;
        ino_t   inode;
} ID;

ID *files;
dev_t device;
int crossmounts, kvalue, listdirs, listfiles, maxfiles, numfiles;
char path[MAXPATHLEN + 1];
char fixpath[MAXPATHLEN + 1];

/* Prototypes */
void du(char *arg);
u_long descend(char *endp);

int main(int argc, char **argv)
{
        int ch;
        char top[MAXPATHLEN + 1];

        listdirs = crossmounts = 1;
        while ((ch = getopt(argc, argv, "aksx")) != EOF)
                switch(ch) {
                case 'a':
                        listfiles = 1;
                        break;
                case 'k':
                        kvalue = 1;
                        break;
                case 's':
                        listfiles = listdirs = 0;
                        break;
                case 'x':
                        crossmounts = 0;
                        break;
                case '?':
                default:
                        (void)fprintf(stderr,
                            "usage: du [-aksx] [name ...]\n");
                        exit(1);
                }
        argv += optind;

        files = (ID *)malloc((size_t)(sizeof(ID) * (maxfiles = 128)));
        (void)getwd(top);
        if (!*argv) {
                du(top);
                if (chdir(top)) {
                        (void)fprintf(stderr, "chdir du: %s: %s\n",top,
                            strerror(errno));
                        exit(1);
                }
        } else {
                for (;;) {
                        du(*argv);
                        if (chdir(top)) {
                                (void)fprintf(stderr, "chdir du: %s: %s\n",
                                    top, strerror(errno));
                                exit(1);
                        }
                        if (!*++argv)
                                break;
                }
        }
        exit(0);
}

struct stat info;

void du(char *arg)
{
        extern int errno;
        u_long total;

        if (lstat(arg, &info)) {
        	(void)fprintf(stderr, "lstat1 du: %s: %s\n", arg, strerror(errno));
        	return;
        }
        if ((info.st_mode&S_IFMT) != S_IFDIR) {
        	(void)printf("%-8.8ld %s\n", kvalue ?
            	howmany(info.st_blocks, 2) : info.st_blocks, arg);
        	return;
        }
        device = info.st_dev;
        (void)strcpy(path, arg);
        total = descend(path);
        if (!listfiles && !listdirs)
                (void)printf("%-8.8lu %s\n",
                    kvalue ? howmany(total, 2) : total, path);
}

u_long descend(char *endp)
{
        extern int errno;
        register DIR *dir;
        register ID *fp;
        register struct dirent *dp;
        u_long total;

        if (info.st_nlink > 1) {
                for (fp = files + numfiles - 1; fp >= files; --fp)
                        if (info.st_ino == fp->inode &&
                            info.st_dev == fp->dev)
                                return(0L);
                if (numfiles == maxfiles)
                        files = (ID *)realloc((char *)files,
                            (u_int)(sizeof(ID) * (maxfiles += 128)));
                files[numfiles].inode = info.st_ino;
                files[numfiles].dev = info.st_dev;
                ++numfiles;
        }
        total = info.st_blocks;
        if ((info.st_mode&S_IFMT) == S_IFDIR) {
                if (info.st_dev != device && !crossmounts)
                        return(0L);
                if (!(dir = opendir(endp)) || chdir(endp)) {
                        (void)fprintf(stderr, "opendir du: %s: %s\n",
                            path, strerror(errno));
                        return(total);
                }
                for (; *endp; ++endp);
#ifdef __ORCAC__
		/* IIGS uses ':'s as separators */
		if (endp[-1] != ':')
			{ *endp++ = ':'; *endp = 0; }
#else
                if (endp[-1] != '/')
                        *endp++ = '/';
#endif
                while (dp = readdir(dir)) {
                    /*    if (dp->d_name[0] == '.' && (!dp->d_name[1] ||
                            dp->d_name[1] == '.' && !dp->d_name[2]))
                                continue;   unix only, we have no links */
                        /* Specify 0: for HFS filenames with slashes */
                        bcopy(dp->d_name, endp, dp->d_namlen + 1);
                        strcpy(fixpath,"0:");
                        strcat(fixpath,dp->d_name);
                        if (lstat(fixpath /*dp->d_name*/, &info)) {
                                (void)fprintf(stderr, "lstat du: %s: %s\n", path,
                                    strerror(errno));
                                continue;
                        }
                        total += descend(endp);
                }
                closedir(dir);
                if (chdir("..")) {
                        (void)fprintf(stderr, "chdir du: ..: %s\n", strerror(errno));
                        exit(1);
                }
                *--endp = '\0';
                if (listdirs)
                        (void)printf("%-8.8lu %s\n",
                            kvalue ? howmany(total, 2) : total, path);
        }
        else if (listfiles)
                (void)printf("%-8.8lu %s\n",
                    kvalue ? howmany(total, 2) : total, path);
        return(total);
}
