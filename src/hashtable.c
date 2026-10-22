/* cadt - C Abstract Data Types
 * SPDX-License-Identifier: MIT
 */
#include "hashtable.h"

#include "internal.h"

#include <stdalign.h>
#include <stdio.h>

typedef struct cadt_hnode {
    struct cadt_hnode *next;
    uint64_t hash;
    alignas(max_align_t) unsigned char data[];
} cadt_hnode;

struct cadt_hash {
    const cadt_allocator *alloc;
    cadt_type elem;
    size_t key_offset;
    size_t key_size;
    cadt_compare_fn key_cmp;
    cadt_hash_fn key_hash;
    void *key_ctx;
    bool multi;
    cadt_hnode **buckets;
    size_t nbuckets;
    size_t size;
};

static const void *key_ptr(const cadt_hash *table, const void *elem)
{
    return (const unsigned char *)elem + table->key_offset;
}

static uint64_t hash_key(const cadt_hash *table, const void *key)
{
    if (table->key_hash != nullptr) {
        return table->key_hash(key, table->key_ctx);
    }
    return cadt_hash_bytes(key, table->key_size);
}

static uint64_t hash_elem(const cadt_hash *table, const void *elem)
{
    return hash_key(table, key_ptr(table, elem));
}

static int cmp_key(const cadt_hash *table, const void *key, const void *elem)
{
    const void *stored = key_ptr(table, elem);

    if (table->key_cmp != nullptr) {
        return table->key_cmp(key, stored, table->key_ctx);
    }
    return memcmp(key, stored, table->key_size);
}

static cadt_hnode *find_in_bucket(const cadt_hash *table, size_t index, const void *key,
                                  uint64_t hash)
{
    cadt_hnode *node = table->buckets[index];

    while (node != nullptr) {
        if (node->hash == hash && cmp_key(table, key, node->data) == 0) {
            return node;
        }
        node = node->next;
    }
    return nullptr;
}

static cadt_status rehash(cadt_hash *table, size_t new_n)
{
    cadt_hnode **fresh;
    size_t bytes;
    size_t i;

    if (new_n < 8u || (new_n & (new_n - 1u)) != 0) {
        return CADT_ERR_INVALID;
    }
    if (!cadt_mul_size(new_n, sizeof(*fresh), &bytes)) {
        return CADT_ERR_OVERFLOW;
    }
    fresh = (cadt_hnode **)cadt_xalloc(table->alloc, bytes);
    if (fresh == nullptr) {
        return CADT_ERR_NOMEM;
    }
    memset(fresh, 0, bytes);
    for (i = 0; i < table->nbuckets; i++) {
        cadt_hnode *node = table->buckets[i];

        while (node != nullptr) {
            cadt_hnode *next = node->next;
            size_t idx = (size_t)(node->hash & (uint64_t)(new_n - 1u));

            node->next = fresh[idx];
            fresh[idx] = node;
            node = next;
        }
    }
    cadt_xfree(table->alloc, table->buckets);
    table->buckets = fresh;
    table->nbuckets = new_n;
    return CADT_OK;
}

static void free_node(cadt_hash *table, cadt_hnode *node)
{
    cadt_type_destroy(&table->elem, node->data);
    cadt_xfree(table->alloc, node);
}

cadt_hash *cadt_hash_create(const cadt_hash_cfg *cfg, const cadt_allocator *alloc)
{
    cadt_hash *table;
    size_t bytes;

    if (cfg == nullptr || cadt_type_check(&cfg->elem) != CADT_OK || cfg->key_size == 0 ||
        cfg->key_offset > cfg->elem.size || cfg->key_size > cfg->elem.size - cfg->key_offset) {
        return nullptr;
    }
    alloc = cadt_alloc_or_default(alloc);
    table = (cadt_hash *)cadt_xalloc(alloc, sizeof(*table));
    if (table == nullptr) {
        return nullptr;
    }
    table->alloc = alloc;
    table->elem = cfg->elem;
    table->key_offset = cfg->key_offset;
    table->key_size = cfg->key_size;
    table->key_cmp = cfg->key_cmp;
    table->key_hash = cfg->key_hash;
    table->key_ctx = cfg->key_ctx;
    table->multi = cfg->multi;
    table->nbuckets = 8u;
    table->size = 0;
    if (!cadt_mul_size(table->nbuckets, sizeof(*table->buckets), &bytes)) {
        cadt_xfree(alloc, table);
        return nullptr;
    }
    table->buckets = (cadt_hnode **)cadt_xalloc(alloc, bytes);
    if (table->buckets == nullptr) {
        cadt_xfree(alloc, table);
        return nullptr;
    }
    memset(table->buckets, 0, bytes);
    return table;
}

void cadt_hash_clear(cadt_hash *table)
{
    size_t i;

    if (table == nullptr) {
        return;
    }
    for (i = 0; i < table->nbuckets; i++) {
        cadt_hnode *node = table->buckets[i];

        while (node != nullptr) {
            cadt_hnode *next = node->next;

            free_node(table, node);
            node = next;
        }
        table->buckets[i] = nullptr;
    }
    table->size = 0;
}

void cadt_hash_destroy(cadt_hash *table)
{
    if (table == nullptr) {
        return;
    }
    cadt_hash_clear(table);
    cadt_xfree(table->alloc, table->buckets);
    cadt_xfree(table->alloc, table);
}

size_t cadt_hash_size(const cadt_hash *table)
{
    return table == nullptr ? 0 : table->size;
}

size_t cadt_hash_buckets(const cadt_hash *table)
{
    return table == nullptr ? 0 : table->nbuckets;
}

bool cadt_hash_is_multi(const cadt_hash *table)
{
    return table != nullptr && table->multi;
}

cadt_status cadt_hash_insert(cadt_hash *table, const void *elem)
{
    uint64_t hash;
    size_t index;
    size_t bytes;
    cadt_hnode *node;
    cadt_status status;

    if (table == nullptr || elem == nullptr) {
        return CADT_ERR_INVALID;
    }
    hash = hash_elem(table, elem);
    index = (size_t)(hash & (uint64_t)(table->nbuckets - 1u));
    if (!table->multi && find_in_bucket(table, index, key_ptr(table, elem), hash) != nullptr) {
        return CADT_ERR_EXISTS;
    }
    if (table->size + 1u > table->nbuckets - table->nbuckets / 4u) {
        if (table->nbuckets > SIZE_MAX / 2u) {
            return CADT_ERR_OVERFLOW;
        }
        status = rehash(table, table->nbuckets * 2u);
        if (status != CADT_OK) {
            return status;
        }
        index = (size_t)(hash & (uint64_t)(table->nbuckets - 1u));
    }
    if (!cadt_add_size(offsetof(cadt_hnode, data), table->elem.size, &bytes)) {
        return CADT_ERR_OVERFLOW;
    }
    node = (cadt_hnode *)cadt_xalloc(table->alloc, bytes);
    if (node == nullptr) {
        return CADT_ERR_NOMEM;
    }
    status = cadt_type_copy(&table->elem, node->data, elem);
    if (status != CADT_OK) {
        cadt_xfree(table->alloc, node);
        return status;
    }
    node->hash = hash;
    node->next = table->buckets[index];
    table->buckets[index] = node;
    table->size++;
    return CADT_OK;
}

size_t cadt_hash_erase(cadt_hash *table, const void *key)
{
    uint64_t hash;
    size_t index;
    cadt_hnode *node;
    cadt_hnode *prev = nullptr;
    size_t removed = 0;

    if (table == nullptr || key == nullptr) {
        return 0;
    }
    hash = hash_key(table, key);
    index = (size_t)(hash & (uint64_t)(table->nbuckets - 1u));
    node = table->buckets[index];
    while (node != nullptr) {
        cadt_hnode *next = node->next;

        if (node->hash == hash && cmp_key(table, key, node->data) == 0) {
            if (prev == nullptr) {
                table->buckets[index] = next;
            } else {
                prev->next = next;
            }
            free_node(table, node);
            if (table->size > 0) {
                table->size--;
            }
            removed++;
            node = next;
            if (!table->multi) {
                break;
            }
            continue;
        }
        prev = node;
        node = next;
    }
    return removed;
}

void *cadt_hash_find(const cadt_hash *table, const void *key)
{
    uint64_t hash;
    size_t index;
    cadt_hnode *node;

    if (table == nullptr || key == nullptr) {
        return nullptr;
    }
    hash = hash_key(table, key);
    index = (size_t)(hash & (uint64_t)(table->nbuckets - 1u));
    node = find_in_bucket(table, index, key, hash);
    return node == nullptr ? nullptr : node->data;
}

size_t cadt_hash_count(const cadt_hash *table, const void *key)
{
    uint64_t hash;
    size_t index;
    cadt_hnode *node;
    size_t count = 0;

    if (table == nullptr || key == nullptr) {
        return 0;
    }
    hash = hash_key(table, key);
    index = (size_t)(hash & (uint64_t)(table->nbuckets - 1u));
    node = table->buckets[index];
    while (node != nullptr) {
        if (node->hash == hash && cmp_key(table, key, node->data) == 0) {
            count++;
        }
        node = node->next;
    }
    return count;
}

static cadt_hash_iter iter_end(const cadt_hash *table)
{
    cadt_hash_iter it;

    it.table = (void *)table;
    it.bucket = table == nullptr ? 0 : table->nbuckets;
    it.node = nullptr;
    return it;
}

cadt_hash_iter cadt_hash_begin(const cadt_hash *table)
{
    size_t i;
    cadt_hash_iter it;

    if (table == nullptr) {
        return iter_end(nullptr);
    }
    for (i = 0; i < table->nbuckets; i++) {
        if (table->buckets[i] != nullptr) {
            it.table = (void *)table;
            it.bucket = i;
            it.node = table->buckets[i];
            return it;
        }
    }
    return iter_end(table);
}

cadt_hash_iter cadt_hash_end(const cadt_hash *table)
{
    return iter_end(table);
}

cadt_hash_iter cadt_hash_next(cadt_hash_iter it)
{
    const cadt_hash *table = (const cadt_hash *)it.table;
    cadt_hnode *node = (cadt_hnode *)it.node;
    size_t i;

    if (table == nullptr || node == nullptr) {
        return it;
    }
    if (node->next != nullptr) {
        it.node = node->next;
        return it;
    }
    for (i = it.bucket + 1u; i < table->nbuckets; i++) {
        if (table->buckets[i] != nullptr) {
            it.bucket = i;
            it.node = table->buckets[i];
            return it;
        }
    }
    return iter_end(table);
}

void *cadt_hash_iter_data(cadt_hash_iter it)
{
    cadt_hnode *node = (cadt_hnode *)it.node;

    return node == nullptr ? nullptr : node->data;
}

cadt_status cadt_hash_validate(const cadt_hash *table)
{
    size_t i;
    size_t seen = 0;

    if (table == nullptr || table->buckets == nullptr || table->nbuckets < 8u ||
        (table->nbuckets & (table->nbuckets - 1u)) != 0) {
        return CADT_ERR_INVALID;
    }
    for (i = 0; i < table->nbuckets; i++) {
        cadt_hnode *node = table->buckets[i];

        while (node != nullptr) {
            size_t expect;
            cadt_hnode *other;

            if (hash_elem(table, node->data) != node->hash) {
                fprintf(stderr, "cadt hash invariant hash\n");
                return CADT_ERR_INVALID;
            }
            expect = (size_t)(node->hash & (uint64_t)(table->nbuckets - 1u));
            if (expect != i) {
                fprintf(stderr, "cadt hash invariant bucket\n");
                return CADT_ERR_INVALID;
            }
            if (!table->multi) {
                for (other = node->next; other != nullptr; other = other->next) {
                    if (other->hash == node->hash &&
                        cmp_key(table, key_ptr(table, node->data), other->data) == 0) {
                        fprintf(stderr, "cadt hash invariant duplicate\n");
                        return CADT_ERR_INVALID;
                    }
                }
            }
            seen++;
            node = node->next;
        }
    }
    if (seen != table->size) {
        fprintf(stderr, "cadt hash invariant size\n");
        return CADT_ERR_INVALID;
    }
    return CADT_OK;
}
