/* cadt - C Abstract Data Types
 * SPDX-License-Identifier: MIT
 */
#include "cadt/algo.h"

#include "internal.h"

typedef struct sort_env {
    unsigned char *base;
    size_t elem_size;
    cadt_compare_fn cmp;
    void *ctx;
    unsigned char *tmp;
} sort_env;

static unsigned char *at_ptr(const sort_env *env, size_t index)
{
    return env->base + index * env->elem_size;
}

static int cmp_index(const sort_env *env, size_t i, size_t j)
{
    if (env->cmp != NULL) {
        return env->cmp(at_ptr(env, i), at_ptr(env, j), env->ctx);
    }
    return memcmp(at_ptr(env, i), at_ptr(env, j), env->elem_size);
}

static int cmp_key(const sort_env *env, size_t index, const void *key)
{
    if (env->cmp != NULL) {
        return env->cmp(at_ptr(env, index), key, env->ctx);
    }
    return memcmp(at_ptr(env, index), key, env->elem_size);
}

static void swap_bytes(unsigned char *a, unsigned char *b, size_t n)
{
    size_t i;

    for (i = 0; i < n; i++) {
        unsigned char tmp = a[i];
        a[i] = b[i];
        b[i] = tmp;
    }
}

static void swap_index(sort_env *env, size_t i, size_t j)
{
    if (i != j) {
        swap_bytes(at_ptr(env, i), at_ptr(env, j), env->elem_size);
    }
}

static void insertion_sort(sort_env *env, size_t lo, size_t hi)
{
    size_t i;

    if (hi <= lo) {
        return;
    }
    for (i = lo + 1u; i <= hi; i++) {
        size_t j = i;

        while (j > lo && cmp_index(env, j - 1u, j) > 0) {
            swap_index(env, j - 1u, j);
            j--;
        }
    }
}

static void sift_down(sort_env *env, size_t lo, size_t hi, size_t root)
{
    for (;;) {
        size_t offset = root - lo;
        size_t left;
        size_t right;
        size_t best = root;

        if (offset > (SIZE_MAX - 1u) / 2u) {
            break;
        }
        left = lo + (offset * 2u + 1u);
        if (left < lo || left > hi) {
            break;
        }
        if (cmp_index(env, left, best) > 0) {
            best = left;
        }
        right = left + 1u;
        if (right > left && right <= hi && cmp_index(env, right, best) > 0) {
            best = right;
        }
        if (best == root) {
            break;
        }
        swap_index(env, root, best);
        root = best;
    }
}

static void heapsort_range(sort_env *env, size_t lo, size_t hi)
{
    size_t count;
    size_t start;

    if (hi <= lo) {
        return;
    }
    count = hi - lo + 1u;
    start = lo + count / 2u;
    while (start > lo) {
        start--;
        sift_down(env, lo, hi, start);
    }
    while (hi > lo) {
        swap_index(env, lo, hi);
        hi--;
        sift_down(env, lo, hi, lo);
    }
}

static size_t partition(sort_env *env, size_t lo, size_t hi)
{
    size_t mid = lo + (hi - lo) / 2u;
    size_t i = lo;
    size_t j;

    if (cmp_index(env, lo, mid) > 0) {
        swap_index(env, lo, mid);
    }
    if (cmp_index(env, lo, hi) > 0) {
        swap_index(env, lo, hi);
    }
    if (cmp_index(env, mid, hi) > 0) {
        swap_index(env, mid, hi);
    }
    swap_index(env, mid, hi);
    for (j = lo; j < hi; j++) {
        if (cmp_index(env, j, hi) <= 0) {
            swap_index(env, i, j);
            i++;
        }
    }
    swap_index(env, i, hi);
    return i;
}

static int depth_limit(size_t count)
{
    int depth = 0;

    while (count > 1u) {
        count >>= 1u;
        depth++;
    }
    return depth * 2;
}

static void introsort(sort_env *env, size_t lo, size_t hi, int depth)
{
    while (hi > lo && hi - lo >= 16u) {
        size_t pivot;

        if (depth <= 0) {
            heapsort_range(env, lo, hi);
            return;
        }
        depth--;
        pivot = partition(env, lo, hi);
        if (pivot > lo) {
            introsort(env, lo, pivot - 1u, depth);
        }
        lo = pivot + 1u;
    }
}

static cadt_status prepare(sort_env *env, void *base, size_t count, size_t elem_size,
                           cadt_compare_fn cmp, void *ctx)
{
    if (elem_size == 0 || (base == NULL && count > 0)) {
        return CADT_ERR_INVALID;
    }
    env->base = (unsigned char *)base;
    env->elem_size = elem_size;
    env->cmp = cmp;
    env->ctx = ctx;
    env->tmp = NULL;
    return CADT_OK;
}

cadt_status cadt_sort(void *base, size_t count, size_t elem_size, cadt_compare_fn cmp, void *ctx)
{
    sort_env env;
    cadt_status status;

    status = prepare(&env, base, count, elem_size, cmp, ctx);
    if (status != CADT_OK || count < 2u) {
        return status;
    }
    introsort(&env, 0, count - 1u, depth_limit(count));
    insertion_sort(&env, 0, count - 1u);
    return CADT_OK;
}

static void merge_sort(sort_env *env, size_t lo, size_t hi)
{
    size_t mid;
    size_t i;
    size_t j;
    size_t k;
    size_t n = hi - lo;

    if (n <= 1u) {
        return;
    }
    if (n <= 16u) {
        insertion_sort(env, lo, hi - 1u);
        return;
    }
    mid = lo + n / 2u;
    merge_sort(env, lo, mid);
    merge_sort(env, mid, hi);
    i = lo;
    j = mid;
    k = lo;
    while (i < mid && j < hi) {
        if (cmp_index(env, i, j) <= 0) {
            memcpy(env->tmp + k * env->elem_size, at_ptr(env, i), env->elem_size);
            i++;
        } else {
            memcpy(env->tmp + k * env->elem_size, at_ptr(env, j), env->elem_size);
            j++;
        }
        k++;
    }
    while (i < mid) {
        memcpy(env->tmp + k * env->elem_size, at_ptr(env, i), env->elem_size);
        i++;
        k++;
    }
    while (j < hi) {
        memcpy(env->tmp + k * env->elem_size, at_ptr(env, j), env->elem_size);
        j++;
        k++;
    }
    for (k = lo; k < hi; k++) {
        memcpy(at_ptr(env, k), env->tmp + k * env->elem_size, env->elem_size);
    }
}

cadt_status cadt_stable_sort(void *base, size_t count, size_t elem_size,
                             cadt_compare_fn cmp, void *ctx)
{
    sort_env env;
    size_t bytes;
    cadt_status status;

    status = prepare(&env, base, count, elem_size, cmp, ctx);
    if (status != CADT_OK || count < 2u) {
        return status;
    }
    if (!cadt_mul_size(count, elem_size, &bytes)) {
        return CADT_ERR_OVERFLOW;
    }
    env.tmp = (unsigned char *)malloc(bytes);
    if (env.tmp == NULL) {
        return CADT_ERR_NOMEM;
    }
    merge_sort(&env, 0, count);
    free(env.tmp);
    return CADT_OK;
}

bool cadt_is_sorted(const void *base, size_t count, size_t elem_size,
                    cadt_compare_fn cmp, void *ctx)
{
    sort_env env;
    size_t i;

    if (prepare(&env, (void *)base, count, elem_size, cmp, ctx) != CADT_OK) {
        return false;
    }
    for (i = 1; i < count; i++) {
        if (cmp_index(&env, i - 1u, i) > 0) {
            return false;
        }
    }
    return true;
}

size_t cadt_lower_bound(const void *base, size_t count, size_t elem_size,
                        const void *key, cadt_compare_fn cmp, void *ctx)
{
    sort_env env;
    size_t lo = 0;
    size_t hi = count;

    if (key == NULL || prepare(&env, (void *)base, count, elem_size, cmp, ctx) != CADT_OK) {
        return 0;
    }
    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2u;

        if (cmp_key(&env, mid, key) < 0) {
            lo = mid + 1u;
        } else {
            hi = mid;
        }
    }
    return lo;
}

size_t cadt_upper_bound(const void *base, size_t count, size_t elem_size,
                        const void *key, cadt_compare_fn cmp, void *ctx)
{
    sort_env env;
    size_t lo = 0;
    size_t hi = count;

    if (key == NULL || prepare(&env, (void *)base, count, elem_size, cmp, ctx) != CADT_OK) {
        return 0;
    }
    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2u;

        if (cmp_key(&env, mid, key) <= 0) {
            lo = mid + 1u;
        } else {
            hi = mid;
        }
    }
    return lo;
}

bool cadt_binary_search(const void *base, size_t count, size_t elem_size,
                        const void *key, cadt_compare_fn cmp, void *ctx)
{
    size_t index = cadt_lower_bound(base, count, elem_size, key, cmp, ctx);
    sort_env env;

    if (index >= count) {
        return false;
    }
    if (prepare(&env, (void *)base, count, elem_size, cmp, ctx) != CADT_OK) {
        return false;
    }
    return cmp_key(&env, index, key) == 0;
}

void *cadt_find(const void *base, size_t count, size_t elem_size,
                const void *key, cadt_compare_fn cmp, void *ctx)
{
    sort_env env;
    size_t i;

    if (key == NULL || prepare(&env, (void *)base, count, elem_size, cmp, ctx) != CADT_OK) {
        return NULL;
    }
    for (i = 0; i < count; i++) {
        if (cmp_key(&env, i, key) == 0) {
            return at_ptr(&env, i);
        }
    }
    return NULL;
}

void *cadt_find_if(const void *base, size_t count, size_t elem_size,
                   cadt_pred_fn pred, void *ctx)
{
    unsigned char *bytes = (unsigned char *)base;
    size_t i;

    if (base == NULL || elem_size == 0 || pred == NULL) {
        return NULL;
    }
    for (i = 0; i < count; i++) {
        void *elem = bytes + i * elem_size;

        if (pred(elem, ctx)) {
            return elem;
        }
    }
    return NULL;
}

size_t cadt_count(const void *base, size_t count, size_t elem_size,
                  const void *key, cadt_compare_fn cmp, void *ctx)
{
    sort_env env;
    size_t i;
    size_t found = 0;

    if (key == NULL || prepare(&env, (void *)base, count, elem_size, cmp, ctx) != CADT_OK) {
        return 0;
    }
    for (i = 0; i < count; i++) {
        if (cmp_key(&env, i, key) == 0) {
            found++;
        }
    }
    return found;
}

size_t cadt_count_if(const void *base, size_t count, size_t elem_size,
                     cadt_pred_fn pred, void *ctx)
{
    unsigned char *bytes = (unsigned char *)base;
    size_t i;
    size_t found = 0;

    if (base == NULL || elem_size == 0 || pred == NULL) {
        return 0;
    }
    for (i = 0; i < count; i++) {
        if (pred(bytes + i * elem_size, ctx)) {
            found++;
        }
    }
    return found;
}

void cadt_for_each(void *base, size_t count, size_t elem_size, cadt_for_each_fn fn, void *ctx)
{
    unsigned char *bytes = (unsigned char *)base;
    size_t i;

    if (base == NULL || elem_size == 0 || fn == NULL) {
        return;
    }
    for (i = 0; i < count; i++) {
        fn(bytes + i * elem_size, ctx);
    }
}

void cadt_reverse(void *base, size_t count, size_t elem_size)
{
    unsigned char *bytes = (unsigned char *)base;
    size_t i;

    if (base == NULL || elem_size == 0 || count < 2u) {
        return;
    }
    for (i = 0; i < count / 2u; i++) {
        swap_bytes(bytes + i * elem_size, bytes + (count - 1u - i) * elem_size, elem_size);
    }
}

void cadt_fill(void *base, size_t count, size_t elem_size, const void *value)
{
    unsigned char *bytes = (unsigned char *)base;
    size_t i;

    if (base == NULL || value == NULL || elem_size == 0) {
        return;
    }
    for (i = 0; i < count; i++) {
        memcpy(bytes + i * elem_size, value, elem_size);
    }
}

void *cadt_min_element(const void *base, size_t count, size_t elem_size,
                       cadt_compare_fn cmp, void *ctx)
{
    sort_env env;
    size_t best = 0;
    size_t i;

    if (count == 0 || prepare(&env, (void *)base, count, elem_size, cmp, ctx) != CADT_OK) {
        return NULL;
    }
    for (i = 1; i < count; i++) {
        if (cmp_index(&env, i, best) < 0) {
            best = i;
        }
    }
    return at_ptr(&env, best);
}

void *cadt_max_element(const void *base, size_t count, size_t elem_size,
                       cadt_compare_fn cmp, void *ctx)
{
    sort_env env;
    size_t best = 0;
    size_t i;

    if (count == 0 || prepare(&env, (void *)base, count, elem_size, cmp, ctx) != CADT_OK) {
        return NULL;
    }
    for (i = 1; i < count; i++) {
        if (cmp_index(&env, i, best) > 0) {
            best = i;
        }
    }
    return at_ptr(&env, best);
}

bool cadt_equal(const void *a, const void *b, size_t count, size_t elem_size,
                cadt_compare_fn cmp, void *ctx)
{
    const unsigned char *aa = (const unsigned char *)a;
    const unsigned char *bb = (const unsigned char *)b;
    size_t i;

    if (count == 0) {
        return true;
    }
    if (a == NULL || b == NULL || elem_size == 0) {
        return false;
    }
    for (i = 0; i < count; i++) {
        const void *ea = aa + i * elem_size;
        const void *eb = bb + i * elem_size;
        int result = cmp != NULL ? cmp(ea, eb, ctx) : memcmp(ea, eb, elem_size);

        if (result != 0) {
            return false;
        }
    }
    return true;
}
