/*
 * STEVIE - Simply Try this Editor for VI Enthusiasts
 *
 * Code Contributions By : Tim Thompson           twitch!tjt
 *                         Tony Andrews           onecom!wldrdg!tony 
 *                         G. R. (Fred) Walter    watmath!grwalter 
 */

/*
 * Definitions of various common control characters 
 */

#define	NUL			'\000'
#define	BS			'\010'
#define	BS_STR			"\010"
#define	TAB			'\011'
#define	NL			'\012'
#define	NL_STR			"\012"
#define	CR			'\015'
#define	ESC			'\033'
#define	ESC_STR			"\033"

#define	UNDO_SHIFTJ		'\333'
#define	UNDO_SHIFTJ_STR		"\333"

#define	ENABLE_REDRAWING	'\334'
#define	ENABLE_REDRAWING_STR	"\334"

#define	CTRL(x)	((x) & 0x1f)
