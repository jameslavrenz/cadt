/* cadt - C Abstract Data Types
 * SPDX-License-Identifier: MIT
 */
#include "cadt/bitset.h"

#include "internal.h"

struct cadt_bitset {
    const cadt_allocator *alloc;
    uint64_t *words;
    size_t nbits;
    size_t nwords;
};

static unsigned popcount64(uint64_t value)
{
#if defined(__GNUC__) || defined(__clang__)
    return (unsigned)__builtin_popcountll(value);
#else
    unsigned count = 0;

    while (value != 0) {
        value &= value - 1u;
        count++;
    }
    return count;
#endif
}

static void mask_tail(cadt_bitset *bs)
{
    unsigned extra;

    if (bs->nbits == 0 || (bs->nbits % 64u) == 0) {
        return;
    }
    extra = (unsigned)(bs->nbits % 64u);
    bs->words[bs->nwords - 1u] &= (1ull << extra) - 1ull;
}

cadt_bitset *cadt_bitset_create(size_t nbits, const cadt_allocator *alloc)
{
    cadt_bitset *bs;
    size_t nwords;
    size_t bytes;

    alloc = cadt_alloc_or_default(alloc);
    nwords = (nbits + 63u) / 64u;
    bs = (cadt_bitset *)cadt_xalloc(alloc, sizeof(*bs));
    if (bs == NULL) {
        return NULL;
    }
    bs->alloc = alloc;
    bs->nbits = nbits;
    bs->nwords = nwords;
    bs->words = NULL;
    if (nwords == 0) {
        return bs;
    }
    if (!cadt_mul_size(nwords, sizeof(uint64_t), &bytes)) {
        cadt_xfree(alloc, bs);
        return NULL;
    }
    bs->words = (uint64_t *)cadt_xalloc(alloc, bytes);
    if (bs->words == NULL) {
        cadt_xfree(alloc, bs);
        return NULL;
    }
    memset(bs->words, 0, bytes);
    return bs;
}

void cadt_bitset_destroy(cadt_bitset *bs)
{
    if (bs == NULL) {
        return;
    }
    cadt_xfree(bs->alloc, bs->words);
    cadt_xfree(bs->alloc, bs);
}

size_t cadt_bitset_size(const cadt_bitset *bs)
{
    return bs == NULL ? 0 : bs->nbits;
}

bool cadt_bitset_test(const cadt_bitset *bs, size_t index)
{
    size_t word;
    unsigned bit;

    if (bs == NULL || index >= bs->nbits) {
        return false;
    }
    word = index / 64u;
    bit = (unsigned)(index % 64u);
    return (bs->words[word] & (1ull << bit)) != 0;
}

cadt_status cadt_bitset_set(cadt_bitset *bs, size_t index, bool value)
{
    size_t word;
    unsigned bit;

    if (bs == NULL) {
        return CADT_ERR_INVALID;
    }
    if (index >= bs->nbits) {
        return CADT_ERR_RANGE;
    }
    word = index / 64u;
    bit = (unsigned)(index % 64u);
    if (value) {
        bs->words[word] |= (1ull << bit);
    } else {
        bs->words[word] &= ~(1ull << bit);
    }
    return CADT_OK;
}

cadt_status cadt_bitset_flip(cadt_bitset *bs, size_t index)
{
    if (bs == NULL) {
        return CADT_ERR_INVALID;
    }
    if (index >= bs->nbits) {
        return CADT_ERR_RANGE;
    }
    return cadt_bitset_set(bs, index, !cadt_bitset_test(bs, index));
}

void cadt_bitset_set_all(cadt_bitset *bs, bool value)
{
    if (bs == NULL || bs->nwords == 0) {
        return;
    }
    memset(bs->words, value ? 0xFF : 0x00, bs->nwords * sizeof(uint64_t));
    if (value) {
        mask_tail(bs);
    }
}

void cadt_bitset_flip_all(cadt_bitset *bs)
{
    size_t i;

    if (bs == NULL) {
        return;
    }
    for (i = 0; i < bs->nwords; i++) {
        bs->words[i] = ~bs->words[i];
    }
    mask_tail(bs);
}

size_t cadt_bitset_count(const cadt_bitset *bs)
{
    size_t i;
    size_t total = 0;

    if (bs == NULL) {
        return 0;
    }
    for (i = 0; i < bs->nwords; i++) {
        total += popcount64(bs->words[i]);
    }
    return total;
}

bool cadt_bitset_any(const cadt_bitset *bs)
{
    size_t i;

    if (bs == NULL) {
        return false;
    }
    for (i = 0; i < bs->nwords; i++) {
        if (bs->words[i] != 0) {
            return true;
        }
    }
    return false;
}

bool cadt_bitset_none(const cadt_bitset *bs)
{
    return !cadt_bitset_any(bs);
}

bool cadt_bitset_all(const cadt_bitset *bs)
{
    if (bs == NULL) {
        return false;
    }
    return cadt_bitset_count(bs) == bs->nbits;
}

static cadt_status bitwise(cadt_bitset *dst, const cadt_bitset *src, int op)
{
    size_t i;

    if (dst == NULL || src == NULL || dst->nbits != src->nbits) {
        return CADT_ERR_INVALID;
    }
    for (i = 0; i < dst->nwords; i++) {
        if (op == 0) {
            dst->words[i] &= src->words[i];
        } else if (op == 1) {
            dst->words[i] |= src->words[i];
        } else {
            dst->words[i] ^= src->words[i];
        }
    }
    return CADT_OK;
}

cadt_status cadt_bitset_and(cadt_bitset *dst, const cadt_bitset *src)
{
    return bitwise(dst, src, 0);
}

cadt_status cadt_bitset_or(cadt_bitset *dst, const cadt_bitset *src)
{
    return bitwise(dst, src, 1);
}

cadt_status cadt_bitset_xor(cadt_bitset *dst, const cadt_bitset *src)
{
    return bitwise(dst, src, 2);
}

cadt_status cadt_bitset_format(const cadt_bitset *bs, char *buf, size_t buflen)
{
    size_t i;

    if (bs == NULL || buf == NULL) {
        return CADT_ERR_INVALID;
    }
    if (buflen < bs->nbits + 1u) {
        return CADT_ERR_RANGE;
    }
    for (i = 0; i < bs->nbits; i++) {
        buf[i] = cadt_bitset_test(bs, i) ? '1' : '0';
    }
    buf[bs->nbits] = '\0';
    return CADT_OK;
}
