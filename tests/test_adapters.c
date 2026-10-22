#include "test.h"

static int g_dtor;

static void dtor_int(void *elem, void *ctx)
{
    (void)elem;
    (void)ctx;
    g_dtor++;
}

void test_adapters(void)
{
    track_state state;
    cadt_allocator alloc;
    cadt_type type;
    cadt_stack *stack;
    cadt_queue *queue;
    cadt_pqueue *heap;
    cadt_pqueue *min_heap;
    int value;
    int out;
    int previous;
    int i;
    uint32_t rng = 42;
    int pushed[64];

    track_init(&state, &alloc);
    type = type_int();
    CHECK(cadt_stack_create(&(cadt_type){0}, &alloc) == NULL);
    stack = cadt_stack_create(&type, &alloc);
    CHECK(stack != NULL && cadt_stack_empty(stack));
    CHECK(cadt_stack_pop(stack, NULL) == CADT_ERR_EMPTY);
    for (i = 0; i < 5; i++) {
        value = i;
        CHECK(cadt_stack_push(stack, &value) == CADT_OK);
        CHECK(*(int *)cadt_stack_top(stack) == i);
    }
    for (i = 4; i >= 0; i--) {
        CHECK(cadt_stack_pop(stack, &out) == CADT_OK);
        CHECK(out == i);
    }
    cadt_stack_destroy(stack);

    g_dtor = 0;
    type.destroy = dtor_int;
    stack = cadt_stack_create(&type, &alloc);
    for (i = 0; i < 3; i++) {
        value = i;
        CHECK(cadt_stack_push(stack, &value) == CADT_OK);
    }
    CHECK(cadt_stack_pop(stack, NULL) == CADT_OK);
    CHECK(g_dtor == 1);
    CHECK(cadt_stack_pop(stack, &out) == CADT_OK);
    CHECK(out == 1 && g_dtor == 1);
    cadt_stack_destroy(stack);
    CHECK(g_dtor == 2);

    type = type_int();
    queue = cadt_queue_create(&type, &alloc);
    CHECK(queue != NULL);
    CHECK(cadt_queue_pop(queue, NULL) == CADT_ERR_EMPTY);
    for (i = 0; i < 4; i++) {
        value = i * 10;
        CHECK(cadt_queue_push(queue, &value) == CADT_OK);
    }
    CHECK(*(int *)cadt_queue_front(queue) == 0);
    CHECK(*(int *)cadt_queue_back(queue) == 30);
    CHECK(cadt_queue_pop(queue, &out) == CADT_OK && out == 0);
    CHECK(cadt_queue_pop(queue, &out) == CADT_OK && out == 10);
    cadt_queue_clear(queue);
    CHECK(cadt_queue_empty(queue));
    cadt_queue_destroy(queue);

    type = type_int();
    CHECK(cadt_pqueue_create(&(cadt_type){.size = sizeof(int)}, true, &alloc) == NULL);
    heap = cadt_pqueue_create(&type, true, &alloc);
    min_heap = cadt_pqueue_create(&type, false, &alloc);
    CHECK(heap != NULL && min_heap != NULL);
    CHECK(cadt_pqueue_pop(heap, NULL) == CADT_ERR_EMPTY);
    for (i = 0; i < 64; i++) {
        pushed[i] = (int)(test_rnd(&rng) % 200u) - 50;
        CHECK(cadt_pqueue_push(heap, &pushed[i]) == CADT_OK);
        CHECK(cadt_pqueue_push(min_heap, &pushed[i]) == CADT_OK);
    }
    previous = 1000000;
    for (i = 0; i < 64; i++) {
        CHECK(cadt_pqueue_pop(heap, &out) == CADT_OK);
        CHECK(out <= previous);
        previous = out;
    }
    previous = -1000000;
    for (i = 0; i < 64; i++) {
        CHECK(cadt_pqueue_pop(min_heap, &out) == CADT_OK);
        CHECK(out >= previous);
        previous = out;
    }
    CHECK(cadt_pqueue_empty(heap) && cadt_pqueue_empty(min_heap));

    g_dtor = 0;
    type.destroy = dtor_int;
    cadt_pqueue_destroy(heap);
    heap = cadt_pqueue_create(&type, true, &alloc);
    for (i = 0; i < 4; i++) {
        value = i;
        CHECK(cadt_pqueue_push(heap, &value) == CADT_OK);
    }
    CHECK(cadt_pqueue_pop(heap, NULL) == CADT_OK);
    CHECK(g_dtor == 1);
    CHECK(cadt_pqueue_pop(heap, &out) == CADT_OK);
    CHECK(g_dtor == 1);
    cadt_pqueue_destroy(heap);
    CHECK(g_dtor == 3);
    cadt_pqueue_destroy(min_heap);
    CHECK(!track_leaked(&state));
}
