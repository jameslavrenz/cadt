/* cadt - C Abstract Data Types
 * SPDX-License-Identifier: MIT
 */
#include "cadt/queue.h"

#include "cadt/deque.h"
#include "internal.h"

struct cadt_queue {
    cadt_deque *data;
    const cadt_allocator *alloc;
};

cadt_queue *cadt_queue_create(const cadt_type *type, const cadt_allocator *alloc)
{
    cadt_queue *queue;

    alloc = cadt_alloc_or_default(alloc);
    queue = (cadt_queue *)cadt_xalloc(alloc, sizeof(*queue));
    if (queue == nullptr) {
        return nullptr;
    }
    queue->alloc = alloc;
    queue->data = cadt_deque_create(type, alloc);
    if (queue->data == nullptr) {
        cadt_xfree(alloc, queue);
        return nullptr;
    }
    return queue;
}

void cadt_queue_destroy(cadt_queue *queue)
{
    if (queue == nullptr) {
        return;
    }
    cadt_deque_destroy(queue->data);
    cadt_xfree(queue->alloc, queue);
}

size_t cadt_queue_size(const cadt_queue *queue)
{
    return queue == nullptr ? 0 : cadt_deque_size(queue->data);
}

bool cadt_queue_empty(const cadt_queue *queue)
{
    return cadt_queue_size(queue) == 0;
}

void cadt_queue_clear(cadt_queue *queue)
{
    if (queue != nullptr) {
        cadt_deque_clear(queue->data);
    }
}

void *cadt_queue_front(const cadt_queue *queue)
{
    return queue == nullptr ? nullptr : cadt_deque_front(queue->data);
}

void *cadt_queue_back(const cadt_queue *queue)
{
    return queue == nullptr ? nullptr : cadt_deque_back(queue->data);
}

cadt_status cadt_queue_push(cadt_queue *queue, const void *elem)
{
    if (queue == nullptr) {
        return CADT_ERR_INVALID;
    }
    return cadt_deque_push_back(queue->data, elem);
}

cadt_status cadt_queue_pop(cadt_queue *queue, void *out)
{
    if (queue == nullptr) {
        return CADT_ERR_INVALID;
    }
    return cadt_deque_pop_front(queue->data, out);
}
