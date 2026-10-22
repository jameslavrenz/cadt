/* cadt - C Abstract Data Types
 * SPDX-License-Identifier: MIT
 */
#include "cadt/map.h"

#include "kv.h"
#include "rbtree.h"

struct cadt_map {
    cadt_tree *tree;
    cadt_kv kv;
    const cadt_allocator *alloc;
};

static cadt_map *create_map(const cadt_type *key, const cadt_type *value, bool multi,
                            const cadt_allocator *alloc)
{
    cadt_map *map;
    cadt_tree_cfg cfg;
    cadt_type elem;

    if (key == nullptr || key->compare == nullptr) {
        return nullptr;
    }
    alloc = cadt_alloc_or_default(alloc);
    map = (cadt_map *)cadt_xalloc(alloc, sizeof(*map));
    if (map == nullptr) {
        return nullptr;
    }
    map->alloc = alloc;
    map->tree = nullptr;
    if (!cadt_kv_init(&map->kv, key, value)) {
        cadt_xfree(alloc, map);
        return nullptr;
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
    cfg.key_ctx = key->ctx;
    cfg.multi = multi;
    map->tree = cadt_tree_create(&cfg, alloc);
    if (map->tree == nullptr) {
        cadt_xfree(alloc, map);
        return nullptr;
    }
    return map;
}

static cadt_map_iter iter_of(const cadt_map *map, void *node)
{
    cadt_map_iter it;

    it.node = node;
    it.owner = map;
    return it;
}

cadt_map *cadt_map_create(const cadt_type *key, const cadt_type *value,
                          const cadt_allocator *alloc)
{
    return create_map(key, value, false, alloc);
}

cadt_map *cadt_multimap_create(const cadt_type *key, const cadt_type *value,
                               const cadt_allocator *alloc)
{
    return create_map(key, value, true, alloc);
}

void cadt_map_destroy(cadt_map *map)
{
    if (map == nullptr) {
        return;
    }
    cadt_tree_destroy(map->tree);
    cadt_xfree(map->alloc, map);
}

size_t cadt_map_size(const cadt_map *map)
{
    return map == nullptr ? 0 : cadt_tree_size(map->tree);
}

bool cadt_map_empty(const cadt_map *map)
{
    return cadt_map_size(map) == 0;
}

bool cadt_map_is_multi(const cadt_map *map)
{
    return map != nullptr && cadt_tree_is_multi(map->tree);
}

void cadt_map_clear(cadt_map *map)
{
    if (map != nullptr) {
        cadt_tree_clear(map->tree);
    }
}

cadt_status cadt_map_insert(cadt_map *map, const void *key, const void *value)
{
    unsigned char *packed;
    cadt_status status;

    if (map == nullptr || key == nullptr || value == nullptr) {
        return CADT_ERR_INVALID;
    }
    status = cadt_kv_pack(map->alloc, &map->kv, key, value, &packed);
    if (status != CADT_OK) {
        return status;
    }
    status = cadt_tree_insert(map->tree, packed);
    if (status != CADT_OK) {
        cadt_kv_pack_destroy(map->alloc, &map->kv, packed);
        return status;
    }
    cadt_xfree(map->alloc, packed);
    return CADT_OK;
}

cadt_status cadt_map_put(cadt_map *map, const void *key, const void *value)
{
    void *existing;
    unsigned char *slot;
    unsigned char *tmp;
    cadt_status status;

    if (map == nullptr || key == nullptr || value == nullptr) {
        return CADT_ERR_INVALID;
    }
    if (cadt_tree_is_multi(map->tree)) {
        return CADT_ERR_INVALID;
    }
    existing = cadt_tree_find(map->tree, key);
    if (existing == nullptr) {
        return cadt_map_insert(map, key, value);
    }
    tmp = (unsigned char *)cadt_xalloc(map->alloc, map->kv.value.size);
    if (tmp == nullptr) {
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

size_t cadt_map_erase(cadt_map *map, const void *key)
{
    return map == nullptr ? 0 : cadt_tree_erase(map->tree, key);
}

bool cadt_map_contains(const cadt_map *map, const void *key)
{
    return cadt_map_get(map, key) != nullptr;
}

size_t cadt_map_count(const cadt_map *map, const void *key)
{
    return map == nullptr ? 0 : cadt_tree_count(map->tree, key);
}

void *cadt_map_get(const cadt_map *map, const void *key)
{
    void *elem;

    if (map == nullptr) {
        return nullptr;
    }
    elem = cadt_tree_find(map->tree, key);
    return cadt_kv_value(&map->kv, elem);
}

cadt_map_iter cadt_map_begin(const cadt_map *map)
{
    if (map == nullptr) {
        return iter_of(nullptr, nullptr);
    }
    return iter_of(map, cadt_tree_minimum(map->tree));
}

cadt_map_iter cadt_map_end(const cadt_map *map)
{
    return iter_of(map, nullptr);
}

cadt_map_iter cadt_map_next(cadt_map_iter it)
{
    if (it.node != nullptr) {
        it.node = cadt_tree_next(it.node);
    }
    return it;
}

cadt_map_iter cadt_map_prev(cadt_map_iter it)
{
    const cadt_map *map = (const cadt_map *)it.owner;

    if (map == nullptr) {
        return it;
    }
    if (it.node == nullptr) {
        it.node = cadt_tree_maximum(map->tree);
        return it;
    }
    it.node = cadt_tree_prev(it.node);
    return it;
}

bool cadt_map_iter_eq(cadt_map_iter a, cadt_map_iter b)
{
    return a.owner == b.owner && a.node == b.node;
}

void *cadt_map_iter_key([[maybe_unused]] const cadt_map *map, cadt_map_iter it)
{
    return it.node;
}

void *cadt_map_iter_value(const cadt_map *map, cadt_map_iter it)
{
    if (map == nullptr) {
        return nullptr;
    }
    return cadt_kv_value(&map->kv, it.node);
}

cadt_map_iter cadt_map_lower_bound(const cadt_map *map, const void *key)
{
    if (map == nullptr) {
        return iter_of(nullptr, nullptr);
    }
    return iter_of(map, cadt_tree_lower(map->tree, key));
}

cadt_map_iter cadt_map_upper_bound(const cadt_map *map, const void *key)
{
    if (map == nullptr) {
        return iter_of(nullptr, nullptr);
    }
    return iter_of(map, cadt_tree_upper(map->tree, key));
}

cadt_status cadt_map_validate(const cadt_map *map)
{
    if (map == nullptr) {
        return CADT_ERR_INVALID;
    }
    return cadt_tree_validate(map->tree);
}
