#include "test.h"

#include <stdlib.h>
#include <stdalign.h>

struct track_hdr {
    struct track_hdr *next;
    struct track_hdr *prev;
    size_t size;
};

static size_t header_bytes(void)
{
    size_t align = alignof(max_align_t);
    size_t size = sizeof(struct track_hdr);

    return (size + align - 1u) & ~(align - 1u);
}

static struct track_hdr *header_of(void *ptr)
{
    return (struct track_hdr *)((unsigned char *)ptr - header_bytes());
}

static void *track_alloc(size_t size, void *ctx)
{
    track_state *state = (track_state *)ctx;
    unsigned char *raw;
    struct track_hdr *hdr;

    if (size == 0) {
        return nullptr;
    }
    raw = (unsigned char *)malloc(header_bytes() + size);
    if (raw == nullptr) {
        return nullptr;
    }
    hdr = (struct track_hdr *)raw;
    hdr->size = size;
    hdr->prev = nullptr;
    hdr->next = state->head;
    if (state->head != nullptr) {
        state->head->prev = hdr;
    }
    state->head = hdr;
    state->live_bytes += size;
    state->live_count++;
    state->allocs++;
    return raw + header_bytes();
}

static void track_free(void *ptr, void *ctx)
{
    track_state *state = (track_state *)ctx;
    struct track_hdr *hdr;

    if (ptr == nullptr) {
        return;
    }
    hdr = header_of(ptr);
    if (hdr->prev != nullptr) {
        hdr->prev->next = hdr->next;
    } else {
        state->head = hdr->next;
    }
    if (hdr->next != nullptr) {
        hdr->next->prev = hdr->prev;
    }
    state->live_bytes -= hdr->size;
    state->live_count--;
    state->frees++;
    free(hdr);
}

static void *track_realloc(void *ptr, size_t old_size, size_t new_size, void *ctx)
{
    void *fresh;
    size_t copy_n;

    if (new_size == 0) {
        track_free(ptr, ctx);
        return nullptr;
    }
    fresh = track_alloc(new_size, ctx);
    if (fresh == nullptr) {
        return nullptr;
    }
    if (ptr != nullptr && old_size > 0) {
        copy_n = old_size < new_size ? old_size : new_size;
        memcpy(fresh, ptr, copy_n);
    }
    if (ptr != nullptr) {
        track_free(ptr, ctx);
    }
    return fresh;
}

void track_init(track_state *state, cadt_allocator *alloc)
{
    *state = (track_state){};
    alloc->alloc = track_alloc;
    alloc->realloc = track_realloc;
    alloc->free = track_free;
    alloc->ctx = state;
}

int track_leaked(const track_state *state)
{
    return state->live_count != 0 || state->live_bytes != 0 || state->allocs != state->frees;
}
