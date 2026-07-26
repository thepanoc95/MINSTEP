/* gray.h - Halftone/gray screen interface for PostScript 1.0 */

#ifndef GRAY_H
#define GRAY_H

#include "graphics.h"

extern Screen curScreen;
extern ColorPart curMinGray, curMaxGray;
extern SCANTYPE *grayPattern;
extern SCANTYPE *grayPatternEnd;
extern integer grayN, grayS;

extern procedure SetUpGrayPattern(/* ColorPart gray, Screen screen */);
extern Screen SetUpScreen(/* real freq, real angle, Object filterFn */);
extern procedure AddScreenRef(/* Screen screen */);
extern procedure RemoveScreenRef(/* Screen screen */);
extern procedure GrayInit(/* InitReason */);

#endif
