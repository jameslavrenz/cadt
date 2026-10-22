/* cadt - C Abstract Data Types
 * SPDX-License-Identifier: MIT
 *
 * Hash map and multimap (std::unordered_map, std::unordered_multimap).
 */
#ifndef CADT_UNORDERED_MAP_H
#define CADT_UNORDERED_MAP_H

#include "cadt/common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cadt_umap cadt_umap;

typedef struct cadt_umap_iter {
    void *table;
    size_t bucket;
    void *node;
} cadt_umap_iter;

cadt_umap *cadt_umap_create(const cadt_type *key, const cadt_type *value,
                            const cadt_allocator *alloc);
cadt_umap *cadt_umultimap_create(const cadt_type *key, const cadt_type *value,
                                 const cadt_allocator *alloc);
void cadt_umap_destroy(cadt_umap *map);

size_t cadt_umap_size(const cadt_umap *map);
size_t cadt_umap_bucket_count(const cadt_umap *map);
bool cadt_umap_empty(const cadt_umap *map);
bool cadt_umap_is_multi(const cadt_umap *map);
void cadt_umap_clear(cadt_umap *map);

cadt_status cadt_umap_insert(cadt_umap *map, const void *key, const void *value);
cadt_status cadt_umap_put(cadt_umap *map, const void *key, const void *value);
size_t cadt_umap_erase(cadt_umap *map, const void *key);

bool cadt_umap_contains(const cadt_umap *map, const void *key);
size_t cadt_umap_count(const cadt_umap *map, const void *key);
void *cadt_umap_get(const cadt_umap *map, const void *key);

cadt_umap_iter cadt_umap_begin(const cadt_umap *map);
cadt_umap_iter cadt_umap_end(const cadt_umap *map);
cadt_umap_iter cadt_umap_next(cadt_umap_iter it);
bool cadt_umap_iter_eq(cadt_umap_iter a, cadt_umap_iter b);
void *cadt_umap_iter_key(const cadt_umap *map, cadt_umap_iter it);
void *cadt_umap_iter_value(const cadt_umap *map, cadt_umap_iter it);

cadt_status cadt_umap_validate(const cadt_umap *map);

#ifdef __cplusplus
}
#endif

#endif /* CADT_UNORDERED_MAP_H */
