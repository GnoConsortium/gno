#if defined(__ORCAC__) && defined(DO_SEGMENTS)
segment "cpp_3_____";
#endif
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "cpp.h"

Includelist	includelist[NINCLUDE];

extern char	*objname;

void
doinclude(Tokenrow *trp)
{
	STATIC char fname[256], iname[256];
	Includelist *ip;
	int angled, len, fd, i;

	CHECKIN();
	trp->tp += 1;
	if (trp->tp>=trp->lp)
		goto syntax;
	if (trp->tp->type!=STRING && trp->tp->type!=LT) {
		len = trp->tp - trp->bp;
		expandrow(trp, "<include>");
		trp->tp = trp->bp+len;
	}
	if (trp->tp->type==STRING) {
		len = trp->tp->len-2;
		if (len > sizeof(fname) - 1)
			len = sizeof(fname) - 1;
		strncpy(fname, (char*)trp->tp->t+1, len);
		angled = 0;
	} else if (trp->tp->type==LT) {
		len = 0;
		trp->tp++;
		while (trp->tp->type!=GT) {
			if (trp->tp>trp->lp || len+trp->tp->len+2 >= sizeof(fname))
				goto syntax;
			strncpy(fname+len, (char*)trp->tp->t, trp->tp->len);
			len += trp->tp->len;
			trp->tp++;
		}
		angled = 1;
	} else
		goto syntax;
	trp->tp += 2;
	if (trp->tp < trp->lp || len==0)
		goto syntax;
	fname[len] = '\0';
	if (fname[0]=='/') {
		fd = open(fname, O_RDONLY); /* gdr: last arg previously zero */
		strcpy(iname, fname);
	} else for (fd = -1,i=NINCLUDE-1; i>=0; i--) {
		ip = &includelist[i];
		if (ip->file==NULL || ip->deleted || (angled && ip->always==0))
			continue;
		if (strlen(fname)+strlen(ip->file)+2 > sizeof(iname))
			continue;
		strcpy(iname, ip->file);
		strcat(iname, "/");
		strcat(iname, fname);
		if ((fd = open(iname, 0)) >= 0)
			break;
	}
	if ( Mflag>1 || ( !angled && Mflag==1 )) {
		write(STDOUT_FILENO,objname,strlen(objname));
		write(STDOUT_FILENO,iname,strlen(iname));
		write(STDOUT_FILENO,"\n",1);
	}
	if (fd >= 0) {
		if (++incdepth > 10)
			error(FATAL, "#include too deeply nested");
		setsource((char*)newstring((uchar*)iname, strlen(iname), 0), fd, NULL);
		genline();
	} else {
		trp->tp = trp->bp+2;
		error(ERROR, "Could not find include file %r", trp);
	}
	CHECKOUT();
	return;
syntax:
	error(ERROR, "Syntax error in #include");
	CHECKOUT();
	return;
}

/*
 * Generate a line directive for cursource
 */
void
genline(void)
{
	static Token ta = { UNCLASS };
	uchar *p;
#ifndef __INSIGHT__
	static Tokenrow tr = { &ta, &ta, &ta+1, 1 };
#else
	static Tokenrow tr;
	static int been_here = 0;

	if (! been_here) {
	    tr = { &ta, &ta, &ta+1, 1 };
	    been_here = 1;
	}
#endif

	ta.t = p = (uchar*)outp;
	strcpy((char*)p, "#line ");
	p += sizeof("#line ")-1;
	p = (uchar*)outnum((char*)p, cursource->line);
	*p++ = ' '; *p++ = '"';
	if (cursource->filename[0]!='/' && wd[0]) {
		strcpy((char*)p, wd);
		p += strlen(wd);
		*p++ = '/';
	}
	strcpy((char*)p, cursource->filename);
	p += strlen((char*)p);
	*p++ = '"'; *p++ = '\n';
	ta.len = (char*)p-outp;
	outp = (char*)p;
	tr.tp = tr.bp;
	puttokens(&tr);
}

void
setobjname(char *f)
{
	int n = strlen(f);
	objname = (char*)domalloc(n+5);
	strcpy(objname,f);
	if(objname[n-2]=='.'){
		strcpy(objname+n-1,"$O: ");
	}else{
		strcpy(objname+n,"$O: ");
	}
}
