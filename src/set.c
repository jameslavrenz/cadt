/* cadt - C Abstract Data Types
 * SPDX-License-Identifier: MIT
 */
#include "cadt/set.h"

#include "internal.h"
#include "rbtree.h"

struct cadt_set {
    cadt_tree *tree;
    const cadt_allocator *alloc;
};

static cadt_set *create_set(const cadt_type *key, bool multi, const cadt_allocator *alloc)
{
    cadt_set *set;
    cadt_tree_cfg cfg;

    if (key == nullptr || key->compare == nullptr || cadt_type_check(key) != CADT_OK) {
        return nullptr;
    }
    alloc = cadt_alloc_or_default(alloc);
    set = (cadt_set *)cadt_xalloc(alloc, sizeof(*set));
    if (set == nullptr) {
        return nullptr;
    }
    memset(&cfg, 0, sizeof(cfg));
    cfg.elem = *key;
    cfg.key_offset = 0;
    cfg.key_size = key->size;
    cfg.key_cmp = key->compare;
    cfg.key_ctx = key->ctx;
    cfg.multi = multi;
    set->alloc = alloc;
    set->tree = cadt_tree_create(&cfg, alloc);
    if (set->tree == nullptr) {
        cadt_xfree(alloc, set);
        return nullptr;
    }
    return set;
}

static cadt_set_iter iter_of(const cadt_set *set, void *node)
{
    cadt_set_iter it;

    it.node = node;
    it.owner = set;
    return it;
}

cadt_set *cadt_set_create(const cadt_type *key, const cadt_allocator *alloc)
{
    return create_set(key, false, alloc);
}

cadt_set *cadt_multiset_create(const cadt_type *key, const cadt_allocator *alloc)
{
    return create_set(key, true, alloc);
}

void cadt_set_destroy(cadt_set *set)
{
    if (set == nullptr) {
        return;
    }
    cadt_tree_destroy(set->tree);
    cadt_xfree(set->alloc, set);
}

size_t cadt_set_size(const cadt_set *set)
{
    return set == nullptr ? 0 : cadt_tree_size(set->tree);
}

bool cadt_set_empty(const cadt_set *set)
{
    return cadt_set_size(set) == 0;
}

bool cadt_set_is_multi(const cadt_set *set)
{
    return set != nullptr && cadt_tree_is_multi(set->tree);
}

void cadt_set_clear(cadt_set *set)
{
    if (set != nullptr) {
        cadt_tree_clear(set->tree);
    }
}

cadt_status cadt_set_insert(cadt_set *set, const void *key)
{
    if (set == nullptr) {
        return CADT_ERR_INVALID;
    }
    return cadt_tree_insert(set->tree, key);
}

size_t cadt_set_erase(cadt_set *set, const void *key)
{
    return set == nullptr ? 0 : cadt_tree_erase(set->tree, key);
}

cadt_status cadt_set_erase_one(cadt_set *set, const void *key, void *out)
{
    if (set == nullptr) {
        return CADT_ERR_INVALID;
    }
    return cadt_tree_erase_one(set->tree, key, out);
}

bool cadt_set_contains(const cadt_set *set, const void *key)
{
    return cadt_set_find(set, key) != nullptr;
}

size_t cadt_set_count(const cadt_set *set, const void *key)
{
    return set == nullptr ? 0 : cadt_tree_count(set->tree, key);
}

void *cadt_set_find(const cadt_set *set, const void *key)
{
    return set == nullptr ? nullptr : cadt_tree_find(set->tree, key);
}

cadt_set_iter cadt_set_begin(const cadt_set *set)
{
    if (set == nullptr) {
        return iter_of(nullptr, nullptr);
    }
    return iter_of(set, cadt_tree_minimum(set->tree));
}

cadt_set_iter cadt_set_end(const cadt_set *set)
{
    return iter_of(set, nullptr);
}

cadt_set_iter cadt_set_next(cadt_set_iter it)
{
    if (it.node != nullptr) {
        it.node = cadt_tree_next(it.node);
    }
    return it;
}

cadt_set_iter cadt_set_prev(cadt_set_iter it)
{
    const cadt_set *set = (const cadt_set *)it.owner;

    if (set == nullptr) {
        return it;
    }
    if (it.node == nullptr) {
        it.node = cadt_tree_maximum(set->tree);
        return it;
    }
    it.node = cadt_tree_prev(it.node);
    return it;
}

bool cadt_set_iter_eq(cadt_set_iter a, cadt_set_iter b)
{
    return a.owner == b.owner && a.node == b.node;
}

void *cadt_set_iter_data(cadt_set_iter it)
{
    return it.node;
}

cadt_set_iter cadt_set_lower_bound(const cadt_set *set, const void *key)
{
    if (set == nullptr) {
        return iter_of(nullptr, nullptr);
    }
    return iter_of(set, cadt_tree_lower(set->tree, key));
}

cadt_set_iter cadt_set_upper_bound(const cadt_set *set, const void *key)
{
    if (set == nullptr) {
        return iter_of(nullptr, nullptr);
    }
    return iter_of(set, cadt_tree_upper(set->tree, key));
}

void *cadt_set_min(const cadt_set *set)
{
    return set == nullptr ? nullptr : cadt_tree_minimum(set->tree);
}

void *cadt_set_max(const cadt_set *set)
{
    return set == nullptr ? nullptr : cadt_tree_maximum(set->tree);
}

cadt_status cadt_set_validate(const cadt_set *set)
{
    if (set == nullptr) {
        return CADT_ERR_INVALID;
    }
    return cadt_tree_validate(set->tree);
}
