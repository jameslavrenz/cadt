#ifndef CADT_TEST_H
#define CADT_TEST_H

#include "cadt.h"

#include <stdio.h>
#include <string.h>

extern int g_failed;
extern int g_passed;

#define CHECK(cond)                                                         \
    do {                                                                    \
        if (cond) {                                                         \
            g_passed++;                                                     \
        } else {                                                            \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
            g_failed++;                                                     \
        }                                                                   \
    } while (0)

static inline int cmp_int(const void *a, const void *b, [[maybe_unused]] void *ctx)
{
    int ia = *(const int *)a;
    int ib = *(const int *)b;
    return (ia > ib) - (ia < ib);
}

static inline cadt_type type_int(void)
{
    cadt_type type;

    memset(&type, 0, sizeof(type));
    type.size = sizeof(int);
    type.compare = cmp_int;
    return type;
}

static inline uint32_t test_rnd(uint32_t *state)
{
    uint32_t x = *state == 0u ? 0xA5A5A5A5u : *state;

    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

typedef struct track_hdr track_hdr;

typedef struct track_state {
    track_hdr *head;
    size_t live_bytes;
    size_t live_count;
    size_t allocs;
    size_t frees;
} track_state;

void track_init(track_state *state, cadt_allocator *alloc);
int track_leaked(const track_state *state);

void test_vector(void);
void test_deque(void);
void test_list(void);
void test_forward_list(void);
void test_array(void);
void test_adapters(void);
void test_set(void);
void test_map(void);
void test_unordered(void);
void test_string(void);
void test_bitset(void);
void test_optional(void);
void test_algo(void);
void test_span(void);

#endif /* CADT_TEST_H */
