#pragma nda daOpen daClose daAction daInit 0xFFFF 0 "  Suspend...\\H***\xBDZ"

#include <signal.h>
#include <sgtty.h>
#include <types.h>
#include <quickdraw.h>
#include <event.h>
#include <control.h>
#include <window.h>
#include <gno/gno.h>
#include <misctool.h>

#pragma databank 1
void stophandle(int sig, int code)
{
byte *bf = (byte *) 0xE100FFl;
byte old;
struct sgttyb sb;
MouseRec oldmr;
ClampRec cl1,cl2;
unsigned int button;

  /* Save off some important information */
    GrafOff();
    gtty(1,&sb);
    sb.sg_flags &= ~RAW;
    stty(1,&sb);

    oldmr = ReadMouse();
    cl1 = GetAbsClamp();
    cl2 = GetMouseClamp();

    old = *bf;
    *bf = 0;
    kill(getpid(),SIGSTOP);
    *bf = old;

  /* Get things set back up for the desktop application again */
    SetMouse(oldmr.mouseMode);
    SetAbsClamp(cl1);
    ClampMouse(cl2);
    /*HomeMouse();*/

    sb.sg_flags |= RAW;
    stty(1,&sb);
    button = Button(0);
    FakeMouse(1,0,oldmr.xPos,oldmr.yPos,button);
    GrafOn();
}
#pragma databank 0

#define NULL1 ((Pointer)NULL)

void *daOpen(void)
{
int oops;

    if (!needsgno()) {
    	oops = AlertWindow(0x0000,NULL1,
               (Ref)"22/Suspend requires that the GNO kernel be active./Cancel");
        return NULL;
    }
    signal(SIGTSTP,stophandle);
    /* This signal will not be delivered immediately because we're
       currently inside a tool call.  Set up a handler to do the dirty
       work outside a tool call */
    kill(getpid(),SIGTSTP);
    return NULL;
}

void daClose(void)
{
}

void daAction(long param, int foo)
{
}

void daInit(int foo)
{
}
