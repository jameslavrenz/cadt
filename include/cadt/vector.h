/* cadt - C Abstract Data Types
 * SPDX-License-Identifier: MIT
 *
 * Contiguous growable array (std::vector).
 * Growth invalidates every outstanding element pointer.
 */
#ifndef CADT_VECTOR_H
#define CADT_VECTOR_H

#include "cadt/common.h"
#include "cadt/span.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cadt_vector cadt_vector;

[[nodiscard]] cadt_vector *cadt_vector_create(const cadt_type *type, const cadt_allocator *alloc);
void cadt_vector_destroy(cadt_vector *vec);

size_t cadt_vector_size(const cadt_vector *vec);
size_t cadt_vector_capacity(const cadt_vector *vec);
size_t cadt_vector_element_size(const cadt_vector *vec);
bool cadt_vector_empty(const cadt_vector *vec);

[[nodiscard]] cadt_status cadt_vector_reserve(cadt_vector *vec, size_t capacity);
[[nodiscard]] cadt_status cadt_vector_shrink_to_fit(cadt_vector *vec);

/*
 * Grow by copy-constructing fill, or shrink by destroying the tail.
 * fill nullptr zero-fills and is rejected when the type has a destructor.
 */
[[nodiscard]] cadt_status cadt_vector_resize(cadt_vector *vec, size_t new_size, const void *fill);
void cadt_vector_clear(cadt_vector *vec);

/* nullptr when index is out of range. */
void *cadt_vector_at(const cadt_vector *vec, size_t index);
void *cadt_vector_front(const cadt_vector *vec);
void *cadt_vector_back(const cadt_vector *vec);

/* Contiguous storage. nullptr when capacity is 0. */
void *cadt_vector_data(const cadt_vector *vec);
cadt_span cadt_vector_as_span(const cadt_vector *vec);

[[nodiscard]] cadt_status cadt_vector_push_back(cadt_vector *vec, const void *elem);
/* out may be nullptr to destroy the element instead of moving it out. */
[[nodiscard]] cadt_status cadt_vector_pop_back(cadt_vector *vec, void *out);
[[nodiscard]] cadt_status cadt_vector_set(cadt_vector *vec, size_t index, const void *elem);
[[nodiscard]] cadt_status cadt_vector_insert(cadt_vector *vec, size_t index, const void *elem);
[[nodiscard]] cadt_status cadt_vector_erase(cadt_vector *vec, size_t index, void *out);
[[nodiscard]] cadt_status cadt_vector_erase_range(cadt_vector *vec, size_t index, size_t count);

/* First element comparing equal, or nullptr. */
void *cadt_vector_find(const cadt_vector *vec, const void *elem);

#ifdef __cplusplus
}
#endif

#endif /* CADT_VECTOR_H */
