/* reducer.h - Scan-line reducer interface for PostScript 1.0 */

#ifndef REDUCER_H
#define REDUCER_H

extern procedure NewPoint(/* integer x, integer y */);
extern procedure ReducerClosePath();
extern procedure Reduce(/* boolean (*trapfn)(), boolean clipflag, boolean evenOdd */);
extern procedure ResetReducer();
extern procedure NewPathIsClip(/* boolean flag */);
extern procedure ReducerInit(/* InitReason */);

#endif
