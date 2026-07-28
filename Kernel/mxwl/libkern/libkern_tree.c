#include "libkern_tree.h"

static void rb_rotate_left(struct rb_root *root, struct rb_node *node)
{
    struct rb_node *right = node->right;
    node->right = right->left;
    if (right->left)
        right->left->parent = node;
    right->parent = node->parent;
    if (!node->parent)
        root->node = right;
    else if (node == node->parent->left)
        node->parent->left = right;
    else
        node->parent->right = right;
    right->left = node;
    node->parent = right;
}

static void rb_rotate_right(struct rb_root *root, struct rb_node *node)
{
    struct rb_node *left = node->left;
    node->left = left->right;
    if (left->right)
        left->right->parent = node;
    left->parent = node->parent;
    if (!node->parent)
        root->node = left;
    else if (node == node->parent->right)
        node->parent->right = left;
    else
        node->parent->left = left;
    left->right = node;
    node->parent = left;
}

void rb_insert(struct rb_root *root, struct rb_node *node,
               int (*cmp)(struct rb_node *a, struct rb_node *b))
{
    struct rb_node *parent = NULL;
    struct rb_node **link = &root->node;

    while (*link) {
        parent = *link;
        if (cmp(node, parent) < 0)
            link = &parent->left;
        else
            link = &parent->right;
    }

    node->parent = parent;
    node->left = node->right = NULL;
    node->color = RB_RED;
    *link = node;

    while (node != root->node && node->parent->color == RB_RED) {
        struct rb_node *grandparent = node->parent->parent;
        if (node->parent == grandparent->left) {
            struct rb_node *uncle = grandparent->right;
            if (uncle && uncle->color == RB_RED) {
                node->parent->color = RB_BLACK;
                uncle->color = RB_BLACK;
                grandparent->color = RB_RED;
                node = grandparent;
            } else {
                if (node == node->parent->right) {
                    node = node->parent;
                    rb_rotate_left(root, node);
                }
                node->parent->color = RB_BLACK;
                grandparent->color = RB_RED;
                rb_rotate_right(root, grandparent);
            }
        } else {
            struct rb_node *uncle = grandparent->left;
            if (uncle && uncle->color == RB_RED) {
                node->parent->color = RB_BLACK;
                uncle->color = RB_BLACK;
                grandparent->color = RB_RED;
                node = grandparent;
            } else {
                if (node == node->parent->left) {
                    node = node->parent;
                    rb_rotate_right(root, node);
                }
                node->parent->color = RB_BLACK;
                grandparent->color = RB_RED;
                rb_rotate_left(root, grandparent);
            }
        }
    }

    root->node->color = RB_BLACK;
}

void rb_erase(struct rb_root *root, struct rb_node *node)
{
    struct rb_node *child, *parent;
    int color;

    if (!node->left) {
        child = node->right;
    } else if (!node->right) {
        child = node->left;
    } else {
        struct rb_node *replace = node->right;
        while (replace->left)
            replace = replace->left;
        if (replace->parent != node) {
            replace->parent->left = replace->right;
            if (replace->right)
                replace->right->parent = replace->parent;
            replace->right = node->right;
            node->right->parent = replace;
        }
        replace->left = node->left;
        node->left->parent = replace;
        color = replace->color;
        replace->color = node->color;
        if (!node->parent)
            root->node = replace;
        else if (node == node->parent->left)
            node->parent->left = replace;
        else
            node->parent->right = replace;
        replace->parent = node->parent;

        if (color == RB_BLACK) {
            struct rb_node *sibling;
            child = replace->right;
            parent = replace;
            goto fixup;
        }
        return;
    }

    if (child)
        child->parent = node->parent;

    if (!node->parent)
        root->node = child;
    else if (node == node->parent->left)
        node->parent->left = child;
    else
        node->parent->right = child;

    color = node->color;
    parent = node->parent;

    if (color == RB_BLACK)
        goto fixup;
    return;

fixup:
    while (child != root->node && (!child || child->color == RB_BLACK)) {
        if (child == parent->left) {
            struct rb_node *sibling = parent->right;
            if (sibling->color == RB_RED) {
                sibling->color = RB_BLACK;
                parent->color = RB_RED;
                rb_rotate_left(root, parent);
                sibling = parent->right;
            }
            if ((!sibling->left || sibling->left->color == RB_BLACK) &&
                (!sibling->right || sibling->right->color == RB_BLACK)) {
                sibling->color = RB_RED;
                child = parent;
                parent = parent->parent;
            } else {
                if (!sibling->right || sibling->right->color == RB_BLACK) {
                    if (sibling->left)
                        sibling->left->color = RB_BLACK;
                    sibling->color = RB_RED;
                    rb_rotate_right(root, sibling);
                    sibling = parent->right;
                }
                sibling->color = parent->color;
                parent->color = RB_BLACK;
                if (sibling->right)
                    sibling->right->color = RB_BLACK;
                rb_rotate_left(root, parent);
                child = root->node;
                break;
            }
        } else {
            struct rb_node *sibling = parent->left;
            if (sibling->color == RB_RED) {
                sibling->color = RB_BLACK;
                parent->color = RB_RED;
                rb_rotate_right(root, parent);
                sibling = parent->left;
            }
            if ((!sibling->left || sibling->left->color == RB_BLACK) &&
                (!sibling->right || sibling->right->color == RB_BLACK)) {
                sibling->color = RB_RED;
                child = parent;
                parent = parent->parent;
            } else {
                if (!sibling->left || sibling->left->color == RB_BLACK) {
                    if (sibling->right)
                        sibling->right->color = RB_BLACK;
                    sibling->color = RB_RED;
                    rb_rotate_left(root, sibling);
                    sibling = parent->left;
                }
                sibling->color = parent->color;
                parent->color = RB_BLACK;
                if (sibling->left)
                    sibling->left->color = RB_BLACK;
                rb_rotate_right(root, parent);
                child = root->node;
                break;
            }
        }
    }
    if (child)
        child->color = RB_BLACK;
}

struct rb_node *rb_first(struct rb_root *root)
{
    struct rb_node *node = root->node;
    if (!node) return NULL;
    while (node->left)
        node = node->left;
    return node;
}

struct rb_node *rb_last(struct rb_root *root)
{
    struct rb_node *node = root->node;
    if (!node) return NULL;
    while (node->right)
        node = node->right;
    return node;
}

struct rb_node *rb_next(struct rb_node *node)
{
    if (node->right) {
        node = node->right;
        while (node->left)
            node = node->left;
        return node;
    }
    while (node->parent && node == node->parent->right)
        node = node->parent;
    return node->parent;
}

struct rb_node *rb_prev(struct rb_node *node)
{
    if (node->left) {
        node = node->left;
        while (node->right)
            node = node->right;
        return node;
    }
    while (node->parent && node == node->parent->left)
        node = node->parent;
    return node->parent;
}
