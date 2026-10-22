/* cadt - C Abstract Data Types
 * SPDX-License-Identifier: MIT
 *
 * LIFO adapter over a vector (std::stack).
 */
#ifndef CADT_STACK_H
#define CADT_STACK_H

#include "cadt/common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cadt_stack cadt_stack;

[[nodiscard]] cadt_stack *cadt_stack_create(const cadt_type *type, const cadt_allocator *alloc);
void cadt_stack_destroy(cadt_stack *stack);

size_t cadt_stack_size(const cadt_stack *stack);
bool cadt_stack_empty(const cadt_stack *stack);
void cadt_stack_clear(cadt_stack *stack);

/* Invalidated by a push that grows the stack. nullptr when empty. */
void *cadt_stack_top(const cadt_stack *stack);
[[nodiscard]] cadt_status cadt_stack_push(cadt_stack *stack, const void *elem);
[[nodiscard]] cadt_status cadt_stack_pop(cadt_stack *stack, void *out);

#ifdef __cplusplus
}
#endif

#endif /* CADT_STACK_H */
