#ifdef __ORCAC__
segment "ftpftp1   ";
#endif

static char sccsid[] = "@(#)ftp.c	5.38 (Berkeley) 4/22/91";

#ifdef __GNO__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <gno/gno.h>
#endif

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/file.h>

#include <netinet/in.h>

#ifndef __GNO__
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#endif

#include <arpa/inet.h>
#include <arpa/ftp.h>
#include <arpa/telnet.h>

#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <netdb.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdarg.h>

#include "ftp.var.h"
#include "cmds.h"
#include "main.h"
#include "domacro.h"
#include "ruserpass.h"
#include "ftp.h"

struct	sockaddr_in hisctladdr;
struct	sockaddr_in data_addr;
int	data = -1;
int	abrtflag = 0;
int	ptflag = 0;
struct	sockaddr_in myctladdr;
off_t	restart_point = 0;

#ifndef __GNO__
extern int connected, errno;
#endif

FILE	*cin, *cout;

char *hookup (char *host, int port)
{
struct hostent *hp = 0;
int s, len, tos;
static char hostnamebuf[80];

	bzero((char *)&hisctladdr, sizeof (hisctladdr));
	hisctladdr.sin_addr.s_addr = inet_addr(host);
	if (hisctladdr.sin_addr.s_addr != -1) {
		hisctladdr.sin_family = AF_INET;
		strncpy(hostnamebuf, host, sizeof(hostnamebuf));
	} else {
		hp = gethostbyname(host);
		if (hp == NULL) {
			fprintf(stderr, "ftp: %s: ", host);
			perror ("gethostbyname");
			code = -1;
			return((char *) 0);
		}
		hisctladdr.sin_family = hp->h_addrtype;
		bcopy(hp->h_addr_list[0],
		    (caddr_t)&hisctladdr.sin_addr, hp->h_length);
		strncpy(hostnamebuf, hp->h_name, sizeof(hostnamebuf));
	}
	hostname = hostnamebuf;
	s = socket(hisctladdr.sin_family, SOCK_STREAM, 0);
	if (s < 0) {
		perror("ftp: socket");
		code = -1;
		return (0);
	}
	hisctladdr.sin_port = port;
	while (connect(s, (struct __SOCKADDR *)&hisctladdr, sizeof (hisctladdr)) < 0) {
		if (hp && hp->h_addr_list[1]) {
			int oerrno = errno;

			fprintf(stderr, "ftp: connect to address %s: ",
				inet_ntoa(hisctladdr.sin_addr));
			errno = oerrno;
			perror((char *) 0);
			hp->h_addr_list++;
			bcopy(hp->h_addr_list[0],
			     (caddr_t)&hisctladdr.sin_addr, hp->h_length);
			fprintf(stdout, "Trying %s...\n",
				inet_ntoa(hisctladdr.sin_addr));
			close(s);
			s = socket(hisctladdr.sin_family, SOCK_STREAM, 0);
			if (s < 0) {
				perror("ftp: socket");
				code = -1;
				return (0);
			}
			continue;
		}
		perror("ftp: connect");
		code = -1;
		goto bad;
	}
	len = sizeof (myctladdr);
	if (getsockname(s, (struct __SOCKADDR *)&myctladdr, &len) < 0) {
		perror("ftp: getsockname");
		code = -1;
		goto bad;
	}
	cin = fdopen(s, "rb");
	cout = fdopen(s, "wb");
	if (cin == NULL || cout == NULL) {
		fprintf(stderr, "ftp: fdopen failed.\n");
		if (cin)
			fclose(cin);
		if (cout)
			fclose(cout);
		code = -1;
		goto bad;
	}
	if (verbose)
		printf("Connected to %s.\n", hostname);
	if (getreply(0) > 2) { 	/* read startup message from server */
		if (cin)
			fclose(cin);
		if (cout)
			fclose(cout);
		code = -1;
		goto bad;
	}

	return (hostname);
bad:
	close(s);
	return ((char *)0);
}

char *getlogin (void)
{
    return (getenv ("USER"));
}

int login (char *host)
{
static char tmp[80];
char *user, *pass, *acct;
int n, aflag = 0;

	user = pass = acct = 0;
	if (ruserpass(host, &user, &pass, &acct) < 0) {
		code = -1;
		return(0);
	}
	while (user == NULL) {
		char *myname = getlogin();

		if (myname == NULL) {
			struct passwd *pp = getpwuid(getuid());

			if (pp != NULL)
				myname = pp->pw_name;
		}
		if (myname)
			printf("Name (%s:%s): ", host, myname);
		else
			printf("Name (%s): ", host);
		fgets(tmp, sizeof(tmp) - 1, stdin);
		tmp[strlen(tmp) - 1] = '\0';
		if (*tmp == '\0')
			user = myname;
		else
			user = tmp;
	}
	n = command("USER %s", user);
	if (n == CONTINUE) {
		if (pass == NULL)
			pass = getpass("Password:");
		n = command("PASS %s", pass);
	}
	if (n == CONTINUE) {
		aflag++;
		acct = getpass("Account:");
		n = command("ACCT %s", acct);
	}
	if (n != COMPLETE) {
		fprintf(stderr, "Login failed.\n");
		return (0);
	}
	if (!aflag && acct != NULL)
		command("ACCT %s", acct);
	if (proxy)
		return(1);
	for (n = 0; n < macnum; ++n) {
		if (!strcmp("init", macros[n].mac_name)) {
			strcpy(line, "$init");
			makeargv();
			domacro(margc, margv);
			break;
		}
	}
	return (1);
}
                
#pragma databank 1
void cmdabort (int sig, int code)
{
extern jmp_buf ptabort;

	printf("\n");
	fflush(stdout);
	abrtflag++;
	if (ptflag)
		longjmp(ptabort, 1);
}
#pragma databank 0

#pragma debug 0
/* assume at least #pragma optimize 8 */
/*VARARGS*/
int command (char *string, ...)
{
va_list ap;
char *fmt;
static char buf[255];
int r;
sig_t oldintr;

	abrtflag = 0;
	if (cout == NULL)
	{
		perror ("No control connection for command");
		code = -1;
		return (0);
	}
	oldintr = signal (SIGINT, cmdabort);
	va_start (ap, string);
	vsprintf (buf, string, ap);
	va_end(ap);
	if (debug) {
		printf("---> ");
		if (strncmp("PASS ", buf, 5) == 0)
			printf("PASS XXXX\n");
		else
			printf("%s\n", buf);
		fflush(stdout);
	}
	fprintf (cout, "%s\r\n", buf);
	fflush(cout);
	cpend = 1;
	r = getreply(!strncmp(string, "QUIT", 4));
	if (abrtflag && oldintr != SIG_IGN)
		(*oldintr)(SIGINT, 0);
	signal(SIGINT, oldintr);

	return(r);
}
/* find some way to put #pragma options back */

char reply_string[BUFSIZ];		/* last line of previous reply */

#include <ctype.h>

int getreply (int expecteof)
{
int c, n;
int dig;
char *cp;
int originalcode = 0, continuation = 0;
sig_t oldintr;
int pflag = 0;
char *pt = pasv;

	oldintr = signal (SIGINT, cmdabort);
	for (;;) {
		dig = n = code = 0;
		cp = reply_string;
		while ((c = getc(cin)) != '\n') {
			if (c == IAC) {     /* handle telnet commands */
				switch (c = getc(cin)) {
				case WILL:
				case WONT:
					c = getc(cin);
					fprintf(cout, "%c%c%c", IAC, DONT, c);
					fflush(cout);
					break;
				case DO:
				case DONT:
					c = getc(cin);
					fprintf(cout, "%c%c%c", IAC, WONT, c);
					fflush(cout);
					break;
				default:
					break;
				}
				continue;
			}
			dig++;
			if (c == EOF) {
				if (expecteof) {
					signal(SIGINT,oldintr);
					code = 221;
					return (0);
				}
				lostpeer(0,0);
				if (verbose) {
					printf("421 Service not available, remote server has closed connection\n");
					fflush(stdout);
				}
				code = 421;
				return(4);
			}
			if (c != '\r' && (verbose > 0 ||
			    (verbose > -1 && n == '5' && dig > 4))) {
				if (proxflag &&
				   (dig == 1 || dig == 5 && verbose == 0))
					printf("%s:",hostname);
				putchar(c);
			}
			if (dig < 4 && isdigit(c))
				code = code * 10 + (c - '0');
			if (!pflag && code == 227)
				pflag = 1;
			if (dig > 4 && pflag == 1 && isdigit(c))
				pflag = 2;
			if (pflag == 2) {
				if (c != '\r' && c != ')')
					*pt++ = c;
				else {
					*pt = '\0';
					pflag = 3;
				}
			}
			if (dig == 4 && c == '-') {
				if (continuation)
					code = 0;
				continuation++;
			}
			if (n == 0)
				n = c;
			if (cp < &reply_string[sizeof(reply_string) - 1])
				*cp++ = c;
		}
		if (verbose > 0 || verbose > -1 && n == '5') {
			putchar(c);
			fflush (stdout);
		}
		if (continuation && code != originalcode) {
			if (originalcode == 0)
				originalcode = code;
			continue;
		}
		cp[0] = '\0';
		if (n != '1')
			cpend = 0;
		signal(SIGINT,oldintr);
		if (code == 421 || originalcode == 421)
			lostpeer(0,0);
		if (abrtflag && oldintr != cmdabort && oldintr != SIG_IGN)
			(*oldintr)(SIGINT, 0);
		return (n - '0');
	}
}
                
int empty (struct fd_set *mask, int sec)
{
struct timeval t;

	t.tv_sec = (long) sec;
	t.tv_usec = 0;
	return(select(32, mask, (struct fd_set *) 0, (struct fd_set *) 0, &t));
}

jmp_buf	sendabort;

#pragma databank 1
void abortsend (int sig, int code)
{
	mflag = 0;
	abrtflag = 0;
	printf("\nsend aborted\nwaiting for remote to finish abort\n");
	fflush(stdout);
	longjmp(sendabort, 1);
}
#pragma databank 0

#define HASHBYTES 1024

void sendrequest (char *cmd, char *local, char *remote, int printnames)
{
struct stat st;
struct timeval start, stop;
int c, d;
FILE *fin, *dout = 0;
sig_t oldintr, oldintp;
long bytes = 0, hashbytes = HASHBYTES;
char *lmode, *bufp;
static char buf[BUFSIZ];
int (*closefunc)(FILE *); 
     
	if (verbose && printnames) {
		if (local && *local != '-')
			printf("local: %s ", local);
		if (remote)
			printf("remote: %s\n", remote);
	}
	if (proxy) {
		proxtrans(cmd, local, remote);
		return;
	}
	if (curtype != type)
		changetype(type, 0);
	closefunc = NULL;
	oldintr = NULL;
	oldintp = NULL;
	lmode = "wb";
	if (setjmp(sendabort)) {
		while (cpend) {
			getreply(0);
		}
		if (data >= 0) {
			close(data);
			data = -1;
		}
		if (oldintr)
			signal(SIGINT,oldintr);
		if (oldintp)
			signal(SIGPIPE,oldintp);
		code = -1;
		return;
	}
	oldintr = signal(SIGINT, abortsend);
	if (strcmp(local, "-") == 0)
		fin = stdin;
	else if (*local == '|') {
		oldintp = signal(SIGPIPE,SIG_IGN);
		fin = popen(local + 1, "r");
		if (fin == NULL) {
			perror(local + 1);
			signal(SIGINT, oldintr);
			signal(SIGPIPE, oldintp);
			code = -1;
			return;
		}
		closefunc = pclose;   
	} else {
		switch (curtype) {
		  case TYPE_I:
		  case TYPE_L:
			fin = fopen(local, "rb");
			break;
		  case TYPE_A:
			fin = fopen(local, "r");
			break;
		  default:	/* shouldn't ever happen */
			fin = NULL;
			break;
		}
		if (fin == NULL) {
			fprintf(stderr, "local: %s: %s\n", local,
				strerror(errno));
			signal(SIGINT, oldintr);
			code = -1;
			return;
		}
		closefunc = fclose;
		if (fstat(fileno(fin), &st) < 0 ||
		    (st.st_mode&S_IFMT) != S_IFREG) {
			fprintf(stdout, "%s: not a plain file.\n", local);
			signal(SIGINT, oldintr);
			fclose(fin);
			code = -1;
			return;
		}
	}
	if (initconn()) {
		signal(SIGINT, oldintr);
		if (oldintp)
			signal(SIGPIPE, oldintp);
		code = -1;
		if (closefunc != NULL)
			(*closefunc)(fin);
		return;
	}
	if (setjmp(sendabort))
		goto abort;

	if (restart_point &&
	    (strcmp(cmd, "STOR") == 0 || strcmp(cmd, "APPE") == 0)) {
		if (fseek(fin, (long) restart_point, 0) < 0) {
			fprintf(stderr, "local: %s: %s\n", local,
				strerror(errno));
			restart_point = 0;
			if (closefunc != NULL)
				(*closefunc)(fin);
			return;
		}
		if (command("REST %ld", (long) restart_point)
			!= CONTINUE) {
			restart_point = 0;
			if (closefunc != NULL)
				(*closefunc)(fin);
			return;
		}
		restart_point = 0;
		lmode = "r+wb";
	}
	if (remote) {
		if (command("%s %s", cmd, remote) != PRELIM) {
			signal(SIGINT, oldintr);
			if (oldintp)
				signal(SIGPIPE, oldintp);
			if (closefunc != NULL)
				(*closefunc)(fin);
			return;
		}
	} else
		if (command("%s", cmd) != PRELIM) {
			signal(SIGINT, oldintr);
			if (oldintp)
				signal(SIGPIPE, oldintp);
			if (closefunc != NULL)
				(*closefunc)(fin);
			return;
		}
	dout = dataconn(lmode);
	if (dout == NULL)
		goto abort;
	gettimeofday(&start, (struct timezone *)0);
	oldintp = signal(SIGPIPE, SIG_IGN);
	switch (curtype) {

	case TYPE_I:
	case TYPE_L:
		errno = d = 0;
		while ((c = read(fileno(fin), buf, sizeof (buf))) > 0) {
			bytes += c;
			for (bufp = buf; c > 0; c -= d, bufp += d)
				if ((d = write(fileno(dout), bufp, c)) <= 0)
					break;
			if (hash) {
				while (bytes >= hashbytes) {
					putchar('#');
					hashbytes += HASHBYTES;
				}
				fflush(stdout);
			}
		}
		if (hash && bytes > 0) {
			if (bytes < HASHBYTES)
				putchar('#');
			putchar('\n');
			fflush(stdout);
		}
		if (c < 0)
			fprintf(stderr, "local: %s: %s\n", local,
				strerror(errno));
		if (d < 0) {
			if (errno != EPIPE)
				perror("netout");
			bytes = -1;
		}
		break;

	case TYPE_A:
		while ((c = getc(fin)) != EOF) {
			if (c == '\n') {
				while (hash && (bytes >= hashbytes)) {
					putchar('#');
					fflush(stdout);
					hashbytes += HASHBYTES;
				}
				if (ferror(dout))
					break;
				putc('\r', dout);
				bytes++;
			}
			putc(c, dout);
			bytes++;
	/*		if (c == '\r') {			  	*/
	/*		(void)	putc('\0', dout);  /* this violates rfc */
	/*			bytes++;				*/
	/*		}                          			*/	
		}
		if (hash) {
			if (bytes < hashbytes)
				putchar('#');
			putchar('\n');
			fflush(stdout);
		}
		if (ferror(fin))
			fprintf(stderr, "local: %s: %s\n", local,
				strerror(errno));
		if (ferror(dout)) {
			if (errno != EPIPE)
				perror("netout");
			bytes = -1;
		}
		break;
	}
	gettimeofday(&stop, (struct timezone *)0);
	if (closefunc != NULL)
		(*closefunc)(fin);
	fclose(dout);
	getreply(0);
	signal(SIGINT, oldintr);
	if (oldintp)
		signal(SIGPIPE, oldintp);
	if (bytes > 0)
		ptransfer("sent", bytes, &start, &stop);
	return;
abort:
	gettimeofday(&stop, (struct timezone *)0);
	signal(SIGINT, oldintr);
	if (oldintp)
		signal(SIGPIPE, oldintp);
	if (!cpend) {
		code = -1;
		return;
	}
	if (data >= 0) {
		close(data);
		data = -1;
	}
	if (dout)
		fclose(dout);
	getreply(0);
	code = -1;
	if (closefunc != NULL && fin != NULL)
		(*closefunc)(fin);
	if (bytes > 0)
		ptransfer("sent", bytes, &start, &stop);
}

segment "ftpftp2   ";

jmp_buf	recvabort;

#pragma databank 1
void abortrecv (int sig, int code)
{
	mflag = 0;
	abrtflag = 0;
	printf("\nreceive aborted\nwaiting for remote to finish abort\n");
	fflush(stdout);
	longjmp(recvabort, 1);
}
#pragma databank 0

void recvrequest (char *cmd, char *local, char *remote, char *lmode, int printnames)
{
FILE *fout, *din = 0;
sig_t oldintr, oldintp;
int is_retr, tcrflag, bare_lfs = 0;
static int bufsize;
static char *buf;
long bytes = 0, hashbytes = HASHBYTES;
int c, d;
struct timeval start, stop;
struct stat st;
int (*closefunc)(FILE *); 
     
	is_retr = strcmp(cmd, "RETR") == 0;
	if (is_retr && verbose && printnames) {
		if (local && *local != '-')
			printf("local: %s ", local);
		if (remote)
			printf("remote: %s\n", remote);
	}
	if (proxy && is_retr) {
		proxtrans(cmd, local, remote);
		return;
	}
	closefunc = NULL;
	oldintr = NULL;
	oldintp = NULL;
	tcrflag = !crflag && is_retr;
	if (setjmp(recvabort)) {
		while (cpend) {
			getreply(0);
		}
		if (data >= 0) {
			close(data);
			data = -1;
		}
		if (oldintr)
			signal(SIGINT, oldintr);
		code = -1;
		return;
	}
	oldintr = signal(SIGINT, abortrecv);
	if (strcmp(local, "-") && *local != '|') {
		if (access(local, 2) < 0) {
		char *dir;

			if (errno != ENOENT && errno != EACCES) {
				fprintf(stderr, "local: %s: %s\n", local,
					strerror(errno));
				signal(SIGINT, oldintr);
				code = -1;
				return;
			}
			dir = rindex(local, (int) '/');
			if (dir != NULL)
				*dir = 0;
			d = access(dir ? local : ".", 2);
			if (dir != NULL)
				*dir = '/';
			if (d < 0) {
				fprintf(stderr, "local: %s: %s\n", local,
					strerror(errno));
				signal(SIGINT, oldintr);
				code = -1;
				return;
			}
			if (!runique && errno == EACCES &&
			    chmod(local, 0600) < 0) {
				fprintf(stderr, "local: %s: %s\n", local,
					strerror(errno));
				signal(SIGINT, oldintr);
				signal(SIGINT, oldintr);
				code = -1;
				return;
			}
			if (runique && errno == EACCES &&
			   (local = gunique(local)) == NULL) {
				signal(SIGINT, oldintr);
				code = -1;
				return;
			}
		}
		else if (runique && (local = gunique(local)) == NULL) {
			signal(SIGINT, oldintr);
			code = -1;
			return;
		}
	}
	if (!is_retr) {
		if (curtype != TYPE_A)
			changetype(TYPE_A, 0);
	} else if (curtype != type)
		changetype(type, 0);
	if (initconn()) {
		signal(SIGINT, oldintr);
		code = -1;
		return;
	}
	if (setjmp(recvabort))
		goto abort;
	if (is_retr && restart_point &&
	    command("REST %ld", (long) restart_point) != CONTINUE)
		return;
	if (remote) {
		if (command("%s %s", cmd, remote) != PRELIM) {
			signal(SIGINT, oldintr);
			return;
		}
	} else {
		if (command("%s", cmd) != PRELIM) {
			signal(SIGINT, oldintr);
			return;
		}
	}
	din = dataconn("rb");
	if (din == NULL)
		goto abort;
	if (strcmp(local, "-") == 0)
		fout = stdout;
	else if (*local == '|') {
		oldintp = signal(SIGPIPE, SIG_IGN);
		fout = popen(local + 1, "w");
		if (fout == NULL) {
			perror(local+1);
			goto abort;
		}
		closefunc = pclose;
	} else {
		fout = fopen(local, lmode);
		if (fout == NULL) {
			fprintf(stderr, "local: %s: %s\n", local,
				strerror(errno));
			goto abort;
		}
		closefunc = fclose;
	}
	if (fstat(fileno(fout), &st) < 0 || st.st_blksize == 0)
		st.st_blksize = BUFSIZ;
	if (st.st_blksize > bufsize) {
		if (buf)
			free(buf);
		buf = malloc((unsigned)st.st_blksize);
		if (buf == NULL) {
			perror("malloc");
			bufsize = 0;
			goto abort;
		}
		bufsize = st.st_blksize;
	}
	gettimeofday(&start, (struct timezone *)0);  
	switch (curtype) {

	case TYPE_I:
	case TYPE_L:
		if (restart_point &&
		    lseek(fileno(fout), (long) restart_point, L_SET) < 0) {
			fprintf(stderr, "local: %s: %s\n", local,
				strerror(errno));
			if (closefunc != NULL)
				(*closefunc)(fout);
			return;
		}
		errno = d = 0;
		while ((c = read(fileno(din), buf, bufsize)) > 0) {
			if ((d = write(fileno(fout), buf, c)) != c)
				break;
			bytes += c;
			if (hash) {
				while (bytes >= hashbytes) {
					putchar('#');
					hashbytes += HASHBYTES;
				}
				fflush(stdout);
			}
		}
		if (hash && bytes > 0) {
			if (bytes < HASHBYTES)
				putchar('#');
			putchar('\n');
			fflush(stdout);
		}
		if (c < 0) {
			if (errno != EPIPE)
				perror("netin");
			bytes = -1;
		}
		if (d < c) {
			if (d < 0)
				fprintf(stderr, "local: %s: %s\n", local,
					strerror(errno));
			else
				fprintf(stderr, "%s: short write\n", local);
		}
		break;

	case TYPE_A:
		if (restart_point) {
			int i, n, ch;

			if (fseek(fout, 0L, L_SET) < 0)
				goto done;
			n = restart_point;
			for (i = 0; i++ < n;) {
				if ((ch = getc(fout)) == EOF)
					goto done;
				if (ch == '\n')
					i++;
			}
			if (fseek(fout, 0L, L_INCR) < 0) {
done:
				fprintf(stderr, "local: %s: %s\n", local,
					strerror(errno));
				if (closefunc != NULL)
					(*closefunc)(fout);
				return;
			}
		}
		while ((c = getc(din)) != EOF) {
			if (c == '\n')
				bare_lfs++;
			while (c == '\r') {
				while (hash && (bytes >= hashbytes)) {
					putchar('#');
					fflush(stdout);
					hashbytes += HASHBYTES;
				}
				bytes++;
				if ((c = getc(din)) != '\n' || tcrflag) {
					if (ferror(fout))
						goto break2;
					putc('\r', fout);
					if (c == '\0') {
						bytes++;
						goto contin2;
					}
					if (c == EOF)
						goto contin2;
				}
			}
			putc(c, fout);
			bytes++;
	contin2:	;
		}
break2:
		if (bare_lfs) {
			printf("WARNING! %d bare linefeeds received in ASCII mode\n", bare_lfs);
			printf("File may not have transferred correctly.\n");
		}
		if (hash) {
			if (bytes < hashbytes)
				putchar('#');
			putchar('\n');
			fflush(stdout);
		}
		if (ferror(din)) {
			if (errno != EPIPE)
				perror("netin");
			bytes = -1;
		}
		if (ferror(fout))
			fprintf(stderr, "local: %s: %s\n", local,
				strerror(errno));
		break;
	}
	if (closefunc != NULL)
		(*closefunc)(fout);
	signal(SIGINT, oldintr);
	if (oldintp)
		signal(SIGPIPE, oldintp);
	gettimeofday(&stop, (struct timezone *)0); 
	fclose(din);
	getreply(0);
	if (bytes > 0 && is_retr)
		ptransfer("received", bytes, &start, &stop);
	return;
abort:

/* abort using RFC959 recommended IP,SYNC sequence  */
	gettimeofday(&stop, (struct timezone *)0);
	if (oldintp)
		signal(SIGPIPE, oldintr);
	signal(SIGINT, SIG_IGN);
	if (!cpend) {
		code = -1;
		signal(SIGINT, oldintr);
		return;
	}

	abort_remote(din);
	code = -1;
	if (data >= 0) {
		close(data);
		data = -1;
	}
	if (closefunc != NULL && fout != NULL)
		(*closefunc)(fout);
	if (din)
		fclose(din);
	if (bytes > 0)
		ptransfer("received", bytes, &start, &stop);
	signal(SIGINT, oldintr);
}

/*
 * Need to start a listen on the data channel before we send the command,
 * otherwise the server's connect may fail.
 */
int initconn (void)
{
unsigned char *p, *a;
int result, len, tmpno = 0;
int on = 1;

noport:
	data_addr = myctladdr;
	if (sendport)
		data_addr.sin_port = 0;	/* let system pick one */ 
	if (data != -1)
		close(data);
	data = socket(AF_INET, SOCK_STREAM, 0);
	if (data < 0) {
		perror("ftp: socket");
		if (tmpno)
			sendport = 1;
		return (1);
	}

	if (!sendport)
		if (setsockopt(data, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof (on)) < 0) {
			perror("ftp: setsockopt (reuse address)");
			goto bad;
		}

	if (bind(data, (struct __SOCKADDR *)&data_addr, sizeof (data_addr)) < 0) {
		perror("ftp: bind");
		goto bad;
	}

	if (options & SO_DEBUG && setsockopt(data, SOL_SOCKET, SO_DEBUG, (char *)&on, sizeof (on)) < 0)
		perror("ftp: setsockopt (ignored)");

	len = sizeof (data_addr);

	if (getsockname(data, (struct __SOCKADDR *)&data_addr, &len) < 0) {
		perror("ftp: getsockname");
		goto bad;
	}

	if (listen(data, 1) < 0)
		perror("ftp: listen");
	if (sendport) {
		a = (unsigned char *)&data_addr.sin_addr;
		p = (unsigned char *)&data_addr.sin_port;
#define	UC(b)	(((unsigned int)b)&0xff)
		result =
		    command("PORT %d,%d,%d,%d,%d,%d",
		      UC(a[0]), UC(a[1]), UC(a[2]), UC(a[3]),
		      UC(p[0]), UC(p[1]));
		if (result == ERROR && sendport == -1) {
			sendport = 0;
			tmpno = 1;
			goto noport;
		}
		return (result != COMPLETE);
	}
	if (tmpno)
		sendport = 1;
	return (0);
bad:
	close(data), data = -1;
	if (tmpno)
		sendport = 1;
	return (1);
}

FILE *dataconn (char *lmode)
{
struct sockaddr_in from;
int s, fromlen = sizeof (from), tos;

	s = accept(data, (struct __SOCKADDR *) &from, &fromlen);
	if (s < 0) {
		perror("ftp: accept");
		close(data), data = -1;
		return (NULL);
	}
	close(data);
	data = s;
	return (fdopen(data, lmode));
}

void ptransfer(char *direction, long bytes, struct timeval *t0, struct timeval *t1)
{
struct timeval td;
float s, bs;

	if (verbose) {
		tvsub(&td, t1, t0);
		s = td.tv_sec + (td.tv_usec / 1000000.);
#define	nz(x)	((x) == 0 ? 1. : (x))
		bs = bytes / nz(s);
		printf("%ld bytes %s in %.2g seconds (%.2g Kbytes/s)\n",
		    bytes, direction, s, bs / 1024.);
	}
}

void tvadd (struct timeval *tsum, struct timeval *t0)
{
	tsum->tv_sec += t0->tv_sec;
	tsum->tv_usec += t0->tv_usec;   
	if (tsum->tv_usec > 1000000)
		tsum->tv_sec++, tsum->tv_usec -= 1000000;
} 

void tvsub (struct timeval *tdiff, struct timeval *t1, struct timeval *t0)
{
	tdiff->tv_sec = t1->tv_sec - t0->tv_sec;
	tdiff->tv_usec = t1->tv_usec - t0->tv_usec;
	if (tdiff->tv_usec < 0)
		tdiff->tv_sec--, tdiff->tv_usec += 1000000;
}

#pragma databank 1
void psabort (int sig, int code)
{
extern int abrtflag;

	abrtflag++;
}
#pragma databank 0

void pswitch (int flag)
{
extern int proxy, abrtflag;
sig_t oldintr;
static struct comvars {
	int connect;
	char name[MAXHOSTNAMELEN];
	struct sockaddr_in mctl;
	struct sockaddr_in hctl;
	FILE *in;
	FILE *out;
	int tpe;
	int curtpe;
	int cpnd;
	int sunqe;
	int runqe;
	int mcse;
	int ntflg;
	char nti[17];
	char nto[17];
	int mapflg;
	char mi[MAXPATHLEN];
	char mo[MAXPATHLEN];
} proxstruct, tmpstruct;
struct comvars *ip, *op;

	abrtflag = 0;
	oldintr = signal(SIGINT, psabort);
	if (flag) {
		if (proxy)
			return;
		ip = &tmpstruct;
		op = &proxstruct;
		proxy++;
	} else {
		if (!proxy)
			return;
		ip = &proxstruct;
		op = &tmpstruct;
		proxy = 0;
	}
	ip->connect = connected;
	connected = op->connect;
	if (hostname) {
		strncpy(ip->name, hostname, sizeof(ip->name) - 1);
		ip->name[strlen(ip->name)] = '\0';
	} else
		ip->name[0] = 0;
	hostname = op->name;
	ip->hctl = hisctladdr;
	hisctladdr = op->hctl;
	ip->mctl = myctladdr;
	myctladdr = op->mctl;
	ip->in = cin;
	cin = op->in;
	ip->out = cout;
	cout = op->out;
	ip->tpe = type;
	type = op->tpe;
	ip->curtpe = curtype;
	curtype = op->curtpe;
	ip->cpnd = cpend;
	cpend = op->cpnd;
	ip->sunqe = sunique;
	sunique = op->sunqe;
	ip->runqe = runique;
	runique = op->runqe;
	ip->mcse = mcase;
	mcase = op->mcse;
	ip->ntflg = ntflag;
	ntflag = op->ntflg;
	strncpy(ip->nti, ntin, 16);
	(ip->nti)[strlen(ip->nti)] = '\0';
	strcpy(ntin, op->nti);
	strncpy(ip->nto, ntout, 16);
	(ip->nto)[strlen(ip->nto)] = '\0';
	strcpy(ntout, op->nto);
	ip->mapflg = mapflag;
	mapflag = op->mapflg;
	strncpy(ip->mi, mapin, MAXPATHLEN - 1);
	(ip->mi)[strlen(ip->mi)] = '\0';
	strcpy(mapin, op->mi);
	strncpy(ip->mo, mapout, MAXPATHLEN - 1);
	(ip->mo)[strlen(ip->mo)] = '\0';
	strcpy(mapout, op->mo);
	signal(SIGINT, oldintr);
	if (abrtflag) {
		abrtflag = 0;
		(*oldintr)(SIGINT, 0);
	}
}

jmp_buf ptabort;
int ptabflg;

#pragma databank 1
void abortpt (int sig, int code)
{
	printf("\n");
	fflush(stdout);
	ptabflg++;
	mflag = 0;
	abrtflag = 0;
	longjmp(ptabort, 1);
}
#pragma databank 0

void proxtrans (char *cmd, char *local, char *remote)
{
sig_t oldintr;
int secndflag = 0, prox_type, nfnd;
extern jmp_buf ptabort;
char *cmd2;
struct fd_set mask;
void abortpt (int sig, int code);

	if (strcmp(cmd, "RETR"))
		cmd2 = "RETR";
	else
		cmd2 = runique ? "STOU" : "STOR";
	if ((prox_type = type) == 0) {
		if (unix_server && unix_proxy)
			prox_type = TYPE_I;
		else
			prox_type = TYPE_A;
	}
	if (curtype != prox_type)
		changetype(prox_type, 1);
	if (command("PASV") != COMPLETE) {
		printf("proxy server does not support third party transfers.\n");
		return;
	}
	pswitch(0);
	if (!connected) {
		printf("No primary connection\n");
		pswitch(1);
		code = -1;
		return;
	}
	if (curtype != prox_type)
		changetype(prox_type, 1);
	if (command("PORT %s", pasv) != COMPLETE) {
		pswitch(1);
		return;
	}
	if (setjmp(ptabort))
		goto abort;
	oldintr = signal(SIGINT, abortpt);
	if (command("%s %s", cmd, remote) != PRELIM) {
		signal(SIGINT, oldintr);
		pswitch(1);
		return;
	}
	sleep(2);
	pswitch(1);
	secndflag++;
	if (command("%s %s", cmd2, local) != PRELIM)
		goto abort;
	ptflag++;
	getreply(0);
	pswitch(0);
	getreply(0);
	signal(SIGINT, oldintr);
	pswitch(1);
	ptflag = 0;
	printf("local: %s remote: %s\n", local, remote);
	return;
abort:
	signal(SIGINT, SIG_IGN);
	ptflag = 0;
	if (strcmp(cmd, "RETR") && !proxy)
		pswitch(1);
	else if (!strcmp(cmd, "RETR") && proxy)
		pswitch(0);
	if (!cpend && !secndflag) {  /* only here if cmd = "STOR" (proxy=1) */
		if (command("%s %s", cmd2, local) != PRELIM) {
			pswitch(0);
			if (cpend)
				abort_remote((FILE *) NULL);
		}
		pswitch(1);
		if (ptabflg)
			code = -1;
		signal(SIGINT, oldintr);
		return;
	}
	if (cpend)
		abort_remote((FILE *) NULL);
	pswitch(!proxy);
	if (!cpend && !secndflag) {  /* only if cmd = "RETR" (proxy=1) */
		if (command("%s %s", cmd2, local) != PRELIM) {
			pswitch(0);
			if (cpend)
				abort_remote((FILE *) NULL);
			pswitch(1);
			if (ptabflg)
				code = -1;
			signal(SIGINT, oldintr);
			return;
		}
	}
	if (cpend)
		abort_remote((FILE *) NULL);
	pswitch(!proxy);
	if (cpend) {
		FD_ZERO(&mask);
		FD_SET(fileno(cin), &mask);
		if ((nfnd = empty(&mask, 10)) <= 0) {
			if (nfnd < 0) {
				perror("abort");
			}
			if (ptabflg)
				code = -1;
			lostpeer(0,0);
		}
		getreply(0);
		getreply(0);
	}
	if (proxy)
		pswitch(0);
	pswitch(1);
	if (ptabflg)
		code = -1;
	signal(SIGINT, oldintr);
}

char *gunique (char *local)
{
static char new[MAXPATHLEN];
char *cp;
int d, count=0;
char ext = '1';

    cp = rindex(local, (int) '/');

	if (cp)
		*cp = '\0';
	d = access(cp ? local : ".", 2);
	if (cp)
		*cp = '/';
	if (d < 0) {
		fprintf(stderr, "local: %s: %s\n", local, strerror(errno));
		return((char *) 0);
	}
	strcpy(new, local);
	cp = new + strlen(new);
	*cp++ = '.';
	while (!d) {
		if (++count == 100) {
			printf("runique: can't find unique file name.\n");
			return((char *) 0);
		}
		*cp++ = ext;
		*cp = '\0';
		if (ext == '9')
			ext = '0';
		else
			ext++;
		if ((d = access(new, 0)) < 0)
			break;
		if (ext != '0')
			cp--;
		else if (*(cp - 2) == '.')
			*(cp - 1) = '1';
		else {
			*(cp - 2) = *(cp - 2) + 1;
			cp--;
		}
	}
	return(new);
}

void abort_remote (FILE *din)
{
static char buf[BUFSIZ];
int nfnd;
struct fd_set mask;

	/*
	 * send IAC in urgent mode instead of DM because 4.3BSD places oob mark
	 * after urgent byte rather than before as is protocol now
	 */
	sprintf(buf, "%c%c%c", IAC, IP, IAC);
	if (send(fileno(cout), buf, 3, MSG_OOB) != 3)
		perror("abort");
	fprintf(cout,"%cABOR\n", DM);
	fflush(cout);
	FD_ZERO(&mask);
	FD_SET(fileno(cin), &mask);
	if (din) { 
		FD_SET(fileno(din), &mask);
	}
	if ((nfnd = empty(&mask, 10)) <= 0) {
		if (nfnd < 0) {
			perror("abort");
		}
		if (ptabflg)
			code = -1;
		lostpeer(0,0);
	}
	if (din && FD_ISSET(fileno(din), &mask)) {
		while (read(fileno(din), buf, BUFSIZ) > 0)
			/* LOOP */;
	}
	if (getreply(0) == ERROR && code == 552) {
		/* 552 needed for nic style abort */
		getreply(0);
	}
	getreply(0);
}
