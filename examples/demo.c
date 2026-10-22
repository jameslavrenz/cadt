#include "cadt.h"

#include <stdio.h>

static int cmp_int(const void *a, const void *b, [[maybe_unused]] void *ctx)
{
    int ia = *(const int *)a;
    int ib = *(const int *)b;
    return (ia > ib) - (ia < ib);
}

static int fail(const char *message)
{
    fprintf(stderr, "demo: %s\n", message);
    return 1;
}

int main(void)
{
    cadt_type type = {
        .size = sizeof(int),
        .compare = cmp_int,
    };
    cadt_vector *numbers;
    cadt_set *unique;
    cadt_stack *stack;
    cadt_pqueue *heap;
    cadt_string *line;
    int values[] = {4, 1, 3, 1, 2};
    int i;
    int out;
    int previous;

    numbers = cadt_vector_create(&type, nullptr);
    if (numbers == nullptr) {
        return fail("vector");
    }
    for (i = 0; i < 5; i++) {
        if (cadt_vector_push_back(numbers, &values[i]) != CADT_OK) {
            return fail("push");
        }
    }
    if (cadt_sort(cadt_vector_data(numbers), cadt_vector_size(numbers), sizeof(int),
                  cmp_int, nullptr) != CADT_OK) {
        return fail("sort");
    }
    for (i = 1; i < 5; i++) {
        int left = *(int *)cadt_vector_at(numbers, (size_t)i - 1u);
        int right = *(int *)cadt_vector_at(numbers, (size_t)i);
        if (left > right) {
            return fail("order");
        }
    }

    unique = cadt_set_create(&type, nullptr);
    if (unique == nullptr) {
        return fail("set");
    }
    for (i = 0; i < 5; i++) {
        cadt_status status = cadt_set_insert(unique, &values[i]);
        if (status != CADT_OK && status != CADT_ERR_EXISTS) {
            return fail("insert");
        }
    }
    if (cadt_set_size(unique) != 4 || *(int *)cadt_set_min(unique) != 1) {
        return fail("unique");
    }

    stack = cadt_stack_create(&type, nullptr);
    heap = cadt_pqueue_create(&type, true, nullptr);
    if (stack == nullptr || heap == nullptr) {
        return fail("adapters");
    }
    for (i = 0; i < 5; i++) {
        if (cadt_stack_push(stack, &values[i]) != CADT_OK ||
            cadt_pqueue_push(heap, &values[i]) != CADT_OK) {
            return fail("push adapter");
        }
    }
    if (cadt_stack_pop(stack, &out) != CADT_OK || out != 2) {
        return fail("stack");
    }
    previous = 100;
    for (i = 0; i < 5; i++) {
        if (cadt_pqueue_pop(heap, &out) != CADT_OK || out > previous) {
            return fail("heap");
        }
        previous = out;
    }

    line = cadt_string_create_cstr("cadt", nullptr);
    if (line == nullptr || cadt_string_append_cstr(line, " works") != CADT_OK ||
        !cadt_string_equals_cstr(line, "cadt works")) {
        return fail("string");
    }

    printf("vector");
    for (i = 0; i < 5; i++) {
        printf(" %d", *(int *)cadt_vector_at(numbers, (size_t)i));
    }
    printf("\nset size %zu min %d\n", cadt_set_size(unique), *(int *)cadt_set_min(unique));
    printf("%s\n", cadt_string_c_str(line));

    cadt_string_destroy(line);
    cadt_pqueue_destroy(heap);
    cadt_stack_destroy(stack);
    cadt_set_destroy(unique);
    cadt_vector_destroy(numbers);
    return 0;
}
