/* cadt - C Abstract Data Types
 * SPDX-License-Identifier: MIT
 *
 * Non-owning view of a contiguous run of elements (std::span).
 */
#ifndef CADT_SPAN_H
#define CADT_SPAN_H

#include "cadt/common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cadt_span {
    void *data;
    size_t size;
    size_t elem_size;
} cadt_span;

/* data may be nullptr when size is 0. elem_size must be non-zero. */
cadt_span cadt_span_make(void *data, size_t size, size_t elem_size);

/* nullptr when index is out of range or the span is empty. */
void *cadt_span_at(cadt_span span, size_t index);

/*
 * Subview starting at offset, covering count elements.
 * A count of CADT_NPOS means "through the end".
 * Returns a zero-size span when the range does not fit.
 */
cadt_span cadt_span_sub(cadt_span span, size_t offset, size_t count);

static inline bool cadt_span_empty(cadt_span span)
{
    return span.size == 0;
}

#ifdef __cplusplus
}
#endif

#endif /* CADT_SPAN_H */
