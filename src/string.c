/* cadt - C Abstract Data Types
 * SPDX-License-Identifier: MIT
 */
#include "cadt/string.h"

#include "internal.h"

struct cadt_string {
    const cadt_allocator *alloc;
    char *data;
    size_t size;
    size_t capacity;
};

static cadt_status ensure_chars(cadt_string *str, size_t chars)
{
    size_t old_bytes;
    size_t new_bytes;
    size_t new_cap;
    char *fresh;

    if (chars <= str->capacity) {
        return CADT_OK;
    }
    new_cap = str->capacity < 8u ? 8u : str->capacity;
    while (new_cap < chars) {
        if (new_cap > SIZE_MAX / 2u) {
            new_cap = chars;
            break;
        }
        new_cap *= 2u;
    }
    if (new_cap < chars) {
        new_cap = chars;
    }
    if (!cadt_add_size(str->capacity, 1u, &old_bytes) ||
        !cadt_add_size(new_cap, 1u, &new_bytes)) {
        return CADT_ERR_OVERFLOW;
    }
    fresh = (char *)cadt_xrealloc(str->alloc, str->data, old_bytes, new_bytes);
    if (fresh == NULL) {
        return CADT_ERR_NOMEM;
    }
    str->data = fresh;
    str->capacity = new_cap;
    return CADT_OK;
}

cadt_string *cadt_string_create(const cadt_allocator *alloc)
{
    cadt_string *str;

    alloc = cadt_alloc_or_default(alloc);
    str = (cadt_string *)cadt_xalloc(alloc, sizeof(*str));
    if (str == NULL) {
        return NULL;
    }
    str->data = (char *)cadt_xalloc(alloc, 1u);
    if (str->data == NULL) {
        cadt_xfree(alloc, str);
        return NULL;
    }
    str->alloc = alloc;
    str->data[0] = '\0';
    str->size = 0;
    str->capacity = 0;
    return str;
}

cadt_string *cadt_string_create_n(const char *data, size_t length, const cadt_allocator *alloc)
{
    cadt_string *str;
    cadt_status status;

    if (data == NULL && length > 0) {
        return NULL;
    }
    str = cadt_string_create(alloc);
    if (str == NULL) {
        return NULL;
    }
    status = cadt_string_append_n(str, data, length);
    if (status != CADT_OK) {
        cadt_string_destroy(str);
        return NULL;
    }
    return str;
}

cadt_string *cadt_string_create_cstr(const char *cstr, const cadt_allocator *alloc)
{
    if (cstr == NULL) {
        return NULL;
    }
    return cadt_string_create_n(cstr, strlen(cstr), alloc);
}

void cadt_string_destroy(cadt_string *str)
{
    if (str == NULL) {
        return;
    }
    cadt_xfree(str->alloc, str->data);
    cadt_xfree(str->alloc, str);
}

size_t cadt_string_size(const cadt_string *str)
{
    return str == NULL ? 0 : str->size;
}

size_t cadt_string_capacity(const cadt_string *str)
{
    return str == NULL ? 0 : str->capacity;
}

bool cadt_string_empty(const cadt_string *str)
{
    return cadt_string_size(str) == 0;
}

const char *cadt_string_c_str(const cadt_string *str)
{
    return str == NULL ? NULL : str->data;
}

char *cadt_string_data(cadt_string *str)
{
    return str == NULL ? NULL : str->data;
}

cadt_status cadt_string_reserve(cadt_string *str, size_t capacity)
{
    if (str == NULL) {
        return CADT_ERR_INVALID;
    }
    return ensure_chars(str, capacity);
}

cadt_status cadt_string_shrink_to_fit(cadt_string *str)
{
    size_t old_bytes;
    size_t new_bytes;
    char *fresh;

    if (str == NULL) {
        return CADT_ERR_INVALID;
    }
    if (str->capacity == str->size) {
        return CADT_OK;
    }
    if (!cadt_add_size(str->capacity, 1u, &old_bytes) ||
        !cadt_add_size(str->size, 1u, &new_bytes)) {
        return CADT_ERR_OVERFLOW;
    }
    fresh = (char *)cadt_xrealloc(str->alloc, str->data, old_bytes, new_bytes);
    if (fresh == NULL) {
        return CADT_ERR_NOMEM;
    }
    str->data = fresh;
    str->capacity = str->size;
    return CADT_OK;
}

void cadt_string_clear(cadt_string *str)
{
    if (str == NULL) {
        return;
    }
    str->size = 0;
    str->data[0] = '\0';
}

cadt_status cadt_string_assign_n(cadt_string *str, const char *data, size_t length)
{
    int inside;
    char *tmp = NULL;
    cadt_status status;

    if (str == NULL || (data == NULL && length > 0)) {
        return CADT_ERR_INVALID;
    }
    inside = data != NULL && data >= str->data && data <= str->data + str->size;
    if (inside) {
        tmp = (char *)cadt_xalloc(str->alloc, length == 0 ? 1u : length);
        if (tmp == NULL) {
            return CADT_ERR_NOMEM;
        }
        if (length > 0) {
            memcpy(tmp, data, length);
        }
        data = tmp;
    }
    status = ensure_chars(str, length);
    if (status != CADT_OK) {
        cadt_xfree(str->alloc, tmp);
        return status;
    }
    if (length > 0) {
        memcpy(str->data, data, length);
    }
    str->size = length;
    str->data[str->size] = '\0';
    cadt_xfree(str->alloc, tmp);
    return CADT_OK;
}

cadt_status cadt_string_append_n(cadt_string *str, const char *data, size_t length)
{
    size_t needed;
    int inside;
    size_t offset = 0;
    cadt_status status;

    if (str == NULL || (data == NULL && length > 0)) {
        return CADT_ERR_INVALID;
    }
    if (length == 0) {
        return CADT_OK;
    }
    if (!cadt_add_size(str->size, length, &needed)) {
        return CADT_ERR_OVERFLOW;
    }
    inside = data >= str->data && data <= str->data + str->size;
    if (inside) {
        offset = (size_t)(data - str->data);
    }
    status = ensure_chars(str, needed);
    if (status != CADT_OK) {
        return status;
    }
    if (inside) {
        data = str->data + offset;
    }
    memcpy(str->data + str->size, data, length);
    str->size = needed;
    str->data[str->size] = '\0';
    return CADT_OK;
}

cadt_status cadt_string_append_cstr(cadt_string *str, const char *cstr)
{
    if (cstr == NULL) {
        return CADT_ERR_INVALID;
    }
    return cadt_string_append_n(str, cstr, strlen(cstr));
}

cadt_status cadt_string_append_char(cadt_string *str, char ch)
{
    return cadt_string_append_n(str, &ch, 1u);
}

cadt_status cadt_string_append(cadt_string *str, const cadt_string *other)
{
    if (other == NULL) {
        return CADT_ERR_INVALID;
    }
    return cadt_string_append_n(str, other->data, other->size);
}

cadt_status cadt_string_insert_n(cadt_string *str, size_t index, const char *data,
                                 size_t length)
{
    size_t needed;
    int inside;
    size_t offset = 0;
    cadt_status status;

    if (str == NULL || (data == NULL && length > 0)) {
        return CADT_ERR_INVALID;
    }
    if (index > str->size) {
        return CADT_ERR_RANGE;
    }
    if (length == 0) {
        return CADT_OK;
    }
    if (!cadt_add_size(str->size, length, &needed)) {
        return CADT_ERR_OVERFLOW;
    }
    inside = data >= str->data && data <= str->data + str->size;
    if (inside) {
        offset = (size_t)(data - str->data);
    }
    status = ensure_chars(str, needed);
    if (status != CADT_OK) {
        return status;
    }
    if (inside) {
        data = str->data + offset;
    }
    memmove(str->data + index + length, str->data + index, str->size - index + 1u);
    if (inside && offset >= index) {
        data += length;
    }
    memcpy(str->data + index, data, length);
    str->size = needed;
    str->data[str->size] = '\0';
    return CADT_OK;
}

cadt_status cadt_string_erase(cadt_string *str, size_t index, size_t count)
{
    if (str == NULL) {
        return CADT_ERR_INVALID;
    }
    if (index > str->size) {
        return CADT_ERR_RANGE;
    }
    if (count > str->size - index) {
        count = str->size - index;
    }
    memmove(str->data + index, str->data + index + count, str->size - index - count + 1u);
    str->size -= count;
    return CADT_OK;
}

int cadt_string_compare(const cadt_string *a, const cadt_string *b)
{
    size_t n;
    int cmp;

    if (a == b) {
        return 0;
    }
    if (a == NULL) {
        return -1;
    }
    if (b == NULL) {
        return 1;
    }
    n = a->size < b->size ? a->size : b->size;
    cmp = n == 0 ? 0 : memcmp(a->data, b->data, n);
    if (cmp != 0) {
        return cmp;
    }
    return (a->size > b->size) - (a->size < b->size);
}

bool cadt_string_equals(const cadt_string *a, const cadt_string *b)
{
    return cadt_string_compare(a, b) == 0;
}

bool cadt_string_equals_cstr(const cadt_string *str, const char *cstr)
{
    size_t n;

    if (str == NULL || cstr == NULL) {
        return false;
    }
    n = strlen(cstr);
    return str->size == n && memcmp(str->data, cstr, n) == 0;
}

bool cadt_string_starts_with(const cadt_string *str, const char *prefix)
{
    size_t n;

    if (str == NULL || prefix == NULL) {
        return false;
    }
    n = strlen(prefix);
    return n <= str->size && memcmp(str->data, prefix, n) == 0;
}

bool cadt_string_ends_with(const cadt_string *str, const char *suffix)
{
    size_t n;

    if (str == NULL || suffix == NULL) {
        return false;
    }
    n = strlen(suffix);
    return n <= str->size && memcmp(str->data + str->size - n, suffix, n) == 0;
}

size_t cadt_string_find_n(const cadt_string *str, const char *needle, size_t needle_len)
{
    size_t i;

    if (str == NULL || (needle == NULL && needle_len > 0)) {
        return CADT_NPOS;
    }
    if (needle_len == 0) {
        return 0;
    }
    if (needle_len > str->size) {
        return CADT_NPOS;
    }
    for (i = 0; i + needle_len <= str->size; i++) {
        if (memcmp(str->data + i, needle, needle_len) == 0) {
            return i;
        }
    }
    return CADT_NPOS;
}

size_t cadt_string_find_cstr(const cadt_string *str, const char *needle)
{
    if (needle == NULL) {
        return CADT_NPOS;
    }
    return cadt_string_find_n(str, needle, strlen(needle));
}

cadt_string *cadt_string_substr(const cadt_string *str, size_t index, size_t count)
{
    if (str == NULL || index > str->size) {
        return NULL;
    }
    if (count == CADT_NPOS) {
        count = str->size - index;
    } else if (count > str->size - index) {
        return NULL;
    }
    return cadt_string_create_n(str->data + index, count, str->alloc);
}
