#include "test.h"

#include <stdlib.h>

enum { KEYS = 48, SET_OPS = 2000 };

static int g_live_strings;

static cadt_status copy_cstr(void *dst, const void *src, [[maybe_unused]] void *ctx)
{
    const char *in = *(const char *const *)src;
    size_t length = strlen(in) + 1u;
    char *copy = (char *)malloc(length);
    if (copy == nullptr) {
        return CADT_ERR_NOMEM;
    }
    memcpy(copy, in, length);
    memcpy(dst, &copy, sizeof(copy));
    g_live_strings++;
    return CADT_OK;
}

static void destroy_cstr(void *elem, [[maybe_unused]] void *ctx)
{
    free(*(char **)elem);
    g_live_strings--;
}

static int cmp_cstr(const void *a, const void *b, [[maybe_unused]] void *ctx)
{
    return strcmp(*(const char *const *)a, *(const char *const *)b);
}

static uint64_t hash_cstr(const void *elem, [[maybe_unused]] void *ctx)
{
    const char *text = *(const char *const *)elem;
    return cadt_hash_bytes(text, strlen(text));
}

static uint64_t hash_const([[maybe_unused]] const void *elem, [[maybe_unused]] void *ctx)
{
    return 1u;
}

static size_t count_present(const int *present)
{
    size_t i;
    size_t n = 0;

    for (i = 0; i < KEYS; i++) {
        n += (size_t)present[i];
    }
    return n;
}

static void check_set_order(cadt_set *set, const int *present)
{
    cadt_set_iter it;
    int previous = -1;
    size_t seen = 0;
    int key;

    CHECK(cadt_set_validate(set) == CADT_OK);
    CHECK(cadt_set_size(set) == count_present(present));
    for (it = cadt_set_begin(set); !cadt_set_iter_eq(it, cadt_set_end(set)); it = cadt_set_next(it)) {
        key = *(int *)cadt_set_iter_data(it);
        CHECK(key > previous);
        CHECK(present[key] == 1);
        previous = key;
        seen++;
    }
    CHECK(seen == cadt_set_size(set));
    if (cadt_set_size(set) > 0) {
        CHECK(*(int *)cadt_set_min(set) == *(int *)cadt_set_iter_data(cadt_set_begin(set)));
        CHECK(*(int *)cadt_set_max(set) == *(int *)cadt_set_iter_data(cadt_set_prev(cadt_set_end(set))));
    }
}

void test_set(void)
{
    track_state state;
    cadt_allocator alloc;
    cadt_type type;
    cadt_set *set;
    cadt_set *multi;
    int present[KEYS];
    int key;
    int out;
    int i;
    uint32_t rng = 7;
    cadt_set_iter it;
    size_t seen;

    track_init(&state, &alloc);
    type = type_int();
    type.compare = nullptr;
    CHECK(cadt_set_create(&type, &alloc) == nullptr);
    type = type_int();
    set = cadt_set_create(&type, &alloc);
    CHECK(set != nullptr && cadt_set_validate(set) == CADT_OK);
    memset(present, 0, sizeof(present));
    for (i = 0; i < SET_OPS; i++) {
        key = (int)(test_rnd(&rng) % (uint32_t)KEYS);
        if ((test_rnd(&rng) & 1u) == 0u) {
            cadt_status status = cadt_set_insert(set, &key);
            if (present[key]) {
                CHECK(status == CADT_ERR_EXISTS);
            } else {
                CHECK(status == CADT_OK);
                present[key] = 1;
            }
        } else {
            size_t removed = cadt_set_erase(set, &key);
            CHECK(removed == (present[key] ? 1u : 0u));
            present[key] = 0;
        }
        if ((i % 25) == 0) {
            check_set_order(set, present);
        }
    }
    check_set_order(set, present);
    for (key = 0; key < KEYS; key++) {
        int probe = key;
        CHECK(cadt_set_contains(set, &probe) == (present[key] != 0));
        if (!present[key]) {
            it = cadt_set_lower_bound(set, &probe);
            if (!cadt_set_iter_eq(it, cadt_set_end(set))) {
                CHECK(*(int *)cadt_set_iter_data(it) > probe);
            }
        }
    }
    cadt_set_clear(set);
    CHECK(cadt_set_empty(set));
    for (i = 0; i < 100; i++) {
        key = i;
        CHECK(cadt_set_insert(set, &key) == CADT_OK);
    }
    for (i = 99; i >= 0; i--) {
        key = i;
        CHECK(cadt_set_insert(set, &key) == CADT_ERR_EXISTS);
    }
    CHECK(cadt_set_validate(set) == CADT_OK);
    CHECK(cadt_set_size(set) == 100);
    key = 0;
    CHECK(*(int *)cadt_set_iter_data(cadt_set_lower_bound(set, &key)) == 0);
    key = 50;
    CHECK(*(int *)cadt_set_iter_data(cadt_set_lower_bound(set, &key)) == 50);
    CHECK(*(int *)cadt_set_iter_data(cadt_set_upper_bound(set, &key)) == 51);
    key = 1000;
    CHECK(cadt_set_iter_eq(cadt_set_lower_bound(set, &key), cadt_set_end(set)));
    for (i = 0; i < 100; i += 2) {
        key = i;
        CHECK(cadt_set_erase_one(set, &key, &out) == CADT_OK);
        CHECK(out == i);
    }
    CHECK(cadt_set_size(set) == 50);
    CHECK(cadt_set_validate(set) == CADT_OK);
    seen = 0;
    for (it = cadt_set_begin(set); !cadt_set_iter_eq(it, cadt_set_end(set)); it = cadt_set_next(it)) {
        CHECK((*(int *)cadt_set_iter_data(it) % 2) == 1);
        seen++;
    }
    CHECK(seen == 50);
    cadt_set_destroy(set);

    multi = cadt_multiset_create(&type, &alloc);
    CHECK(cadt_set_is_multi(multi));
    for (i = 0; i < 30; i++) {
        key = i % 10;
        CHECK(cadt_set_insert(multi, &key) == CADT_OK);
    }
    CHECK(cadt_set_size(multi) == 30);
    CHECK(cadt_set_validate(multi) == CADT_OK);
    key = 3;
    CHECK(cadt_set_count(multi, &key) == 3);
    CHECK(cadt_set_erase(multi, &key) == 3);
    CHECK(cadt_set_count(multi, &key) == 0);
    CHECK(cadt_set_size(multi) == 27);
    it = cadt_set_begin(multi);
    key = *(int *)cadt_set_iter_data(it);
    while (!cadt_set_iter_eq(it, cadt_set_end(multi))) {
        int current = *(int *)cadt_set_iter_data(it);
        CHECK(current >= key);
        key = current;
        it = cadt_set_next(it);
    }
    cadt_set_destroy(multi);
    CHECK(!track_leaked(&state));
}

void test_map(void)
{
    track_state state;
    cadt_allocator alloc;
    cadt_type key_type;
    cadt_type value_type;
    cadt_map *map;
    cadt_map *multi;
    int stored[KEYS];
    int present[KEYS];
    int key;
    int value;
    int i;
    uint32_t rng = 11;
    cadt_map_iter it;
    int previous;
    cadt_type str_key;
    const char *word;
    char *owned;

    track_init(&state, &alloc);
    key_type = type_int();
    value_type = type_int();
    key_type.compare = nullptr;
    CHECK(cadt_map_create(&key_type, &value_type, &alloc) == nullptr);
    key_type = type_int();
    map = cadt_map_create(&key_type, &value_type, &alloc);
    CHECK(map != nullptr);
    memset(present, 0, sizeof(present));
    for (i = 0; i < 1500; i++) {
        key = (int)(test_rnd(&rng) % (uint32_t)KEYS);
        value = (int)test_rnd(&rng);
        if ((test_rnd(&rng) % 5u) == 0u) {
            CHECK(cadt_map_erase(map, &key) == (present[key] ? 1u : 0u));
            present[key] = 0;
        } else {
            CHECK(cadt_map_put(map, &key, &value) == CADT_OK);
            present[key] = 1;
            stored[key] = value;
        }
        if ((i % 40) == 0) {
            CHECK(cadt_map_validate(map) == CADT_OK);
        }
    }
    CHECK(cadt_map_validate(map) == CADT_OK);
    previous = -1;
    for (it = cadt_map_begin(map); !cadt_map_iter_eq(it, cadt_map_end(map)); it = cadt_map_next(it)) {
        int got = *(int *)cadt_map_iter_key(map, it);
        CHECK(got > previous);
        CHECK(present[got] == 1);
        CHECK(*(int *)cadt_map_iter_value(map, it) == stored[got]);
        previous = got;
    }
    for (key = 0; key < KEYS; key++) {
        int *slot = (int *)cadt_map_get(map, &key);
        if (!present[key]) {
            CHECK(slot == nullptr);
        } else {
            CHECK(slot != nullptr && *slot == stored[key]);
        }
    }
    key = 1;
    value = 5;
    CHECK(cadt_map_put(map, &key, &value) == CADT_OK);
    value = 9;
    CHECK(cadt_map_put(map, &key, &value) == CADT_OK);
    CHECK(*(int *)cadt_map_get(map, &key) == 9);
    CHECK(cadt_map_count(map, &key) == 1);
    cadt_map_destroy(map);

    multi = cadt_multimap_create(&key_type, &value_type, &alloc);
    CHECK(cadt_map_is_multi(multi));
    CHECK(cadt_map_put(multi, &key, &value) == CADT_ERR_INVALID);
    value = 10;
    CHECK(cadt_map_insert(multi, &key, &value) == CADT_OK);
    value = 20;
    CHECK(cadt_map_insert(multi, &key, &value) == CADT_OK);
    key = 2;
    value = 30;
    CHECK(cadt_map_insert(multi, &key, &value) == CADT_OK);
    CHECK(cadt_map_size(multi) == 3);
    CHECK(cadt_map_count(multi, &key) == 1);
    key = 1;
    CHECK(cadt_map_count(multi, &key) == 2);
    CHECK(cadt_map_validate(multi) == CADT_OK);
    it = cadt_map_lower_bound(multi, &key);
    CHECK(*(int *)cadt_map_iter_value(multi, it) == 10);
    it = cadt_map_next(it);
    CHECK(*(int *)cadt_map_iter_value(multi, it) == 20);
    it = cadt_map_upper_bound(multi, &key);
    CHECK(*(int *)cadt_map_iter_key(multi, it) == 2);
    CHECK(cadt_map_erase(multi, &key) == 2);
    CHECK(cadt_map_size(multi) == 1);
    cadt_map_destroy(multi);

    str_key = (cadt_type){
        .size = sizeof(char *),
        .copy = copy_cstr,
        .destroy = destroy_cstr,
        .compare = cmp_cstr,
    };
    g_live_strings = 0;
    map = cadt_map_create(&str_key, &value_type, &alloc);
    word = "red";
    value = 1;
    CHECK(cadt_map_insert(map, &word, &value) == CADT_OK);
    word = "blue";
    value = 2;
    CHECK(cadt_map_insert(map, &word, &value) == CADT_OK);
    word = "red";
    CHECK(cadt_map_insert(map, &word, &value) == CADT_ERR_EXISTS);
    owned = *(char **)cadt_map_iter_key(map, cadt_map_begin(map));
    CHECK(strcmp(owned, "blue") == 0);
    CHECK(*(int *)cadt_map_get(map, &word) == 1);
    cadt_map_destroy(map);
    CHECK(g_live_strings == 0);
    CHECK(!track_leaked(&state));
}

void test_unordered(void)
{
    track_state state;
    cadt_allocator alloc;
    cadt_type type;
    cadt_type collided;
    cadt_uset *set;
    cadt_uset *multi;
    cadt_umap *map;
    int present[200];
    int key;
    int value;
    int i;
    uint32_t rng = 99;
    size_t seen;
    int visits[200];
    cadt_uset_iter it;
    cadt_type str_key;
    cadt_type value_type;
    const char *word;

    track_init(&state, &alloc);
    type = type_int();
    set = cadt_uset_create(&type, &alloc);
    CHECK(set != nullptr);
    memset(present, 0, sizeof(present));
    for (i = 0; i < 200; i++) {
        key = i;
        CHECK(cadt_uset_insert(set, &key) == CADT_OK);
        present[i] = 1;
    }
    CHECK(cadt_uset_size(set) == 200);
    CHECK(cadt_uset_bucket_count(set) > 8);
    CHECK(cadt_uset_validate(set) == CADT_OK);
    key = 50;
    CHECK(cadt_uset_insert(set, &key) == CADT_ERR_EXISTS);
    memset(visits, 0, sizeof(visits));
    seen = 0;
    for (it = cadt_uset_begin(set); !cadt_uset_iter_eq(it, cadt_uset_end(set)); it = cadt_uset_next(it)) {
        int got = *(int *)cadt_uset_iter_data(it);
        CHECK(got >= 0 && got < 200 && visits[got] == 0);
        visits[got] = 1;
        seen++;
    }
    CHECK(seen == 200);
    for (i = 0; i < 200; i += 2) {
        key = i;
        CHECK(cadt_uset_erase(set, &key) == 1);
        present[i] = 0;
    }
    CHECK(cadt_uset_size(set) == 100);
    CHECK(cadt_uset_validate(set) == CADT_OK);
    for (i = 0; i < 200; i++) {
        key = i;
        CHECK(cadt_uset_contains(set, &key) == (present[i] != 0));
    }
    cadt_uset_destroy(set);

    collided = type_int();
    collided.hash = hash_const;
    set = cadt_uset_create(&collided, &alloc);
    for (i = 0; i < 40; i++) {
        key = i;
        CHECK(cadt_uset_insert(set, &key) == CADT_OK);
    }
    CHECK(cadt_uset_validate(set) == CADT_OK);
    CHECK(cadt_uset_bucket_count(set) >= 8);
    for (i = 0; i < 40; i++) {
        key = i;
        CHECK(cadt_uset_find(set, &key) != nullptr);
    }
    key = 7;
    CHECK(cadt_uset_erase(set, &key) == 1);
    CHECK(!cadt_uset_contains(set, &key));
    CHECK(cadt_uset_validate(set) == CADT_OK);
    cadt_uset_destroy(set);

    multi = cadt_umultiset_create(&type, &alloc);
    key = 4;
    CHECK(cadt_uset_insert(multi, &key) == CADT_OK);
    CHECK(cadt_uset_insert(multi, &key) == CADT_OK);
    CHECK(cadt_uset_count(multi, &key) == 2);
    CHECK(cadt_uset_is_multi(multi));
    CHECK(cadt_uset_erase(multi, &key) == 2);
    CHECK(cadt_uset_empty(multi));
    cadt_uset_destroy(multi);

    value_type = type_int();
    map = cadt_umap_create(&type, &value_type, &alloc);
    for (i = 0; i < 80; i++) {
        key = (int)(test_rnd(&rng) % 30u);
        value = i;
        CHECK(cadt_umap_put(map, &key, &value) == CADT_OK);
        CHECK(*(int *)cadt_umap_get(map, &key) == i);
    }
    CHECK(cadt_umap_validate(map) == CADT_OK);
    CHECK(cadt_umap_size(map) <= 30);
    key = 1000;
    CHECK(cadt_umap_get(map, &key) == nullptr);
    cadt_umap_destroy(map);

    str_key = (cadt_type){
        .size = sizeof(char *),
        .copy = copy_cstr,
        .destroy = destroy_cstr,
        .compare = cmp_cstr,
        .hash = hash_cstr,
    };
    g_live_strings = 0;
    map = cadt_umap_create(&str_key, &value_type, &alloc);
    word = "alpha";
    value = 3;
    CHECK(cadt_umap_insert(map, &word, &value) == CADT_OK);
    word = "beta";
    value = 4;
    CHECK(cadt_umap_insert(map, &word, &value) == CADT_OK);
    word = "alpha";
    value = 8;
    CHECK(cadt_umap_put(map, &word, &value) == CADT_OK);
    CHECK(*(int *)cadt_umap_get(map, &word) == 8);
    CHECK(cadt_umap_size(map) == 2);
    cadt_umap_destroy(map);
    CHECK(g_live_strings == 0);
    CHECK(!track_leaked(&state));
}
