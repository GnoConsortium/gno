/*
 * stty.c
 *
 * Set terminal parameters
 */

#pragma stacksize 1280
#pragma optimize 9

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sgtty.h>
#include <sys/ioctl.h>
#include <gno/gno.h>
#include <unistd.h>

typedef struct opts {
    char *name;
    int item;
} opts_s;

/* B0 - B57600 are defined before these others */
#define LCRTERA_ON      16
#define LCRTERA_OFF     17
#define LCTLECH_ON      18
#define LCTLECH_OFF     19
#define CRMOD_ON        20
#define CRMOD_OFF       21
#define ECHO_ON         22
#define ECHO_OFF        23
#define RAW_ON          24
#define RAW_OFF         25
#define CBREAK_ON       26
#define CBREAK_OFF      27
#define SUSP_C          28
#define STOP_C          29
#define START_C         30
#define QUIT_C          31
#define ERASE_C         32
#define INTR_C          33
#define EOF_C           34
#define WERASE_C        35
#define RPRNT_C         36
#define FLUSH_C         37
#define LNEXT_C         38
#define DSUSP_C         39

char *baudtbl[] = {
"0",
"50",
"75",
"110",
"134.5",
"150",
"57600",
"300",
"600",
"1200",
"1800",
"2400",
"4800",
"9600",
"19200",
"38400"};

opts_s options[] = {
    "75",	B75,
    "110",	B110,
    "134",	B134,
    "150",	B150,
    "300",	B300,
    "600",	B600,
    "1200",	B1200,
    "1800",	B1800,
    "2400",	B2400,
    "4800",	B4800,
    "9600",	B9600,
    "19200",	B19200,
    "38400",	B38400,
    "57600",	B57600,
    "lcrtera", 	LCRTERA_ON,
    "-lcrtera", LCRTERA_OFF,
    "lctlech",	LCTLECH_ON,
    "-lctlech",	LCTLECH_OFF,
    "crmod",	CRMOD_ON,
    "-crmod",	CRMOD_OFF,
    "echo",	ECHO_ON,
    "-echo",	ECHO_OFF,
    "raw",	RAW_ON,
    "-raw",	RAW_OFF,
    "cbreak",	CBREAK_ON,
    "-cbreak",	CBREAK_OFF,
    "susp",	SUSP_C,
    "stop",	STOP_C,
    "start",	START_C,
    "quit",	QUIT_C,
    "erase",	ERASE_C,
    "intr",	INTR_C,
    "eof",	EOF_C,
    "werase",   WERASE_C,
    "rprnt",    RPRNT_C,
    "flush",    FLUSH_C,
    "lnext",    LNEXT_C,
    "dsusp",    DSUSP_C,
    "",		0
};

int lookup(char *s)
{
int i;

    for (i = 0; options[i].item; i++) {
        if (!strcmp(options[i].name,s)) return (options[i].item);
    }
    return 0;
}

void usage(void)
{
    fprintf(stderr,"usage: stty [ option ]...\n"
    		"\toption: [-]raw,[-]echo,[-]cbreak,[baud]\n"
    		"\toption c: intr, susp, stop, start, eof, erase\n"
		"\t\twhere c is ^X or \\0OCTAL or \\xHEX\n");
    exit(1);
}

char parsechar(char *s)
{
int x;

    if (s[0] == '^') {
	if (strlen(s) != 2) usage();
	if (s[1] == '?') return 0x7f;
        else return toupper(s[1])-64;
    }
    else if (s[0] == '\\') {
	    if (isdigit(s[1]) && (s[0] != 0))
        	sscanf(s+1,"%d",&x);
        else if (toupper(s[1]) == 'X')
	        sscanf(s+2,"%x",&x);
        else if (s[1] == '0')
	        sscanf(s+1,"%o",&x);
        else usage();
        printf("char: %d\n",x);
        return x;
    }
    if (strlen(s) != 1) usage();
    return s[0];
}

struct sgttyb sg;
struct tchars tc;
struct ltchars ltc;
struct winsize wz;
long localmode;

char *dash[] = {"","-"};

char *doctrl(char c)
{
static char ss[3] = "  ";

    if (c == -1) c = ' ';
    if (c == 0x7F) {
    	ss[0] = '^';
	ss[1] = '?';
    }
    else if (c < 32) {
        ss[0] = '^';
        ss[1] = c + '@';
    }
    else {
        ss[0] = c;
        ss[1] = ' ';
    }
    return ss;
}

void printCurSettings(void)
{
    printf("old tty, speed %s baud, %d rows, %d columns\n",
    	baudtbl[sg.sg_ispeed & 0xF],wz.ws_row,wz.ws_col);
    printf("%seven %sodd %sraw %snl %secho %slcase %standem %stabs %scbreak\n",
    	dash[(sg.sg_flags & EVENP) == 0],
	dash[(sg.sg_flags & ODDP) == 0],
	dash[(sg.sg_flags & RAW) == 0],
	dash[(sg.sg_flags & NLDELAY) == 0],
	dash[(sg.sg_flags & ECHO) == 0],
	dash[(sg.sg_flags & LCASE) == 0],
	dash[(sg.sg_flags & TANDEM) == 0],
	dash[(sg.sg_flags & XTABS) == 0],
	dash[(sg.sg_flags & CBREAK) == 0]);
    printf("%stilde %sflusho %slitout %spass8 %snohang\n",
	dash[(localmode & LTILDE) == 0],
	dash[(localmode & LFLUSHO) == 0],
	dash[(localmode & LLITOUT) == 0],
	dash[(localmode & LPASS8) == 0],
	dash[(localmode & LNOHANG) == 0]);
    printf("%spendin %snoflsh\n",
	dash[(localmode & LPENDIN) == 0],
	dash[(localmode & LNOFLSH) == 0]);

    printf("erase  kill   werase rprnt  flush  lnext  susp   intr   quit   stop   eof\n");
    printf("%-7s",doctrl(sg.sg_erase));
    printf("%-7s",doctrl(sg.sg_kill));
    printf("%-7s",doctrl(ltc.t_werasc));
    printf("%-7s",doctrl(ltc.t_rprntc));
    printf("%-7s",doctrl(ltc.t_flushc));
    printf("%-7s",doctrl(ltc.t_lnextc));
    printf("%2s",doctrl(ltc.t_suspc));
    printf("/%2s  ",doctrl(ltc.t_dsuspc));
    printf("%-7s",doctrl(tc.t_intrc));
    printf("%-7s",doctrl(tc.t_quitc));
    printf("%2s",doctrl(tc.t_stopc));
    printf("/%2s  ",doctrl(tc.t_startc));
    printf("%-7s\n",doctrl(tc.t_eofc));
}

int main(int argc, char *argv[])
{
int i,item;

    ioctl(STDIN_FILENO,TIOCGETP,&sg);
    ioctl(STDIN_FILENO,TIOCGETC,&tc);
    ioctl(STDIN_FILENO,TIOCGLTC,&ltc);
    ioctl(STDIN_FILENO,TIOCLGET,&localmode);
    ioctl(STDIN_FILENO,TIOCGWINSZ,&wz);
    if (argc < 2) {
    	printCurSettings();
        exit(0);
    }

    for (i = 1; i < argc;) {
      switch (item = lookup(argv[i])) {

        case INTR_C:
	        tc.t_intrc = parsechar(argv[i+1]);
	        i++;
        	break;
	case SUSP_C:
	        ltc.t_suspc = parsechar(argv[i+1]);
	        i++;
        	break;
        case DSUSP_C:
                ltc.t_dsuspc = parsechar(argv[i+1]);
                i++;
                break;
        case STOP_C:
            	tc.t_stopc = parsechar(argv[i+1]);
	        i++;
        	break;
	case START_C:
	        tc.t_startc = parsechar(argv[i+1]);
	        i++;
        	break;
        case QUIT_C:
	        tc.t_quitc = parsechar(argv[i+1]);
	        i++;
        	break;
	case ERASE_C:
		sg.sg_erase = parsechar(argv[i+1]);
		i++;
		break;
	case EOF_C:
		tc.t_eofc = parsechar(argv[i+1]);
		i++;
		break;
        case WERASE_C:
                ltc.t_werasc = parsechar(argv[i+1]);
                i++;
                break;
        case FLUSH_C:
                ltc.t_flushc = parsechar(argv[i+1]);
                i++;
                break;
        case LNEXT_C:
                ltc.t_lnextc = parsechar(argv[i+1]);
                i++;
                break;
        case RPRNT_C:
                ltc.t_rprntc = parsechar(argv[i+1]);
                i++;
                break;
	case ECHO_ON:
	        sg.sg_flags |= ECHO;
        	break;
        case ECHO_OFF:
	        sg.sg_flags &= ~ECHO;
        	break;
        case RAW_OFF:
        	sg.sg_flags &= ~RAW;
        	break;
        case RAW_ON:
	        sg.sg_flags |= RAW;
        	break;
        case CBREAK_OFF:
        	sg.sg_flags &= ~CBREAK;
        	break;
        case CBREAK_ON:
	        sg.sg_flags |= CBREAK;
        	break;
        case CRMOD_OFF:
        	sg.sg_flags &= ~CRMOD;
        	break;
        case CRMOD_ON:
	        sg.sg_flags |= CRMOD;
        	break;
        case LCTLECH_ON:
        	localmode |= LCTLECH;
	        break;
	case LCTLECH_OFF:
		localmode &= ~LCTLECH;
        	break;
        case LCRTERA_ON:
        	localmode |= LCRTERA;
	        break;
	case LCRTERA_OFF:
		localmode &= ~LCRTERA;
		break;
        default:
        	if ((item != 0) && (item <= B38400)) {
	            sg.sg_ispeed = item;
	            sg.sg_ospeed = item;
                } else usage();
                break;
      }
      i++;
    }
    ioctl(STDIN_FILENO,TIOCSETP,&sg);
    ioctl(STDIN_FILENO,TIOCSETC,&tc);
    ioctl(STDIN_FILENO,TIOCSLTC,&ltc);
    ioctl(STDIN_FILENO,TIOCLSET,&localmode);
}
