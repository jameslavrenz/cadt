#include "test.h"

static int g_dtor;
static int g_copies;

static void dtor_int([[maybe_unused]] void *elem, [[maybe_unused]] void *ctx)
{
    g_dtor++;
}

static cadt_status copy_limited(void *dst, const void *src, [[maybe_unused]] void *ctx)
{
    if (g_copies <= 0) {
        return CADT_ERR_NOMEM;
    }
    g_copies--;
    memcpy(dst, src, sizeof(int));
    return CADT_OK;
}

static void expect_vector(cadt_vector *vec, const int *values, size_t count)
{
    size_t i;

    CHECK(cadt_vector_size(vec) == count);
    CHECK(cadt_vector_empty(vec) == (count == 0));
    for (i = 0; i < count; i++) {
        int *slot = (int *)cadt_vector_at(vec, i);
        CHECK(slot != nullptr && *slot == values[i]);
    }
    CHECK(cadt_vector_at(vec, count) == nullptr);
}

void test_vector(void)
{
    track_state state;
    cadt_allocator alloc;
    cadt_type type;
    cadt_vector *vec;
    cadt_vector *plain;
    int value;
    int out;
    int oracle[48];
    size_t osize;
    uint32_t rng = 1;
    int i;

    track_init(&state, &alloc);
    CHECK(cadt_vector_create(nullptr, &alloc) == nullptr);
    type = type_int();
    type.size = 0;
    CHECK(cadt_vector_create(&type, &alloc) == nullptr);

    type = type_int();
    vec = cadt_vector_create(&type, &alloc);
    CHECK(vec != nullptr);
    value = 0;
    CHECK(cadt_vector_push_back(nullptr, &value) == CADT_ERR_INVALID);
    CHECK(cadt_vector_pop_back(vec, nullptr) == CADT_ERR_EMPTY);
    for (i = 0; i < 40; i++) {
        value = i;
        CHECK(cadt_vector_push_back(vec, &value) == CADT_OK);
        oracle[i] = i;
    }
    expect_vector(vec, oracle, 40);
    CHECK(*(int *)cadt_vector_front(vec) == 0);
    CHECK(*(int *)cadt_vector_back(vec) == 39);
    CHECK(cadt_vector_reserve(vec, 100) == CADT_OK);
    CHECK(cadt_vector_capacity(vec) >= 100);
    value = 7;
    CHECK(cadt_vector_insert(vec, 0, &value) == CADT_OK);
    memmove(oracle + 1, oracle, 40 * sizeof(int));
    oracle[0] = 7;
    value = 8;
    CHECK(cadt_vector_insert(vec, 10, &value) == CADT_OK);
    memmove(oracle + 11, oracle + 10, 31 * sizeof(int));
    oracle[10] = 8;
    expect_vector(vec, oracle, 42);
    CHECK(cadt_vector_erase(vec, 10, &out) == CADT_OK);
    CHECK(out == 8);
    memmove(oracle + 10, oracle + 11, 31 * sizeof(int));
    expect_vector(vec, oracle, 41);
    CHECK(cadt_vector_erase_range(vec, 1, 4) == CADT_OK);
    memmove(oracle + 1, oracle + 5, 36 * sizeof(int));
    expect_vector(vec, oracle, 37);
    value = 99;
    CHECK(cadt_vector_set(vec, 0, &value) == CADT_OK);
    oracle[0] = 99;
    CHECK(*(int *)cadt_vector_find(vec, &value) == 99);
    CHECK(cadt_vector_resize(vec, 10, nullptr) == CADT_OK);
    expect_vector(vec, oracle, 10);
    value = 3;
    CHECK(cadt_vector_resize(vec, 12, &value) == CADT_OK);
    oracle[10] = 3;
    oracle[11] = 3;
    expect_vector(vec, oracle, 12);
    CHECK(cadt_vector_shrink_to_fit(vec) == CADT_OK);
    CHECK(cadt_vector_capacity(vec) == 12);
    CHECK(cadt_vector_pop_back(vec, &out) == CADT_OK);
    CHECK(out == 3);
    cadt_vector_clear(vec);
    CHECK(cadt_vector_empty(vec));
    CHECK(cadt_vector_capacity(vec) >= 11);
    cadt_vector_destroy(vec);

    g_dtor = 0;
    type = type_int();
    type.destroy = dtor_int;
    vec = cadt_vector_create(&type, &alloc);
    for (i = 0; i < 3; i++) {
        value = i;
        CHECK(cadt_vector_push_back(vec, &value) == CADT_OK);
    }
    CHECK(cadt_vector_erase(vec, 1, nullptr) == CADT_OK);
    CHECK(g_dtor == 1);
    CHECK(cadt_vector_pop_back(vec, &out) == CADT_OK);
    CHECK(out == 2);
    CHECK(g_dtor == 1);
    cadt_vector_destroy(vec);
    CHECK(g_dtor == 2);

    g_copies = 0;
    type = type_int();
    type.copy = copy_limited;
    type.destroy = dtor_int;
    vec = cadt_vector_create(&type, &alloc);
    value = 4;
    CHECK(cadt_vector_push_back(vec, &value) == CADT_ERR_NOMEM);
    CHECK(cadt_vector_empty(vec));
    g_copies = 1;
    CHECK(cadt_vector_push_back(vec, &value) == CADT_OK);
    g_copies = 0;
    value = 5;
    CHECK(cadt_vector_insert(vec, 0, &value) == CADT_ERR_NOMEM);
    CHECK(cadt_vector_size(vec) == 1);
    CHECK(*(int *)cadt_vector_at(vec, 0) == 4);
    CHECK(cadt_vector_resize(vec, 3, nullptr) == CADT_ERR_INVALID);
    cadt_vector_destroy(vec);

    osize = 0;
    type = type_int();
    vec = cadt_vector_create(&type, &alloc);
    for (i = 0; i < 300; i++) {
        uint32_t roll = test_rnd(&rng) % 4u;
        if (roll == 0 && osize < 48) {
            value = (int)(test_rnd(&rng) % 1000u);
            CHECK(cadt_vector_push_back(vec, &value) == CADT_OK);
            oracle[osize++] = value;
        } else if (roll == 1 && osize > 0) {
            CHECK(cadt_vector_pop_back(vec, nullptr) == CADT_OK);
            osize--;
        } else if (roll == 2 && osize < 48) {
            size_t index = (size_t)(test_rnd(&rng) % (uint32_t)(osize + 1u));
            value = (int)(test_rnd(&rng) % 1000u);
            CHECK(cadt_vector_insert(vec, index, &value) == CADT_OK);
            memmove(oracle + index + 1, oracle + index, (osize - index) * sizeof(int));
            oracle[index] = value;
            osize++;
        } else if (osize > 0) {
            size_t index = (size_t)(test_rnd(&rng) % (uint32_t)osize);
            CHECK(cadt_vector_erase(vec, index, nullptr) == CADT_OK);
            memmove(oracle + index, oracle + index + 1, (osize - index - 1u) * sizeof(int));
            osize--;
        }
        expect_vector(vec, oracle, osize);
    }
    cadt_vector_destroy(vec);

    plain = cadt_vector_create(&type, nullptr);
    value = 1;
    CHECK(plain != nullptr && cadt_vector_push_back(plain, &value) == CADT_OK);
    cadt_vector_destroy(plain);
    cadt_vector_destroy(nullptr);
    CHECK(!track_leaked(&state));
}
