/*	$Id: ep.c,v 1.2 2001/06/23 23:55:28 kelvin Exp $ */

/* Things to do before this code can be installed:
	kernel must mutex all prefixes
        this code should be changed from using GetPrefix to grabbing
          the prefixes directly from the process entry
*/
#pragma optimize 79
segment "KERN2     ";

#include "proc.h"
#include "sys.h"
#include "/lang/orca/libraries/orcacdefs/stdio.h"
#include "/lang/orca/libraries/orcacdefs/stdlib.h"
#include "/lang/orca/libraries/orcacdefs/string.h"
#include "/lang/orca/libraries/orcacdefs/ctype.h"
#include <gsos.h>

unsigned short err;
extern kernelStructPtr kp;
typedef GSString255Ptr Gstr;

ExpandPathRecGS ep;
PrefixRecGS gp;

/* this is for P16_EXPANDPATH */
struct P16String {
    byte length;
    char text[255];
} ;
typedef struct P16String P16String, *P16StringPtr;

#define PRIME 31
#define NUM_NP 20

struct h {
    Gstr pfx;
    Gstr exp_pfx;
    struct h *next;
};

struct h h_pool[NUM_NP];
unsigned int pool_ind = 0;
struct h *h_table[PRIME];

typedef struct stack {
    Gstr name;
    unsigned short ind;
    char sep;
} *stackPtr;

struct stack the_stack[3];
unsigned short stack_ind = 0;

void printGS(Gstr path)
{
unsigned short i;
   for (i = 0; i < path->length; i++) fputc(path->text[i],stderr);
}

static unsigned long
hashpjw(Gstr s,unsigned short leng)
{
char *p;
unsigned long h = 0, g;
unsigned short i;

    for (i = 1; i < leng; i++) {
	h = (h << 4) + (toupper(s->text[i]));
        if (g = h & 0xF0000000) {
	    h ^= (g >> 24);
	    h ^= g;
        }
    }
    return h % PRIME;
}

void init_htable(void)
{
FILE *np;
Gstr p,map;
unsigned short hent;
unsigned short i;
short x;
char *line,*pt,*pt1;

    for (i = 0; i < NUM_NP; i++) {
        h_pool[i].pfx = NULL;
        h_pool[i].exp_pfx = NULL;
    }
    line = malloc(128l);
    np = fopen("9/etc/namespace","r");
    if (np != NULL) {
      while (!feof(np)) {
        if (pool_ind == NUM_NP) {
            printf("warning: more than 20 entries in the /etc/namespace file\n");
            break;
        }
        p = malloc(19);
        map = malloc(67);
        fgets(line,127,np);
        if ((line[0] == 0) || (line[0] == '\n')) continue;
        pt = line;
        /* find end of first string */
        while ((*pt != 0) && (!isspace(*pt))) pt++;
        if (*pt == 0) PANIC("invalid namespace specifier");
        /* find start of second string */
        *pt++ = 0;
	while ((*pt != 0) && (isspace(*pt))) pt++;
        if (*pt == 0) PANIC("invalid namespace specifier");
        pt1 = pt;
        /* find end of second string */
        while ((*pt != 0) && (!isspace(*pt))) pt++;
        *pt = 0;

        strcpy(p->text,line);
        strcpy(map->text,pt1);
        p->length = strlen(p->text);
        map->length = strlen(map->text);

        hent = (unsigned short) hashpjw(p,p->length);
        h_pool[pool_ind].next = h_table[hent];
        h_table[hent] = &h_pool[pool_ind];
        h_pool[pool_ind].pfx = p;
        h_pool[pool_ind].exp_pfx = map;
     /*	printf("prefix: ");
        printGS(p);
        printf("  location: %d pool: %d\n",hent,pool_ind); */
	pool_ind++;
      }
      fclose(np);
    } else printf("warning: could not locate :etc:namespace\n");
    free(line);
}

#undef TOLOWER
#define TOLOWER(c) isupper(c) ? _tolower(c) : c

int
strincmp (const char *s1, const char *s2, size_t n)
{
	unsigned int c1, c2;
	size_t i;

	for (i=0; i<n; i++) {
		c1 = TOLOWER(*s1);
		c2 = TOLOWER(*s2);
		if (c1 == '\0' && c2 == '\0') {
			return 0;
		} else if (c1 == c2) {
			s1++; s2++;
		} else {
			/* don't do subtraction -- see man page */
			return (c1 > c2) ? 1 : -1;
		}
	}
	return 0;
}                                                     

/* needs a fully expanded GS string */
Gstr match(Gstr fname, unsigned short leng, char sep)
{
unsigned short i,hent;
struct h *l;

    i = 1;
    while ((i < leng) && (fname->text[i] != sep)) i++;
    hent = (unsigned short) hashpjw(fname,i);
    l = h_table[hent];
    while (l != NULL) {
        /* make sure vol. name is same length as hashed entry */
        if ((l->pfx->length == i)
          && (!strincmp(fname->text+1,l->pfx->text+1,i-1))) break;
        l = l->next;
    }
    if (l == NULL) return NULL;
    else return l->exp_pfx;
}

unsigned short isSeparator(char c)
{
    if ((c == ':') || (c == '/')) return 1;
    else return 0;
}

void push(Gstr x, unsigned short y, char sep)
{
    the_stack[stack_ind].name = x;
    the_stack[stack_ind].ind = y;
    the_stack[stack_ind].sep = sep;
    stack_ind++;
}

stackPtr top(void)
{
    if (!stack_ind) return NULL;
    else return &the_stack[stack_ind-1];
}

stackPtr pop(void)
{
    if (!stack_ind) return NULL;
    else return &the_stack[--stack_ind];
}

void clearstack(void)
{
    stack_ind = 0;
}

ResultBuf32 rbuf;
Gstr go = NULL;
static word nullGSOS = 0;

#pragma databank 1
Gstr gno_ExpandPath(Gstr i_path, int num, word npFlag)
{
unsigned short i,j,pfxNum,oldlen,outind,outind2;
Gstr g_out,g1;
char SEP,nSEP;
Gstr g;
stackPtr sp;
extern int OldGSOSSt(word callnum, void *pBlock);

    if (go == NULL) go = malloc(1026l);

    if (i_path->length > 1024) return (Gstr) 0xFFFF0040l;
    g_out = go;
    rbuf.bufSize = 32;
    ep.pCount = 3;
    ep.flags = 0;

/* Separator conversion and determination */
  i = 0;
  while ((i < i_path->length) && !isSeparator(i_path->text[i])) i++;
  if (i == i_path->length) SEP = ':';
  else SEP = i_path->text[i]; /* grab the separator character */

  push(i_path,0,SEP); /* use this separator here */

  if (!isSeparator(i_path->text[0])) {

    /* Scan for first component */
    i = 0;
    while ((i < i_path->length) && (i_path->text[i] != SEP)) i++;

    /* Check for * prefix */
    if ( ((i == 1) && (i_path->text[0] == '*')) ||
    /* Check for device prefix */
         ((i > 1) && (i_path->text[0] == '.') && (i_path->text[1] != '.')) )
    {
       /*  Expand the prefix with old expandpath (source is input string
       	*  with length temporarily set to trick ExpandPath).
        *  Add to copy stack, fix length.
        */

        oldlen = i_path->length;
        i_path->length = i;
        ep.inputPath = (Gstr) i_path;
        ep.outputPath = (ResultBuf255Ptr) &rbuf;
        err = OldGSOSSt(0x200e,&ep);
      /*  ExpandPathGS(&ep); */
        i_path->length = oldlen;
        if (rbuf.bufString.text[0] == '.') {
            clearstack();
            g_out = (Gstr) &rbuf.bufString;
            goto goaway;
        }
        push((Gstr) &rbuf.bufString,i,':');
        goto phase2;
    }
    /* Check for @ prefix, add to copy stack if present */
    else if ((i == 1) && (i_path->text[0] == '@')) {
        push((Gstr) PROC->prefix[0],i,':');
        goto phase2;
    }
    /* Check for numeric prefix */
    else if (isdigit(i_path->text[0])) {
       /* Do a GetPrefix call on the parsed prefix number
	* add to copy stack
        */

        j = 0; pfxNum = 0;
        while ((j < i) && (isdigit(i_path->text[j]))) {
             pfxNum = (pfxNum * 10) + (i_path->text[j] - '0');
             j++;
        }
        if (j == i) {
            if (pfxNum > 31) {
                 clearstack();
                 return (Gstr) 0xFFFF0040l; /* syntax error */
            }
            g = (Gstr) PROC->prefix[pfxNum+1];
    	    if (g == NULL) push((Gstr) &nullGSOS,i,':');
            else push(g,i,':');
	    goto phase2;
        } /* if j != i, there was a non-numeral in the prefix number,
             so fall through to concat prefix 0 onto it */
    }
    /* else expand by prefix 0 (adding the pfx 0 ptr directly to the stack) */
    g = (Gstr) PROC->prefix[num+1];
    if (g == NULL) push((Gstr) &nullGSOS,0,':');
    else push(g,0,':');
  } 

phase2:
  /* Check for named prefix */

  if (!npFlag) {
      g = top()->name;
    /*  SEP = top()->sep; */
      i = 1;
      while ((i < g->length) && (g->text[i] != top()->sep)) i++;
      /* If there's a match, add the result string to the top of the stack */
      if (i > 1)
        if (g1 = match(g,i,SEP))
	  push(g1,i,':');
  }

/* Start at the top of the stack
  scan each path component, checking for '.' or '..' removing bits
    as appropriate
  copy each component into the output buffer
  at the end of the string, pop the stack
  if the stack is empty, we're done, so return */

    nSEP = ':';
    outind = i = 0;
    while (sp = pop()) {
        g = sp->name;
     /*	nSEP = sp->sep;*/
        if (top() == NULL) nSEP = SEP;
        oldlen = g->length;
        if (g->text[oldlen-1] == nSEP) oldlen--;

        while (i < oldlen) {
            g_out->text[outind++] = ':';
	    if (g->text[i] == nSEP) i++;
            j = i;	/* beginning of segment */

            /* locate end of segment */
            while ((i < oldlen) && (g->text[i] != nSEP)) i++;
            if ((i-j == 1) && (g->text[j] == '.')) outind--;
            else if ((i-j == 2) && (g->text[j] == '.') && (g->text[j+1] == '.')) {
              outind2 = outind-2;
              while ((outind2 > 0) && (g_out->text[outind2] != ':')) outind2--;
              if (outind2 != 0) outind = outind2; /* move outind back */
              else outind--; /* just zap the colon */
            }
            else
              while (j < i) g_out->text[outind++] = g->text[j++];
        }
        i = sp->ind;
    }
    g_out->length = outind;
goaway:
    if (kp->gsosDebug & 2) {
	fprintf(stderr,"EP[");
	printGS(g_out); fprintf(stderr,"]\n");
    }
    if ((g_out->length == 1) && (g_out->text[0] == ':')) g_out->length = 0;
    return g_out;
}

GSString255 p_buf;
Gstr p16_ExpandPath(P16StringPtr path, int num, word npFlag)
{
    p_buf.length = path->length;
    memcpy(p_buf.text, path->text, (longword) p_buf.length);
    return gno_ExpandPath((Gstr) &p_buf,num,npFlag);
}

#pragma databank 0

#ifndef KERNEL
char in[80];

int main(int argc, char *argv[])
{
unsigned l;
Gstr g,g1;

    init_htable();

    while (1) {    	
        gets(in);
        l = strlen(in);
	g = malloc(l+2);
        memcpy(g->text,in,l);
        g->length = l;
        g1 = GNO_EXPANDPATH(g,0);
        printGS(g1);
        printf("\n");
        free(g);
    }
}
#endif

