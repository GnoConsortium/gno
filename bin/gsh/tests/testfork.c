/*
 * testfork: See how fork/_execve/wait/kill interact
 *
 * Written by Dave Tribby to test gsh  *  Oct 1998
 */

#define INCLUDE_WAIT
#define INCLUDE_PWAIT

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/wait.h> 
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <orca.h>

int  fork_mutex = 0;		/* Mutual exclusion during fork */
int  event_mutex = 0;		/* Mutual exclusion while adding an event */
int  wait_in_handler = 0;	/* Should wait() be called in sig handler? */
int  root_pid;			/* Root program's pid */
int  child1_pid;		/* Child program's pid */

/* Values to be passed to _execve() */
char *progname;
char *cmdline;

/* Default values for _execve() if none are provided on command line */
char *d_progname="/bin/ps";
char *d_cmdline="/bin/ps -l";



/* ---------- Data and code for handling program events ----------*/

/* Structure for recording program events */
typedef struct {
	int	pid;
	int	verb;
        int	v1,v2;
	} EventRec;

EventRec *prog_events;		/* Pointer to array of events */
int num_events=0, max_events;	/* Current # of events and maximum */

/* Values for verb in event record */
enum { e_start, e_fork, e_forkmc, e_exec, e_exfail, e_sigp0, e_sigp1,
	e_wait, e_kill, e_getsig, e_noproc, e_end };
char *e_name[] ={"Start", "Fork  child pid =", "Fork mutex count =",
		"_execve", "_execve failed", "Calling sigpause",
                "Sigpause returned", "Wait returns child",
                "Kill returns", "Receive signal", "Process not found!", "End"};


/*
 * Add a program event
 */
#pragma databank 1
void add_event(int pid, int verb, int v1, int v2)
{
	EventRec *p;

	/* Lock the event mutual exclusion key */
/* -------------------------------------------------
	asm {
	        phb
                phk
                plb
		lda #1
	test:	tsb event_mutex
                beq alldone
                cop 0x7f
		bra test
	alldone: plb
        }
---------------------------------------------------- */

	if (num_events >= max_events )   {
	        prog_events = realloc((char *)prog_events, 64*sizeof(EventRec));
                max_events += 64;
                }
	p = &(prog_events[num_events]);
        p->pid = pid;
        p->verb = verb;
        p->v1 = v1;
        p->v2 = v2;
        num_events++;

        /* Unlock the event mutual exclusion key */
        event_mutex = 0;
}
#pragma databank 0


/*
 * Print program event records
 */
void print_events(void)
{
	int i;
	EventRec *p;

	p =  prog_events;
        for (i=0; i < num_events; i++) {
		printf("%3d: %4d %s", i, p->pid, e_name[p->verb]);
		switch (p->verb) {
			case e_getsig:
			case e_fork:
			case e_forkmc:
				printf(" %d", p->v1);
	                        break;
			case e_wait:
			case e_kill:
				printf(" pid %d status = 0x%04X",
                                	p->v1,p->v2);
	                        break;                                
                	}
		printf("\n");
                p++;
	        }
}




/* ---------- Data and code for handling process list ----------*/

#define MAX_SIG 33

/* Linked list for each invocation of this program sharing the same space */
struct ProgListRec {
	struct ProgListRec *next;
	int pid;
        int *sig_rcv;
        } ProgListRec;
typedef struct ProgListRec ProgListRec;

ProgListRec *plist_head = NULL;

/*
 * Clear the signal received array
 */
#pragma databank 1
void clear_sig_flags(int *sig_array)
{
        int sig_num;
        for (sig_num = 0; sig_num < MAX_SIG; sig_num++)   {
           sig_array[sig_num] = 0;
           }
}
#pragma databank 0

/*
 * Create a new entry in the process list
 */
#pragma databank 1
void new_process_entry(int pid)
{
	ProgListRec *new;

        new = malloc(sizeof(ProgListRec));
        new->pid = pid;
        new->sig_rcv = malloc(MAX_SIG*sizeof(int));
        clear_sig_flags(new->sig_rcv);
        new->next = plist_head;
        plist_head = new;
}
#pragma databank 0
             

#pragma databank 1
ProgListRec *find_process_entry(int my_pid)
{
	ProgListRec *plist;

        /* Find process list entry and set signal received bit */
        plist = plist_head;
        while (plist && plist->pid != my_pid)   {
 		plist = plist->next;
		}
	if (!plist)   {
	        add_event(my_pid, e_noproc, 0, 0);
		}	
	return plist;
}
#pragma databank 0

/*
 * Print process list
 */
#pragma databank 1
void print_proc_list(void)
{
	ProgListRec *plist;
        int signum;

        plist = plist_head;
        while (plist)   {
		printf("Process # %d received signals:", plist->pid);
                for (signum = 1; signum < MAX_SIG; signum++)   {
			if (plist->sig_rcv[signum])
                        	printf(" %d", signum);
                	}
                printf("\n");
 		plist = plist->next;
		}
}
#pragma databank 0



/* ---------------------------------------------------------------*/

                       
/*
 * Entry point for forked process to schedule "progname"
 */
#pragma databank 1
void prog_exec(void)
{
	int child_pid;
        int my_pid;

	/* Get process id */
        my_pid = getpid();

	fork_mutex = 0;
        add_event(my_pid, e_exec, 0, 0);
        child_pid = _execve(progname,cmdline);
        add_event(my_pid, e_exfail, 0, 0);
}
#pragma databank 0


/*
 * Signal handler
 */
#pragma databank 1
void SigHandler(int sig, int code)
{
        pid_t wpid;
	union wait status;
        int my_pid;
	ProgListRec *pentry;


	/* Get process id */
        my_pid = getpid();

	add_event(my_pid, e_getsig, sig, code);

        /* Find process list entry and set signal received bit */
	if (pentry = find_process_entry(my_pid))   {
        	pentry->sig_rcv[sig] = 1;
                }

	switch (sig) {
		case SIGCHLD:
                	if (wait_in_handler)   {
				/* Get child's completion status */
			        wpid = wait(&status);
                                add_event(my_pid, e_wait, wpid, status.w_status);
	                        }
	                /* Make sure parent isn't waiting on mutex */
			fork_mutex = 0;
                        break;
		case SIGUSR1:	/* User-defined signal 1 */
			if (my_pid == root_pid)
		                kill(child1_pid,SIGUSR2);
	                break;
                case SIGUSR2:	/* User-defined signal 2 */
	                break;
		case SIGHUP:	/* Hang-up */
		case SIGTERM:	/* Kill or hang-up */
			fprintf(stderr, "...bye!...\n");
			exit(1);
	}
}
#pragma databank 0


/*
 * Establish signal handlers
 */
#pragma databank 1
void set_sig_handler(void)
{
	signal(SIGINT,  SigHandler);
	signal(SIGTERM, SigHandler);
	signal(SIGHUP,  SigHandler);
        signal(SIGTSTP, SigHandler);
        signal(SIGCHLD, SigHandler);
        signal(SIGUSR1, SigHandler);
        signal(SIGUSR2, SigHandler);
}
#pragma databank 0


/*
 * Wait for foreground process to finish up
 */
#pragma databank 1
void pwait(void)
{
	long oldmask;
        int my_pid;
	ProgListRec *pentry;

	/* Get process id */
        my_pid = getpid();

        /* Wait for signal */
	pentry = find_process_entry(my_pid);
        while (!pentry->sig_rcv[SIGCHLD])   {
	        add_event(my_pid, e_sigp0, 0, 0);
	        sigpause(0);
                add_event(my_pid, e_sigp1, 0, 0);
                }
}
#pragma databank 0


/*
 * Invoke a child program
 */
#pragma databank 1
int invoke(void *addr)
{
        int my_pid;
	int child_pid;
        int mutex_count = 0;

	my_pid = getpid();
        fork_mutex = 1;
	child_pid = fork(addr);
	add_event(my_pid, e_fork, child_pid, 0);
        while (fork_mutex)   {
	        asm {
                	cop 0x7f
                        }
                mutex_count++;
                }
	add_event(my_pid, e_forkmc, mutex_count, 0);
        return child_pid;
}
#pragma databank 0



/*
 * Do process startup stuff, and invoke the proper child
 */
#pragma databank 1
void command(int pid, void *addr)
{
	long oldmask;
	int child_pid;
        int kill_stat;

	/* Temporarially block reception of all signals */
        oldmask = sigblock(-1L);

	/* Invoke the appropriate child process */
	child_pid = invoke(addr);

	/* Enable the signal handlers for current process */
	set_sig_handler();

	/* Create process entry for this process */
	new_process_entry(pid);
        
        /* Restore previous signal mask */
        sigsetmask(oldmask);

	/* If this is the child, let parent know we're here */
	if (pid != root_pid)
		kill(root_pid,SIGUSR1);

	/* If child process is still running, wait for it to complete */
	kill_stat = kill(child_pid,0);
	add_event(pid, e_kill, child_pid, kill_stat);
	if (!kill_stat) {
		pwait();
		}
}
#pragma databank 0



/*
 * Forked child of root process starts here
 */
int process2(void)
#pragma databank 1
{
	child1_pid = getpid();
	add_event(child1_pid, e_start, 0, 0);

	command(child1_pid,&prog_exec);

        add_event(child1_pid, e_end, 0, 0);
        return 1;
}
#pragma databank 0


/*
 * Main program starts here
 */
int main(int argc, char *argv[])
{
        pid_t wpid;
	union wait status;
        int argnum;

	root_pid = getpid();
	max_events = 64;
	if (!(prog_events = malloc(64*sizeof(EventRec))))   {
	        fprintf(stderr, "ERROR: cannot malloc %d bytes\n",
			64*sizeof(EventRec));
		exit(1);
                }
                                                                  
        /* Print listing of events and process list when program terminates */
	atexit(print_events);
	atexit(print_proc_list);

	/* Set value of grandchild program to be executed */
        if (argc <= 1)   {
		/* Use default if nothing provided in arguments */
		progname = d_progname;
		cmdline = d_cmdline;
        	}
        else {
	        progname = argv[1];
                /* Command line for _execve() is root program's */
                /* cmd line without the original program name.  */
                cmdline = commandline();
                while (*cmdline && *cmdline != ' ') cmdline++;
                while (*cmdline && *cmdline == ' ') cmdline++;
		}

	/* Record that we made it through startup */
	add_event(root_pid, e_start, 0, 0);

#ifdef INCLUDE_WAIT
	/* This section of code does a simple wait */
	wait_in_handler = 0;
        signal(SIGCHLD, SigHandler);
	invoke(&prog_exec);
        wpid = wait(&status);
	add_event(root_pid, e_wait, wpid, status.w_status);
#endif

#ifdef INCLUDE_PWAIT
	/* This section of code does a more complex wait two levels deep */
	wait_in_handler = 1;

	command(root_pid,&process2);
#endif

	add_event(root_pid, e_end, 0, 0);

	return 0;
}
