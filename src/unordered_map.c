/* cadt - C Abstract Data Types
 * SPDX-License-Identifier: MIT
 */
#include "cadt/unordered_map.h"

#include "hashtable.h"
#include "kv.h"

struct cadt_umap {
    cadt_hash *table;
    cadt_kv kv;
    const cadt_allocator *alloc;
};

static cadt_umap *create_map(const cadt_type *key, const cadt_type *value, bool multi,
                             const cadt_allocator *alloc)
{
    cadt_umap *map;
    cadt_hash_cfg cfg;
    cadt_type elem;

    alloc = cadt_alloc_or_default(alloc);
    map = (cadt_umap *)cadt_xalloc(alloc, sizeof(*map));
    if (map == NULL) {
        return NULL;
    }
    map->alloc = alloc;
    map->table = NULL;
    if (!cadt_kv_init(&map->kv, key, value)) {
        cadt_xfree(alloc, map);
        return NULL;
    }
    memset(&elem, 0, sizeof(elem));
    elem.size = map->kv.elem_size;
    elem.destroy = cadt_kv_elem_destroy;
    elem.ctx = &map->kv;
    memset(&cfg, 0, sizeof(cfg));
    cfg.elem = elem;
    cfg.key_offset = 0;
    cfg.key_size = key->size;
    cfg.key_cmp = key->compare;
    cfg.key_hash = key->hash;
    cfg.key_ctx = key->ctx;
    cfg.multi = multi;
    map->table = cadt_hash_create(&cfg, alloc);
    if (map->table == NULL) {
        cadt_xfree(alloc, map);
        return NULL;
    }
    return map;
}

static cadt_umap_iter from_hash(cadt_hash_iter it)
{
    cadt_umap_iter out;

    out.table = it.table;
    out.bucket = it.bucket;
    out.node = it.node;
    return out;
}

static cadt_hash_iter to_hash(cadt_umap_iter it)
{
    cadt_hash_iter out;

    out.table = it.table;
    out.bucket = it.bucket;
    out.node = it.node;
    return out;
}

cadt_umap *cadt_umap_create(const cadt_type *key, const cadt_type *value,
                            const cadt_allocator *alloc)
{
    return create_map(key, value, false, alloc);
}

cadt_umap *cadt_umultimap_create(const cadt_type *key, const cadt_type *value,
                                 const cadt_allocator *alloc)
{
    return create_map(key, value, true, alloc);
}

void cadt_umap_destroy(cadt_umap *map)
{
    if (map == NULL) {
        return;
    }
    cadt_hash_destroy(map->table);
    cadt_xfree(map->alloc, map);
}

size_t cadt_umap_size(const cadt_umap *map)
{
    return map == NULL ? 0 : cadt_hash_size(map->table);
}

size_t cadt_umap_bucket_count(const cadt_umap *map)
{
    return map == NULL ? 0 : cadt_hash_buckets(map->table);
}

bool cadt_umap_empty(const cadt_umap *map)
{
    return cadt_umap_size(map) == 0;
}

bool cadt_umap_is_multi(const cadt_umap *map)
{
    return map != NULL && cadt_hash_is_multi(map->table);
}

void cadt_umap_clear(cadt_umap *map)
{
    if (map != NULL) {
        cadt_hash_clear(map->table);
    }
}

cadt_status cadt_umap_insert(cadt_umap *map, const void *key, const void *value)
{
    unsigned char *packed;
    cadt_status status;

    if (map == NULL || key == NULL || value == NULL) {
        return CADT_ERR_INVALID;
    }
    status = cadt_kv_pack(map->alloc, &map->kv, key, value, &packed);
    if (status != CADT_OK) {
        return status;
    }
    status = cadt_hash_insert(map->table, packed);
    if (status != CADT_OK) {
        cadt_kv_pack_destroy(map->alloc, &map->kv, packed);
        return status;
    }
    cadt_xfree(map->alloc, packed);
    return CADT_OK;
}

cadt_status cadt_umap_put(cadt_umap *map, const void *key, const void *value)
{
    void *existing;
    unsigned char *slot;
    unsigned char *tmp;
    cadt_status status;

    if (map == NULL || key == NULL || value == NULL) {
        return CADT_ERR_INVALID;
    }
    if (cadt_hash_is_multi(map->table)) {
        return CADT_ERR_INVALID;
    }
    existing = cadt_hash_find(map->table, key);
    if (existing == NULL) {
        return cadt_umap_insert(map, key, value);
    }
    tmp = (unsigned char *)cadt_xalloc(map->alloc, map->kv.value.size);
    if (tmp == NULL) {
        return CADT_ERR_NOMEM;
    }
    status = cadt_type_copy(&map->kv.value, tmp, value);
    if (status != CADT_OK) {
        cadt_xfree(map->alloc, tmp);
        return status;
    }
    slot = (unsigned char *)cadt_kv_value(&map->kv, existing);
    cadt_type_destroy(&map->kv.value, slot);
    memcpy(slot, tmp, map->kv.value.size);
    cadt_xfree(map->alloc, tmp);
    return CADT_OK;
}

size_t cadt_umap_erase(cadt_umap *map, const void *key)
{
    return map == NULL ? 0 : cadt_hash_erase(map->table, key);
}

bool cadt_umap_contains(const cadt_umap *map, const void *key)
{
    return cadt_umap_get(map, key) != NULL;
}

size_t cadt_umap_count(const cadt_umap *map, const void *key)
{
    return map == NULL ? 0 : cadt_hash_count(map->table, key);
}

void *cadt_umap_get(const cadt_umap *map, const void *key)
{
    void *elem;

    if (map == NULL) {
        return NULL;
    }
    elem = cadt_hash_find(map->table, key);
    return cadt_kv_value(&map->kv, elem);
}

cadt_umap_iter cadt_umap_begin(const cadt_umap *map)
{
    if (map == NULL) {
        return from_hash(cadt_hash_begin(NULL));
    }
    return from_hash(cadt_hash_begin(map->table));
}

cadt_umap_iter cadt_umap_end(const cadt_umap *map)
{
    if (map == NULL) {
        return from_hash(cadt_hash_end(NULL));
    }
    return from_hash(cadt_hash_end(map->table));
}

cadt_umap_iter cadt_umap_next(cadt_umap_iter it)
{
    return from_hash(cadt_hash_next(to_hash(it)));
}

bool cadt_umap_iter_eq(cadt_umap_iter a, cadt_umap_iter b)
{
    if (a.node == NULL && b.node == NULL) {
        return true;
    }
    return a.node == b.node;
}

void *cadt_umap_iter_key(const cadt_umap *map, cadt_umap_iter it)
{
    (void)map;
    return cadt_hash_iter_data(to_hash(it));
}

void *cadt_umap_iter_value(const cadt_umap *map, cadt_umap_iter it)
{
    void *elem;

    if (map == NULL) {
        return NULL;
    }
    elem = cadt_hash_iter_data(to_hash(it));
    return cadt_kv_value(&map->kv, elem);
}

cadt_status cadt_umap_validate(const cadt_umap *map)
{
    if (map == NULL) {
        return CADT_ERR_INVALID;
    }
    return cadt_hash_validate(map->table);
}
