#ifndef MXWL_LIBKERN_QUEUE_H
#define MXWL_LIBKERN_QUEUE_H

#include "libkern_list.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct fifo_queue {
    struct list_head head;
    size_t           count;
} fifo_queue_t;

#define fifo_queue_init(q) do { \
    list_init(&(q)->head);      \
    (q)->count = 0;             \
} while (0)

static inline void fifo_enqueue(fifo_queue_t *q, struct list_head *entry)
{
    list_add_tail(entry, &q->head);
    q->count++;
}

static inline struct list_head *fifo_dequeue(fifo_queue_t *q)
{
    struct list_head *entry;
    if (list_empty(&q->head))
        return NULL;
    entry = q->head.next;
    list_del(entry);
    q->count--;
    return entry;
}

static inline struct list_head *fifo_peek(const fifo_queue_t *q)
{
    if (list_empty(&q->head))
        return NULL;
    return q->head.next;
}

static inline int fifo_empty(const fifo_queue_t *q)
{
    return q->count == 0;
}

static inline size_t fifo_count(const fifo_queue_t *q)
{
    return q->count;
}

#define fifo_entry(ptr, type, member) list_entry(ptr, type, member)

typedef struct fifo_queue    prio_queue_t;

#endif /* MXWL_LIBKERN_QUEUE_H */
