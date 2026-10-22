/* cadt - C Abstract Data Types
 * SPDX-License-Identifier: MIT
 */
#include "rbtree.h"

#include "internal.h"

#include <stdalign.h>
#include <stdio.h>

enum { CADT_BLACK = 0, CADT_RED = 1 };

typedef struct cadt_rbnode {
    struct cadt_rbnode *parent;
    struct cadt_rbnode *left;
    struct cadt_rbnode *right;
    unsigned char color;
    _Alignas(max_align_t) unsigned char data[];
} cadt_rbnode;

struct cadt_tree {
    const cadt_allocator *alloc;
    cadt_type elem;
    size_t key_offset;
    size_t key_size;
    cadt_compare_fn key_cmp;
    void *key_ctx;
    bool multi;
    cadt_rbnode *root;
    size_t size;
};

static cadt_rbnode *node_from_data(void *elem)
{
    return (cadt_rbnode *)((unsigned char *)elem - offsetof(cadt_rbnode, data));
}

static int cmp_elems(const cadt_tree *tree, const void *a, const void *b)
{
    const unsigned char *ka = (const unsigned char *)a + tree->key_offset;
    const unsigned char *kb = (const unsigned char *)b + tree->key_offset;

    if (tree->key_cmp != NULL) {
        return tree->key_cmp(ka, kb, tree->key_ctx);
    }
    return memcmp(ka, kb, tree->key_size);
}

static int cmp_key(const cadt_tree *tree, const void *key, const void *elem)
{
    const unsigned char *stored = (const unsigned char *)elem + tree->key_offset;

    if (tree->key_cmp != NULL) {
        return tree->key_cmp(key, stored, tree->key_ctx);
    }
    return memcmp(key, stored, tree->key_size);
}

static void rotate_left(cadt_tree *tree, cadt_rbnode *x)
{
    cadt_rbnode *y = x->right;

    x->right = y->left;
    if (y->left != NULL) {
        y->left->parent = x;
    }
    y->parent = x->parent;
    if (x->parent == NULL) {
        tree->root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }
    y->left = x;
    x->parent = y;
}

static void rotate_right(cadt_tree *tree, cadt_rbnode *x)
{
    cadt_rbnode *y = x->left;

    x->left = y->right;
    if (y->right != NULL) {
        y->right->parent = x;
    }
    y->parent = x->parent;
    if (x->parent == NULL) {
        tree->root = y;
    } else if (x == x->parent->right) {
        x->parent->right = y;
    } else {
        x->parent->left = y;
    }
    y->right = x;
    x->parent = y;
}

static void insert_fixup(cadt_tree *tree, cadt_rbnode *z)
{
    while (z->parent != NULL && z->parent->color == CADT_RED) {
        cadt_rbnode *gp = z->parent->parent;

        if (gp == NULL) {
            break;
        }
        if (z->parent == gp->left) {
            cadt_rbnode *y = gp->right;

            if (y != NULL && y->color == CADT_RED) {
                z->parent->color = CADT_BLACK;
                y->color = CADT_BLACK;
                gp->color = CADT_RED;
                z = gp;
            } else {
                if (z == z->parent->right) {
                    z = z->parent;
                    rotate_left(tree, z);
                }
                z->parent->color = CADT_BLACK;
                z->parent->parent->color = CADT_RED;
                rotate_right(tree, z->parent->parent);
            }
        } else {
            cadt_rbnode *y = gp->left;

            if (y != NULL && y->color == CADT_RED) {
                z->parent->color = CADT_BLACK;
                y->color = CADT_BLACK;
                gp->color = CADT_RED;
                z = gp;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    rotate_right(tree, z);
                }
                z->parent->color = CADT_BLACK;
                z->parent->parent->color = CADT_RED;
                rotate_left(tree, z->parent->parent);
            }
        }
    }
    if (tree->root != NULL) {
        tree->root->color = CADT_BLACK;
    }
}

static void transplant(cadt_tree *tree, cadt_rbnode *u, cadt_rbnode *v)
{
    if (u->parent == NULL) {
        tree->root = v;
    } else if (u == u->parent->left) {
        u->parent->left = v;
    } else {
        u->parent->right = v;
    }
    if (v != NULL) {
        v->parent = u->parent;
    }
}

static cadt_rbnode *minimum_node(cadt_rbnode *node)
{
    while (node != NULL && node->left != NULL) {
        node = node->left;
    }
    return node;
}

static cadt_rbnode *maximum_node(cadt_rbnode *node)
{
    while (node != NULL && node->right != NULL) {
        node = node->right;
    }
    return node;
}

static cadt_rbnode *successor_node(cadt_rbnode *node)
{
    cadt_rbnode *parent;

    if (node == NULL) {
        return NULL;
    }
    if (node->right != NULL) {
        return minimum_node(node->right);
    }
    parent = node->parent;
    while (parent != NULL && node == parent->right) {
        node = parent;
        parent = parent->parent;
    }
    return parent;
}

static cadt_rbnode *predecessor_node(cadt_rbnode *node)
{
    cadt_rbnode *parent;

    if (node == NULL) {
        return NULL;
    }
    if (node->left != NULL) {
        return maximum_node(node->left);
    }
    parent = node->parent;
    while (parent != NULL && node == parent->left) {
        node = parent;
        parent = parent->parent;
    }
    return parent;
}

static void delete_fixup(cadt_tree *tree, cadt_rbnode *x, cadt_rbnode *parent, bool x_is_left)
{
    while (x != tree->root && (x == NULL || x->color == CADT_BLACK)) {
        cadt_rbnode *w;

        if (x != NULL) {
            parent = x->parent;
        }
        if (parent == NULL) {
            break;
        }
        if (x != NULL) {
            x_is_left = (x == parent->left);
        }
        if (x_is_left) {
            w = parent->right;
            if (w == NULL) {
                break;
            }
            if (w->color == CADT_RED) {
                w->color = CADT_BLACK;
                parent->color = CADT_RED;
                rotate_left(tree, parent);
                w = parent->right;
                if (w == NULL) {
                    break;
                }
            }
            if ((w->left == NULL || w->left->color == CADT_BLACK) &&
                (w->right == NULL || w->right->color == CADT_BLACK)) {
                w->color = CADT_RED;
                x = parent;
                parent = x->parent;
                x_is_left = (parent != NULL && x == parent->left);
            } else {
                if (w->right == NULL || w->right->color == CADT_BLACK) {
                    if (w->left != NULL) {
                        w->left->color = CADT_BLACK;
                    }
                    w->color = CADT_RED;
                    rotate_right(tree, w);
                    w = parent->right;
                }
                if (w == NULL) {
                    break;
                }
                w->color = parent->color;
                parent->color = CADT_BLACK;
                if (w->right != NULL) {
                    w->right->color = CADT_BLACK;
                }
                rotate_left(tree, parent);
                x = tree->root;
                parent = NULL;
            }
        } else {
            w = parent->left;
            if (w == NULL) {
                break;
            }
            if (w->color == CADT_RED) {
                w->color = CADT_BLACK;
                parent->color = CADT_RED;
                rotate_right(tree, parent);
                w = parent->left;
                if (w == NULL) {
                    break;
                }
            }
            if ((w->right == NULL || w->right->color == CADT_BLACK) &&
                (w->left == NULL || w->left->color == CADT_BLACK)) {
                w->color = CADT_RED;
                x = parent;
                parent = x->parent;
                x_is_left = (parent != NULL && x == parent->left);
            } else {
                if (w->left == NULL || w->left->color == CADT_BLACK) {
                    if (w->right != NULL) {
                        w->right->color = CADT_BLACK;
                    }
                    w->color = CADT_RED;
                    rotate_left(tree, w);
                    w = parent->left;
                }
                if (w == NULL) {
                    break;
                }
                w->color = parent->color;
                parent->color = CADT_BLACK;
                if (w->left != NULL) {
                    w->left->color = CADT_BLACK;
                }
                rotate_right(tree, parent);
                x = tree->root;
                parent = NULL;
            }
        }
    }
    if (x != NULL) {
        x->color = CADT_BLACK;
    }
    if (tree->root != NULL) {
        tree->root->color = CADT_BLACK;
    }
}

static void erase_node(cadt_tree *tree, cadt_rbnode *z, void *out)
{
    cadt_rbnode *y = z;
    cadt_rbnode *x;
    cadt_rbnode *x_parent;
    bool x_is_left = false;
    int y_was_black;

    if (z->left == NULL) {
        x = z->right;
        x_parent = z->parent;
        x_is_left = (z->parent != NULL && z == z->parent->left);
        y_was_black = (y->color == CADT_BLACK);
        transplant(tree, z, z->right);
    } else if (z->right == NULL) {
        x = z->left;
        x_parent = z->parent;
        x_is_left = (z->parent != NULL && z == z->parent->left);
        y_was_black = (y->color == CADT_BLACK);
        transplant(tree, z, z->left);
    } else {
        y = minimum_node(z->right);
        y_was_black = (y->color == CADT_BLACK);
        x = y->right;
        if (y->parent == z) {
            x_parent = y;
            x_is_left = false;
        } else {
            x_parent = y->parent;
            x_is_left = true;
            transplant(tree, y, y->right);
            y->right = z->right;
            if (y->right != NULL) {
                y->right->parent = y;
            }
        }
        transplant(tree, z, y);
        y->left = z->left;
        if (y->left != NULL) {
            y->left->parent = y;
        }
        y->color = z->color;
    }

    if (out != NULL) {
        memcpy(out, z->data, tree->elem.size);
    } else {
        cadt_type_destroy(&tree->elem, z->data);
    }
    cadt_xfree(tree->alloc, z);
    if (tree->size > 0) {
        tree->size--;
    }
    if (y_was_black) {
        delete_fixup(tree, x, x_parent, x_is_left);
    }
}

static cadt_rbnode *find_node(const cadt_tree *tree, const void *key)
{
    cadt_rbnode *cur = tree->root;

    while (cur != NULL) {
        int cmp = cmp_key(tree, key, cur->data);

        if (cmp == 0) {
            return cur;
        }
        cur = cmp < 0 ? cur->left : cur->right;
    }
    return NULL;
}

static void free_subtree(cadt_tree *tree, cadt_rbnode *node)
{
    if (node == NULL) {
        return;
    }
    free_subtree(tree, node->left);
    free_subtree(tree, node->right);
    cadt_type_destroy(&tree->elem, node->data);
    cadt_xfree(tree->alloc, node);
}

cadt_tree *cadt_tree_create(const cadt_tree_cfg *cfg, const cadt_allocator *alloc)
{
    cadt_tree *tree;

    if (cfg == NULL || cadt_type_check(&cfg->elem) != CADT_OK || cfg->key_size == 0 ||
        cfg->key_offset > cfg->elem.size || cfg->key_size > cfg->elem.size - cfg->key_offset) {
        return NULL;
    }
    alloc = cadt_alloc_or_default(alloc);
    tree = (cadt_tree *)cadt_xalloc(alloc, sizeof(*tree));
    if (tree == NULL) {
        return NULL;
    }
    tree->alloc = alloc;
    tree->elem = cfg->elem;
    tree->key_offset = cfg->key_offset;
    tree->key_size = cfg->key_size;
    tree->key_cmp = cfg->key_cmp;
    tree->key_ctx = cfg->key_ctx;
    tree->multi = cfg->multi;
    tree->root = NULL;
    tree->size = 0;
    return tree;
}

void cadt_tree_clear(cadt_tree *tree)
{
    if (tree == NULL) {
        return;
    }
    free_subtree(tree, tree->root);
    tree->root = NULL;
    tree->size = 0;
}

void cadt_tree_destroy(cadt_tree *tree)
{
    if (tree == NULL) {
        return;
    }
    cadt_tree_clear(tree);
    cadt_xfree(tree->alloc, tree);
}

size_t cadt_tree_size(const cadt_tree *tree)
{
    return tree == NULL ? 0 : tree->size;
}

bool cadt_tree_is_multi(const cadt_tree *tree)
{
    return tree != NULL && tree->multi;
}

cadt_status cadt_tree_insert(cadt_tree *tree, const void *elem)
{
    cadt_rbnode *parent = NULL;
    cadt_rbnode *cur;
    cadt_rbnode *node;
    size_t bytes;
    int cmp = 0;
    cadt_status status;

    if (tree == NULL || elem == NULL) {
        return CADT_ERR_INVALID;
    }
    cur = tree->root;
    while (cur != NULL) {
        cmp = cmp_elems(tree, elem, cur->data);
        parent = cur;
        if (cmp < 0) {
            cur = cur->left;
        } else if (cmp > 0) {
            cur = cur->right;
        } else if (!tree->multi) {
            return CADT_ERR_EXISTS;
        } else {
            cur = cur->right;
        }
    }
    if (!cadt_add_size(offsetof(cadt_rbnode, data), tree->elem.size, &bytes)) {
        return CADT_ERR_OVERFLOW;
    }
    node = (cadt_rbnode *)cadt_xalloc(tree->alloc, bytes);
    if (node == NULL) {
        return CADT_ERR_NOMEM;
    }
    status = cadt_type_copy(&tree->elem, node->data, elem);
    if (status != CADT_OK) {
        cadt_xfree(tree->alloc, node);
        return status;
    }
    node->parent = parent;
    node->left = NULL;
    node->right = NULL;
    node->color = CADT_RED;
    if (parent == NULL) {
        tree->root = node;
    } else if (cmp < 0) {
        parent->left = node;
    } else {
        parent->right = node;
    }
    tree->size++;
    insert_fixup(tree, node);
    return CADT_OK;
}

size_t cadt_tree_erase(cadt_tree *tree, const void *key)
{
    size_t removed = 0;
    cadt_rbnode *node;

    if (tree == NULL || key == NULL) {
        return 0;
    }
    for (;;) {
        node = find_node(tree, key);
        if (node == NULL) {
            break;
        }
        erase_node(tree, node, NULL);
        removed++;
        if (!tree->multi) {
            break;
        }
    }
    return removed;
}

cadt_status cadt_tree_erase_one(cadt_tree *tree, const void *key, void *out)
{
    cadt_rbnode *node;

    if (tree == NULL || key == NULL) {
        return CADT_ERR_INVALID;
    }
    node = find_node(tree, key);
    if (node == NULL) {
        return CADT_ERR_NOT_FOUND;
    }
    erase_node(tree, node, out);
    return CADT_OK;
}

void *cadt_tree_find(const cadt_tree *tree, const void *key)
{
    cadt_rbnode *node;

    if (tree == NULL || key == NULL) {
        return NULL;
    }
    node = find_node(tree, key);
    return node == NULL ? NULL : node->data;
}

size_t cadt_tree_count(const cadt_tree *tree, const void *key)
{
    size_t count = 0;
    void *elem;

    if (tree == NULL || key == NULL) {
        return 0;
    }
    elem = cadt_tree_lower(tree, key);
    while (elem != NULL && cmp_key(tree, key, elem) == 0) {
        count++;
        elem = cadt_tree_next(elem);
    }
    return count;
}

void *cadt_tree_lower(const cadt_tree *tree, const void *key)
{
    cadt_rbnode *cur;
    cadt_rbnode *best = NULL;

    if (tree == NULL || key == NULL) {
        return NULL;
    }
    cur = tree->root;
    while (cur != NULL) {
        if (cmp_key(tree, key, cur->data) <= 0) {
            best = cur;
            cur = cur->left;
        } else {
            cur = cur->right;
        }
    }
    return best == NULL ? NULL : best->data;
}

void *cadt_tree_upper(const cadt_tree *tree, const void *key)
{
    cadt_rbnode *cur;
    cadt_rbnode *best = NULL;

    if (tree == NULL || key == NULL) {
        return NULL;
    }
    cur = tree->root;
    while (cur != NULL) {
        if (cmp_key(tree, key, cur->data) < 0) {
            best = cur;
            cur = cur->left;
        } else {
            cur = cur->right;
        }
    }
    return best == NULL ? NULL : best->data;
}

void *cadt_tree_minimum(const cadt_tree *tree)
{
    cadt_rbnode *node;

    if (tree == NULL) {
        return NULL;
    }
    node = minimum_node(tree->root);
    return node == NULL ? NULL : node->data;
}

void *cadt_tree_maximum(const cadt_tree *tree)
{
    cadt_rbnode *node;

    if (tree == NULL) {
        return NULL;
    }
    node = maximum_node(tree->root);
    return node == NULL ? NULL : node->data;
}

void *cadt_tree_next(void *elem)
{
    cadt_rbnode *node;

    if (elem == NULL) {
        return NULL;
    }
    node = successor_node(node_from_data(elem));
    return node == NULL ? NULL : node->data;
}

void *cadt_tree_prev(void *elem)
{
    cadt_rbnode *node;

    if (elem == NULL) {
        return NULL;
    }
    node = predecessor_node(node_from_data(elem));
    return node == NULL ? NULL : node->data;
}

static size_t count_nodes(const cadt_rbnode *node)
{
    if (node == NULL) {
        return 0;
    }
    return 1u + count_nodes(node->left) + count_nodes(node->right);
}

static int check_node(const cadt_tree *tree, cadt_rbnode *node, cadt_rbnode *parent,
                      const void *lo, bool has_lo, const void *hi, bool has_hi,
                      int blacks, int *expected, bool *seen)
{
    int add;
    int left;
    int right;

    if (node == NULL) {
        int height = blacks + 1;

        if (!*seen) {
            *expected = height;
            *seen = true;
        } else if (*expected != height) {
            return 4;
        }
        return 0;
    }
    if (node->parent != parent) {
        return 1;
    }
    if (node->color != CADT_RED && node->color != CADT_BLACK) {
        return 2;
    }
    if (node->color == CADT_RED && parent != NULL && parent->color == CADT_RED) {
        return 2;
    }
    /* Equal keys may sit on either side after a rotation. In-order stays
     * sorted when every left-subtree key is <= this node and every
     * right-subtree key is >= this node. */
    if (has_lo && cmp_elems(tree, node->data, lo) < 0) {
        return 3;
    }
    if (has_hi && cmp_elems(tree, node->data, hi) > 0) {
        return 3;
    }
    add = node->color == CADT_BLACK ? 1 : 0;
    left = check_node(tree, node->left, node, lo, has_lo, node->data, true,
                      blacks + add, expected, seen);
    if (left != 0) {
        return left;
    }
    right = check_node(tree, node->right, node, node->data, true, hi, has_hi,
                       blacks + add, expected, seen);
    return right;
}

cadt_status cadt_tree_validate(const cadt_tree *tree)
{
    int expected = 0;
    bool seen = false;
    int code;

    if (tree == NULL) {
        return CADT_ERR_INVALID;
    }
    if (tree->root == NULL) {
        if (tree->size != 0) {
            fprintf(stderr, "cadt tree invariant 5\n");
            return CADT_ERR_INVALID;
        }
        return CADT_OK;
    }
    if (tree->root->color != CADT_BLACK || tree->root->parent != NULL) {
        fprintf(stderr, "cadt tree invariant 2\n");
        return CADT_ERR_INVALID;
    }
    code = check_node(tree, tree->root, NULL, NULL, false, NULL, false, 0, &expected, &seen);
    if (code != 0) {
        fprintf(stderr, "cadt tree invariant %d\n", code);
        return CADT_ERR_INVALID;
    }
    if (count_nodes(tree->root) != tree->size) {
        fprintf(stderr, "cadt tree invariant 5\n");
        return CADT_ERR_INVALID;
    }
    return CADT_OK;
}
