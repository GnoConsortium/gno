#ifdef __ORCAC__
segment "ftpmain   ";
#endif

static char sccsid[] = "@(#)domacro.c	1.8 (Berkeley) 9/28/90";

#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <sys/ttychars.h>

#include "ftp.var.h"
#include "cmds.h"
#include "main.h"
#include "domacro.h"

void domacro (int argc, char **argv)
{
int i, j;
char *cp1, *cp2;
int count = 2, loopflg = 0;
static char line2[200];
struct cmd *c;

	if (argc < 2 && !another(&argc, &argv, "macro name")) {
		printf("Usage: %s macro_name.\n", argv[0]);
		code = -1;
		return;
	}
	for (i = 0; i < macnum; ++i) {
		if (!strncmp(argv[1], macros[i].mac_name, 9)) {
			break;
		}
	}
	if (i == macnum) {
		printf("'%s' macro not found.\n", argv[1]);
		code = -1;
		return;
	}
	(void) strcpy(line2, line);
TOP:
	cp1 = macros[i].mac_start;
	while (cp1 != macros[i].mac_end) {
		while (isspace(*cp1)) {
			cp1++;
		}
		cp2 = line;
		while (*cp1 != '\0') {
		      switch(*cp1) {
		   	    case '\\':
				 *cp2++ = *++cp1;
				 break;
			    case '$':
				 if (isdigit(*(cp1+1))) {
				    j = 0;
				    while (isdigit(*++cp1)) {
					  j = 10*j +  *cp1 - '0';
				    }
				    cp1--;
				    if (argc - 2 >= j) {
					(void) strcpy(cp2, argv[j+1]);
					cp2 += strlen(argv[j+1]);
				    }
				    break;
				 }
				 if (*(cp1+1) == 'i') {
					loopflg = 1;
					cp1++;
					if (count < argc) {
					   (void) strcpy(cp2, argv[count]);
					   cp2 += strlen(argv[count]);
					}
					break;
				}
				/* intentional drop through */
			    default:
				*cp2++ = *cp1;
				break;
		      }
		      if (*cp1 != '\0') {
			 cp1++;
		      }
		}
		*cp2 = '\0';
		makeargv();
		c = getcmd(margv[0]);
		if (c == (struct cmd *)-1) {
			printf("?Ambiguous command\n");
			code = -1;
		}
		else if (c == 0) {
			printf("?Invalid command\n");
			code = -1;
		}
		else if (c->c_conn && !connected) {
			printf("Not connected.\n");
			code = -1;
		}
		else {
			if (verbose) {
				printf("%s\n",line);
			}
			(*c->c_handler)(margc, margv);
			if (bell && c->c_bell) {
				(void) putchar('\007');
			}
			(void) strcpy(line, line2);
			makeargv();
			argc = margc;
			argv = margv;
		}
		if (cp1 != macros[i].mac_end) {
			cp1++;
		}
	}
	if (loopflg && ++count < argc) {
		goto TOP;
	}
}
