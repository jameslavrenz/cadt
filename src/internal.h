/* cadt - C Abstract Data Types
 * SPDX-License-Identifier: MIT
 *
 * Private helpers shared by the implementation files. Not part of the
 * installed API.
 */
#ifndef CADT_INTERNAL_H
#define CADT_INTERNAL_H

#include "cadt/common.h"

#include <stdlib.h>
#include <string.h>

void *cadt_xalloc(const cadt_allocator *alloc, size_t size);
void cadt_xfree(const cadt_allocator *alloc, void *ptr);
void *cadt_xrealloc(const cadt_allocator *alloc, void *ptr, size_t old_size,
                    size_t new_size);

const cadt_allocator *cadt_alloc_or_default(const cadt_allocator *alloc);

bool cadt_mul_size(size_t a, size_t b, size_t *out);
bool cadt_add_size(size_t a, size_t b, size_t *out);

cadt_status cadt_type_check(const cadt_type *type);
cadt_status cadt_type_copy(const cadt_type *type, void *dst, const void *src);
void cadt_type_destroy(const cadt_type *type, void *elem);
int cadt_type_compare(const cadt_type *type, const void *a, const void *b);
uint64_t cadt_type_hash(const cadt_type *type, const void *elem);

static inline size_t cadt_align_up(size_t value, size_t align)
{
    return (value + align - 1u) & ~(align - 1u);
}

#endif /* CADT_INTERNAL_H */
