/*
 * conf.h --	Configuration and size constants.  Note that most of these
 *		are meaningless on the IIgs.
 *
 * $Id: conf.h,v 1.1 1997/02/28 04:42:06 gdr Exp $
 */

#ifndef _GNO_CONF_H_
#define _GNO_CONF_H_

/*
 * the following defines can be changed but not deleted
 */
 
#define NPROC 32 /* max user processes */

/*
 * the following defines can be changed or deleted
 */
 
#define GNOVERSION \
	"for GS/OS 3.0 on Apple IIgs (65816) (5/28/91) - Jawaid Bazyar"
#define MESSAGE		/* message passing available */
#define NSEM	50	/* total number of semaphores */
#define RTCLOCK		/* system has a real-time clock */
#define QUANTUM	100	/* milliseconds a process is allowed 
			   to run without being rescheduled */

#define DEBUG_RESCHED	/* if defined, displays information
			   about the processes being scheduled */

#endif	/* _GNO_CONF_H_ */
