#include "test.h"

typedef struct pair_item {
    int key;
    int seq;
} pair_item;

static int cmp_pair(const void *a, const void *b, [[maybe_unused]] void *ctx)
{
    const pair_item *pa = (const pair_item *)a;
    const pair_item *pb = (const pair_item *)b;
    return (pa->key > pb->key) - (pa->key < pb->key);
}

static void expect_list(cadt_list *list, const int *values, size_t count)
{
    cadt_list_iter it;
    size_t i = 0;

    CHECK(cadt_list_size(list) == count);
    for (it = cadt_list_begin(list); !cadt_list_iter_eq(it, cadt_list_end(list));
         it = cadt_list_next(it)) {
        CHECK(i < count);
        CHECK(*(int *)cadt_list_iter_data(it) == values[i]);
        i++;
    }
    CHECK(i == count);
    CHECK(cadt_list_iter_data(cadt_list_end(list)) == nullptr);
}

void test_list(void)
{
    track_state state;
    cadt_allocator alloc;
    cadt_type type;
    cadt_list *list;
    cadt_list_iter it;
    cadt_list_iter next;
    int values[6];
    int out;
    int i;
    pair_item items[4];
    cadt_type pairs;

    track_init(&state, &alloc);
    type = type_int();
    list = cadt_list_create(&type, &alloc);
    CHECK(list != nullptr);
    CHECK(cadt_list_pop_front(list, nullptr) == CADT_ERR_EMPTY);
    for (i = 0; i < 5; i++) {
        values[i] = i;
        if (i % 2 == 0) {
            CHECK(cadt_list_push_back(list, &values[i]) == CADT_OK);
        } else {
            CHECK(cadt_list_push_front(list, &values[i]) == CADT_OK);
        }
    }
    /* pushes: back 0, front 1, back 2, front 3, back 4 -> 3,1,0,2,4 */
    {
        int expect[] = {3, 1, 0, 2, 4};
        expect_list(list, expect, 5);
        CHECK(*(int *)cadt_list_front(list) == 3);
        CHECK(*(int *)cadt_list_back(list) == 4);
    }
    it = cadt_list_begin(list);
    it = cadt_list_next(it);
    values[0] = 9;
    CHECK(cadt_list_insert(list, it, &values[0], &next) == CADT_OK);
    CHECK(*(int *)cadt_list_iter_data(next) == 9);
    CHECK(cadt_list_erase(list, next, &out, &it) == CADT_OK);
    CHECK(out == 9);
    CHECK(*(int *)cadt_list_iter_data(it) == 1);
    cadt_list_reverse(list);
    {
        int expect[] = {4, 2, 0, 1, 3};
        expect_list(list, expect, 5);
        it = cadt_list_end(list);
        it = cadt_list_prev(it);
        CHECK(*(int *)cadt_list_iter_data(it) == 3);
        it = cadt_list_prev(it);
        CHECK(*(int *)cadt_list_iter_data(it) == 1);
    }
    values[0] = 0;
    CHECK(cadt_list_find(list, &values[0]) != nullptr);
    cadt_list_sort(list);
    {
        int expect[] = {0, 1, 2, 3, 4};
        expect_list(list, expect, 5);
    }
    cadt_list_destroy(list);

    memset(&pairs, 0, sizeof(pairs));
    pairs.size = sizeof(pair_item);
    pairs.compare = cmp_pair;
    list = cadt_list_create(&pairs, &alloc);
    items[0].key = 2;
    items[0].seq = 0;
    items[1].key = 1;
    items[1].seq = 1;
    items[2].key = 2;
    items[2].seq = 2;
    items[3].key = 1;
    items[3].seq = 3;
    for (i = 0; i < 4; i++) {
        CHECK(cadt_list_push_back(list, &items[i]) == CADT_OK);
    }
    cadt_list_sort(list);
    it = cadt_list_begin(list);
    CHECK(((pair_item *)cadt_list_iter_data(it))->seq == 1);
    it = cadt_list_next(it);
    CHECK(((pair_item *)cadt_list_iter_data(it))->seq == 3);
    it = cadt_list_next(it);
    CHECK(((pair_item *)cadt_list_iter_data(it))->seq == 0);
    it = cadt_list_next(it);
    CHECK(((pair_item *)cadt_list_iter_data(it))->seq == 2);
    cadt_list_clear(list);
    CHECK(cadt_list_empty(list));
    cadt_list_destroy(list);
    CHECK(!track_leaked(&state));
}

void test_forward_list(void)
{
    track_state state;
    cadt_allocator alloc;
    cadt_type type;
    cadt_flist *list;
    cadt_flist_iter it;
    cadt_flist_iter inserted;
    int value;
    int out;
    int i;

    track_init(&state, &alloc);
    type = type_int();
    list = cadt_flist_create(&type, &alloc);
    CHECK(list != nullptr);
    CHECK(cadt_flist_empty(list));
    CHECK(cadt_flist_iter_eq(cadt_flist_begin(list), cadt_flist_end(list)));
    CHECK(cadt_flist_pop_front(list, nullptr) == CADT_ERR_EMPTY);
    for (i = 1; i <= 4; i++) {
        value = i;
        CHECK(cadt_flist_push_front(list, &value) == CADT_OK);
    }
    /* 4,3,2,1 */
    CHECK(*(int *)cadt_flist_front(list) == 4);
    it = cadt_flist_begin(list);
    value = 9;
    CHECK(cadt_flist_insert_after(list, it, &value, &inserted) == CADT_OK);
    CHECK(*(int *)cadt_flist_iter_data(inserted) == 9);
    CHECK(cadt_flist_erase_after(list, it, &out) == CADT_OK);
    CHECK(out == 9);
    value = 8;
    CHECK(cadt_flist_insert_after(list, cadt_flist_before_begin(list), &value, nullptr) == CADT_OK);
    CHECK(*(int *)cadt_flist_front(list) == 8);
    CHECK(cadt_flist_erase_after(list, cadt_flist_before_begin(list), &out) == CADT_OK);
    CHECK(out == 8);
    cadt_flist_reverse(list);
    /* was 4,3,2,1 reversed -> 1,2,3,4 */
    it = cadt_flist_begin(list);
    CHECK(*(int *)cadt_flist_iter_data(it) == 1);
    it = cadt_flist_next(list, it);
    CHECK(*(int *)cadt_flist_iter_data(it) == 2);
    cadt_flist_sort(list);
    it = cadt_flist_begin(list);
    CHECK(*(int *)cadt_flist_iter_data(it) == 1);
    it = cadt_flist_next(list, it);
    CHECK(*(int *)cadt_flist_iter_data(it) == 2);
    value = 3;
    CHECK(cadt_flist_find(list, &value) != nullptr);
    value = 100;
    CHECK(cadt_flist_find(list, &value) == nullptr);
    CHECK(cadt_flist_size(list) == 4);
    cadt_flist_clear(list);
    CHECK(cadt_flist_empty(list));
    cadt_flist_destroy(list);
    CHECK(!track_leaked(&state));
}

static int g_array_dtor;

static void array_dtor([[maybe_unused]] void *elem, [[maybe_unused]] void *ctx)
{
    g_array_dtor++;
}

void test_array(void)
{
    track_state state;
    cadt_allocator alloc;
    cadt_type type;
    cadt_array *arr;
    cadt_span span;
    int fill = 5;
    int value = 9;

    track_init(&state, &alloc);
    type = type_int();
    type.destroy = array_dtor;
    CHECK(cadt_array_create(&type, 3, nullptr, &alloc) == nullptr);
    g_array_dtor = 0;
    type = type_int();
    type.destroy = array_dtor;
    arr = cadt_array_create(&type, 4, &fill, &alloc);
    CHECK(arr != nullptr);
    CHECK(cadt_array_size(arr) == 4);
    CHECK(*(int *)cadt_array_front(arr) == 5);
    CHECK(*(int *)cadt_array_back(arr) == 5);
    CHECK(cadt_array_set(arr, 2, &value) == CADT_OK);
    CHECK(*(int *)cadt_array_at(arr, 2) == 9);
    CHECK(cadt_array_at(arr, 4) == nullptr);
    CHECK(cadt_array_set(arr, 4, &value) == CADT_ERR_RANGE);
    span = cadt_array_as_span(arr);
    CHECK(span.size == 4);
    CHECK(*(int *)cadt_span_at(span, 2) == 9);
    cadt_array_destroy(arr);
    CHECK(g_array_dtor == 5);

    type = type_int();
    arr = cadt_array_create(&type, 0, nullptr, &alloc);
    CHECK(arr != nullptr && cadt_array_empty(arr));
    cadt_array_destroy(arr);
    CHECK(!track_leaked(&state));
}
