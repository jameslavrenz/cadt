/* cadt - C Abstract Data Types
 * SPDX-License-Identifier: MIT
 */
#include "cadt/vector.h"

#include "internal.h"

struct cadt_vector {
    const cadt_allocator *alloc;
    cadt_type type;
    unsigned char *data;
    size_t size;
    size_t capacity;
};

static unsigned char *slot(const cadt_vector *vec, size_t index)
{
    return vec->data + index * vec->type.size;
}

static void destroy_range(cadt_vector *vec, size_t index, size_t count)
{
    size_t i;

    for (i = 0; i < count; i++) {
        cadt_type_destroy(&vec->type, slot(vec, index + i));
    }
}

cadt_vector *cadt_vector_create(const cadt_type *type, const cadt_allocator *alloc)
{
    cadt_vector *vec;

    if (cadt_type_check(type) != CADT_OK) {
        return nullptr;
    }
    alloc = cadt_alloc_or_default(alloc);
    vec = (cadt_vector *)cadt_xalloc(alloc, sizeof(*vec));
    if (vec == nullptr) {
        return nullptr;
    }
    vec->alloc = alloc;
    vec->type = *type;
    vec->data = nullptr;
    vec->size = 0;
    vec->capacity = 0;
    return vec;
}

void cadt_vector_destroy(cadt_vector *vec)
{
    if (vec == nullptr) {
        return;
    }
    cadt_vector_clear(vec);
    cadt_xfree(vec->alloc, vec->data);
    cadt_xfree(vec->alloc, vec);
}

size_t cadt_vector_size(const cadt_vector *vec)
{
    return vec == nullptr ? 0 : vec->size;
}

size_t cadt_vector_capacity(const cadt_vector *vec)
{
    return vec == nullptr ? 0 : vec->capacity;
}

size_t cadt_vector_element_size(const cadt_vector *vec)
{
    return vec == nullptr ? 0 : vec->type.size;
}

bool cadt_vector_empty(const cadt_vector *vec)
{
    return cadt_vector_size(vec) == 0;
}

cadt_status cadt_vector_reserve(cadt_vector *vec, size_t capacity)
{
    size_t old_bytes;
    size_t new_bytes;
    unsigned char *fresh;

    if (vec == nullptr) {
        return CADT_ERR_INVALID;
    }
    if (capacity <= vec->capacity) {
        return CADT_OK;
    }
    if (!cadt_mul_size(vec->capacity, vec->type.size, &old_bytes) ||
        !cadt_mul_size(capacity, vec->type.size, &new_bytes)) {
        return CADT_ERR_OVERFLOW;
    }
    fresh = (unsigned char *)cadt_xrealloc(vec->alloc, vec->data, old_bytes, new_bytes);
    if (fresh == nullptr) {
        return CADT_ERR_NOMEM;
    }
    vec->data = fresh;
    vec->capacity = capacity;
    return CADT_OK;
}

cadt_status cadt_vector_shrink_to_fit(cadt_vector *vec)
{
    size_t old_bytes;
    size_t new_bytes;
    unsigned char *fresh;

    if (vec == nullptr) {
        return CADT_ERR_INVALID;
    }
    if (vec->capacity == vec->size) {
        return CADT_OK;
    }
    if (vec->size == 0) {
        cadt_xfree(vec->alloc, vec->data);
        vec->data = nullptr;
        vec->capacity = 0;
        return CADT_OK;
    }
    if (!cadt_mul_size(vec->capacity, vec->type.size, &old_bytes) ||
        !cadt_mul_size(vec->size, vec->type.size, &new_bytes)) {
        return CADT_ERR_OVERFLOW;
    }
    fresh = (unsigned char *)cadt_xrealloc(vec->alloc, vec->data, old_bytes, new_bytes);
    if (fresh == nullptr) {
        return CADT_ERR_NOMEM;
    }
    vec->data = fresh;
    vec->capacity = vec->size;
    return CADT_OK;
}

static cadt_status grow(cadt_vector *vec, size_t min_cap)
{
    size_t cap;

    if (min_cap <= vec->capacity) {
        return CADT_OK;
    }
    cap = vec->capacity == 0 ? 8u : vec->capacity;
    while (cap < min_cap) {
        if (cap > SIZE_MAX / 2u) {
            cap = min_cap;
            break;
        }
        cap *= 2u;
    }
    return cadt_vector_reserve(vec, cap);
}

cadt_status cadt_vector_resize(cadt_vector *vec, size_t new_size, const void *fill)
{
    cadt_status status;
    size_t i;

    if (vec == nullptr) {
        return CADT_ERR_INVALID;
    }
    if (new_size < vec->size) {
        destroy_range(vec, new_size, vec->size - new_size);
        vec->size = new_size;
        return CADT_OK;
    }
    if (new_size == vec->size) {
        return CADT_OK;
    }
    if (fill == nullptr && vec->type.destroy != nullptr) {
        return CADT_ERR_INVALID;
    }
    status = grow(vec, new_size);
    if (status != CADT_OK) {
        return status;
    }
    for (i = vec->size; i < new_size; i++) {
        if (fill == nullptr) {
            memset(slot(vec, i), 0, vec->type.size);
        } else {
            status = cadt_type_copy(&vec->type, slot(vec, i), fill);
            if (status != CADT_OK) {
                vec->size = i;
                return status;
            }
        }
    }
    vec->size = new_size;
    return CADT_OK;
}

void cadt_vector_clear(cadt_vector *vec)
{
    if (vec == nullptr) {
        return;
    }
    destroy_range(vec, 0, vec->size);
    vec->size = 0;
}

void *cadt_vector_at(const cadt_vector *vec, size_t index)
{
    if (vec == nullptr || index >= vec->size) {
        return nullptr;
    }
    return slot(vec, index);
}

void *cadt_vector_front(const cadt_vector *vec)
{
    return cadt_vector_at(vec, 0);
}

void *cadt_vector_back(const cadt_vector *vec)
{
    if (vec == nullptr || vec->size == 0) {
        return nullptr;
    }
    return slot(vec, vec->size - 1u);
}

void *cadt_vector_data(const cadt_vector *vec)
{
    if (vec == nullptr) {
        return nullptr;
    }
    return vec->data;
}

cadt_span cadt_vector_as_span(const cadt_vector *vec)
{
    if (vec == nullptr) {
        return cadt_span_make(nullptr, 0, 0);
    }
    return cadt_span_make(vec->data, vec->size, vec->type.size);
}

cadt_status cadt_vector_push_back(cadt_vector *vec, const void *elem)
{
    cadt_status status;

    if (vec == nullptr || elem == nullptr) {
        return CADT_ERR_INVALID;
    }
    if (vec->size == SIZE_MAX) {
        return CADT_ERR_OVERFLOW;
    }
    status = grow(vec, vec->size + 1u);
    if (status != CADT_OK) {
        return status;
    }
    status = cadt_type_copy(&vec->type, slot(vec, vec->size), elem);
    if (status != CADT_OK) {
        return status;
    }
    vec->size++;
    return CADT_OK;
}

cadt_status cadt_vector_pop_back(cadt_vector *vec, void *out)
{
    if (vec == nullptr) {
        return CADT_ERR_INVALID;
    }
    if (vec->size == 0) {
        return CADT_ERR_EMPTY;
    }
    vec->size--;
    if (out != nullptr) {
        memcpy(out, slot(vec, vec->size), vec->type.size);
    } else {
        cadt_type_destroy(&vec->type, slot(vec, vec->size));
    }
    return CADT_OK;
}

cadt_status cadt_vector_set(cadt_vector *vec, size_t index, const void *elem)
{
    unsigned char *tmp;
    cadt_status status;

    if (vec == nullptr || elem == nullptr) {
        return CADT_ERR_INVALID;
    }
    if (index >= vec->size) {
        return CADT_ERR_RANGE;
    }
    tmp = (unsigned char *)cadt_xalloc(vec->alloc, vec->type.size);
    if (tmp == nullptr) {
        return CADT_ERR_NOMEM;
    }
    status = cadt_type_copy(&vec->type, tmp, elem);
    if (status != CADT_OK) {
        cadt_xfree(vec->alloc, tmp);
        return status;
    }
    cadt_type_destroy(&vec->type, slot(vec, index));
    memcpy(slot(vec, index), tmp, vec->type.size);
    cadt_xfree(vec->alloc, tmp);
    return CADT_OK;
}

cadt_status cadt_vector_insert(cadt_vector *vec, size_t index, const void *elem)
{
    cadt_status status;
    size_t bytes;

    if (vec == nullptr || elem == nullptr) {
        return CADT_ERR_INVALID;
    }
    if (index > vec->size) {
        return CADT_ERR_RANGE;
    }
    if (vec->size == SIZE_MAX) {
        return CADT_ERR_OVERFLOW;
    }
    status = grow(vec, vec->size + 1u);
    if (status != CADT_OK) {
        return status;
    }
    if (!cadt_mul_size(vec->size - index, vec->type.size, &bytes)) {
        return CADT_ERR_OVERFLOW;
    }
    if (bytes > 0) {
        memmove(slot(vec, index + 1u), slot(vec, index), bytes);
    }
    status = cadt_type_copy(&vec->type, slot(vec, index), elem);
    if (status != CADT_OK) {
        if (bytes > 0) {
            memmove(slot(vec, index), slot(vec, index + 1u), bytes);
        }
        return status;
    }
    vec->size++;
    return CADT_OK;
}

cadt_status cadt_vector_erase(cadt_vector *vec, size_t index, void *out)
{
    size_t bytes;

    if (vec == nullptr) {
        return CADT_ERR_INVALID;
    }
    if (index >= vec->size) {
        return CADT_ERR_RANGE;
    }
    if (!cadt_mul_size(vec->size - index - 1u, vec->type.size, &bytes)) {
        return CADT_ERR_OVERFLOW;
    }
    if (out != nullptr) {
        memcpy(out, slot(vec, index), vec->type.size);
    } else {
        cadt_type_destroy(&vec->type, slot(vec, index));
    }
    if (bytes > 0) {
        memmove(slot(vec, index), slot(vec, index + 1u), bytes);
    }
    vec->size--;
    return CADT_OK;
}

cadt_status cadt_vector_erase_range(cadt_vector *vec, size_t index, size_t count)
{
    size_t bytes;

    if (vec == nullptr) {
        return CADT_ERR_INVALID;
    }
    if (count == 0) {
        return index > vec->size ? CADT_ERR_RANGE : CADT_OK;
    }
    if (index >= vec->size || count > vec->size - index) {
        return CADT_ERR_RANGE;
    }
    if (!cadt_mul_size(vec->size - index - count, vec->type.size, &bytes)) {
        return CADT_ERR_OVERFLOW;
    }
    destroy_range(vec, index, count);
    if (bytes > 0) {
        memmove(slot(vec, index), slot(vec, index + count), bytes);
    }
    vec->size -= count;
    return CADT_OK;
}

void *cadt_vector_find(const cadt_vector *vec, const void *elem)
{
    size_t i;

    if (vec == nullptr || elem == nullptr) {
        return nullptr;
    }
    for (i = 0; i < vec->size; i++) {
        if (cadt_type_compare(&vec->type, slot(vec, i), elem) == 0) {
            return slot(vec, i);
        }
    }
    return nullptr;
}
