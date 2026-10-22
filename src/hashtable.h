/* cadt - C Abstract Data Types
 * SPDX-License-Identifier: MIT
 *
 * Private chained hash table used by the unordered containers.
 */
#ifndef CADT_HASHTABLE_H
#define CADT_HASHTABLE_H

#include "cadt/common.h"

typedef struct cadt_hash cadt_hash;

typedef struct cadt_hash_cfg {
    cadt_type elem;
    size_t key_offset;
    size_t key_size;
    cadt_compare_fn key_cmp;
    cadt_hash_fn key_hash;
    void *key_ctx;
    bool multi;
} cadt_hash_cfg;

typedef struct cadt_hash_iter {
    void *table;
    size_t bucket;
    void *node;
} cadt_hash_iter;

cadt_hash *cadt_hash_create(const cadt_hash_cfg *cfg, const cadt_allocator *alloc);
void cadt_hash_destroy(cadt_hash *table);
void cadt_hash_clear(cadt_hash *table);

size_t cadt_hash_size(const cadt_hash *table);
size_t cadt_hash_buckets(const cadt_hash *table);
bool cadt_hash_is_multi(const cadt_hash *table);

cadt_status cadt_hash_insert(cadt_hash *table, const void *elem);
size_t cadt_hash_erase(cadt_hash *table, const void *key);
void *cadt_hash_find(const cadt_hash *table, const void *key);
size_t cadt_hash_count(const cadt_hash *table, const void *key);

cadt_hash_iter cadt_hash_begin(const cadt_hash *table);
cadt_hash_iter cadt_hash_end(const cadt_hash *table);
cadt_hash_iter cadt_hash_next(cadt_hash_iter it);
void *cadt_hash_iter_data(cadt_hash_iter it);

cadt_status cadt_hash_validate(const cadt_hash *table);

#endif /* CADT_HASHTABLE_H */
