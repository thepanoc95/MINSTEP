/* PostScript X11 Display Device Module

    Copyright 1983 -- Adobe Systems, Inc.
    PostScript is a trademark of Adobe Systems, Inc.
NOTICE:  All information contained herein or attendant hereto is, and
remains, the property of Adobe Systems, Inc.  Many of the intellectual
and technical concepts contained herein are proprietary to Adobe Systems,
Inc. and may be covered by U.S. and Foreign Patents or Patents Pending or
are protected as trade secrets.  Any dissemination of this information or
reproduction of this material are strictly forbidden unless prior written
permission is obtained from Adobe Systems, Inc.

X11 display backend.  Wraps the frame device: opens an X11 window,
delegates to framedevice for the rasterizer, then patches ShowPage
to blit the 1-bit frame buffer to the window.
*/

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define Screen PSScreen
#define Mask PSMask

#include "postscript.h"
#include "graphicspriv.h"
#include "framedevice.h"

#undef Screen
#undef Mask

private Display *x11dpy;
private Window x11win;
private GC x11gc;
private integer x11width, x11height;
private unsigned long x11blackpixel, x11whitepixel;

private procedure X11ShowPage()
{
integer row, col;
SCANTYPE *src;
SCANTYPE bits;
integer scanunits, imgwidth;
Pixmap bm;
GC bmgc;
XGCValues gcv;

if (x11dpy == NIL) return;

scanunits = frameunitwidth;
imgwidth = scanunits * SCANUNIT;

bm = XCreatePixmap(x11dpy, x11win, imgwidth, x11height, 1);
if (bm == 0) return;

gcv.foreground = 0;
bmgc = XCreateGC(x11dpy, bm, GCForeground, &gcv);

XFillRectangle(x11dpy, bm, bmgc, 0, 0, imgwidth, x11height);

gcv.foreground = 1;
XChangeGC(x11dpy, bmgc, GCForeground, &gcv);

for (row = 0; row < x11height; row++)
  {
  src = framebase + row * scanunits;
  for (col = 0; col < imgwidth; col += SCANUNIT)
    {
    bits = *src++;
    {register integer b;
    register integer limit;
    limit = SCANUNIT;
    if (col + limit > imgwidth) limit = imgwidth - col;
    for (b = 0; b < limit; b++)
      {
#if SCANUNIT==16
      if (bits & (0x8000 >> b))
#elif SCANUNIT==32
      if (bits & (0x80000000L >> b))
#else
      if (bits & (0x80 >> b))
#endif
        {
        integer bit;
        bit = col + b;
        XDrawPoint(x11dpy, bm, bmgc, bit, row);
        }
      }
    }
    }
  }

XCopyPlane(x11dpy, bm, x11win, x11gc, 0, 0, imgwidth, x11height, 0, 0, 1);
XSync(x11dpy, False);
XFreeGC(x11dpy, bmgc);
XFreePixmap(x11dpy, bm);
}

private procedure X11Device()
{
Object showPageExecObj;
integer width, height;
arrayObject matArray;
Object elem;
XSetWindowAttributes attr;
XSizeHints hints;
XEvent event;
XGCValues gcv;

showPageExecObj = psPop(opStack);
height = psPopInteger(opStack);
width = psPopInteger(opStack);

x11height = height;
x11width = ((width + SCANUNIT - 1) / SCANUNIT) * SCANUNIT;

x11dpy = XOpenDisplay((char *)NIL);
if (x11dpy == NIL) ERROR(undefinedfilename);

x11blackpixel = BlackPixel(x11dpy, DefaultScreen(x11dpy));
x11whitepixel = WhitePixel(x11dpy, DefaultScreen(x11dpy));

attr.background_pixel = x11whitepixel;
attr.border_pixel = x11blackpixel;
attr.event_mask = ExposureMask | StructureNotifyMask;

x11win = XCreateWindow(x11dpy, RootWindow(x11dpy, DefaultScreen(x11dpy)),
    0, 0, x11width, height, 1, CopyFromParent, InputOutput,
    CopyFromParent,
    CWBackPixel | CWBorderPixel | CWEventMask, &attr);

hints.flags = PMinSize | PMaxSize;
hints.min_width = hints.max_width = x11width;
hints.min_height = hints.max_height = height;
XSetWMNormalHints(x11dpy, x11win, &hints);

XStoreName(x11dpy, x11win, "PostScript");
XMapWindow(x11dpy, x11win);
XFlush(x11dpy);

while (true)
  {
  XNextEvent(x11dpy, &event);
  if (event.type == Expose) break;
  }

gcv.foreground = x11blackpixel;
gcv.background = x11whitepixel;
x11gc = XCreateGC(x11dpy, x11win, GCForeground | GCBackground, &gcv);

matArray = AllocArray(6);
{integer i; for (i = 0; i < 6; i++) {realObjL(elem, 0.0); VMPutElem(matArray, i, elem);}}
realObjL(elem, 1.0);
VMPutElem(matArray, 0, elem);
VMPutElem(matArray, 3, elem);
psPush(opStack, matArray);
psPushInteger(opStack, (x11width / SCANUNIT) * sizeof(SCANTYPE));
psPushInteger(opStack, height);
psPush(opStack, showPageExecObj);
FrameDevice();

gs->outputDevice->ShowPage = X11ShowPage;
}

public procedure X11DeviceInit(reason) InitReason reason;
{
switch (reason)
  {
  case init:
    x11dpy = NIL;
    x11win = 0;
    x11gc = NIL;
    break;
  case romreg:
    RegisterExplicit("x11device", X11Device);
    break;
  endswitch}
}
