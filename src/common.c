/* cadt - C Abstract Data Types
 * SPDX-License-Identifier: MIT
 */
#include "internal.h"

#include <string.h>

const char *cadt_status_str(cadt_status status)
{
    switch (status) {
    case CADT_OK:
        return "ok";
    case CADT_ERR_NOMEM:
        return "out of memory";
    case CADT_ERR_RANGE:
        return "out of range";
    case CADT_ERR_EMPTY:
        return "empty";
    case CADT_ERR_NOT_FOUND:
        return "not found";
    case CADT_ERR_EXISTS:
        return "already exists";
    case CADT_ERR_INVALID:
        return "invalid argument";
    case CADT_ERR_OVERFLOW:
        return "size overflow";
    default:
        return "unknown status";
    }
}

const char *cadt_version(void)
{
    return "1.0.0";
}

static void *default_alloc(size_t size, [[maybe_unused]] void *ctx)
{
    return malloc(size);
}

static void *default_realloc(void *ptr, [[maybe_unused]] size_t old_size, size_t new_size,
                             [[maybe_unused]] void *ctx)
{
    return realloc(ptr, new_size);
}

static void default_free(void *ptr, [[maybe_unused]] void *ctx)
{
    free(ptr);
}

const cadt_allocator *cadt_default_allocator(void)
{
    static const cadt_allocator allocator = {
        default_alloc,
        default_realloc,
        default_free,
        nullptr
    };
    return &allocator;
}

const cadt_allocator *cadt_alloc_or_default(const cadt_allocator *alloc)
{
    return alloc != nullptr ? alloc : cadt_default_allocator();
}

void *cadt_xalloc(const cadt_allocator *alloc, size_t size)
{
    if (size == 0) {
        return nullptr;
    }
    return alloc->alloc(size, alloc->ctx);
}

void cadt_xfree(const cadt_allocator *alloc, void *ptr)
{
    if (ptr != nullptr) {
        alloc->free(ptr, alloc->ctx);
    }
}

void *cadt_xrealloc(const cadt_allocator *alloc, void *ptr, size_t old_size,
                    size_t new_size)
{
    void *fresh;
    size_t copy_n;

    if (new_size == 0) {
        cadt_xfree(alloc, ptr);
        return nullptr;
    }
    if (alloc->realloc != nullptr) {
        return alloc->realloc(ptr, old_size, new_size, alloc->ctx);
    }
    fresh = alloc->alloc(new_size, alloc->ctx);
    if (fresh == nullptr) {
        return nullptr;
    }
    if (ptr != nullptr && old_size > 0) {
        copy_n = old_size < new_size ? old_size : new_size;
        memcpy(fresh, ptr, copy_n);
        alloc->free(ptr, alloc->ctx);
    }
    return fresh;
}

bool cadt_mul_size(size_t a, size_t b, size_t *out)
{
    if (a != 0 && b > SIZE_MAX / a) {
        return false;
    }
    *out = a * b;
    return true;
}

bool cadt_add_size(size_t a, size_t b, size_t *out)
{
    if (a > SIZE_MAX - b) {
        return false;
    }
    *out = a + b;
    return true;
}

cadt_status cadt_type_check(const cadt_type *type)
{
    if (type == nullptr || type->size == 0) {
        return CADT_ERR_INVALID;
    }
    return CADT_OK;
}

cadt_status cadt_type_copy(const cadt_type *type, void *dst, const void *src)
{
    if (type->copy != nullptr) {
        return type->copy(dst, src, type->ctx);
    }
    memcpy(dst, src, type->size);
    return CADT_OK;
}

void cadt_type_destroy(const cadt_type *type, void *elem)
{
    if (type->destroy != nullptr) {
        type->destroy(elem, type->ctx);
    }
}

int cadt_type_compare(const cadt_type *type, const void *a, const void *b)
{
    if (type->compare != nullptr) {
        return type->compare(a, b, type->ctx);
    }
    return memcmp(a, b, type->size);
}

uint64_t cadt_type_hash(const cadt_type *type, const void *elem)
{
    if (type->hash != nullptr) {
        return type->hash(elem, type->ctx);
    }
    return cadt_hash_bytes(elem, type->size);
}

uint64_t cadt_hash_bytes(const void *data, size_t length)
{
    const unsigned char *bytes = (const unsigned char *)data;
    uint64_t hash = 14695981039346656037ull;
    size_t i;

    for (i = 0; i < length; i++) {
        hash ^= (uint64_t)bytes[i];
        hash *= 1099511628211ull;
    }
    return hash;
}
