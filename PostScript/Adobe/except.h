/*
 * except.h - Structured exception handling for PostScript 1.0 interpreter
 *
 * Copyright 1983 -- Adobe Systems, Inc.
 *
 * Provides DURING/HANDLER/END_HANDLER exception constructs implemented
 * on top of setjmp/longjmp.  Each DURING block pushes an exception
 * buffer onto a linked stack so that handlers may nest.
 */

#ifndef EXCEPT_H
#define EXCEPT_H

#include <setjmp.h>

/* ── Exception codes ────────────────────────────────────────────── */
#define PS_ERROR       1   /* a PostScript-level error (via ERROR)  */
#define PS_STKOVRFLW   2   /* operand/exec/dict stack overflow      */
#define PS_STOPEXEC    3   /* stop / interrupt / quit request       */

/* ── Exception mode flags (OR-ed into ExceptMode) ───────────────── */
#define EX_MODE_REPORT  1
#define EX_MODE_ABORT   2

/* ── Per-DURING exception buffer (stack-linked) ─────────────────── */
typedef struct ExceptBuf {
    jmp_buf          env;   /* setjmp/longjmp target                */
    int              code;  /* exception code being raised          */
    struct ExceptBuf *prev; /* next outer buffer, NIL at bottom     */
} ExceptBuf;

/* ── Global exception state (defined below; see implementation) ─── */
extern ExceptBuf *_except_top;   /* head of the exception-buffer stack */
extern int        ExceptMode;     /* reporting/abort mode flags         */

/*
 * Exception record — visible inside HANDLER blocks.
 * Usage:  switch (Exception.Code) { case PS_ERROR: … }
 */
typedef struct {
    int Code;
} ExceptionRecord;
extern ExceptionRecord Exception;

/* ── Initialisation (call once before first DURING) ─────────────── */
extern void LIBexceptinit(void);

/* ════════════════════════════════════════════════════════════════ *
 *  DURING / HANDLER / END_HANDLER                                  *
 *                                                                  *
 *  DURING                                                          *
 *      … protected body …                                          *
 *  HANDLER                                                         *
 *      switch (Exception.Code) {                                   *
 *          case PS_ERROR:     …; break;                            *
 *          case PS_STKOVRFLW: RERAISE;                             *
 *          case PS_STOPEXEC:  RERAISE;                             *
 *          default:          CantHappen();                         *
 *      }                                                           *
 *  END_HANDLER;                                                    *
 * ════════════════════════════════════════════════════════════════ */

#define DURING                                                      \
    {                                                               \
        ExceptBuf __ebuf;                                           \
        __ebuf.prev = _except_top;                                  \
        _except_top   = &__ebuf;                                    \
        if (setjmp(__ebuf.env) == 0) {

#define HANDLER                                                     \
            _except_top = __ebuf.prev;                              \
        } else {                                                    \
            Exception.Code = _except_top->code;                     \
            _except_top    = __ebuf.prev;

#define END_HANDLER                                                 \
        }                                                           \
    }

/* ── Raise an exception ─────────────────────────────────────────── *
 *  code  — one of the PS_* constants above                         *
 *  msg   — descriptive string (reserved for diagnostics; may be "") */
#define PS_RAISE(ecode, msg)                                            \
    do {                                                            \
        if (_except_top) {                                          \
            _except_top->code = (ecode);                             \
            longjmp(_except_top->env, 1);                           \
        }                                                           \
    } while (0)

/* ── Re-raise the current exception to the enclosing DURING ─────── */
#define RERAISE                                                     \
    do {                                                            \
        if (_except_top) {                                          \
            _except_top->code = Exception.Code;                     \
            longjmp(_except_top->env, 1);                           \
        }                                                           \
    } while (0)

#endif /* EXCEPT_H */
