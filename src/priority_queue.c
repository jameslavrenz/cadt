/* cadt - C Abstract Data Types
 * SPDX-License-Identifier: MIT
 */
#include "cadt/priority_queue.h"

#include "cadt/vector.h"
#include "internal.h"

struct cadt_pqueue {
    cadt_vector *data;
    cadt_type type;
    const cadt_allocator *alloc;
    unsigned char *scratch;
    bool max_heap;
};

static int heap_cmp(const cadt_pqueue *pq, size_t i, size_t j)
{
    int cmp = cadt_type_compare(&pq->type,
                                cadt_vector_at(pq->data, i),
                                cadt_vector_at(pq->data, j));
    return pq->max_heap ? cmp : -cmp;
}

static void swap_at(cadt_pqueue *pq, size_t i, size_t j)
{
    void *a;
    void *b;

    if (i == j) {
        return;
    }
    a = cadt_vector_at(pq->data, i);
    b = cadt_vector_at(pq->data, j);
    memcpy(pq->scratch, a, pq->type.size);
    memcpy(a, b, pq->type.size);
    memcpy(b, pq->scratch, pq->type.size);
}

static void sift_up(cadt_pqueue *pq, size_t index)
{
    while (index > 0) {
        size_t parent = (index - 1u) / 2u;

        if (heap_cmp(pq, index, parent) <= 0) {
            break;
        }
        swap_at(pq, index, parent);
        index = parent;
    }
}

static void sift_down(cadt_pqueue *pq, size_t index)
{
    size_t size = cadt_vector_size(pq->data);

    for (;;) {
        size_t left = index * 2u + 1u;
        size_t right = left + 1u;
        size_t best = index;

        if (left < size && heap_cmp(pq, left, best) > 0) {
            best = left;
        }
        if (right < size && heap_cmp(pq, right, best) > 0) {
            best = right;
        }
        if (best == index) {
            break;
        }
        swap_at(pq, index, best);
        index = best;
    }
}

cadt_pqueue *cadt_pqueue_create(const cadt_type *type, bool max_heap,
                                const cadt_allocator *alloc)
{
    cadt_pqueue *pq;

    if (cadt_type_check(type) != CADT_OK || type->compare == NULL) {
        return NULL;
    }
    alloc = cadt_alloc_or_default(alloc);
    pq = (cadt_pqueue *)cadt_xalloc(alloc, sizeof(*pq));
    if (pq == NULL) {
        return NULL;
    }
    pq->alloc = alloc;
    pq->type = *type;
    pq->max_heap = max_heap;
    pq->data = cadt_vector_create(type, alloc);
    pq->scratch = (unsigned char *)cadt_xalloc(alloc, type->size);
    if (pq->data == NULL || pq->scratch == NULL) {
        cadt_vector_destroy(pq->data);
        cadt_xfree(alloc, pq->scratch);
        cadt_xfree(alloc, pq);
        return NULL;
    }
    return pq;
}

void cadt_pqueue_destroy(cadt_pqueue *pq)
{
    if (pq == NULL) {
        return;
    }
    cadt_vector_destroy(pq->data);
    cadt_xfree(pq->alloc, pq->scratch);
    cadt_xfree(pq->alloc, pq);
}

size_t cadt_pqueue_size(const cadt_pqueue *pq)
{
    return pq == NULL ? 0 : cadt_vector_size(pq->data);
}

bool cadt_pqueue_empty(const cadt_pqueue *pq)
{
    return cadt_pqueue_size(pq) == 0;
}

void cadt_pqueue_clear(cadt_pqueue *pq)
{
    if (pq != NULL) {
        cadt_vector_clear(pq->data);
    }
}

void *cadt_pqueue_top(const cadt_pqueue *pq)
{
    return pq == NULL ? NULL : cadt_vector_at(pq->data, 0);
}

cadt_status cadt_pqueue_push(cadt_pqueue *pq, const void *elem)
{
    cadt_status status;
    size_t index;

    if (pq == NULL) {
        return CADT_ERR_INVALID;
    }
    status = cadt_vector_push_back(pq->data, elem);
    if (status != CADT_OK) {
        return status;
    }
    index = cadt_vector_size(pq->data) - 1u;
    sift_up(pq, index);
    return CADT_OK;
}

cadt_status cadt_pqueue_pop(cadt_pqueue *pq, void *out)
{
    size_t size;
    void *top;
    cadt_status status;

    if (pq == NULL) {
        return CADT_ERR_INVALID;
    }
    size = cadt_vector_size(pq->data);
    if (size == 0) {
        return CADT_ERR_EMPTY;
    }
    if (size == 1u) {
        return cadt_vector_pop_back(pq->data, out);
    }
    top = cadt_vector_at(pq->data, 0);
    if (out != NULL) {
        memcpy(out, top, pq->type.size);
    } else {
        cadt_type_destroy(&pq->type, top);
    }
    status = cadt_vector_pop_back(pq->data, pq->scratch);
    if (status != CADT_OK) {
        return status;
    }
    memcpy(top, pq->scratch, pq->type.size);
    sift_down(pq, 0);
    return CADT_OK;
}
