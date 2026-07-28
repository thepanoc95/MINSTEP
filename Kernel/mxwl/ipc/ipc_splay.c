/*
 * mxwl/ipc/ipc_splay.c
 *
 * Primitive splay tree operations.
 *
 * Ported from osfmk/kernel/ipc/ipc_splay.c (Rich Draves, 1989).
 * Adapted for the Maxxwell usermode nanokernel.
 *
 * Splay trees are self-adjusting binary search trees with:
 *   1) Space efficiency -- only two pointers per entry
 *   2) Robust performance -- amortized O(log n) per operation
 *   3) No recursion needed
 *
 * This makes them ideal as a fall-back for entries that don't fit
 * in the flat lookup table.
 *
 * The tree is stored in unassembled form.  If ist_root is null,
 * then the tree has no entries.  Otherwise, ist_name records the
 * value used for the last lookup.  ist_root points to the middle
 * tree obtained from the top-down splay.  ist_ltree and ist_rtree
 * point to left and right subtrees.
 */

#include <mach/port.h>
#include <kern/assert.h>
#include <kern/macro_help.h>
#include <ipc/ipc_entry.h>
#include <ipc/ipc_splay.h>

/*
 * Boundary values for splay lookups:
 */
#define MACH_PORT_SMALLEST  ((mach_port_t) 0)
#define MACH_PORT_LARGEST   ((mach_port_t) ~0)

/*
 * Routine:	ipc_splay_prim_lookup
 * Purpose:
 *		Searches for the node labeled name in the splay tree.
 *		Returns three nodes (treep, ltreep, rtreep) and
 *		two pointers to nodes (ltreepp, rtreepp).
 *
 *		ipc_splay_prim_lookup splits the supplied tree into
 *		three subtrees, left, middle, and right, returned
 *		in ltreep, treep, and rtreep.
 *
 *		If name is present in the tree, then it is at
 *		the root of the middle tree.  Otherwise, the root
 *		of the middle tree is the last node traversed.
 *
 *		ipc_splay_prim_lookup returns a pointer into
 *		the left subtree, to the rchild field of its
 *		largest node, in ltreepp.  It returns a pointer
 *		into the right subtree, to the lchild field of its
 *		smallest node, in rtreepp.
 */

static void
ipc_splay_prim_lookup(
    mach_port_t         name,
    ipc_tree_entry_t    tree,
    ipc_tree_entry_t   *treep,
    ipc_tree_entry_t   *ltreep,
    ipc_tree_entry_t  **ltreepp,
    ipc_tree_entry_t   *rtreep,
    ipc_tree_entry_t  **rtreepp)
{
    mach_port_t tname;                  /* temp name */
    ipc_tree_entry_t lchild, rchild;    /* temp child pointers */

    assert(tree != ITE_NULL);

#define link_left                       \
MACRO_BEGIN                             \
    *ltreep = tree;                     \
    ltreep = &tree->ite_rchild;         \
    tree = *ltreep;                     \
MACRO_END

#define link_right                      \
MACRO_BEGIN                             \
    *rtreep = tree;                     \
    rtreep = &tree->ite_lchild;         \
    tree = *rtreep;                     \
MACRO_END

#define rotate_left                     \
MACRO_BEGIN                             \
    ipc_tree_entry_t temp = tree;       \
                                        \
    tree = temp->ite_rchild;            \
    temp->ite_rchild = tree->ite_lchild;\
    tree->ite_lchild = temp;            \
MACRO_END

#define rotate_right                    \
MACRO_BEGIN                             \
    ipc_tree_entry_t temp = tree;       \
                                        \
    tree = temp->ite_lchild;            \
    temp->ite_lchild = tree->ite_rchild;\
    tree->ite_rchild = temp;            \
MACRO_END

    while (name != (tname = tree->ite_name)) {
        if (name < tname) {
            /* descend to left */

            lchild = tree->ite_lchild;
            if (lchild == ITE_NULL)
                break;
            tname = lchild->ite_name;

            if ((name < tname) &&
                (lchild->ite_lchild != ITE_NULL))
                rotate_right;
            link_right;
            if ((name > tname) &&
                (lchild->ite_rchild != ITE_NULL))
                link_left;
        } else {
            /* descend to right */

            rchild = tree->ite_rchild;
            if (rchild == ITE_NULL)
                break;
            tname = rchild->ite_name;

            if ((name > tname) &&
                (rchild->ite_rchild != ITE_NULL))
                rotate_left;
            link_left;
            if ((name < tname) &&
                (rchild->ite_lchild != ITE_NULL))
                link_right;
        }

        assert(tree != ITE_NULL);
    }

    *treep = tree;
    *ltreepp = ltreep;
    *rtreepp = rtreep;

#undef link_left
#undef link_right
#undef rotate_left
#undef rotate_right
}

/*
 * Routine:	ipc_splay_prim_assemble
 * Purpose:
 *		Assembles the results of ipc_splay_prim_lookup
 *		into a splay tree with the found node at the root.
 *
 *		ltree and rtree are by-reference so storing
 *		through ltreep and rtreep can change them.
 */

static void
ipc_splay_prim_assemble(
    ipc_tree_entry_t    tree,
    ipc_tree_entry_t   *ltree,
    ipc_tree_entry_t   *ltreep,
    ipc_tree_entry_t   *rtree,
    ipc_tree_entry_t   *rtreep)
{
    assert(tree != ITE_NULL);

    *ltreep = tree->ite_lchild;
    *rtreep = tree->ite_rchild;

    tree->ite_lchild = *ltree;
    tree->ite_rchild = *rtree;
}

/*
 * Routine:	ipc_splay_tree_init
 * Purpose:
 *		Initialize a raw splay tree for use.
 */

void
ipc_splay_tree_init(
    ipc_splay_tree_t    splay)
{
    splay->ist_root = ITE_NULL;
}

/*
 * Routine:	ipc_splay_tree_pick
 * Purpose:
 *		Picks and returns a random entry in a splay tree.
 *		Returns FALSE if the splay tree is empty.
 */

boolean_t
ipc_splay_tree_pick(
    ipc_splay_tree_t    splay,
    mach_port_t        *namep,
    ipc_tree_entry_t   *entryp)
{
    ipc_tree_entry_t root;

    ist_lock(splay);

    root = splay->ist_root;
    if (root != ITE_NULL) {
        *namep = root->ite_name;
        *entryp = root;
    }

    ist_unlock(splay);

    return root != ITE_NULL;
}

/*
 * Routine:	ipc_splay_tree_lookup
 * Purpose:
 *		Finds an entry in a splay tree.
 *		Returns ITE_NULL if not found.
 */

ipc_tree_entry_t
ipc_splay_tree_lookup(
    ipc_splay_tree_t    splay,
    mach_port_t         name)
{
    ipc_tree_entry_t root;

    ist_lock(splay);

    root = splay->ist_root;
    if (root != ITE_NULL) {
        if (splay->ist_name != name) {
            ipc_splay_prim_assemble(root,
                &splay->ist_ltree, splay->ist_ltreep,
                &splay->ist_rtree, splay->ist_rtreep);
            ipc_splay_prim_lookup(name, root, &root,
                &splay->ist_ltree, &splay->ist_ltreep,
                &splay->ist_rtree, &splay->ist_rtreep);
            splay->ist_name = name;
            splay->ist_root = root;
        }

        if (name != root->ite_name)
            root = ITE_NULL;
    }

    ist_unlock(splay);

    return root;
}

/*
 * Routine:	ipc_splay_tree_insert
 * Purpose:
 *		Inserts a new entry into a splay tree.
 *		The caller supplies a new entry.
 *		The name can't already be present in the tree.
 */

void
ipc_splay_tree_insert(
    ipc_splay_tree_t    splay,
    mach_port_t         name,
    ipc_tree_entry_t    entry)
{
    ipc_tree_entry_t root;

    assert(entry != ITE_NULL);

    ist_lock(splay);

    root = splay->ist_root;
    if (root == ITE_NULL) {
        entry->ite_lchild = ITE_NULL;
        entry->ite_rchild = ITE_NULL;
    } else {
        if (splay->ist_name != name) {
            ipc_splay_prim_assemble(root,
                &splay->ist_ltree, splay->ist_ltreep,
                &splay->ist_rtree, splay->ist_rtreep);
            ipc_splay_prim_lookup(name, root, &root,
                &splay->ist_ltree, &splay->ist_ltreep,
                &splay->ist_rtree, &splay->ist_rtreep);
        }

        assert(root->ite_name != name);

        if (name < root->ite_name) {
            assert(root->ite_lchild == ITE_NULL);

            *splay->ist_ltreep = ITE_NULL;
            *splay->ist_rtreep = root;
        } else {
            assert(root->ite_rchild == ITE_NULL);

            *splay->ist_ltreep = root;
            *splay->ist_rtreep = ITE_NULL;
        }

        entry->ite_lchild = splay->ist_ltree;
        entry->ite_rchild = splay->ist_rtree;
    }

    entry->ite_name = name;
    splay->ist_root = entry;
    splay->ist_name = name;
    splay->ist_ltreep = &splay->ist_ltree;
    splay->ist_rtreep = &splay->ist_rtree;

    ist_unlock(splay);
}

/*
 * Routine:	ipc_splay_tree_delete
 * Purpose:
 *		Deletes an entry from a splay tree.
 *		The name must be present in the tree.
 *		Frees the entry.
 */

void
ipc_splay_tree_delete(
    ipc_splay_tree_t    splay,
    mach_port_t         name,
    ipc_tree_entry_t    entry)
{
    ipc_tree_entry_t root, saved;

    ist_lock(splay);

    root = splay->ist_root;
    assert(root != ITE_NULL);

    if (splay->ist_name != name) {
        ipc_splay_prim_assemble(root,
            &splay->ist_ltree, splay->ist_ltreep,
            &splay->ist_rtree, splay->ist_rtreep);
        ipc_splay_prim_lookup(name, root, &root,
            &splay->ist_ltree, &splay->ist_ltreep,
            &splay->ist_rtree, &splay->ist_rtreep);
    }

    assert(root->ite_name == name);
    assert(root == entry);

    *splay->ist_ltreep = root->ite_lchild;
    *splay->ist_rtreep = root->ite_rchild;
    ite_free(root);

    root = splay->ist_ltree;
    saved = splay->ist_rtree;

    if (root == ITE_NULL)
        root = saved;
    else if (saved != ITE_NULL) {
        /*
         * Find the largest node in the left subtree,
         * and splay it to the root.  Then add the saved
         * right subtree.
         */

        ipc_splay_prim_lookup(MACH_PORT_LARGEST, root, &root,
            &splay->ist_ltree, &splay->ist_ltreep,
            &splay->ist_rtree, &splay->ist_rtreep);
        ipc_splay_prim_assemble(root,
            &splay->ist_ltree, splay->ist_ltreep,
            &splay->ist_rtree, splay->ist_rtreep);

        assert(root->ite_rchild == ITE_NULL);
        root->ite_rchild = saved;
    }

    splay->ist_root = root;
    if (root != ITE_NULL) {
        splay->ist_name = root->ite_name;
        splay->ist_ltreep = &splay->ist_ltree;
        splay->ist_rtreep = &splay->ist_rtree;
    }

    ist_unlock(splay);
}

/*
 * Routine:	ipc_splay_tree_split
 * Purpose:
 *		Split a splay tree.  Puts all entries smaller than "name"
 *		into a new tree, "small".
 */

void
ipc_splay_tree_split(
    ipc_splay_tree_t    splay,
    mach_port_t         name,
    ipc_splay_tree_t    small)
{
    ipc_tree_entry_t root;

    ipc_splay_tree_init(small);

    ist_lock(splay);

    root = splay->ist_root;
    if (root != ITE_NULL) {
        if (splay->ist_name != name) {
            ipc_splay_prim_assemble(root,
                &splay->ist_ltree, splay->ist_ltreep,
                &splay->ist_rtree, splay->ist_rtreep);
            ipc_splay_prim_lookup(name, root, &root,
                &splay->ist_ltree, &splay->ist_ltreep,
                &splay->ist_rtree, &splay->ist_rtreep);
        }

        if (root->ite_name < name) {
            /* root goes into small */

            *splay->ist_ltreep = root->ite_lchild;
            *splay->ist_rtreep = ITE_NULL;
            root->ite_lchild = splay->ist_ltree;
            assert(root->ite_rchild == ITE_NULL);

            small->ist_root = root;
            small->ist_name = root->ite_name;
            small->ist_ltreep = &small->ist_ltree;
            small->ist_rtreep = &small->ist_rtree;

            /* rtree goes into splay */

            root = splay->ist_rtree;
            splay->ist_root = root;
            if (root != ITE_NULL) {
                splay->ist_name = root->ite_name;
                splay->ist_ltreep = &splay->ist_ltree;
                splay->ist_rtreep = &splay->ist_rtree;
            }
        } else {
            /* root stays in splay */

            *splay->ist_ltreep = root->ite_lchild;
            root->ite_lchild = ITE_NULL;

            splay->ist_root = root;
            splay->ist_name = name;
            splay->ist_ltreep = &splay->ist_ltree;

            /* ltree goes into small */

            root = splay->ist_ltree;
            small->ist_root = root;
            if (root != ITE_NULL) {
                small->ist_name = root->ite_name;
                small->ist_ltreep = &small->ist_ltree;
                small->ist_rtreep = &small->ist_rtree;
            }
        }
    }

    ist_unlock(splay);
}

/*
 * Routine:	ipc_splay_tree_join
 * Purpose:
 *		Joins two splay trees.  Merges the entries in "small",
 *		which must all be smaller than the entries in "splay",
 *		into "splay".
 */

void
ipc_splay_tree_join(
    ipc_splay_tree_t    splay,
    ipc_splay_tree_t    small)
{
    ipc_tree_entry_t sroot;

    /* pull entries out of small */

    ist_lock(small);

    sroot = small->ist_root;
    if (sroot != ITE_NULL) {
        ipc_splay_prim_assemble(sroot,
            &small->ist_ltree, small->ist_ltreep,
            &small->ist_rtree, small->ist_rtreep);
        small->ist_root = ITE_NULL;
    }

    ist_unlock(small);

    /* put entries, if any, into splay */

    if (sroot != ITE_NULL) {
        ipc_tree_entry_t root;

        ist_lock(splay);

        root = splay->ist_root;
        if (root == ITE_NULL) {
            root = sroot;
        } else {
            /* get smallest entry in splay tree to top */

            if (splay->ist_name != MACH_PORT_SMALLEST) {
                ipc_splay_prim_assemble(root,
                    &splay->ist_ltree, splay->ist_ltreep,
                    &splay->ist_rtree, splay->ist_rtreep);
                ipc_splay_prim_lookup(MACH_PORT_SMALLEST,
                    root, &root,
                    &splay->ist_ltree, &splay->ist_ltreep,
                    &splay->ist_rtree, &splay->ist_rtreep);
            }

            ipc_splay_prim_assemble(root,
                &splay->ist_ltree, splay->ist_ltreep,
                &splay->ist_rtree, splay->ist_rtreep);

            assert(root->ite_lchild == ITE_NULL);
            assert(sroot->ite_name < root->ite_name);
            root->ite_lchild = sroot;
        }

        splay->ist_root = root;
        splay->ist_name = root->ite_name;
        splay->ist_ltreep = &splay->ist_ltree;
        splay->ist_rtreep = &splay->ist_rtree;

        ist_unlock(splay);
    }
}

/*
 * Routine:	ipc_splay_tree_bounds
 * Purpose:
 *		Given a name, returns the largest value present
 *		in the tree that is smaller than or equal to the name,
 *		or ~0 if no such value exists.  Similarly, returns
 *		the smallest value present that is greater than or
 *		equal to the name, or 0 if no such value exists.
 */

void
ipc_splay_tree_bounds(
    ipc_splay_tree_t    splay,
    mach_port_t         name,
    mach_port_t        *lowerp,
    mach_port_t        *upperp)
{
    ipc_tree_entry_t root;

    ist_lock(splay);

    root = splay->ist_root;
    if (root == ITE_NULL) {
        *lowerp = MACH_PORT_LARGEST;
        *upperp = MACH_PORT_SMALLEST;
    } else {
        mach_port_t rname;

        if (splay->ist_name != name) {
            ipc_splay_prim_assemble(root,
                &splay->ist_ltree, splay->ist_ltreep,
                &splay->ist_rtree, splay->ist_rtreep);
            ipc_splay_prim_lookup(name, root, &root,
                &splay->ist_ltree, &splay->ist_ltreep,
                &splay->ist_rtree, &splay->ist_rtreep);
            splay->ist_name = name;
            splay->ist_root = root;
        }

        rname = root->ite_name;

        if (rname <= name)
            *lowerp = rname;
        else if (splay->ist_ltreep == &splay->ist_ltree)
            *lowerp = MACH_PORT_LARGEST;
        else {
            ipc_tree_entry_t entry;

            entry = (ipc_tree_entry_t)
                ((char *)splay->ist_ltreep -
                 ((char *)&root->ite_rchild -
                  (char *)root));
            *lowerp = entry->ite_name;
        }

        if (rname >= name)
            *upperp = rname;
        else if (splay->ist_rtreep == &splay->ist_rtree)
            *upperp = MACH_PORT_SMALLEST;
        else {
            ipc_tree_entry_t entry;

            entry = (ipc_tree_entry_t)
                ((char *)splay->ist_rtreep -
                 ((char *)&root->ite_lchild -
                  (char *)root));
            *upperp = entry->ite_name;
        }
    }

    ist_unlock(splay);
}

/*
 * Routine:	ipc_splay_traverse_start
 * Routine:	ipc_splay_traverse_next
 * Routine:	ipc_splay_traverse_finish
 * Purpose:
 *		Perform a symmetric order traversal of a splay tree.
 *	Usage:
 *		for (entry = ipc_splay_traverse_start(splay);
 *		     entry != ITE_NULL;
 *		     entry = ipc_splay_traverse_next(splay, delete)) {
 *			do something with entry
 *		}
 *		ipc_splay_traverse_finish(splay);
 *
 *		If "delete" is TRUE, then the current entry
 *		is removed from the tree and deallocated.
 *
 *		During the traversal, the splay tree is locked.
 */

ipc_tree_entry_t
ipc_splay_traverse_start(
    ipc_splay_tree_t    splay)
{
    ipc_tree_entry_t current, parent;

    ist_lock(splay);

    current = splay->ist_root;
    if (current != ITE_NULL) {
        ipc_splay_prim_assemble(current,
            &splay->ist_ltree, splay->ist_ltreep,
            &splay->ist_rtree, splay->ist_rtreep);

        parent = ITE_NULL;

        while (current->ite_lchild != ITE_NULL) {
            ipc_tree_entry_t next;

            next = current->ite_lchild;
            current->ite_lchild = parent;
            parent = current;
            current = next;
        }

        splay->ist_ltree = current;
        splay->ist_rtree = parent;
    }

    return current;
}

ipc_tree_entry_t
ipc_splay_traverse_next(
    ipc_splay_tree_t    splay,
    boolean_t           delete)
{
    ipc_tree_entry_t current, parent;

    /* pick up where traverse_entry left off */

    current = splay->ist_ltree;
    parent = splay->ist_rtree;
    assert(current != ITE_NULL);

    if (!delete)
        goto traverse_right;

    /* we must delete current and patch the tree */

    if (current->ite_lchild == ITE_NULL) {
        if (current->ite_rchild == ITE_NULL) {
            /* like traverse_back, but with deletion */

            if (parent == ITE_NULL) {
                ite_free(current);

                splay->ist_root = ITE_NULL;
                return ITE_NULL;
            }

            if (current->ite_name < parent->ite_name) {
                ite_free(current);

                current = parent;
                parent = current->ite_lchild;
                current->ite_lchild = ITE_NULL;
                goto traverse_entry;
            } else {
                ite_free(current);

                current = parent;
                parent = current->ite_rchild;
                current->ite_rchild = ITE_NULL;
                goto traverse_back;
            }
        } else {
            ipc_tree_entry_t prev;

            prev = current;
            current = current->ite_rchild;
            ite_free(prev);
            goto traverse_left;
        }
    } else {
        if (current->ite_rchild == ITE_NULL) {
            ipc_tree_entry_t prev;

            prev = current;
            current = current->ite_lchild;
            ite_free(prev);
            goto traverse_back;
        } else {
            ipc_tree_entry_t prev;
            ipc_tree_entry_t ltree, rtree;
            ipc_tree_entry_t *ltreep, *rtreep;

            /* replace current with largest of left children */

            prev = current;
            ipc_splay_prim_lookup(MACH_PORT_LARGEST,
                current->ite_lchild, &current,
                &ltree, &ltreep, &rtree, &rtreep);
            ipc_splay_prim_assemble(current,
                &ltree, ltreep, &rtree, rtreep);

            assert(current->ite_rchild == ITE_NULL);
            current->ite_rchild = prev->ite_rchild;
            ite_free(prev);
            goto traverse_right;
        }
    }
    /*NOTREACHED*/

    /*
     * A state machine:  for each entry, we
     *   1) traverse left subtree
     *   2) traverse the entry
     *   3) traverse right subtree
     *   4) traverse back to parent
     */

    traverse_left:
    if (current->ite_lchild != ITE_NULL) {
        ipc_tree_entry_t next;

        next = current->ite_lchild;
        current->ite_lchild = parent;
        parent = current;
        current = next;
        goto traverse_left;
    }

    traverse_entry:
    splay->ist_ltree = current;
    splay->ist_rtree = parent;
    return current;

    traverse_right:
    if (current->ite_rchild != ITE_NULL) {
        ipc_tree_entry_t next;

        next = current->ite_rchild;
        current->ite_rchild = parent;
        parent = current;
        current = next;
        goto traverse_left;
    }

    traverse_back:
    if (parent == ITE_NULL) {
        splay->ist_root = current;
        return ITE_NULL;
    }

    if (current->ite_name < parent->ite_name) {
        ipc_tree_entry_t prev;

        prev = current;
        current = parent;
        parent = current->ite_lchild;
        current->ite_lchild = prev;
        goto traverse_entry;
    } else {
        ipc_tree_entry_t prev;

        prev = current;
        current = parent;
        parent = current->ite_rchild;
        current->ite_rchild = prev;
        goto traverse_back;
    }
}

void
ipc_splay_traverse_finish(
    ipc_splay_tree_t    splay)
{
    ipc_tree_entry_t root;

    root = splay->ist_root;
    if (root != ITE_NULL) {
        splay->ist_name = root->ite_name;
        splay->ist_ltreep = &splay->ist_ltree;
        splay->ist_rtreep = &splay->ist_rtree;
    }

    ist_unlock(splay);
}
