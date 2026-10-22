/* cadt - C Abstract Data Types
 * SPDX-License-Identifier: MIT
 *
 * Hash set and multiset (std::unordered_set, std::unordered_multiset).
 * Separate chaining. A nullptr hash function hashes the raw key bytes.
 * A nullptr compare treats equal bytes as equal keys.
 * Rehash invalidates iterators and element pointers.
 */
#ifndef CADT_UNORDERED_SET_H
#define CADT_UNORDERED_SET_H

#include "cadt/common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cadt_uset cadt_uset;

typedef struct cadt_uset_iter {
    void *table;
    size_t bucket;
    void *node;
} cadt_uset_iter;

[[nodiscard]] cadt_uset *cadt_uset_create(const cadt_type *key, const cadt_allocator *alloc);
[[nodiscard]] cadt_uset *cadt_umultiset_create(const cadt_type *key, const cadt_allocator *alloc);
void cadt_uset_destroy(cadt_uset *set);

size_t cadt_uset_size(const cadt_uset *set);
size_t cadt_uset_bucket_count(const cadt_uset *set);
bool cadt_uset_empty(const cadt_uset *set);
bool cadt_uset_is_multi(const cadt_uset *set);
void cadt_uset_clear(cadt_uset *set);

[[nodiscard]] cadt_status cadt_uset_insert(cadt_uset *set, const void *key);
size_t cadt_uset_erase(cadt_uset *set, const void *key);
bool cadt_uset_contains(const cadt_uset *set, const void *key);
size_t cadt_uset_count(const cadt_uset *set, const void *key);
void *cadt_uset_find(const cadt_uset *set, const void *key);

cadt_uset_iter cadt_uset_begin(const cadt_uset *set);
cadt_uset_iter cadt_uset_end(const cadt_uset *set);
cadt_uset_iter cadt_uset_next(cadt_uset_iter it);
bool cadt_uset_iter_eq(cadt_uset_iter a, cadt_uset_iter b);
void *cadt_uset_iter_data(cadt_uset_iter it);

[[nodiscard]] cadt_status cadt_uset_validate(const cadt_uset *set);

#ifdef __cplusplus
}
#endif

#endif /* CADT_UNORDERED_SET_H */
