/* cadt - C Abstract Data Types
 * SPDX-License-Identifier: MIT
 *
 * Fixed-size array (std::array). The length is chosen at creation and
 * never changes. Every slot is a live element until the array is destroyed.
 */
#ifndef CADT_ARRAY_H
#define CADT_ARRAY_H

#include "cadt/common.h"
#include "cadt/span.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cadt_array cadt_array;

/*
 * count elements are copy-constructed from fill.
 * fill nullptr zero-fills and is rejected when the type has a destructor.
 * count 0 yields an empty array.
 */
[[nodiscard]] cadt_array *cadt_array_create(const cadt_type *type, size_t count, const void *fill,
                              const cadt_allocator *alloc);
void cadt_array_destroy(cadt_array *arr);

size_t cadt_array_size(const cadt_array *arr);
bool cadt_array_empty(const cadt_array *arr);
void *cadt_array_at(const cadt_array *arr, size_t index);
void *cadt_array_data(const cadt_array *arr);
void *cadt_array_front(const cadt_array *arr);
void *cadt_array_back(const cadt_array *arr);
cadt_span cadt_array_as_span(const cadt_array *arr);
[[nodiscard]] cadt_status cadt_array_set(cadt_array *arr, size_t index, const void *elem);

#ifdef __cplusplus
}
#endif

#endif /* CADT_ARRAY_H */
