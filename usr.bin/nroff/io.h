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


#define PRCHAR(c, fp) \
{ \
    __i = c; \
    switch(__i) { \
    case S_STANDOUT: __s = s_standout;    break; \
    case E_STANDOUT: __s = e_standout;    break; \
    case S_BOLD:     __s = s_bold;        break; \
    case E_BOLD:     __s = e_bold;        break; \
    case S_ITALIC:   __s = s_italic;      break; \
    case E_ITALIC:   __s = e_italic;      break; \
    case 13:         __s = (char *)0x01L; break; \
    default:         __s = NULL; \
    } \
    switch((unsigned long)__s) { \
    case 0L:  putc(c, fp);  break;    \
    case 1L:  ;             break;    \
    default:  fpGLOB = fp; tputs(__s, 1, tprchar); \
    } \
}

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
