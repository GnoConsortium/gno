/*
 * rcmp.c
 *
 * Opens the resource forks of two files and sets the file descriptors to
 * the open forks (if present).
 *
 * Catches files with no resource fork.
 *
 * Part of cmp for GNO/ME v2.0.6
 *
 * $Id: rcmp.c,v 1.1 1997/10/03 05:13:23 gdr Exp $
 */

#include <gsos.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "extern.h"

static GSString255 path;

void rcmp(int *fd1, int *fd2, char *file1, char *file2)
{
	OpenRecGS ropen = {4, 0, &path, readEnable, 1};

	strcpy(path.text, file1);
	path.length = strlen(file1);
	OpenGS(&ropen);
	*fd1 = ropen.refNum;

	strcpy(path.text, file2);
	path.length = strlen(file2);
	OpenGS(&ropen);
	*fd2 = ropen.refNum;

	if(*fd1 == 0) 
		if(*fd2 == 0)
			return;
		else {
			(void) printf("%s has a resource fork, %s does not.\n", 
					file2, file1);
			return;
		}
	else if(*fd2 == 0) {
		(void) printf("%s has a resource fork, %s does not.\n", file1,
				file2);	
			return;
	}

	c_special(*fd1, file1, 0, *fd2, file2, 0);
}
