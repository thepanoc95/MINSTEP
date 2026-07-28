#ifndef MXWL_LIBKERN_TREE_H
#define MXWL_LIBKERN_TREE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct rb_node {
    struct rb_node *parent;
    struct rb_node *left;
    struct rb_node *right;
    int             color;
};

struct rb_root {
    struct rb_node *node;
};

#define RB_ROOT (struct rb_root) { NULL }

#define RB_RED   0
#define RB_BLACK 1

#define rb_entry(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

static inline void rb_init_node(struct rb_node *node)
{
    node->parent = NULL;
    node->left   = NULL;
    node->right  = NULL;
    node->color  = RB_RED;
}

static inline int rb_empty(struct rb_root *root)
{
    return root->node == NULL;
}

struct rb_node *rb_first(struct rb_root *root);
struct rb_node *rb_last(struct rb_root *root);
struct rb_node *rb_next(struct rb_node *node);
struct rb_node *rb_prev(struct rb_node *node);

void rb_insert(struct rb_root *root, struct rb_node *node,
               int (*cmp)(struct rb_node *a, struct rb_node *b));

void rb_erase(struct rb_root *root, struct rb_node *node);

struct avl_node {
    struct avl_node *parent;
    struct avl_node *left;
    struct avl_node *right;
    int              balance;
};

struct avl_root {
    struct avl_node *node;
};

#define AVL_ROOT (struct avl_root) { NULL }

#define avl_entry(ptr, type, member) rb_entry(ptr, type, member)

#ifdef __cplusplus
}
#endif

#endif /* MXWL_LIBKERN_TREE_H */
