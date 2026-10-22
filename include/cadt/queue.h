/* cadt - C Abstract Data Types
 * SPDX-License-Identifier: MIT
 *
 * FIFO adapter over a deque (std::queue).
 */
#ifndef CADT_QUEUE_H
#define CADT_QUEUE_H

#include "cadt/common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cadt_queue cadt_queue;

[[nodiscard]] cadt_queue *cadt_queue_create(const cadt_type *type, const cadt_allocator *alloc);
void cadt_queue_destroy(cadt_queue *queue);

size_t cadt_queue_size(const cadt_queue *queue);
bool cadt_queue_empty(const cadt_queue *queue);
void cadt_queue_clear(cadt_queue *queue);

void *cadt_queue_front(const cadt_queue *queue);
void *cadt_queue_back(const cadt_queue *queue);
[[nodiscard]] cadt_status cadt_queue_push(cadt_queue *queue, const void *elem);
[[nodiscard]] cadt_status cadt_queue_pop(cadt_queue *queue, void *out);

#ifdef __cplusplus
}
#endif

#endif /* CADT_QUEUE_H */
