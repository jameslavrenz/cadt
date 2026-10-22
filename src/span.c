/* cadt - C Abstract Data Types
 * SPDX-License-Identifier: MIT
 */
#include "cadt/span.h"

#include <stddef.h>

cadt_span cadt_span_make(void *data, size_t size, size_t elem_size)
{
    cadt_span span;

    span.data = size == 0 ? nullptr : data;
    span.size = elem_size == 0 ? 0 : size;
    span.elem_size = elem_size;
    return span;
}

void *cadt_span_at(cadt_span span, size_t index)
{
    if (span.data == nullptr || span.elem_size == 0 || index >= span.size) {
        return nullptr;
    }
    return (unsigned char *)span.data + index * span.elem_size;
}

cadt_span cadt_span_sub(cadt_span span, size_t offset, size_t count)
{
    cadt_span empty;

    empty.data = nullptr;
    empty.size = 0;
    empty.elem_size = span.elem_size;
    if (offset > span.size) {
        return empty;
    }
    if (count == CADT_NPOS || count > span.size - offset) {
        if (count != CADT_NPOS) {
            return empty;
        }
        count = span.size - offset;
    }
    return cadt_span_make(span.data == nullptr
                              ? nullptr
                              : (unsigned char *)span.data + offset * span.elem_size,
                          count, span.elem_size);
}
