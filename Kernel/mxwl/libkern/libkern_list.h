#ifndef MXWL_LIBKERN_LIST_H
#define MXWL_LIBKERN_LIST_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct list_head {
    struct list_head *next;
    struct list_head *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }
#define LIST_HEAD(name) struct list_head name = LIST_HEAD_INIT(name)

#define list_entry(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

#define list_first_entry(head, type, member) \
    list_entry((head)->next, type, member)

#define list_last_entry(head, type, member) \
    list_entry((head)->prev, type, member)

#define list_next_entry(pos, member) \
    list_entry((pos)->member.next, typeof(*(pos)), member)

#define list_prev_entry(pos, member) \
    list_entry((pos)->member.prev, typeof(*(pos)), member)

#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_safe(pos, n, head) \
    for (pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)

#define list_for_each_entry(pos, head, member) \
    for (pos = list_first_entry(head, typeof(*pos), member); \
         &pos->member != (head); \
         pos = list_next_entry(pos, member))

#define list_for_each_entry_safe(pos, n, head, member) \
    for (pos = list_first_entry(head, typeof(*pos), member), \
         n = list_next_entry(pos, member); \
         &pos->member != (head); \
         pos = n, n = list_next_entry(n, member))

static inline void list_init(struct list_head *head)
{
    head->next = head;
    head->prev = head;
}

static inline void list_add(struct list_head *new_node, struct list_head *prev, struct list_head *next)
{
    next->prev = new_node;
    new_node->next = next;
    new_node->prev = prev;
    prev->next = new_node;
}

static inline void list_add_head(struct list_head *new_node, struct list_head *head)
{
    list_add(new_node, head, head->next);
}

static inline void list_add_tail(struct list_head *new_node, struct list_head *head)
{
    list_add(new_node, head->prev, head);
}

static inline void list_del(struct list_head *entry)
{
    entry->prev->next = entry->next;
    entry->next->prev = entry->prev;
    entry->next = NULL;
    entry->prev = NULL;
}

static inline void list_del_init(struct list_head *entry)
{
    list_del(entry);
    list_init(entry);
}

static inline int list_empty(const struct list_head *head)
{
    return head->next == head;
}

static inline int list_is_last(const struct list_head *entry, const struct list_head *head)
{
    return entry->next == head;
}

static inline size_t list_count(struct list_head *head)
{
    struct list_head *pos;
    size_t count = 0;
    list_for_each(pos, head)
        count++;
    return count;
}

static inline void list_splice(struct list_head *src, struct list_head *dst)
{
    if (!list_empty(src)) {
        struct list_head *first = src->next;
        struct list_head *last = src->prev;
        struct list_head *at = dst->next;

        first->prev = dst;
        dst->next = first;
        last->next = at;
        at->prev = last;
    }
}

static inline void list_splice_tail(struct list_head *src, struct list_head *dst)
{
    if (!list_empty(src)) {
        struct list_head *first = src->next;
        struct list_head *last = src->prev;
        struct list_head *at = dst->prev;

        first->prev = at;
        at->next = first;
        last->next = dst;
        dst->prev = last;
    }
}

static inline void list_move(struct list_head *entry, struct list_head *head)
{
    list_del(entry);
    list_add_tail(entry, head);
}

static inline int list_is_singular(struct list_head *head)
{
    return !list_empty(head) && (head->next == head->prev);
}

#ifdef __cplusplus
}
#endif

#endif /* MXWL_LIBKERN_LIST_H */
