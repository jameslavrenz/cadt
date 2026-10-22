/* cadt - C Abstract Data Types
 * SPDX-License-Identifier: MIT
 *
 * Shared status codes, allocator, and element-type descriptor.
 * C has no templates, so every container copies, destroys, orders, and
 * hashes elements through a cadt_type.
 */
#ifndef CADT_COMMON_H
#define CADT_COMMON_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CADT_VERSION_MAJOR 1
#define CADT_VERSION_MINOR 0
#define CADT_VERSION_PATCH 0

/* Sentinel index returned by searches that do not find a match. */
#define CADT_NPOS ((size_t)-1)

typedef enum cadt_status {
    CADT_OK = 0,
    CADT_ERR_NOMEM,     /* allocation failed */
    CADT_ERR_RANGE,     /* index or iterator out of range */
    CADT_ERR_EMPTY,     /* operation needs an element and the container is empty */
    CADT_ERR_NOT_FOUND, /* key is not present */
    CADT_ERR_EXISTS,    /* unique container already holds an equivalent key */
    CADT_ERR_INVALID,   /* nullptr argument or type descriptor the operation cannot use */
    CADT_ERR_OVERFLOW   /* size computation would overflow size_t */
} cadt_status;

/* Human-readable name for a status code. Never returns nullptr. */
const char *cadt_status_str(cadt_status status);

/* Library version "major.minor.patch". Never returns nullptr. */
const char *cadt_version(void);

/*
 * Memory source for a container. The allocator object must outlive every
 * container created with it. nullptr means the process malloc/realloc/free.
 *
 * alloc() is never called with size 0.
 * realloc() receives the previous allocation size in bytes. It returns nullptr
 * on failure and leaves the old block allocated. A nullptr realloc hook is
 * emulated with alloc, copy, and free.
 * free() is never called with nullptr.
 */
typedef struct cadt_allocator {
    void *(*alloc)(size_t size, void *ctx);
    void *(*realloc)(void *ptr, size_t old_size, size_t new_size, void *ctx);
    void (*free)(void *ptr, void *ctx);
    void *ctx;
} cadt_allocator;

const cadt_allocator *cadt_default_allocator(void);

/*
 * Element protocol.
 *
 * size      byte size of one element (sizeof the value)
 * copy      dst/src are element pointers. nullptr copies with memcpy.
 *           On failure dst is left uninitialized. The source stays owned
 *           by the caller.
 * destroy   called when a container drops an element it owns. nullptr is a no-op.
 *           Not called when an operation moves the element out to the caller.
 * compare   <0, 0, >0 like memcmp. nullptr compares the raw bytes.
 *           Ordered containers and priority queues reject a nullptr compare
 *           because byte order is not numeric order.
 * hash      64-bit hash of one element. nullptr hashes the raw bytes (FNV-1a).
 * ctx       passed through to the callbacks. Must outlive the container.
 *
 * Elements are stored by value. Pointers inside an element are owned by the
 * container only if copy duplicated them and destroy releases them.
 */
typedef cadt_status (*cadt_copy_fn)(void *dst, const void *src, void *ctx);
typedef void (*cadt_destroy_fn)(void *elem, void *ctx);
typedef int (*cadt_compare_fn)(const void *a, const void *b, void *ctx);
typedef uint64_t (*cadt_hash_fn)(const void *elem, void *ctx);

typedef struct cadt_type {
    size_t size;
    cadt_copy_fn copy;
    cadt_destroy_fn destroy;
    cadt_compare_fn compare;
    cadt_hash_fn hash;
    void *ctx;
} cadt_type;

/* FNV-1a over an arbitrary byte string. */
uint64_t cadt_hash_bytes(const void *data, size_t length);

#ifdef __cplusplus
}
#endif

#endif /* CADT_COMMON_H */
