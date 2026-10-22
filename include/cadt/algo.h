/* cadt - C Abstract Data Types
 * SPDX-License-Identifier: MIT
 *
 * Algorithms over contiguous trivially copyable elements.
 * These routines move bytes with memcpy and never call element destructors.
 * A nullptr compare orders raw bytes with memcmp.
 */
#ifndef CADT_ALGO_H
#define CADT_ALGO_H

#include "cadt/common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef bool (*cadt_pred_fn)(const void *elem, void *ctx);
typedef void (*cadt_for_each_fn)(void *elem, void *ctx);

[[nodiscard]] cadt_status cadt_sort(void *base, size_t count, size_t elem_size,
                      cadt_compare_fn cmp, void *ctx);
[[nodiscard]] cadt_status cadt_stable_sort(void *base, size_t count, size_t elem_size,
                             cadt_compare_fn cmp, void *ctx);
bool cadt_is_sorted(const void *base, size_t count, size_t elem_size,
                    cadt_compare_fn cmp, void *ctx);

/* Index of the first element not ordered before key. count means "all are less". */
size_t cadt_lower_bound(const void *base, size_t count, size_t elem_size,
                        const void *key, cadt_compare_fn cmp, void *ctx);
size_t cadt_upper_bound(const void *base, size_t count, size_t elem_size,
                        const void *key, cadt_compare_fn cmp, void *ctx);
bool cadt_binary_search(const void *base, size_t count, size_t elem_size,
                        const void *key, cadt_compare_fn cmp, void *ctx);

void *cadt_find(const void *base, size_t count, size_t elem_size,
                const void *key, cadt_compare_fn cmp, void *ctx);
void *cadt_find_if(const void *base, size_t count, size_t elem_size,
                   cadt_pred_fn pred, void *ctx);
size_t cadt_count(const void *base, size_t count, size_t elem_size,
                  const void *key, cadt_compare_fn cmp, void *ctx);
size_t cadt_count_if(const void *base, size_t count, size_t elem_size,
                     cadt_pred_fn pred, void *ctx);

void cadt_for_each(void *base, size_t count, size_t elem_size,
                   cadt_for_each_fn fn, void *ctx);
void cadt_reverse(void *base, size_t count, size_t elem_size);
void cadt_fill(void *base, size_t count, size_t elem_size, const void *value);

void *cadt_min_element(const void *base, size_t count, size_t elem_size,
                       cadt_compare_fn cmp, void *ctx);
void *cadt_max_element(const void *base, size_t count, size_t elem_size,
                       cadt_compare_fn cmp, void *ctx);

bool cadt_equal(const void *a, const void *b, size_t count, size_t elem_size,
                cadt_compare_fn cmp, void *ctx);

#ifdef __cplusplus
}
#endif

#endif /* CADT_ALGO_H */
