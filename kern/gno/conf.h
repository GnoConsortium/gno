/*	$Id: conf.h,v 1.1 1998/02/02 08:18:21 taubert Exp $ */

/* conf.h - configuration and size constants */

/* the following defines can be changed but not deleted */
#define NPROC 32 /* max user processes */

/* the following defines can be changed or deleted */
#define GNOVERSION "for GS/OS 3.0 on Apple IIgs (65816) (5/28/91) - Jawaid Bazyar"
#define MESSAGE /* message passing available */
#define NSEM 200 /* total number of semaphores */
#define RTCLOCK /* system has a real-time clock */
#define QUANTUM 100 /* milliseconds a process is allowed 
				   to run without being rescheduled */

#define DEBUG_RESCHED /* if defined, displays information \
					about the processes being scheduled */

/* note: most of these are meaningless on the IIgs */
