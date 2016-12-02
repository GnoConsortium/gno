/*
 * $Id: io.h,v 1.1 1997/03/14 06:22:27 gdr Exp $
 *
 * prchar
 *	print character with test for printer -- macro version
 *
 *	profiling has shown that this is where we spend most of our time
 *	Don't use a case statement here; the macros are out of range
 *	for some compilers.
 */

#if 0

#define PRCHAR(__c, fp) \
{ \
    if (__c == S_STANDOUT) { \
	fpGLOB = fp; \
	tputs(s_standout,1,tprchar); \
    } else if (__c == E_STANDOUT) { \
	fpGLOB = fp; \
	tputs(e_standout,1,tprchar); \
    } else if (__c == S_BOLD) { \
	fpGLOB = fp; \
	tputs(s_bold,1,tprchar); \
    } else if (__c == E_BOLD) { \
	fpGLOB = fp; \
	tputs(e_bold,1,tprchar); \
    } else if (__c == S_ITALIC) { \
	fpGLOB = fp; \
	tputs(s_italic,1,tprchar); \
    } else if (__c == E_ITALIC) { \
	fpGLOB = fp; \
	tputs(e_italic,1,tprchar); \
    } else if (__c == 13) { \
	; \
    } else { \
	putc(c, fp); \
    } \
}

#else 

#ifdef SHORT_STANDOUT

#define PRCHAR(c, fp) \
{ \
    unsigned char x = (unsigned char)c; \
    fpGLOB = fp; \
    switch(x) { \
    case S_STANDOUT: tputs(s_standout, 1, tprchar);    break; \
    case E_STANDOUT: tputs(e_standout, 1, tprchar);    break; \
    case S_BOLD:     tputs(s_bold, 1, tprchar);        break; \
    case E_BOLD:     tputs(e_bold, 1, tprchar);        break; \
    case S_ITALIC:   tputs(s_italic, 1, tprchar);      break; \
    case E_ITALIC:   tputs(e_italic, 1, tprchar);      break; \
    case 13:         break; \
    default:        putc(c,fp); \
    } \
}

#else

#define PRCHAR(c,fp) putc(c,fp)

#endif

#define PRCHAR2(c,fp) putc(c,fp)

extern char  __i;
extern char *__s;
extern FILE *fpGLOB;

int getlin (char *p, FILE *in_buf);
void pbstr (char *str);
void put (char *p);
void putlin (char *p, FILE *pbuf);
int tprchar(char c);
#endif
