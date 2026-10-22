/* cadt - C Abstract Data Types
 * SPDX-License-Identifier: MIT
 */
#include "cadt/array.h"

#include "internal.h"

struct cadt_array {
    const cadt_allocator *alloc;
    cadt_type type;
    unsigned char *data;
    size_t size;
};

static unsigned char *slot(const cadt_array *arr, size_t index)
{
    return arr->data + index * arr->type.size;
}

cadt_array *cadt_array_create(const cadt_type *type, size_t count, const void *fill,
                              const cadt_allocator *alloc)
{
    cadt_array *arr;
    size_t bytes;
    size_t i;
    cadt_status status;

    if (cadt_type_check(type) != CADT_OK) {
        return NULL;
    }
    if (count > 0 && fill == NULL && type->destroy != NULL) {
        return NULL;
    }
    alloc = cadt_alloc_or_default(alloc);
    arr = (cadt_array *)cadt_xalloc(alloc, sizeof(*arr));
    if (arr == NULL) {
        return NULL;
    }
    arr->alloc = alloc;
    arr->type = *type;
    arr->size = count;
    arr->data = NULL;
    if (count == 0) {
        return arr;
    }
    if (!cadt_mul_size(count, type->size, &bytes)) {
        cadt_xfree(alloc, arr);
        return NULL;
    }
    arr->data = (unsigned char *)cadt_xalloc(alloc, bytes);
    if (arr->data == NULL) {
        cadt_xfree(alloc, arr);
        return NULL;
    }
    for (i = 0; i < count; i++) {
        if (fill == NULL) {
            memset(slot(arr, i), 0, arr->type.size);
        } else {
            status = cadt_type_copy(&arr->type, slot(arr, i), fill);
            if (status != CADT_OK) {
                while (i > 0) {
                    i--;
                    cadt_type_destroy(&arr->type, slot(arr, i));
                }
                cadt_xfree(alloc, arr->data);
                cadt_xfree(alloc, arr);
                return NULL;
            }
        }
    }
    return arr;
}

void cadt_array_destroy(cadt_array *arr)
{
    size_t i;

    if (arr == NULL) {
        return;
    }
    for (i = 0; i < arr->size; i++) {
        cadt_type_destroy(&arr->type, slot(arr, i));
    }
    cadt_xfree(arr->alloc, arr->data);
    cadt_xfree(arr->alloc, arr);
}

size_t cadt_array_size(const cadt_array *arr)
{
    return arr == NULL ? 0 : arr->size;
}

bool cadt_array_empty(const cadt_array *arr)
{
    return cadt_array_size(arr) == 0;
}

void *cadt_array_at(const cadt_array *arr, size_t index)
{
    if (arr == NULL || index >= arr->size) {
        return NULL;
    }
    return slot(arr, index);
}

void *cadt_array_data(const cadt_array *arr)
{
    return arr == NULL ? NULL : arr->data;
}

void *cadt_array_front(const cadt_array *arr)
{
    return cadt_array_at(arr, 0);
}

void *cadt_array_back(const cadt_array *arr)
{
    if (arr == NULL || arr->size == 0) {
        return NULL;
    }
    return slot(arr, arr->size - 1u);
}

cadt_span cadt_array_as_span(const cadt_array *arr)
{
    if (arr == NULL) {
        return cadt_span_make(NULL, 0, 0);
    }
    return cadt_span_make(arr->data, arr->size, arr->type.size);
}

cadt_status cadt_array_set(cadt_array *arr, size_t index, const void *elem)
{
    unsigned char *tmp;
    cadt_status status;

    if (arr == NULL || elem == NULL) {
        return CADT_ERR_INVALID;
    }
    if (index >= arr->size) {
        return CADT_ERR_RANGE;
    }
    tmp = (unsigned char *)cadt_xalloc(arr->alloc, arr->type.size);
    if (tmp == NULL) {
        return CADT_ERR_NOMEM;
    }
    status = cadt_type_copy(&arr->type, tmp, elem);
    if (status != CADT_OK) {
        cadt_xfree(arr->alloc, tmp);
        return status;
    }
    cadt_type_destroy(&arr->type, slot(arr, index));
    memcpy(slot(arr, index), tmp, arr->type.size);
    cadt_xfree(arr->alloc, tmp);
    return CADT_OK;
}
