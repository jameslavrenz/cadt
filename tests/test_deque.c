#include "test.h"

static void expect_deque(cadt_deque *deq, const int *values, size_t count)
{
    size_t i;

    CHECK(cadt_deque_size(deq) == count);
    for (i = 0; i < count; i++) {
        int *slot = (int *)cadt_deque_at(deq, i);
        CHECK(slot != nullptr && *slot == values[i]);
    }
    CHECK(cadt_deque_at(deq, count) == nullptr);
}

void test_deque(void)
{
    track_state state;
    cadt_allocator alloc;
    cadt_type type;
    cadt_deque *deq;
    int oracle[64];
    size_t osize = 0;
    int value;
    int out;
    int i;
    uint32_t rng = 9;

    track_init(&state, &alloc);
    type = type_int();
    deq = cadt_deque_create(&type, &alloc);
    CHECK(deq != nullptr);
    CHECK(cadt_deque_pop_front(deq, nullptr) == CADT_ERR_EMPTY);
    CHECK(cadt_deque_pop_back(deq, nullptr) == CADT_ERR_EMPTY);
    for (i = 0; i < 100; i++) {
        value = i;
        CHECK(cadt_deque_push_front(deq, &value) == CADT_OK);
    }
    for (i = 0; i < 100; i++) {
        value = 100 + i;
        CHECK(cadt_deque_push_back(deq, &value) == CADT_OK);
    }
    CHECK(cadt_deque_size(deq) == 200);
    CHECK(*(int *)cadt_deque_front(deq) == 99);
    CHECK(*(int *)cadt_deque_at(deq, 99) == 0);
    CHECK(*(int *)cadt_deque_at(deq, 100) == 100);
    CHECK(*(int *)cadt_deque_back(deq) == 199);
    for (i = 0; i < 150; i++) {
        CHECK(cadt_deque_pop_front(deq, &out) == CADT_OK);
        if (i < 100) {
            CHECK(out == 99 - i);
        } else {
            CHECK(out == i);
        }
    }
    CHECK(cadt_deque_size(deq) == 50);
    CHECK(*(int *)cadt_deque_front(deq) == 150);
    cadt_deque_clear(deq);
    CHECK(cadt_deque_empty(deq));

    for (i = 0; i < 8; i++) {
        value = i;
        CHECK(cadt_deque_push_back(deq, &value) == CADT_OK);
    }
    CHECK(cadt_deque_pop_front(deq, nullptr) == CADT_OK);
    CHECK(cadt_deque_pop_front(deq, nullptr) == CADT_OK);
    value = 42;
    CHECK(cadt_deque_insert(deq, 2, &value) == CADT_OK);
    CHECK(*(int *)cadt_deque_at(deq, 0) == 2);
    CHECK(*(int *)cadt_deque_at(deq, 2) == 42);
    CHECK(*(int *)cadt_deque_at(deq, 3) == 4);
    CHECK(cadt_deque_erase(deq, 2, &out) == CADT_OK);
    CHECK(out == 42);
    CHECK(*(int *)cadt_deque_at(deq, 2) == 4);
    cadt_deque_destroy(deq);

    deq = cadt_deque_create(&type, &alloc);
    for (i = 0; i < 400; i++) {
        uint32_t roll = test_rnd(&rng) % 5u;
        if ((roll == 0 || osize == 0) && osize < 64) {
            value = (int)(test_rnd(&rng) % 500u);
            CHECK(cadt_deque_push_back(deq, &value) == CADT_OK);
            oracle[osize++] = value;
        } else if (roll == 1 && osize < 64) {
            value = (int)(test_rnd(&rng) % 500u);
            CHECK(cadt_deque_push_front(deq, &value) == CADT_OK);
            memmove(oracle + 1, oracle, osize * sizeof(int));
            oracle[0] = value;
            osize++;
        } else if (roll == 2 && osize > 0) {
            CHECK(cadt_deque_pop_back(deq, &out) == CADT_OK);
            CHECK(out == oracle[osize - 1u]);
            osize--;
        } else if (roll == 3 && osize > 0) {
            CHECK(cadt_deque_pop_front(deq, &out) == CADT_OK);
            CHECK(out == oracle[0]);
            memmove(oracle, oracle + 1, (osize - 1u) * sizeof(int));
            osize--;
        } else if (osize < 64) {
            size_t index = (size_t)(test_rnd(&rng) % (uint32_t)(osize + 1u));
            value = (int)test_rnd(&rng);
            CHECK(cadt_deque_insert(deq, index, &value) == CADT_OK);
            memmove(oracle + index + 1, oracle + index, (osize - index) * sizeof(int));
            oracle[index] = value;
            osize++;
        }
        expect_deque(deq, oracle, osize);
    }
    CHECK(cadt_deque_reserve(deq, osize + 20) == CADT_OK);
    expect_deque(deq, oracle, osize);
    cadt_deque_destroy(deq);
    CHECK(!track_leaked(&state));
}
