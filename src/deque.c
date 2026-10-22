/* cadt - C Abstract Data Types
 * SPDX-License-Identifier: MIT
 */
#include "cadt/deque.h"

#include "internal.h"

struct cadt_deque {
    const cadt_allocator *alloc;
    cadt_type type;
    unsigned char *data;
    size_t capacity;
    size_t head;
    size_t size;
};

static unsigned char *at_ptr(const cadt_deque *deq, size_t logical)
{
    size_t index = deq->head + logical;

    if (deq->capacity != 0 && index >= deq->capacity) {
        index -= deq->capacity;
    }
    return deq->data + index * deq->type.size;
}

/* Copy the live range to the front of a fresh buffer of new_cap elements. */
static cadt_status relayout(cadt_deque *deq, size_t new_cap)
{
    size_t bytes;
    unsigned char *fresh;
    size_t i;

    if (new_cap < deq->size) {
        return CADT_ERR_INVALID;
    }
    if (new_cap == 0) {
        return CADT_OK;
    }
    if (!cadt_mul_size(new_cap, deq->type.size, &bytes)) {
        return CADT_ERR_OVERFLOW;
    }
    fresh = (unsigned char *)cadt_xalloc(deq->alloc, bytes);
    if (fresh == NULL) {
        return CADT_ERR_NOMEM;
    }
    for (i = 0; i < deq->size; i++) {
        memcpy(fresh + i * deq->type.size, at_ptr(deq, i), deq->type.size);
    }
    cadt_xfree(deq->alloc, deq->data);
    deq->data = fresh;
    deq->capacity = new_cap;
    deq->head = 0;
    return CADT_OK;
}

/* Grow so capacity >= min_cap and, when the buffer is circular, linearize it. */
static cadt_status grow_for(cadt_deque *deq, size_t min_cap)
{
    size_t new_cap;

    if (min_cap <= deq->capacity && deq->head == 0) {
        return CADT_OK;
    }
    new_cap = deq->capacity == 0 ? 8u : deq->capacity;
    while (new_cap < min_cap) {
        if (new_cap > SIZE_MAX / 2u) {
            new_cap = min_cap;
            break;
        }
        new_cap *= 2u;
    }
    if (new_cap < min_cap) {
        new_cap = min_cap;
    }
    return relayout(deq, new_cap);
}

cadt_deque *cadt_deque_create(const cadt_type *type, const cadt_allocator *alloc)
{
    cadt_deque *deq;

    if (cadt_type_check(type) != CADT_OK) {
        return NULL;
    }
    alloc = cadt_alloc_or_default(alloc);
    deq = (cadt_deque *)cadt_xalloc(alloc, sizeof(*deq));
    if (deq == NULL) {
        return NULL;
    }
    deq->alloc = alloc;
    deq->type = *type;
    deq->data = NULL;
    deq->capacity = 0;
    deq->head = 0;
    deq->size = 0;
    return deq;
}

void cadt_deque_clear(cadt_deque *deq)
{
    size_t i;

    if (deq == NULL) {
        return;
    }
    for (i = 0; i < deq->size; i++) {
        cadt_type_destroy(&deq->type, at_ptr(deq, i));
    }
    deq->size = 0;
    deq->head = 0;
}

void cadt_deque_destroy(cadt_deque *deq)
{
    if (deq == NULL) {
        return;
    }
    cadt_deque_clear(deq);
    cadt_xfree(deq->alloc, deq->data);
    cadt_xfree(deq->alloc, deq);
}

size_t cadt_deque_size(const cadt_deque *deq)
{
    return deq == NULL ? 0 : deq->size;
}

size_t cadt_deque_capacity(const cadt_deque *deq)
{
    return deq == NULL ? 0 : deq->capacity;
}

bool cadt_deque_empty(const cadt_deque *deq)
{
    return cadt_deque_size(deq) == 0;
}

cadt_status cadt_deque_reserve(cadt_deque *deq, size_t capacity)
{
    if (deq == NULL) {
        return CADT_ERR_INVALID;
    }
    if (capacity <= deq->capacity) {
        return CADT_OK;
    }
    return grow_for(deq, capacity);
}

void *cadt_deque_at(const cadt_deque *deq, size_t index)
{
    if (deq == NULL || index >= deq->size) {
        return NULL;
    }
    return at_ptr(deq, index);
}

void *cadt_deque_front(const cadt_deque *deq)
{
    return cadt_deque_at(deq, 0);
}

void *cadt_deque_back(const cadt_deque *deq)
{
    if (deq == NULL || deq->size == 0) {
        return NULL;
    }
    return at_ptr(deq, deq->size - 1u);
}

cadt_status cadt_deque_push_back(cadt_deque *deq, const void *elem)
{
    cadt_status status;
    size_t index;

    if (deq == NULL || elem == NULL) {
        return CADT_ERR_INVALID;
    }
    if (deq->size == deq->capacity) {
        if (deq->size == SIZE_MAX) {
            return CADT_ERR_OVERFLOW;
        }
        status = grow_for(deq, deq->size + 1u);
        if (status != CADT_OK) {
            return status;
        }
    }
    index = deq->head + deq->size;
    if (index >= deq->capacity) {
        index -= deq->capacity;
    }
    status = cadt_type_copy(&deq->type, deq->data + index * deq->type.size, elem);
    if (status != CADT_OK) {
        return status;
    }
    deq->size++;
    return CADT_OK;
}

cadt_status cadt_deque_push_front(cadt_deque *deq, const void *elem)
{
    cadt_status status;
    size_t head;

    if (deq == NULL || elem == NULL) {
        return CADT_ERR_INVALID;
    }
    if (deq->size == deq->capacity) {
        if (deq->size == SIZE_MAX) {
            return CADT_ERR_OVERFLOW;
        }
        status = grow_for(deq, deq->size + 1u);
        if (status != CADT_OK) {
            return status;
        }
    }
    head = deq->head == 0 ? deq->capacity - 1u : deq->head - 1u;
    status = cadt_type_copy(&deq->type, deq->data + head * deq->type.size, elem);
    if (status != CADT_OK) {
        return status;
    }
    deq->head = head;
    deq->size++;
    return CADT_OK;
}

cadt_status cadt_deque_pop_back(cadt_deque *deq, void *out)
{
    unsigned char *elem;

    if (deq == NULL) {
        return CADT_ERR_INVALID;
    }
    if (deq->size == 0) {
        return CADT_ERR_EMPTY;
    }
    elem = at_ptr(deq, deq->size - 1u);
    if (out != NULL) {
        memcpy(out, elem, deq->type.size);
    } else {
        cadt_type_destroy(&deq->type, elem);
    }
    deq->size--;
    return CADT_OK;
}

cadt_status cadt_deque_pop_front(cadt_deque *deq, void *out)
{
    unsigned char *elem;

    if (deq == NULL) {
        return CADT_ERR_INVALID;
    }
    if (deq->size == 0) {
        return CADT_ERR_EMPTY;
    }
    elem = at_ptr(deq, 0);
    if (out != NULL) {
        memcpy(out, elem, deq->type.size);
    } else {
        cadt_type_destroy(&deq->type, elem);
    }
    deq->head++;
    if (deq->head == deq->capacity) {
        deq->head = 0;
    }
    deq->size--;
    return CADT_OK;
}

cadt_status cadt_deque_insert(cadt_deque *deq, size_t index, const void *elem)
{
    cadt_status status;
    size_t i;

    if (deq == NULL || elem == NULL) {
        return CADT_ERR_INVALID;
    }
    if (index > deq->size) {
        return CADT_ERR_RANGE;
    }
    if (index == 0) {
        return cadt_deque_push_front(deq, elem);
    }
    if (index == deq->size) {
        return cadt_deque_push_back(deq, elem);
    }
    if (deq->size == SIZE_MAX) {
        return CADT_ERR_OVERFLOW;
    }
    status = grow_for(deq, deq->size + 1u);
    if (status != CADT_OK) {
        return status;
    }
    for (i = deq->size; i > index; i--) {
        memcpy(at_ptr(deq, i), at_ptr(deq, i - 1u), deq->type.size);
    }
    status = cadt_type_copy(&deq->type, at_ptr(deq, index), elem);
    if (status != CADT_OK) {
        for (i = index; i < deq->size; i++) {
            memcpy(at_ptr(deq, i), at_ptr(deq, i + 1u), deq->type.size);
        }
        return status;
    }
    deq->size++;
    return CADT_OK;
}

cadt_status cadt_deque_erase(cadt_deque *deq, size_t index, void *out)
{
    size_t i;
    cadt_status status;

    if (deq == NULL) {
        return CADT_ERR_INVALID;
    }
    if (index >= deq->size) {
        return CADT_ERR_RANGE;
    }
    if (index == 0) {
        return cadt_deque_pop_front(deq, out);
    }
    if (index + 1u == deq->size) {
        return cadt_deque_pop_back(deq, out);
    }
    status = grow_for(deq, deq->capacity == 0 ? 1u : deq->capacity);
    if (status != CADT_OK) {
        return status;
    }
    if (out != NULL) {
        memcpy(out, at_ptr(deq, index), deq->type.size);
    } else {
        cadt_type_destroy(&deq->type, at_ptr(deq, index));
    }
    for (i = index; i + 1u < deq->size; i++) {
        memcpy(at_ptr(deq, i), at_ptr(deq, i + 1u), deq->type.size);
    }
    deq->size--;
    return CADT_OK;
}
