/* cadt - C Abstract Data Types
 * SPDX-License-Identifier: MIT
 *
 * Key/value element packing shared by map and unordered_map.
 */
#ifndef CADT_KV_H
#define CADT_KV_H

#include "internal.h"

#include <stdalign.h>
#include <string.h>

typedef struct cadt_kv {
    cadt_type key;
    cadt_type value;
    size_t value_offset;
    size_t elem_size;
} cadt_kv;

static inline bool cadt_kv_init(cadt_kv *kv, const cadt_type *key, const cadt_type *value)
{
    if (cadt_type_check(key) != CADT_OK || cadt_type_check(value) != CADT_OK) {
        return false;
    }
    kv->key = *key;
    kv->value = *value;
    kv->value_offset = cadt_align_up(key->size, alignof(max_align_t));
    return cadt_add_size(kv->value_offset, value->size, &kv->elem_size);
}

static inline void cadt_kv_elem_destroy(void *elem, void *ctx)
{
    const cadt_kv *kv = (const cadt_kv *)ctx;

    cadt_type_destroy(&kv->key, elem);
    cadt_type_destroy(&kv->value, (unsigned char *)elem + kv->value_offset);
}

static inline cadt_status cadt_kv_pack(const cadt_allocator *alloc, const cadt_kv *kv,
                                       const void *key, const void *value,
                                       unsigned char **out)
{
    unsigned char *tmp;
    cadt_status status;

    tmp = (unsigned char *)cadt_xalloc(alloc, kv->elem_size);
    if (tmp == nullptr) {
        return CADT_ERR_NOMEM;
    }
    memset(tmp, 0, kv->elem_size);
    status = cadt_type_copy(&kv->key, tmp, key);
    if (status != CADT_OK) {
        cadt_xfree(alloc, tmp);
        return status;
    }
    status = cadt_type_copy(&kv->value, tmp + kv->value_offset, value);
    if (status != CADT_OK) {
        cadt_type_destroy(&kv->key, tmp);
        cadt_xfree(alloc, tmp);
        return status;
    }
    *out = tmp;
    return CADT_OK;
}

static inline void cadt_kv_pack_destroy(const cadt_allocator *alloc, const cadt_kv *kv,
                                        unsigned char *packed)
{
    if (packed == nullptr) {
        return;
    }
    cadt_kv_elem_destroy(packed, (void *)kv);
    cadt_xfree(alloc, packed);
}

static inline void *cadt_kv_value(const cadt_kv *kv, void *elem)
{
    if (elem == nullptr) {
        return nullptr;
    }
    return (unsigned char *)elem + kv->value_offset;
}

#endif /* CADT_KV_H */
