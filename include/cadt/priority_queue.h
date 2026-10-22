/* cadt - C Abstract Data Types
 * SPDX-License-Identifier: MIT
 *
 * Binary heap (std::priority_queue).
 * The type's compare function is required. With max_heap true (the STL
 * default), top() is the greatest element under that ordering.
 */
#ifndef CADT_PRIORITY_QUEUE_H
#define CADT_PRIORITY_QUEUE_H

#include "cadt/common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cadt_pqueue cadt_pqueue;

[[nodiscard]] cadt_pqueue *cadt_pqueue_create(const cadt_type *type, bool max_heap,
                                const cadt_allocator *alloc);
void cadt_pqueue_destroy(cadt_pqueue *pq);

size_t cadt_pqueue_size(const cadt_pqueue *pq);
bool cadt_pqueue_empty(const cadt_pqueue *pq);
void cadt_pqueue_clear(cadt_pqueue *pq);

/* nullptr when empty. Invalidated by push and pop. */
void *cadt_pqueue_top(const cadt_pqueue *pq);
[[nodiscard]] cadt_status cadt_pqueue_push(cadt_pqueue *pq, const void *elem);
[[nodiscard]] cadt_status cadt_pqueue_pop(cadt_pqueue *pq, void *out);

#ifdef __cplusplus
}
#endif

#endif /* CADT_PRIORITY_QUEUE_H */
