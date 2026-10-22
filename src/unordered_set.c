/* cadt - C Abstract Data Types
 * SPDX-License-Identifier: MIT
 */
#include "cadt/unordered_set.h"

#include "hashtable.h"
#include "internal.h"

struct cadt_uset {
    cadt_hash *table;
    const cadt_allocator *alloc;
};

static cadt_uset *create_set(const cadt_type *key, bool multi, const cadt_allocator *alloc)
{
    cadt_uset *set;

    if (cadt_type_check(key) != CADT_OK) {
        return nullptr;
    }
    alloc = cadt_alloc_or_default(alloc);
    set = (cadt_uset *)cadt_xalloc(alloc, sizeof(*set));
    if (set == nullptr) {
        return nullptr;
    }
    cadt_hash_cfg cfg = {
        .elem = *key,
        .key_size = key->size,
        .key_cmp = key->compare,
        .key_hash = key->hash,
        .key_ctx = key->ctx,
        .multi = multi,
    };
    set->alloc = alloc;
    set->table = cadt_hash_create(&cfg, alloc);
    if (set->table == nullptr) {
        cadt_xfree(alloc, set);
        return nullptr;
    }
    return set;
}

static cadt_uset_iter from_hash(cadt_hash_iter it)
{
    cadt_uset_iter out;

    out.table = it.table;
    out.bucket = it.bucket;
    out.node = it.node;
    return out;
}

static cadt_hash_iter to_hash(cadt_uset_iter it)
{
    cadt_hash_iter out;

    out.table = it.table;
    out.bucket = it.bucket;
    out.node = it.node;
    return out;
}

cadt_uset *cadt_uset_create(const cadt_type *key, const cadt_allocator *alloc)
{
    return create_set(key, false, alloc);
}

cadt_uset *cadt_umultiset_create(const cadt_type *key, const cadt_allocator *alloc)
{
    return create_set(key, true, alloc);
}

void cadt_uset_destroy(cadt_uset *set)
{
    if (set == nullptr) {
        return;
    }
    cadt_hash_destroy(set->table);
    cadt_xfree(set->alloc, set);
}

size_t cadt_uset_size(const cadt_uset *set)
{
    return set == nullptr ? 0 : cadt_hash_size(set->table);
}

size_t cadt_uset_bucket_count(const cadt_uset *set)
{
    return set == nullptr ? 0 : cadt_hash_buckets(set->table);
}

bool cadt_uset_empty(const cadt_uset *set)
{
    return cadt_uset_size(set) == 0;
}

bool cadt_uset_is_multi(const cadt_uset *set)
{
    return set != nullptr && cadt_hash_is_multi(set->table);
}

void cadt_uset_clear(cadt_uset *set)
{
    if (set != nullptr) {
        cadt_hash_clear(set->table);
    }
}

cadt_status cadt_uset_insert(cadt_uset *set, const void *key)
{
    if (set == nullptr) {
        return CADT_ERR_INVALID;
    }
    return cadt_hash_insert(set->table, key);
}

size_t cadt_uset_erase(cadt_uset *set, const void *key)
{
    return set == nullptr ? 0 : cadt_hash_erase(set->table, key);
}

bool cadt_uset_contains(const cadt_uset *set, const void *key)
{
    return cadt_uset_find(set, key) != nullptr;
}

size_t cadt_uset_count(const cadt_uset *set, const void *key)
{
    return set == nullptr ? 0 : cadt_hash_count(set->table, key);
}

void *cadt_uset_find(const cadt_uset *set, const void *key)
{
    return set == nullptr ? nullptr : cadt_hash_find(set->table, key);
}

cadt_uset_iter cadt_uset_begin(const cadt_uset *set)
{
    if (set == nullptr) {
        return from_hash(cadt_hash_begin(nullptr));
    }
    return from_hash(cadt_hash_begin(set->table));
}

cadt_uset_iter cadt_uset_end(const cadt_uset *set)
{
    if (set == nullptr) {
        return from_hash(cadt_hash_end(nullptr));
    }
    return from_hash(cadt_hash_end(set->table));
}

cadt_uset_iter cadt_uset_next(cadt_uset_iter it)
{
    return from_hash(cadt_hash_next(to_hash(it)));
}

bool cadt_uset_iter_eq(cadt_uset_iter a, cadt_uset_iter b)
{
    if (a.node == nullptr && b.node == nullptr) {
        return true;
    }
    return a.node == b.node;
}

void *cadt_uset_iter_data(cadt_uset_iter it)
{
    return cadt_hash_iter_data(to_hash(it));
}

cadt_status cadt_uset_validate(const cadt_uset *set)
{
    if (set == nullptr) {
        return CADT_ERR_INVALID;
    }
    return cadt_hash_validate(set->table);
}
