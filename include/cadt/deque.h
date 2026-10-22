/* cadt - C Abstract Data Types
 * SPDX-License-Identifier: MIT
 *
 * Double-ended queue (std::deque) stored as a circular buffer.
 * Push and pop at either end are amortized O(1). Middle insert and erase
 * are O(n). Growth invalidates outstanding element pointers.
 */
#ifndef CADT_DEQUE_H
#define CADT_DEQUE_H

#include "cadt/common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cadt_deque cadt_deque;

[[nodiscard]] cadt_deque *cadt_deque_create(const cadt_type *type, const cadt_allocator *alloc);
void cadt_deque_destroy(cadt_deque *deq);

size_t cadt_deque_size(const cadt_deque *deq);
size_t cadt_deque_capacity(const cadt_deque *deq);
bool cadt_deque_empty(const cadt_deque *deq);

[[nodiscard]] cadt_status cadt_deque_reserve(cadt_deque *deq, size_t capacity);
void cadt_deque_clear(cadt_deque *deq);

void *cadt_deque_at(const cadt_deque *deq, size_t index);
void *cadt_deque_front(const cadt_deque *deq);
void *cadt_deque_back(const cadt_deque *deq);

[[nodiscard]] cadt_status cadt_deque_push_back(cadt_deque *deq, const void *elem);
[[nodiscard]] cadt_status cadt_deque_push_front(cadt_deque *deq, const void *elem);
[[nodiscard]] cadt_status cadt_deque_pop_back(cadt_deque *deq, void *out);
[[nodiscard]] cadt_status cadt_deque_pop_front(cadt_deque *deq, void *out);
[[nodiscard]] cadt_status cadt_deque_insert(cadt_deque *deq, size_t index, const void *elem);
[[nodiscard]] cadt_status cadt_deque_erase(cadt_deque *deq, size_t index, void *out);

#ifdef __cplusplus
}
#endif

#endif /* CADT_DEQUE_H */
