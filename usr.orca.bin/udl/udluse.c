/*
 * udl - Convert EOL formats freely between MS-DOS (CR/LF), Unix/Amiga (LF),
 *       and Apple (CR).
 *
 * Usage strings.
 *
 * $Id: udluse.c,v 1.1 1994/12/13 18:08:34 gdr Exp $
 *
 * Copyright (c) 1993-1994 Soenke Behrens
 */

char use1 [] =
"udl 1.13 by Soenke Behrens\n"
"Usage: udl -u|g|m [-Rv] file1 [file2 ...]\n\n"
"Options:\n"
"  -u   Convert file to use LF as EOL character.\n"
"  -g   Convert file to use CR as EOL character.\n"
"  -m   Convert file to use LF/CR as EOL character.\n"
"  -R   Recurse through subdirectories.\n"
"  -p   Be pedantic.\n"
"  -v   Be verbose about it.\n\n"
"udl creates a temporary file on 14:, the original file(s) are over-\n"
"written when it is done.\n";

char use2 [] =
"files may contain ORCA/Shell style wildcards.\n\n";

/* End Of File */

