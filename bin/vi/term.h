/*
 * STEVIE - Simply Try this Editor for VI Enthusiasts
 *
 * Code Contributions By : Tim Thompson           twitch!tjt
 *                         Tony Andrews           onecom!wldrdg!tony 
 *                         G. R. (Fred) Walter    watmath!grwalter 
 */

/*
 * This file contains the machine dependent escape sequences that the editor
 * needs to perform various operations. Some of the sequences here are
 * optional. Anything not available should be indicated by a null string.
 *
 * Insert/delete line sequences are necessary.
 */

#ifdef GSOS
#include <termcap.h>
#endif

/*
 * The macro names here correspond (more or less) to the actual ANSI names 
 */

#ifdef	ATARI
#define	T_ED	"\033E"		/* erase display (may optionally home cursor) */
#define	T_EL	"\033l"		/* erase the entire current line */
#define	T_IL	"\033L"		/* insert one line */
#define	T_DL	"\033M"		/* delete one line */
#define	T_CI	"\033f"		/* invisible cursor (very optional) */
#define	T_CV	"\033e"		/* visible cursor (very optional) */
#define T_TP    ""		/* plain text */
#define T_TI    ""		/* inverse-video text */
#endif

#ifdef	UNIX
/* The UNIX sequences are hard-wired for ansi-like terminals. */
#define	T_ED	"\033[2J"	/* erase display (may optionally home cursor) */
#define	T_END_D	"\033[J"	/* erase to end of display */
#define	T_EL	"\033[2K"	/* erase the entire current line */
#define	T_END_L	"\033[K"	/* erase to the end of the current line */
#define	T_IL	"\033[L"	/* insert one line */
#define	T_DL	"\033[M"	/* delete one line */
#define	T_CI	""		/* invisible cursor (very optional) */
#define	T_CV	""		/* visible cursor (very optional) */
#define T_TP    ""		/* plain text */
#define T_TI    ""		/* inverse-video text */
#endif

#ifdef	BSD
/* The BSD 4.3 sequences are hard-wired for ansi-like terminals. */
#define	T_ED	"\033[2J"	/* erase display (may optionally home cursor) */
#define	T_END_D	"\033[J"	/* erase to end of display */
#define	T_EL	"\033[2K"	/* erase the entire current line */
#define	T_END_L	"\033[K"	/* erase to the end of the current line */
#define	T_IL	"\033[L"	/* insert line */
#define	T_DL	"\033[M"	/* delete line */
#define	T_CI	""		/* invisible cursor */
#define	T_CV	""		/* visible cursor */
#define T_TP    "\033[m"	/* plain text */
#define T_TI    "\033[7m"	/* inverse-video text */
#endif

#ifdef	OS2
/*
 * The OS/2 ansi console driver is pretty deficient. No insert or delete line
 * sequences. The erase line sequence only erases from the cursor to the end
 * of the line. For our purposes that works out okay, since the only time
 * T_EL is used is when the cursor is in column 0.
 *
 * The insert/delete line sequences marked here are actually implemented in
 * the file os2.c using direct OS/2 system calls. This makes the capability
 * available for the rest of the editor via appropriate escape sequences
 * passed to outstr().
 */
#define	T_ED	"\033[2J"	/* erase display (may optionally home cursor) */
#define	T_EL	"\033[K"	/* erase the entire current line */
#define	T_END_L	"\033[K"	/* erase to the end of the current line */
#define	T_IL	"\033[L"	/* insert one line - fake (see os2.c) */
#define	T_DL	"\033[M"	/* delete one line - fake (see os2.c) */
#define	T_CI	""		/* invisible cursor (very optional) */
#define	T_CV	""		/* visible cursor (very optional) */
#define T_TP    ""		/* plain text */
#define T_TI    ""		/* inverse-video text */
#endif

#ifdef AMIGA
/*
 * The erase line sequence only erases from the cursor to the end of the
 * line. For our purposes that works out okay, since the only time T_EL is
 * used is when the cursor is in column 0. 
 */
#define	T_ED	"\014"		/* erase display (may optionally home cursor) */
#define	T_END_D	"\033[J"	/* erase to end of display */
#define	T_EL	"\033[K"	/* erase the entire current line */
#define	T_END_L	"\033[K"	/* erase to the end of the current line */
#define	T_IL	"\033["		/* insert line */
#define	T_IL_B	"L"
#define	T_DL	"\033["		/* delete line */
#define	T_DL_B	"M"
#define	T_CI	"\033[0 p"	/* invisible cursor (very optional) */
#define	T_CV	"\033[1 p"	/* visible cursor (very optional) */
#define T_TP    "\033[0m"	/* plain text */
#define T_TI    "\033[7m"	/* inverse-video text */
#endif

#ifdef	DOS
/*
 * DOS sequences
 *
 * Some of the following sequences require the use of the "nansi.sys"
 * console driver. The standard "ansi.sys" driver doesn't support
 * sequences for insert/delete line.
 */
#define	T_ED	"\033[2J"	/* erase display (may optionally home cursor) */
#define	T_EL	"\033[K"	/* erase the entire current line */
#define	T_IL	"\033[L"	/* insert line (requires nansi.sys driver) */
#define	T_DL	"\033[M"	/* delete line (requires nansi.sys driver) */
#define	T_CI	""		/* invisible cursor (very optional) */
#define	T_CV	""		/* visible cursor (very optional) */
#define T_TP    ""		/* plain text */
#define T_TI    ""		/* inverse-video text */
#endif

#ifdef GSOS
/*
 * Apple //gs sequences
 *
 *  the insert and delete line sequences, since they are not supported by
 *  the display firmware :-(, are kluged.  We set a window and scroll either
 *  up or down depending.  But it works.
 */

extern char *tc_ED;
extern char *tc_EL;
extern char *tc_IL;
extern char *tc_DL;
extern char *tc_CI;
extern char *tc_CV;
extern char *tc_TP;
extern char *tc_TI;
extern char *tc_END_D;
extern char *tc_END_L;

#define T_ED tc_ED
#define T_EL tc_EL
#define T_IL tc_IL
#define T_DL tc_DL
#define T_CI tc_CI
#define T_CV tc_CV
#define T_TP tc_TP
#define T_TI tc_TI
#define T_END_D tc_END_D
#define T_END_L tc_END_L

#ifdef NOTDEFINED

#define T_ED "\014" /* erase display */
#define T_EL "\032" /* erase the current line */
#define T_IL "\033L" /* our own user-defined code */
#define T_DL "\033M" /* our own user-defined code */
#define T_CI "\006" /* 006 make cursor invisible */
#define T_CV "\005" /* 005 make cursor visible */
#define T_TP "\016" /* normal display format */
#define T_TI "\017" /* inverse-video */
#define T_END_D "\013" /* clear to end of screen */
#define T_END_L "\035" /* erase to the end of the current line */
#endif /* notdefined */

#endif
