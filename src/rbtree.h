/* cadt - C Abstract Data Types
 * SPDX-License-Identifier: MIT
 *
 * Private red-black tree used by set and map. Not a public header.
 */
#ifndef CADT_RBTREE_H
#define CADT_RBTREE_H

#include "cadt/common.h"

typedef struct cadt_tree cadt_tree;

typedef struct cadt_tree_cfg {
    cadt_type elem;
    size_t key_offset;
    size_t key_size;
    cadt_compare_fn key_cmp;
    void *key_ctx;
    bool multi;
} cadt_tree_cfg;

cadt_tree *cadt_tree_create(const cadt_tree_cfg *cfg, const cadt_allocator *alloc);
void cadt_tree_destroy(cadt_tree *tree);
void cadt_tree_clear(cadt_tree *tree);

size_t cadt_tree_size(const cadt_tree *tree);
bool cadt_tree_is_multi(const cadt_tree *tree);

/* Copies elem (or memcpy when copy is NULL). Caller keeps the source. */
cadt_status cadt_tree_insert(cadt_tree *tree, const void *elem);

/* Erase every element whose key compares equal. Returns how many were removed. */
size_t cadt_tree_erase(cadt_tree *tree, const void *key);

/* Erase one match. CADT_ERR_NOT_FOUND when absent. out may be NULL. */
cadt_status cadt_tree_erase_one(cadt_tree *tree, const void *key, void *out);

void *cadt_tree_find(const cadt_tree *tree, const void *key);
size_t cadt_tree_count(const cadt_tree *tree, const void *key);
void *cadt_tree_lower(const cadt_tree *tree, const void *key);
void *cadt_tree_upper(const cadt_tree *tree, const void *key);
void *cadt_tree_minimum(const cadt_tree *tree);
void *cadt_tree_maximum(const cadt_tree *tree);

/* Element-data pointers. NULL stays NULL. Walking off either end yields NULL. */
void *cadt_tree_next(void *elem);
void *cadt_tree_prev(void *elem);

cadt_status cadt_tree_validate(const cadt_tree *tree);

#endif /* CADT_RBTREE_H */
