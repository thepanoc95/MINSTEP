#ifndef MANGO_COMPAT_IPC_IPC_THREAD_H
#define MANGO_COMPAT_IPC_IPC_THREAD_H

#include <kern/lock.h>
#include <kern/macro_help.h>

/* In the usermode nanokernel, we don't have real threads.
 * We define a minimal ipc_thread structure that can be used
 * in wait queues (blocked sender queues on ports). */

typedef struct mango_ipc_thread {
    struct mango_ipc_thread *ith_next;
    struct mango_ipc_thread *ith_prev;
    /* Usermode: track the pthread and wait state */
    pthread_t           ith_pthread;
    void              (*ith_wait_handler)(void);
    int                 ith_state;
    decl_simple_lock_data(, ith_lock_data)
} *ipc_thread_t;

#define THREAD_NULL ((ipc_thread_t) 0)
#define ITH_NULL    THREAD_NULL

#define ith_lock_init(thread)   simple_lock_init(&(thread)->ith_lock_data)
#define ith_lock(thread)        simple_lock(&(thread)->ith_lock_data)
#define ith_unlock(thread)      simple_unlock(&(thread)->ith_lock_data)

typedef struct ipc_thread_queue {
    ipc_thread_t ithq_base;
} *ipc_thread_queue_t;

#define ITHQ_NULL ((ipc_thread_queue_t) 0)

#define ipc_thread_links_init(thread)       \
MACRO_BEGIN                                 \
    (thread)->ith_next = (thread);          \
    (thread)->ith_prev = (thread);          \
MACRO_END

#define ipc_thread_queue_init(queue)        \
MACRO_BEGIN                                 \
    (queue)->ithq_base = ITH_NULL;          \
MACRO_END

#define ipc_thread_queue_empty(queue)   ((queue)->ithq_base == ITH_NULL)
#define ipc_thread_queue_first(queue)   ((queue)->ithq_base)

#define ipc_thread_rmqueue_first_macro(queue, thread)    \
MACRO_BEGIN                                              \
    ipc_thread_t _next;                                  \
    assert((queue)->ithq_base == (thread));              \
    _next = (thread)->ith_next;                          \
    if (_next == (thread)) {                             \
        assert((thread)->ith_prev == (thread));          \
        (queue)->ithq_base = ITH_NULL;                   \
    } else {                                             \
        ipc_thread_t _prev = (thread)->ith_prev;         \
        (queue)->ithq_base = _next;                      \
        _next->ith_prev = _prev;                         \
        _prev->ith_next = _next;                         \
        ipc_thread_links_init(thread);                   \
    }                                                    \
MACRO_END

#define ipc_thread_enqueue_macro(queue, thread)          \
MACRO_BEGIN                                              \
    ipc_thread_t _first = (queue)->ithq_base;            \
    if (_first == ITH_NULL) {                            \
        (queue)->ithq_base = (thread);                   \
        assert((thread)->ith_next == (thread));          \
        assert((thread)->ith_prev == (thread));          \
    } else {                                             \
        ipc_thread_t _last = _first->ith_prev;           \
        (thread)->ith_next = _first;                     \
        (thread)->ith_prev = _last;                      \
        _first->ith_prev = (thread);                     \
        _last->ith_next = (thread);                      \
    }                                                    \
MACRO_END

extern void ipc_thread_enqueue(ipc_thread_queue_t queue, ipc_thread_t thread);
extern ipc_thread_t ipc_thread_dequeue(ipc_thread_queue_t queue);
extern void ipc_thread_rmqueue(ipc_thread_queue_t queue, ipc_thread_t thread);

#endif
