/* cadt - C Abstract Data Types
 * SPDX-License-Identifier: MIT
 *
 * Packed bit vector (std::bitset, with a runtime length).
 */
#ifndef CADT_BITSET_H
#define CADT_BITSET_H

#include "cadt/common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cadt_bitset cadt_bitset;

cadt_bitset *cadt_bitset_create(size_t nbits, const cadt_allocator *alloc);
void cadt_bitset_destroy(cadt_bitset *bs);

size_t cadt_bitset_size(const cadt_bitset *bs);
bool cadt_bitset_test(const cadt_bitset *bs, size_t index);
cadt_status cadt_bitset_set(cadt_bitset *bs, size_t index, bool value);
cadt_status cadt_bitset_flip(cadt_bitset *bs, size_t index);

void cadt_bitset_set_all(cadt_bitset *bs, bool value);
void cadt_bitset_flip_all(cadt_bitset *bs);

size_t cadt_bitset_count(const cadt_bitset *bs);
bool cadt_bitset_any(const cadt_bitset *bs);
bool cadt_bitset_none(const cadt_bitset *bs);
bool cadt_bitset_all(const cadt_bitset *bs);

/* Both operands must have the same length. */
cadt_status cadt_bitset_and(cadt_bitset *dst, const cadt_bitset *src);
cadt_status cadt_bitset_or(cadt_bitset *dst, const cadt_bitset *src);
cadt_status cadt_bitset_xor(cadt_bitset *dst, const cadt_bitset *src);

/*
 * Writes nbits characters of '0' and '1', plus a trailing NUL.
 * buflen must be at least nbits + 1. Bit 0 is the first character.
 */
cadt_status cadt_bitset_format(const cadt_bitset *bs, char *buf, size_t buflen);

#ifdef __cplusplus
}
#endif

#endif /* CADT_BITSET_H */
