/*
 * This implementation of syslogd was written by Devin Reade for GNO v2.0.6.
 * It was written because Phil Vandry's original version of syslogd didn't
 * have sufficient bits for the facility/priority values, and at the time
 * his sources weren't available for modification.
 *
 * $Id: syslogd2.c,v 1.2 2012/08/26 02:55:00 gdr Exp $
 */

/*
 * To Do:
 *	- get all our configuration from the syslog.conf file, and use that
 *	  information in logMessage()
 *	- background ourselves
 *	- eliminate the command line args, except for "-f conf_file"
 *	- internal error messages should go through a logInternal
 *	  routine so that they can potentially go to a file as well
 *	  as (instead of?) the console
 *	- the main loop should be rewritten so that it immediately copies
 *	  the data and releases the caller rather than waiting until it
 *	  has parsed the buffer.
 *	- eliminate various pieces of dead code, clean up comments
 *	- ensure all routines are properly documented
 *	- should writeConsole be exported to libc?
 *	- in order to do 'mark' entries, we could set up a signal handler
 *	  for SIGALRM.  We'll either need a semaphore or a signal mask
 *	  set up so that we can't get interrupted by SIGHUP or SIGTERM
 *	  while we're in the SIGALRM handler.
 *	- should logInternal be using the internal "none" priority?
 *	- logMessage should be modified to ensure that there is a trailing
 *	  newline.  Internal messages (at least) via logInternal are currently
 *	  missing newlines.
 */
 
/*
 * How many processes can send to us before they block?  We don't need many
 * since syslog(3), vsyslog(3), syslogmt(3), and vsyslogmt(3) won't return
 * until we handle the request, anyway.
 *
 * Since sendPort() in the syslog(3) implementation is currently doing a
 * busy wait while waiting for syslogd to release it's buffer, we set NPORTS
 * to 1 so that any additional senders get blocked by the kernel instead of
 * using up clock cycles.
 */
#define NPORTS	1

/*
 * Define DEBUG to get syslogd to exit after a NLOOPS loops.  Why do we
 * need this?  Because there is a bug in the v2.0.6 kernel which kills our
 * shell if we send a SIGTERM to a child process.  Blech.  Limiting the
 * number of loops lets us proceed with debugging without always having to
 * log in again.  See PR#53 in the GNO bug report system.
 *
#if 0
#define DEBUG
#define NLOOPS 5
#endif

/* We need this for internal structs in <sys/syslog.h> */
#define __SYSLOG_INTERNALS

#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/ports.h>
#include <sys/syslog.h>
#include <time.h>
#include <errno.h>
#include <gno/gno.h>
#include <signal.h>
#include <paths.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <memory.h>
#include <misctool.h>
#ifdef __STACK_CHECK__
#include <err.h>
#endif
#include <assert.h>
#include <ctype.h>	/* debugging only */
#include "syslogd.h"

#ifndef EOF
#define EOF (-1)
#endif

#define MIN(a,b) ((a) < (b) ? (a) :(b))

#if 0
static void	die (const char *message);
#endif
static int	logMessage (int facpri, char *msg, int len);
static int	writeConsole (const char *buf, size_t size);
#if 0
static void	handle_HUP (int sig, int code);
#endif
static void	handle_TERM (int sig, int code);


int	Port = -1;
char	MessageBuffer[_SYSLOG_BUFFERLEN];
int	MessageBufferLen = 0;		/* number of used chars in MessageBuffer */
int	bytesToCopy;

int	FacPri;				/* facility/priorty */
time_t	Now;

char *	LogFile = NULL;			/* temporary kludge */
int	LogConsole = 0;			/* temporary kludge */

static void handleVersionZero (SyslogDataBuffer0_t *dataptr0);

int
main(int argc, char **argv) {
        int fd, ch, isVersionZero;
	union {
		SyslogDataBuffer0_t *v0;
	        SyslogDataBuffer_t *vN;
	} datap;
        char *p, *q;
	Handle datahandle;
	Word oldID, myID;

#ifdef DEBUG
	int loopcount = 0;
#endif

#ifdef __STACK_CHECK__
	/*
	 * Don't use the __REPORT_STACK() macro here; we don't want
	 * to have to call atexit() since we may be in a signal handler
	 * when we have to die.
	 */
	_beginStackCheck();
#endif

	/* We'll be needing our mem mgr ID later ... */
	myID = _getUserID();

	while ((ch = getopt(argc, argv, "cF:")) != EOF) {
		switch(ch) {
		case 'c':
			LogConsole = 1;
			break;
		case 'F':
			LogFile = optarg;
			break;
		default:
			logInternal("usage: syslogd [-c] [-F logfile]");
			exit(1);
		}
	}

#if 0
	{
		char *myname;
		myname = __prognameGS();
		writeConsole(argv[0], strlen(argv[0]));
		writeConsole("/", 1);
		writeConsole(myname, strlen(myname));
	}
#endif
		
#if 0
	signal(SIGHUP,  handle_HUP);
#endif
	signal(SIGTERM, handle_TERM);

	/* detach from controlling terminal */
	if ((fd = open(_PATH_TTY, O_RDWR)) < 0) {
		logInternal("couldn't open controlling terminal: %s",
			    strerror(errno));
		exit(1);
	}

#if 0	
	if (tcnewpgrp(fd) < 0) {
		logInternal("tcnewpgrp failed: %s", strerror(errno));
		exit(1);
	}
#endif

	if (ioctl(fd, TIOCNOTTY, 0) < 0) {
		logInternal("ioctl failed: %s", strerror(errno));
		exit(1);
	}
	close(fd);

	/* create and bind a port on which programs can contact us */
	if ((Port = pcreate(NPORTS)) == -1) {
		/*
		 * Does this actually ever happen?  The ports(2) man page
		 * does not document an error condition for pcreate(2).
		 */
		logInternal("couldn't create port: %s", strerror(errno));
		exit(1);
	}
	logInternal("DEBUG: got port %d", Port);
#undef  __SYSLOG_PORT_NAME
#define __SYSLOG_PORT_NAME "syslogd_test"

	if (pbind (Port, __SYSLOG_PORT_NAME) == -1) {
#if 0
		logInternal("DEBUG: Bound port first time");
		/* KLUDGE KLUDGE KLUDGE
		 * Kill off the old v1 syslogd
		 */
		kill(3, SIGTERM);
		sleep(1);
		if (pbind(Port, __SYSLOG_PORT_NAME) == -1) {
#endif
		logInternal("couldn't bind port: %s", strerror(errno));
		exit(1);
#if 0
		}
#endif
	}
	logInternal("DEBUG: bound to port");

	/* now loop forever waiting for messages */
	for (;;) {
#ifdef DEBUG
		loopcount++;
		if (loopcount > NLOOPS) {
# ifdef __STACK_CHECK__
			logInternal("%d bytes used", _endStackCheck());
# endif
			logInternal("Done debugging loops.  Exiting.");
			exit(0);
		}
#endif
	
		logInternal("Entering loop");

		/* block until a message comes in */
		datahandle = (Handle) preceive(Port);

		/* paranoia */
		if (datahandle == NULL) {
			logInternal("Received NULL pointer. Discarded.");
			continue;
		}
		CheckHandle(datahandle);
		if (_toolErr) {
			if (_toolErr == handleErr) {
				logInternal("invalid handle 0x%lx: message dropped",
					    (unsigned long) datahandle);
			} else {
				logInternal("CheckHandle on 0x%lx failed with code %d",
					    (unsigned long) datahandle,
					    _toolErr);
			}
			continue;
		}

		/* Change the memory block to be owned by our ID */
		oldID = SetHandleID(myID, datahandle);

		/* lock the handle */
		HLock(datahandle);
		if (_toolErr) {
			logInternal("HLock on 0x%lx failed with code %d",
				    (unsigned long) datahandle, _toolErr);
		}

		/* , and
		 * free up the old ID
		 */

		{
		  int *iptr, j;
		  char *cptr, c;

		  iptr = (int *) *datahandle;
		  cptr = (char *) &iptr[5];
		  logInternal("buffer: %d %d %d %d %d:\r",
			      iptr[0], iptr[1], iptr[2], iptr[3], iptr[4]);
		  for (j=0; j<iptr[4]; j++) {
		    c = cptr[j];
		    logInternal("GSString[%d] = 0x%x '%c'",
				j, (int) c, isprint(c) ? c : '?');
		  }
		  continue;
		}
#ifdef BORK
		/* dereference the handle */
		datap.vN = (SyslogDataBuffer_t *) *datahandle;

		/* determine if it's an old version zero message */
		isVersionZero = (datap.v0->sdb0_version == 0);

		if (isVersionZero) {
			handleVersionZero(datap.v0);
			HUnlock(datahandle); /* ignore errors */
			
			continue;
		} else {
			DeleteID (oldID);
			logInternal("not version zero");
			HUnlock(datahandle); /* ignore errors */
			continue;
		}

		if (isVersionZero) {
			FacPri = datap.v0->sdb0_prio;
		} else {

		/* verify that this isn't a garbage pointer */
		if (datap.vN->sdb_magic != _SYSLOG_MAGIC) {
			logInternal("Bad magic number 0x%X; message "
				    "discarded. Caller may hang.",
				    datap.vN->sdb_magic);
			HUnlock(datahandle); /* ignore errors */
			continue;
		}

		/*
		 * Do the library and daemon agree on the format of the
		 * SyslogDataBuffer_t structure?
		 */
		if (datap.vN->sdb_version != _SYSLOG_STRUCT_VERSION) {
			logInternal("Message version mismatch.  Expected %d "
				    "got %d. Message discarded. Caller may hang.",
				    _SYSLOG_STRUCT_VERSION,
				    datap.vN->sdb_version);
			HUnlock(datahandle); /* ignore errors */
			continue;
		}
		
		/*
		 * Do we have a facility/priority prefix?  It's of the
		 * form "<nnnnn>rest_of_message", where the angle brackets
		 * are literals.  Set FacPri to this value if present,
		 * otherwise set it to the default value, user.notice.
		 *
		 * This should be changed so that we immediately copy the
		 * buffer and any other required info, then release the caller.
		 */
		p = datap.vN->sdb_buffer;
		bytesToCopy = datap.vN->sdb_msglen;
		if (*p == '<') {
			p++;
			FacPri = strtol(p, &q, 10);
			if (p == q) {
				FacPri = LOG_MAKEPRI(LOG_USER, LOG_NOTICE);
				--p;
			} else if (*q == '>') {
				p = q + 1;
			} else {
				p = q;
			}
			bytesToCopy -= (p - datap.vN->sdb_buffer);
		} else {
			FacPri = LOG_MAKEPRI(LOG_USER, LOG_NOTICE);
		}
		}

		/*
		 * At this point, p points the point in the caller's buffer
		 * where we should start copying bytes.  bytesToCopy
		 * contains the number of bytes in the callers buffer that
		 * we should copy.
		 *
		 * If the 'needtime' flag is set, we now copy a time stamp
		 * into our own buffer (the user's buffer is untouched).
		 */
		if (datap.vN->sdb_needtime) {
			time(&Now);
			q = ctime(&Now) + 4;
			q[16] = '\0';
			strcpy(MessageBuffer, q);
			q = MessageBuffer + 16;
			MessageBufferLen = 16;
		} else {
			q = MessageBuffer;
			*q = '\0';
			MessageBufferLen = 0;
		}
		/*
		 * Now we can also say that q points to the point in
		 * MessageBuffer to where we should start copying characters.
		 * MessageBufferLen is the number of characters we've used
		 * in MessageBuffer.
		 *
		 * Copy the message to our buffer, minus any prefix, and
		 * append a newline.  Make sure MessageBuffer is NULL-
		 * terminated.
		 */
		if (bytesToCopy > _SYSLOG_BUFFERLEN-2) {
			bytesToCopy = _SYSLOG_BUFFERLEN-2;
		}
		MessageBufferLen += bytesToCopy;
		if (bytesToCopy == 0) {
			q[0] = '\r';
			q[1] = '\0';
			MessageBufferLen++;
		} else {
			memcpy(q, p, bytesToCopy);
			if (q[bytesToCopy-1] != '\r') {
				q[bytesToCopy++] = '\r';
				MessageBufferLen++;
			}
			q[bytesToCopy] = '\0';
		}
				
		/*
		 * We have our own version of the message now, so we can
		 * release the caller's data buffer and then proceed to print
		 * our own copy.
		 *
		 * See the comments in the syslog(3) code as to why we do
		 * it with a busy-wait.
		 */
		datap.vN->sdb_busywait = 0;

		/* print the message */
		logMessage(FacPri, MessageBuffer, MessageBufferLen);
#endif /* BORK */
	}
	
	/*NOTREACHED*/
	return 0;
}

static void
handleVersionZero (SyslogDataBuffer0_t *dataptr0) 
{
	int *offset;	/* offset from dataptr0 of current string */
	int *length;
	char *string;
	
	if (dataptr0->sdb0_numstrings == 0) {
		logInternal("[zero length message]");
		return;
	}
	offset = (int *) ((char *) dataptr0 + sizeof(SyslogDataBuffer0_t));
	length = (int *) ((char *) dataptr0 + *offset);
	string = ((char *) length) + sizeof(int);

	logMessage(dataptr0->sdb0_prio, string, *length);
	if (dataptr0->sdb0_numstrings > 1) {
		logInternal("Cannot handle multiple strings.  Last %d ignored",
			    dataptr0->sdb0_numstrings -1);
	}
	return;
}

/*
 * Print the message <msg> of length <len> to the relevent files based on
 * the facility/priority value <facpri>.
 *
 * This routine is still a kludge.  We need information parsed from the
 * /etc/syslog.conf file to be referenced here.
 *
 * The following globals must already be set:
 *	LogConsole		(temporary kludge)
 *	LogFile			(temporary kludge)
 */
static int
logMessage (int facpri, const char *msg, int len) {
	int result = 0;

	if (LogConsole) {
		if (writeConsole(msg, len) != 0) {
			result = 1;
		}
	}
	if (LogFile != NULL) {
		int logfd;

		if ((logfd = open(LogFile, O_RDWR | O_APPEND)) > -1) {
			/*
			 * Don't throw an error if it fails; the file may
			 * not exist.
			 *
			 * Always having to open/write/close the file is
			 * a pain in the ass, but on the GS we need to do
			 * this to ensure that the file remains readable
			 * to other processes.
			 */
			if (write(logfd, msg, len) < 0) {
				result = 1;
			}
			close(logfd);
		}
	}
	return result;
}

/*
 * Choke and puke.
 * If <message> is not the empty string, print it out to console, first,
 * prefixed by "syslogd: ".
 *
 * Careful what you call here; we may be inside a signal handler.
 */
#if 0
static void
die (const char *message)
{
#define BUFFER_LEN 80
#define HEADER "syslogd: "
	static char buffer[BUFFER_LEN];

	if ((message != NULL) && (*message != '\0')) {
		sprintmt(buffer, BUFFER_LEN, "syslogd: %s\r", message);
		writeConsole(buffer, strlen(buffer));
	}
	kill(getpid(), SIGKILL);
}
#endif

#pragma databank 1
#define MSG_QUIT "received a quit signal\r"

#if 0
static void
handle_HUP (int sig, int code) {
#define MSG_HUP  "received a SIGHUP\r"
	/* we should re-read the config file here */
	writeConsole(MSG_HUP, sizeof(MSG_HUP)-1);
}
#endif

static void
handle_TERM (int sig, int code) {
#if 1
	if (Port != -1) {
		pdelete(Port, NULL);
	}
	kill(getpid(), SIGKILL);
#else
	if (pdelete(Port, NULL) != 0) {
		die("failed to delete port on exit");
	}
	die("");	/* don't print a message */
#endif
}                                                        

#pragma databank 0

static int
writeConsole (const char *buf, size_t size) {
#if 1
	int fd;

	if ((fd = open(_PATH_CONSOLE, O_WRONLY)) < 0) {
		return -1;
	}
	/* we should probably loop to ensure that the write completes */
	write(fd, buf, size);
	close(fd);
	return 0;
#else
#endif
}


#pragma optimize 78
#pragma debug 0

void
logInternal (const char *message, ...) {
#define BUFFER_SIZE 256
	static char buffer[BUFFER_SIZE];
	time_t now;
	va_list ap;
	char *p;

	va_start(ap, message);

	/*
	 * Put our prefix at the front of the message.
	 */
	time(&now);
	p = ctime(&now) + 4;
	p[16] = '\0';
	p = sprintmt(buffer, BUFFER_SIZE, "%s syslogd[%d]: ", p, getpid());
#if 1
	p = vsprintmt(p, BUFFER_SIZE - (p - buffer), message, ap);
#else
	p = vsprintmt(p, p - buffer, message, ap);
#endif

#if 0
	/* this is the one we *should* be using */
	logMessage(LOG_MAKEPRI(LOG_DAEMON, LOG_CRIT), buffer, p - buffer);
#else
	/* for debugging only */
	if ((p - buffer) < BUFFER_SIZE) {
		*p++ = '\r';
		*p = '\0';
	} else {
		*(p-1) = '\r';
	}
	writeConsole(buffer, p-buffer);
#endif
	va_end(ap);
}

