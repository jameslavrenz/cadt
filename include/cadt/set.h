/* cadt - C Abstract Data Types
 * SPDX-License-Identifier: MIT
 *
 * Ordered unique set and multiset (std::set, std::multiset).
 * Backed by a red-black tree. compare is required.
 * Iterators point at element storage inside nodes and stay valid across
 * insert and across erase of other elements.
 */
#ifndef CADT_SET_H
#define CADT_SET_H

#include "cadt/common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cadt_set cadt_set;

typedef struct cadt_set_iter {
    void *node;
    const void *owner;
} cadt_set_iter;

cadt_set *cadt_set_create(const cadt_type *key, const cadt_allocator *alloc);
cadt_set *cadt_multiset_create(const cadt_type *key, const cadt_allocator *alloc);
void cadt_set_destroy(cadt_set *set);

size_t cadt_set_size(const cadt_set *set);
bool cadt_set_empty(const cadt_set *set);
bool cadt_set_is_multi(const cadt_set *set);
void cadt_set_clear(cadt_set *set);

/* CADT_ERR_EXISTS when a unique set already contains an equivalent key. */
cadt_status cadt_set_insert(cadt_set *set, const void *key);
/* Number of elements removed (all equivalents). */
size_t cadt_set_erase(cadt_set *set, const void *key);
/* Removes one equivalent element. out may be NULL. */
cadt_status cadt_set_erase_one(cadt_set *set, const void *key, void *out);

bool cadt_set_contains(const cadt_set *set, const void *key);
size_t cadt_set_count(const cadt_set *set, const void *key);
void *cadt_set_find(const cadt_set *set, const void *key);

cadt_set_iter cadt_set_begin(const cadt_set *set);
cadt_set_iter cadt_set_end(const cadt_set *set);
cadt_set_iter cadt_set_next(cadt_set_iter it);
cadt_set_iter cadt_set_prev(cadt_set_iter it);
bool cadt_set_iter_eq(cadt_set_iter a, cadt_set_iter b);
void *cadt_set_iter_data(cadt_set_iter it);

/* First element not less than key, or end. */
cadt_set_iter cadt_set_lower_bound(const cadt_set *set, const void *key);
/* First element greater than key, or end. */
cadt_set_iter cadt_set_upper_bound(const cadt_set *set, const void *key);

void *cadt_set_min(const cadt_set *set);
void *cadt_set_max(const cadt_set *set);

/* CADT_OK when red-black and ordering invariants hold. */
cadt_status cadt_set_validate(const cadt_set *set);

#ifdef __cplusplus
}
#endif

#endif /* CADT_SET_H */
