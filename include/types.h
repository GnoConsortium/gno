/********************************************
*
* Types, defines used in more that one header file.
*
* Copyright Apple Computer, Inc.1986-90
* All Rights Reserved
*
* Copyright 1992, 1993, Byte Works, Inc.
*
********************************************/

#ifndef __TYPES__
#define __TYPES__

#ifndef noError
#define noError 0x0000
#endif
#ifndef nil
#define nil 0x0L
#endif
#ifndef NULL
#define NULL 0x0L
#endif
#ifndef NIL
#define NIL 0x0L
#endif

#define dispatcher 0xE10000L            /* tool locator dispatch address */

#define TRUE 1
#define true TRUE
#define FALSE 0
#define false FALSE

/* RefDescriptors */
#define refIsPointer 0x0000
#define refIsHandle 0x0001
#define refIsResource 0x0002
#define refIsNewHandle 0x0003

typedef unsigned char byte, Byte;
typedef unsigned int word, Word;
typedef int integer, Integer;
typedef long longint, Longint;
typedef long Long;
typedef unsigned long longword, Longword, LongWord;
typedef unsigned long Dblword, DblWord; /* retained for back compatibility */
typedef long Fixed;
typedef long Frac;
typedef extended Extended;
typedef extended *ExtendedPtr;
typedef char *ptr, *Ptr, *pointer, *Pointer;
typedef ptr *handle, *Handle;
typedef Handle *HandlePtr;
typedef char *CStringPtr, **CStringHndl, ***CStringHndlPtr;
typedef long (*ProcPtr)(void);          /* retained for back compatibility */
typedef pascal void (*VoidProcPtr)(void);
typedef pascal Word (*WordProcPtr)(void);
typedef pascal LongWord (*LongProcPtr)(void);

typedef unsigned int boolean, Boolean, BOOLEAN;
typedef short OSErr;
typedef int *IntPtr ;
typedef Ptr FPTPtr;

#define String(size) struct {unsigned char textLength; unsigned char text[size];}
typedef String(255) Str255, *StringPtr, **StringHandle;
typedef String(32) Str32, *String32Ptr, **String32Handle;

struct Point {
   short v;
   short h;
   };
typedef struct Point Point, *PointPtr;

struct Rect {
   short v1;
   short h1;
   short v2;
   short h2;
   };
typedef struct Rect Rect, *RectPtr, **RectHndl;

struct TimeRec {
   Byte second;
   Byte minute;
   Byte hour;
   Byte year;
   Byte day;
   Byte month;
   Byte extra;
   Byte weekDay;
   };
typedef struct TimeRec TimeRec, *TimeRecPtr, **TimeRecHndl;

typedef Word RefDescriptor;

extern unsigned _ownerid;
extern int _toolErr;

#ifndef Ref
#define Ref Long
#endif

/* Formerly in GSOS.h */

#ifdef __GNO__
typedef struct GSString {    /* don't use "struct GSString"; use pointers */
   Word length;                         /* Number of chars in text field  */
   char text[1];                        /* Is enlarged dynamically */
} *GSStringPtr, **GSStringHndl;
typedef GSStringHndl *GSStringHndlPtr;
#endif

typedef struct GSString255 {
   Word length;                         /* Number of Chars in text field  */
   char text[255];
   } GSString255, *GSString255Ptr, **GSString255Hndl;
typedef GSString255Hndl *GSString255HndlPtr;

typedef struct GSString32 {
   Word length;                         /* Number of characters in text field */
   char text[32];
   } GSString32, *GSString32Ptr, **GSString32Hndl;

#ifdef __GNO__
typedef struct ResultBuf {      /* don't use "struct ResultBuf"; use pointers */
   Word  bufSize;                    /* Maximum number of chars in text field */
   struct GSString bufString;        /* This one is enlarged dynamically */
} *ResultBufPtr, **ResultBufHndl;
typedef ResultBufHndl *ResultBufHndlPtr ;
#endif

typedef struct ResultBuf255 {
   Word  bufSize;
   GSString255 bufString;
   } ResultBuf255, *ResultBuf255Ptr, **ResultBuf255Hndl;
typedef ResultBuf255Hndl *ResultBuf255HndlPtr ;

typedef struct ResultBuf32 {
   Word  bufSize;
   GSString32 bufString;
   } ResultBuf32, *ResultBuf32Ptr, **ResultBuf32Hndl;

#if defined(__GNO__) && defined(__USE_DYNAMIC_GSSTRING__)
/*
 * Now that we've nicely typedef'd all those structs, we throw most
 * of them away.  Note that this can cause confusion if, for example,
 * one has a GSString32Ptr to which they try to assign a variable of
 * type "pointer to GSString32"; it will cause a type mismatch error
 * that is not exactly obvious as to the cause.
 *
 * So why do it at all?  Because this allows code that knows about (and
 * uses) the dynamic GSStringPtr to have assignments to GS/OS parm blocks
 * without generating an error and without using casts.  Casts are
 * otherwise necessary, but can handle *real* type mismatches, such as
 * that which happens when a GSStringPtr is accidentally cast to a
 * ResultBufPtr.
 *
 * _Don't_ insert a #define for the basic struct here, because that would
 * change the sizeof() the struct and blow the world to pieces.  Defines
 * should only be used for pointer types, because all pointers are the
 * same size.
 */
#define	GSString255Ptr		GSStringPtr
#define	GSString255Hndl		GSStringHndl
#define	GSString255HndlPtr	GSStringHndlPtr

#define	GSString32Ptr		GSStringPtr
#define	GSString32Hndl		GSStringHndl
#define	GSString32HndlPtr	GSStringHndlPtr

#define	ResultBuf255Ptr		ResultBufPtr
#define	ResultBuf255Hndl	ResultBufHndl
#define	ResultBuf255HndlPtr	ResultBufHndlPtr

#define	ResultBuf32Ptr		ResultBufPtr
#define	ResultBuf32Hndl		ResultBufHndl
#define	ResultBuf32HndlPtr	ResultBufHndlPtr

#endif	/* __GNO__ && __USE_DYNAMIC_GSSTRING__ */

/* Formerly in QuickDraw.h */

typedef unsigned char Pattern[32], *PatternPtr;
typedef unsigned char Mask[8];
typedef Word ColorTable[16], *ColorTablePtr, **ColorTableHndl;

/* TextStyle */
#define plainMask 0x0000                /* Mask for plain text bit */
#define boldMask 0x0001                 /* Mask for bold bit */
#define italicMask 0x0002               /* Mask for italic bit */
#define underlineMask 0x0004            /* Mask for underline bit */
#define outlineMask 0x0008              /* Mask for outline bit */
#define shadowMask 0x0010               /* Mask for shadow bit */
#define fUseShadowing 0x8000		/* corrected 26-May-92 DAL */
#define fFastPortAware 0x4000

typedef Integer TextStyle;

struct LocInfo {
   Word portSCB;                        /* SCBByte in low byte */
   Pointer ptrToPixImage;               /* ImageRef */
   Word width;                          /* Width */
   Rect boundsRect;                     /* BoundsRect */
   };
typedef struct LocInfo LocInfo, *LocInfoPtr, **LocInfoHndl;

struct Region {
   Word rgnSize;                        /* size in bytes */
   Rect rgnBBox;                        /* enclosing rectangle */
   };
typedef struct Region Region, *RegionPtr, **RegionHndl;

struct Font {
   Word offseToMF;                      /* fully defined front of the Font record. */
   Word family;
   TextStyle style;
   Word size;
   Word version;
   Word fbrExtent;
   Word highowTLoc;
   };
typedef struct Font Font, *FontPtr, **FontHndl;

union FontID {
   struct {
      Word famNum;
      Byte fontStyle;
      Byte fontSize;
      } fidRec;
   Long fidLong;
   };
typedef union FontID FontID, *FontIDPtr, **FontIDHndl;

struct QDProcs {
   VoidProcPtr stdText;
   VoidProcPtr stdLine;
   VoidProcPtr stdRect;
   VoidProcPtr stdRRect;
   VoidProcPtr stdOval;
   VoidProcPtr stdArc;
   VoidProcPtr stdPoly;
   VoidProcPtr stdRgn;
   VoidProcPtr stdPixels;
   VoidProcPtr stdComment;
   VoidProcPtr stdTxMeas;
   VoidProcPtr stdTxBnds;
   VoidProcPtr stdGetPic;
   VoidProcPtr stdPutPic;
   };
typedef struct QDProcs QDProcs, *QDProcsPtr, **QDProcsHndl;

struct GrafPort {
   LocInfo portInfo;
   Rect portRect;                       /* PortRect */
   RegionHndl clipRgn;                  /* Clip Rgn. Pointer */
   RegionHndl visRgn;                   /* Vis. Rgn. Pointer */
   Pattern bkPat;                       /* BackGround Pattern */
   Point pnLoc;                         /* Pen Location */
   Point pnSize;                        /* Pen Size */
   Word pnMode;                         /* Pen Mode */
   Pattern pnPat;                       /* Pen Pattern */
   Mask pnMask;                         /* Pen Mask */
   Word pnVis;                          /* Pen Visable */
   FontHndl fontHandle;
   FontID fontID;                       /* Font ID */
   Word fontFlags;                      /* FontFlags */
   Word txSize;                         /* Text Size */
   TextStyle txFace;                    /* Text Face */
   Word txMode;                         /* Text Mode */
   Fixed spExtra;                       /* Fixed Point Value */
   Fixed chExtra;                       /* Fixed Point Value */
   Word fgColor;                        /* ForeGround Color */
   Word bgColor;                        /* BackGround Color */
   Handle picSave;                      /* PicSave */
   Handle rgnSave;                      /* RgnSave */
   Handle polySave;                     /* PolySave */
   QDProcsPtr grafProcs;
   Word arcRot;                         /* ArcRot */
   Longint userField;                   /* UserField */
   Longint sysField;                    /* SysField */
   };
typedef struct GrafPort GrafPort, *GrafPortPtr, **GrafPortHndl;

/* Formerly in Control.h */

typedef GrafPortPtr WindowPtr;

struct CtlRec {
   struct CtlRec **ctlNext;             /* Handle of next control. */
   WindowPtr ctlOwner;                  /* Pointer to control's window. */
   Rect ctlRect;                        /* Enclosing rectangle. */
   Byte ctlFlag;                        /* Bit flags. */
   Byte ctlHilite;                      /* Highlighted part. */
   Word ctlValue;                       /* Control's value. */
   LongProcPtr ctlProc;                 /* Control's definition procedure. */
   LongProcPtr ctlAction;               /* Control's action procedure. */
   Longint ctlData;                     /* Reserved for CtrlProc's use. */
   Longint ctlRefCon;                   /* Reserved for application's use. */
   Pointer ctlColor;                    /* Pointer to appropriate color table. */
   Byte ctlReserved[16];                /* Reserved for future expansion */
   LongWord ctlID;
   Word ctlMoreFlags;
   Word ctlVersion;
   };
typedef struct CtlRec CtlRec, *CtlRecPtr, **CtlRecHndl, ***CtlRecHndlPtr;

struct BarColors {
   Word barOutline;                     /* color for outlining bar, arrows, and thumb */
   Word barNorArrow;                    /* color of arrows when not highlighted */
   Word barSelArrow;                    /* color of arrows when highlighted */
   Word barArrowBack;                   /* color of arrow box's background */
   Word barNorThumb;                    /* color of thumb's background when not highlighted */
   Word barSelThumb;                    /* color of thumb's background when highlighted */
   Word barPageRgn;                     /* color and pattern page region: high byte - 1= dither, 0 = solid */
   Word barInactive;                    /* color of scroll bar's interior when inactive */
   };
typedef struct BarColors BarColors, *BarColorsPtr, **BarColorsHndl;

/* Formerly in Event.h */

struct EventRecord {
   Word what;                           /* event code */
   LongWord message;                    /* event message */
   LongWord when;                       /* ticks since startup */
   Point where;                         /* mouse location */
   Word modifiers;                      /* modifier flags */
   LongWord wmTaskData;
   LongWord wmTaskMask;
   LongWord wmLastClickTick;
   Word wmClickCount;
   LongWord wmTaskData2;
   LongWord wmTaskData3;
   LongWord wmTaskData4;
   Point wmLastClickPt;
   };
typedef struct EventRecord EventRecord, *EventRecordPtr, **EventRecordHndl;

/* Formerly in Window.h */

typedef EventRecord WmTaskRec;
typedef EventRecordPtr WmTaskRecPtr;

#endif
