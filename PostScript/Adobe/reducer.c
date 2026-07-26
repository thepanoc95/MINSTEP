/* PostScript Scan-Line Reducer

		Copyright 1983 -- Adobe Systems, Inc.
	    PostScript is a trademark of Adobe Systems, Inc.
NOTICE:  All information contained herein or attendant hereto is, and
remains, the property of Adobe Systems, Inc.  Many of the intellectual
and technical concepts contained herein are proprietary to Adobe Systems,
Inc. and may be covered by U.S. and Foreign Patents or Patents Pending or
are protected as trade secrets.  Any dissemination of this information or
reproduction of this material are strictly forbidden unless prior written
permission is obtained from Adobe Systems, Inc.

Reconstructed scan-line polygon reducer for Linux/x86-64.
Converts paths accumulated via NewPoint/ReducerClosePath into
trapezoids delivered to the device ColorTrap callback.
*/

#include "postscript.h"
#include "reducer.h"

#define MAXEDGES 4000
#define MAXEVENTS (MAXEDGES * 2)
#define MAXACTIVE 500

typedef struct {
    integer yTop, yBot;
    integer xTop, xBot;
    integer wind;
    boolean isClip;
} EdgeRec;

private EdgeRec edges[MAXEDGES];
private integer nEdges;

private integer cpx, cpy, spx, spy;
private boolean hasCP;
private boolean clipSubpath;

private boolean (*outTrapFn)();

private procedure addEdge(x1, y1, x2, y2, isclip)
    integer x1, y1, x2, y2; boolean isclip;
{
    EdgeRec *e;
    if (y1 == y2) return;
    if (nEdges >= MAXEDGES) return;
    e = &edges[nEdges];
    if (y1 > y2) {
        e->yTop = y1; e->yBot = y2;
        e->xTop = x1; e->xBot = x2;
        e->wind = 1;
    } else {
        e->yTop = y2; e->yBot = y1;
        e->xTop = x2; e->xBot = x1;
        e->wind = -1;
    }
    e->isClip = isclip;
    nEdges++;
}

private Fixed edgeXFixed(e, y) EdgeRec *e; integer y;
{
    integer dy = e->yTop - e->yBot;
    if (dy == 0) return (Fixed)e->xTop << 16;
    return ((Fixed)e->xBot << 16) +
        (Fixed)((((long long)(e->xTop - e->xBot)) *
                 (long long)(y - e->yBot) * 65536LL) / (long long)dy);
}

public procedure NewPoint(x, y) integer x, y;
{
    if (!hasCP) {
        cpx = x; cpy = y;
        spx = x; spy = y;
        hasCP = true;
        return;
    }
    addEdge(cpx, cpy, x, y, clipSubpath);
    cpx = x; cpy = y;
}

public procedure ReducerClosePath()
{
    if (hasCP) {
        addEdge(cpx, cpy, spx, spy, clipSubpath);
        hasCP = false;
    }
}

public procedure ResetReducer()
{
    nEdges = 0;
    hasCP = false;
    clipSubpath = false;
}

public procedure NewPathIsClip(flag) boolean flag;
{
    if (hasCP) ReducerClosePath();
    clipSubpath = flag;
}

private procedure emitTrap(yt, yb, xtl, xtr, xbl, xbr)
    Fixed yt, yb, xtl, xtr, xbl, xbr;
{
    if (yt > yb) (*outTrapFn)(yt, yb, xtl, xtr, xbl, xbr);
}

private procedure sortDescInt(arr, n) integer arr[]; integer n;
{
    integer i, j, tmp;
    for (i = 1; i < n; i++) {
        tmp = arr[i];
        j = i - 1;
        while (j >= 0 && arr[j] < tmp) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = tmp;
    }
}

private procedure sortEdgesByX(act, n, y) integer act[]; integer n; integer y;
{
    integer i, j, tmp;
    Fixed xi;
    for (i = 1; i < n; i++) {
        tmp = act[i];
        xi = edgeXFixed(&edges[tmp], y);
        j = i - 1;
        while (j >= 0 && edgeXFixed(&edges[act[j]], y) > xi) {
            act[j + 1] = act[j];
            j--;
        }
        act[j + 1] = tmp;
    }
}

public procedure Reduce(trapfn, clipflag, evenOdd)
    boolean (*trapfn)(); boolean clipflag;
    boolean evenOdd;
{
    integer events[MAXEVENTS];
    integer nEvents;
    integer i, k;
    integer yUpper, yLower;
    integer actList[MAXACTIVE];
    integer nAct;
    Fixed xUp[MAXACTIVE], xLo[MAXACTIVE];
    integer windVal[MAXACTIVE];
    boolean clipEdge[MAXACTIVE];
    integer figWind, clipWind;
    boolean prevFilled, filled;
    integer spanStart;
    Fixed yt_F, yb_F;

    outTrapFn = trapfn;

    if (nEdges == 0) return;

    nEvents = 0;
    for (i = 0; i < nEdges; i++) {
        events[nEvents++] = edges[i].yTop;
        events[nEvents++] = edges[i].yBot;
    }

    sortDescInt(events, nEvents);
    {
        integer prev, j;
        prev = events[0];
        j = 1;
        for (i = 1; i < nEvents; i++) {
            if (events[i] != prev) {
                events[j++] = events[i];
                prev = events[i];
            }
        }
        nEvents = j;
    }

    for (k = 0; k < nEvents - 1; k++) {
        yUpper = events[k];
        yLower = events[k + 1];
        if (yUpper <= yLower) continue;

        nAct = 0;
        for (i = 0; i < nEdges; i++) {
            if (edges[i].yTop >= yUpper && edges[i].yBot <= yLower) {
                actList[nAct] = i;
                if (++nAct >= MAXACTIVE) break;
            }
        }
        if (nAct == 0) continue;

        sortEdgesByX(actList, nAct, yUpper);

        for (i = 0; i < nAct; i++) {
            xUp[i] = edgeXFixed(&edges[actList[i]], yUpper);
            xLo[i] = edgeXFixed(&edges[actList[i]], yLower);
            windVal[i] = edges[actList[i]].wind;
            clipEdge[i] = edges[actList[i]].isClip;
        }

        yt_F = (Fixed)yUpper << 16;
        yb_F = (Fixed)yLower << 16;

        figWind = 0;
        clipWind = 0;
        prevFilled = false;
        spanStart = 0;

        for (i = 0; i < nAct; i++) {
            if (clipEdge[i]) clipWind += windVal[i];
            else figWind += windVal[i];

            if (evenOdd) {
                filled = clipflag
                    ? ((figWind & 1) != 0 && (clipWind & 1) != 0)
                    : ((figWind & 1) != 0);
            } else {
                filled = clipflag
                    ? (figWind != 0 && clipWind != 0)
                    : (figWind != 0);
            }

            if (!prevFilled && filled) {
                spanStart = i;
            } else if (prevFilled && !filled) {
                emitTrap(yt_F, yb_F,
                    xUp[spanStart], xUp[i],
                    xLo[spanStart], xLo[i]);
            }

            prevFilled = filled;
        }

        if (prevFilled) {
            emitTrap(yt_F, yb_F,
                xUp[spanStart], xUp[nAct - 1],
                xLo[spanStart], xLo[nAct - 1]);
        }
    }
}

public procedure ReducerInit(reason) InitReason reason;
{
}
