/* cadt - C Abstract Data Types
 * SPDX-License-Identifier: MIT
 */
#include "cadt/stack.h"

#include "cadt/vector.h"
#include "internal.h"

struct cadt_stack {
    cadt_vector *data;
    const cadt_allocator *alloc;
};

cadt_stack *cadt_stack_create(const cadt_type *type, const cadt_allocator *alloc)
{
    cadt_stack *stack;

    alloc = cadt_alloc_or_default(alloc);
    stack = (cadt_stack *)cadt_xalloc(alloc, sizeof(*stack));
    if (stack == nullptr) {
        return nullptr;
    }
    stack->alloc = alloc;
    stack->data = cadt_vector_create(type, alloc);
    if (stack->data == nullptr) {
        cadt_xfree(alloc, stack);
        return nullptr;
    }
    return stack;
}

void cadt_stack_destroy(cadt_stack *stack)
{
    if (stack == nullptr) {
        return;
    }
    cadt_vector_destroy(stack->data);
    cadt_xfree(stack->alloc, stack);
}

size_t cadt_stack_size(const cadt_stack *stack)
{
    return stack == nullptr ? 0 : cadt_vector_size(stack->data);
}

bool cadt_stack_empty(const cadt_stack *stack)
{
    return cadt_stack_size(stack) == 0;
}

void cadt_stack_clear(cadt_stack *stack)
{
    if (stack != nullptr) {
        cadt_vector_clear(stack->data);
    }
}

void *cadt_stack_top(const cadt_stack *stack)
{
    return stack == nullptr ? nullptr : cadt_vector_back(stack->data);
}

cadt_status cadt_stack_push(cadt_stack *stack, const void *elem)
{
    if (stack == nullptr) {
        return CADT_ERR_INVALID;
    }
    return cadt_vector_push_back(stack->data, elem);
}

cadt_status cadt_stack_pop(cadt_stack *stack, void *out)
{
    if (stack == nullptr) {
        return CADT_ERR_INVALID;
    }
    return cadt_vector_pop_back(stack->data, out);
}
