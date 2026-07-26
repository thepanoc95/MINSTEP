/* gray.c - Halftone/gray screen (stub) for PostScript 1.0 */

#include "postscript.h"
#include "graphics.h"
#include "gray.h"

public Screen curScreen;
public ColorPart curMinGray, curMaxGray;
public SCANTYPE *grayPattern;
public SCANTYPE *grayPatternEnd;
public integer grayN, grayS;

public procedure SetUpGrayPattern(gray, screen)
  ColorPart gray; Screen screen;
{
  curScreen = screen;
  curMinGray = gray;
  curMaxGray = gray + 1;
}

public Screen SetUpScreen(freq, angle, filterFn)
  real freq, angle; Object filterFn;
{
  Screen s;
  s = (Screen)NEW(1, sizeof(ScreenRec));
  s->freq = freq;
  s->angle = angle;
  s->filterFn = filterFn;
  s->w = 1; s->h = 1; s->d = 0;
  s->ref = 1;
  s->grays = (ColorPartPtr)NEW(MAXCOLOR + 1, sizeof(ColorPart));
  return s;
}

public procedure AddScreenRef(screen) Screen screen;
{
  if (screen != NIL) screen->ref++;
}

public procedure RemoveScreenRef(screen) Screen screen;
{
  if (screen != NIL) {
    if (--screen->ref == 0) {
      FREE((charptr)screen->grays);
      FREE((charptr)screen);
    }
  }
}

public procedure GrayInit(reason) InitReason reason;
{
  grayPattern = NIL;
  grayPatternEnd = NIL;
  grayN = 0;
  grayS = 0;
}
