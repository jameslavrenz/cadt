/* cadt - C Abstract Data Types
 * SPDX-License-Identifier: MIT
 *
 * Ordered map and multimap (std::map, std::multimap).
 * Keys are ordered with key->compare, which is required.
 * get() returns a pointer to the mapped value inside the node.
 */
#ifndef CADT_MAP_H
#define CADT_MAP_H

#include "cadt/common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cadt_map cadt_map;

typedef struct cadt_map_iter {
    void *node;
    const void *owner;
} cadt_map_iter;

cadt_map *cadt_map_create(const cadt_type *key, const cadt_type *value,
                          const cadt_allocator *alloc);
cadt_map *cadt_multimap_create(const cadt_type *key, const cadt_type *value,
                               const cadt_allocator *alloc);
void cadt_map_destroy(cadt_map *map);

size_t cadt_map_size(const cadt_map *map);
bool cadt_map_empty(const cadt_map *map);
bool cadt_map_is_multi(const cadt_map *map);
void cadt_map_clear(cadt_map *map);

cadt_status cadt_map_insert(cadt_map *map, const void *key, const void *value);
/* Insert or overwrite the value of an equivalent key. Unique maps only. */
cadt_status cadt_map_put(cadt_map *map, const void *key, const void *value);
size_t cadt_map_erase(cadt_map *map, const void *key);

bool cadt_map_contains(const cadt_map *map, const void *key);
size_t cadt_map_count(const cadt_map *map, const void *key);
/* NULL when the key is absent. */
void *cadt_map_get(const cadt_map *map, const void *key);

cadt_map_iter cadt_map_begin(const cadt_map *map);
cadt_map_iter cadt_map_end(const cadt_map *map);
cadt_map_iter cadt_map_next(cadt_map_iter it);
cadt_map_iter cadt_map_prev(cadt_map_iter it);
bool cadt_map_iter_eq(cadt_map_iter a, cadt_map_iter b);
void *cadt_map_iter_key(const cadt_map *map, cadt_map_iter it);
void *cadt_map_iter_value(const cadt_map *map, cadt_map_iter it);

cadt_map_iter cadt_map_lower_bound(const cadt_map *map, const void *key);
cadt_map_iter cadt_map_upper_bound(const cadt_map *map, const void *key);

cadt_status cadt_map_validate(const cadt_map *map);

#ifdef __cplusplus
}
#endif

#endif /* CADT_MAP_H */
