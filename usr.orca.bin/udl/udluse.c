/*
 * udl - Convert EOL formats freely between MS-DOS (CR/LF), Unix/Amiga (LF),
 *       and Apple (CR).
 *
 * Usage strings.
 *
 * $Id: udluse.c,v 1.11 1997/12/08 16:07:19 gdr Exp $
 *
 * Copyright (c) 1993-1995 Soenke Behrens, Devin Reade
 */

#ifdef __GNO__
#pragma noroot
#endif

char use1 [] =
"udl 1.1.5 by Soenke Behrens, Devin Reade\n"
"Usage: udl -u|g|m [-RvpVh] file1 [file2 ...]\n\n"
"Options:\n"
"  -u   Convert file to use LF as EOL character (Unix).\n"
"  -g   Convert file to use CR as EOL character (Apple).\n"
"  -m   Convert file to use LF/CR as EOL character (MS-DOS).\n"
"  -R   Recurse through subdirectories.\n"
"  -p   Be pedantic.\n"
"  -v   Be verbose about it.\n"
"  -V   Print out version number.\n"
"  -h   Display this help screen.\n\n"
"udl creates a temporary file in the directory of the original file.\n"
"The original file is overwritten after conversion.\n";

char use2 [] =
"\nFiles may contain ORCA/Shell style wildcards.\n";

/* End Of File */
