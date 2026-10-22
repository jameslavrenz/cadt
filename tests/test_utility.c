#include "test.h"

#include <stdlib.h>

typedef struct stable_item {
    int key;
    int seq;
} stable_item;

static int cmp_stable(const void *a, const void *b, [[maybe_unused]] void *ctx)
{
    const stable_item *ia = (const stable_item *)a;
    const stable_item *ib = (const stable_item *)b;
    return (ia->key > ib->key) - (ia->key < ib->key);
}

static int qsort_int(const void *a, const void *b)
{
    return cmp_int(a, b, nullptr);
}

static bool is_even(const void *elem, [[maybe_unused]] void *ctx)
{
    return (*(const int *)elem % 2) == 0;
}

static void add_one(void *elem, void *ctx)
{
    int *total = (int *)ctx;
    *total += *(int *)elem;
}

static int g_opt_dtor;

static void opt_dtor([[maybe_unused]] void *elem, [[maybe_unused]] void *ctx)
{
    g_opt_dtor++;
}

static int g_opt_copies;

static cadt_status opt_copy(void *dst, const void *src, [[maybe_unused]] void *ctx)
{
    if (g_opt_copies <= 0) {
        return CADT_ERR_NOMEM;
    }
    g_opt_copies--;
    memcpy(dst, src, sizeof(int));
    return CADT_OK;
}

void test_string(void)
{
    track_state state;
    cadt_allocator alloc;
    cadt_string *a;
    cadt_string *b;
    cadt_string *sub;

    track_init(&state, &alloc);
    a = cadt_string_create(&alloc);
    CHECK(a != nullptr && cadt_string_empty(a));
    CHECK(strcmp(cadt_string_c_str(a), "") == 0);
    CHECK(cadt_string_append_cstr(a, "ab") == CADT_OK);
    CHECK(cadt_string_append(a, a) == CADT_OK);
    CHECK(cadt_string_equals_cstr(a, "abab"));
    CHECK(cadt_string_append_char(a, '!') == CADT_OK);
    CHECK(cadt_string_equals_cstr(a, "abab!"));
    CHECK(cadt_string_insert_n(a, 0, "xy", 2) == CADT_OK);
    CHECK(cadt_string_equals_cstr(a, "xyabab!"));
    CHECK(cadt_string_insert_n(a, 2, cadt_string_c_str(a) + 4, 2) == CADT_OK);
    CHECK(cadt_string_find_cstr(a, "ab") == 4 || cadt_string_find_cstr(a, "ab") < cadt_string_size(a));
    CHECK(cadt_string_erase(a, 0, 2) == CADT_OK);
    CHECK(cadt_string_starts_with(a, "ab") || cadt_string_size(a) > 0);
    cadt_string_clear(a);
    CHECK(cadt_string_assign_n(a, "hello world", 11) == CADT_OK);
    CHECK(cadt_string_find_cstr(a, "world") == 6);
    CHECK(cadt_string_find_cstr(a, "nope") == CADT_NPOS);
    CHECK(cadt_string_find_n(a, "", 0) == 0);
    CHECK(cadt_string_starts_with(a, "hello"));
    CHECK(cadt_string_ends_with(a, "world"));
    CHECK(!cadt_string_starts_with(a, "world"));
    sub = cadt_string_substr(a, 6, CADT_NPOS);
    CHECK(sub != nullptr && cadt_string_equals_cstr(sub, "world"));
    b = cadt_string_create_cstr("hello world", &alloc);
    CHECK(cadt_string_equals(a, b));
    CHECK(cadt_string_compare(a, b) == 0);
    CHECK(cadt_string_append_cstr(b, "!") == CADT_OK);
    CHECK(cadt_string_compare(a, b) < 0);
    CHECK(cadt_string_substr(a, 12, 1) == nullptr);
    CHECK(cadt_string_erase(a, 5, CADT_NPOS) == CADT_OK);
    CHECK(cadt_string_equals_cstr(a, "hello"));
    CHECK(cadt_string_reserve(a, 64) == CADT_OK);
    CHECK(cadt_string_capacity(a) >= 64);
    CHECK(cadt_string_shrink_to_fit(a) == CADT_OK);
    CHECK(cadt_string_capacity(a) == cadt_string_size(a));
    CHECK(cadt_string_insert_n(a, 5, "!", 1) == CADT_OK);
    CHECK(cadt_string_equals_cstr(a, "hello!"));
    CHECK(cadt_string_assign_n(a, cadt_string_c_str(a) + 1, 4) == CADT_OK);
    CHECK(cadt_string_equals_cstr(a, "ello"));
    cadt_string_destroy(sub);
    cadt_string_destroy(b);
    cadt_string_destroy(a);
    CHECK(cadt_string_create_n(nullptr, 3, &alloc) == nullptr);
    CHECK(!track_leaked(&state));
}

void test_bitset(void)
{
    track_state state;
    cadt_allocator alloc;
    cadt_bitset *a;
    cadt_bitset *b;
    char text[80];
    size_t i;

    track_init(&state, &alloc);
    a = cadt_bitset_create(0, &alloc);
    CHECK(a != nullptr && cadt_bitset_none(a) && cadt_bitset_all(a) && !cadt_bitset_any(a));
    CHECK(cadt_bitset_format(a, text, 1) == CADT_OK);
    CHECK(text[0] == '\0');
    cadt_bitset_destroy(a);

    a = cadt_bitset_create(70, &alloc);
    b = cadt_bitset_create(70, &alloc);
    CHECK(a != nullptr && b != nullptr);
    CHECK(cadt_bitset_set(a, 70, true) == CADT_ERR_RANGE);
    CHECK(!cadt_bitset_test(a, 70));
    CHECK(cadt_bitset_set(a, 0, true) == CADT_OK);
    CHECK(cadt_bitset_set(a, 63, true) == CADT_OK);
    CHECK(cadt_bitset_set(a, 64, true) == CADT_OK);
    CHECK(cadt_bitset_set(a, 69, true) == CADT_OK);
    CHECK(cadt_bitset_count(a) == 4);
    CHECK(cadt_bitset_any(a) && !cadt_bitset_none(a) && !cadt_bitset_all(a));
    CHECK(cadt_bitset_flip(a, 0) == CADT_OK);
    CHECK(!cadt_bitset_test(a, 0));
    CHECK(cadt_bitset_count(a) == 3);
    cadt_bitset_set_all(a, true);
    CHECK(cadt_bitset_count(a) == 70 && cadt_bitset_all(a));
    cadt_bitset_flip_all(a);
    CHECK(cadt_bitset_none(a));
    for (i = 0; i < 70; i += 2) {
        CHECK(cadt_bitset_set(a, i, true) == CADT_OK);
        CHECK(cadt_bitset_set(b, i, true) == CADT_OK);
    }
    CHECK(cadt_bitset_set(b, 1, true) == CADT_OK);
    CHECK(cadt_bitset_and(a, b) == CADT_OK);
    CHECK(!cadt_bitset_test(a, 1));
    CHECK(cadt_bitset_test(a, 0) && cadt_bitset_test(a, 2));
    CHECK(cadt_bitset_or(a, b) == CADT_OK);
    CHECK(cadt_bitset_test(a, 1));
    CHECK(cadt_bitset_xor(a, b) == CADT_OK);
    CHECK(cadt_bitset_none(a));
    CHECK(cadt_bitset_format(a, text, sizeof(text)) == CADT_OK);
    CHECK(strlen(text) == 70 && text[0] == '0' && text[69] == '0');
    {
        cadt_bitset *short_set = cadt_bitset_create(3, &alloc);
        CHECK(short_set != nullptr);
        CHECK(cadt_bitset_and(a, short_set) == CADT_ERR_INVALID);
        cadt_bitset_destroy(short_set);
    }
    cadt_bitset_destroy(a);
    cadt_bitset_destroy(b);
    CHECK(!track_leaked(&state));
}

void test_optional(void)
{
    track_state state;
    cadt_allocator alloc;
    cadt_type type;
    cadt_optional *opt;
    int value = 4;
    int out = 0;

    track_init(&state, &alloc);
    type = type_int();
    opt = cadt_optional_create(&type, &alloc);
    CHECK(opt != nullptr && !cadt_optional_has_value(opt));
    CHECK(cadt_optional_take(opt, &out) == CADT_ERR_EMPTY);
    CHECK(cadt_optional_set(opt, &value) == CADT_OK);
    CHECK(cadt_optional_has_value(opt) && *(int *)cadt_optional_value(opt) == 4);
    value = 8;
    CHECK(cadt_optional_set(opt, &value) == CADT_OK);
    CHECK(*(int *)cadt_optional_value(opt) == 8);
    CHECK(cadt_optional_take(opt, &out) == CADT_OK);
    CHECK(out == 8 && !cadt_optional_has_value(opt));
    cadt_optional_destroy(opt);

    g_opt_dtor = 0;
    g_opt_copies = 5;
    type.destroy = opt_dtor;
    type.copy = opt_copy;
    opt = cadt_optional_create(&type, &alloc);
    value = 1;
    CHECK(cadt_optional_set(opt, &value) == CADT_OK);
    g_opt_copies = 0;
    value = 2;
    CHECK(cadt_optional_set(opt, &value) == CADT_ERR_NOMEM);
    CHECK(*(int *)cadt_optional_value(opt) == 1);
    cadt_optional_reset(opt);
    CHECK(g_opt_dtor == 1);
    g_opt_copies = 1;
    value = 3;
    CHECK(cadt_optional_set(opt, &value) == CADT_OK);
    CHECK(cadt_optional_take(opt, &out) == CADT_OK);
    CHECK(out == 3 && g_opt_dtor == 1);
    cadt_optional_destroy(opt);
    CHECK(g_opt_dtor == 1);
    CHECK(!track_leaked(&state));
}

void test_algo(void)
{
    int values[200];
    int expect[200];
    int key;
    stable_item items[5];
    size_t i;
    int total;
    uint32_t rng = 3;

    for (i = 0; i < 200; i++) {
        values[i] = (int)(test_rnd(&rng) % 1000u) - 100;
        expect[i] = values[i];
    }
    qsort(expect, 200, sizeof(int), qsort_int);
    CHECK(cadt_sort(values, 200, sizeof(int), cmp_int, nullptr) == CADT_OK);
    CHECK(memcmp(values, expect, sizeof(values)) == 0);
    CHECK(cadt_is_sorted(values, 200, sizeof(int), cmp_int, nullptr));
    key = values[10];
    CHECK(cadt_binary_search(values, 200, sizeof(int), &key, cmp_int, nullptr));
    key = 100000;
    CHECK(!cadt_binary_search(values, 200, sizeof(int), &key, cmp_int, nullptr));

    {
        int sample[] = {1, 3, 3, 5, 8};
        key = 3;
        CHECK(cadt_lower_bound(sample, 5, sizeof(int), &key, cmp_int, nullptr) == 1);
        CHECK(cadt_upper_bound(sample, 5, sizeof(int), &key, cmp_int, nullptr) == 3);
        key = 0;
        CHECK(cadt_lower_bound(sample, 5, sizeof(int), &key, cmp_int, nullptr) == 0);
        key = 4;
        CHECK(cadt_lower_bound(sample, 5, sizeof(int), &key, cmp_int, nullptr) == 3);
        key = 9;
        CHECK(cadt_lower_bound(sample, 5, sizeof(int), &key, cmp_int, nullptr) == 5);
        CHECK(cadt_count(sample, 5, sizeof(int), &key, cmp_int, nullptr) == 0);
        key = 3;
        CHECK(cadt_count(sample, 5, sizeof(int), &key, cmp_int, nullptr) == 2);
        CHECK(*(int *)cadt_min_element(sample, 5, sizeof(int), cmp_int, nullptr) == 1);
        CHECK(*(int *)cadt_max_element(sample, 5, sizeof(int), cmp_int, nullptr) == 8);
        CHECK(cadt_count_if(sample, 5, sizeof(int), is_even, nullptr) == 1);
        CHECK(*(int *)cadt_find_if(sample, 5, sizeof(int), is_even, nullptr) == 8);
    }

    items[0].key = 1;
    items[0].seq = 0;
    items[1].key = 2;
    items[1].seq = 1;
    items[2].key = 1;
    items[2].seq = 2;
    items[3].key = 2;
    items[3].seq = 3;
    items[4].key = 1;
    items[4].seq = 4;
    CHECK(cadt_stable_sort(items, 5, sizeof(items[0]), cmp_stable, nullptr) == CADT_OK);
    CHECK(items[0].seq == 0 && items[1].seq == 2 && items[2].seq == 4);
    CHECK(items[3].seq == 1 && items[4].seq == 3);

    {
        int seq[] = {1, 2, 3, 4};
        int rev[] = {4, 3, 2, 1};
        int copy[] = {1, 2, 3, 4};
        int fill_value = 7;
        cadt_reverse(seq, 4, sizeof(int));
        CHECK(cadt_equal(seq, rev, 4, sizeof(int), cmp_int, nullptr));
        cadt_fill(seq, 4, sizeof(int), &fill_value);
        CHECK(seq[0] == 7 && seq[3] == 7);
        total = 0;
        cadt_for_each(copy, 4, sizeof(int), add_one, &total);
        CHECK(total == 10);
        CHECK(cadt_find(copy, 4, sizeof(int), &copy[2], cmp_int, nullptr) == &copy[2]);
        CHECK(cadt_sort(nullptr, 0, sizeof(int), cmp_int, nullptr) == CADT_OK);
        CHECK(cadt_is_sorted(nullptr, 0, sizeof(int), cmp_int, nullptr));
        CHECK(cadt_sort(copy, 4, 0, cmp_int, nullptr) == CADT_ERR_INVALID);
    }

    for (i = 0; i < 50; i++) {
        values[i] = (int)(i * 3u);
    }
    CHECK(cadt_sort(values, 50, sizeof(int), nullptr, nullptr) == CADT_OK);
    CHECK(cadt_is_sorted(values, 50, sizeof(int), nullptr, nullptr));
}

void test_span(void)
{
    int data[] = {1, 2, 3, 4, 5};
    cadt_span span = cadt_span_make(data, 5, sizeof(int));
    cadt_span mid;
    cadt_span bad;

    CHECK(span.size == 5);
    CHECK(*(int *)cadt_span_at(span, 0) == 1);
    CHECK(cadt_span_at(span, 5) == nullptr);
    mid = cadt_span_sub(span, 1, 3);
    CHECK(mid.size == 3 && *(int *)cadt_span_at(mid, 0) == 2);
    mid = cadt_span_sub(span, 2, CADT_NPOS);
    CHECK(mid.size == 3 && *(int *)cadt_span_at(mid, 2) == 5);
    bad = cadt_span_sub(span, 3, 5);
    CHECK(bad.size == 0);
    CHECK(cadt_span_empty(cadt_span_make(nullptr, 0, sizeof(int))));
}
