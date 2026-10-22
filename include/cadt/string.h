/* cadt - C Abstract Data Types
 * SPDX-License-Identifier: MIT
 *
 * Owning byte string (the contiguous subset of std::string).
 * Length is independent of embedded NUL bytes. c_str() always points at a
 * buffer with an extra NUL written at size.
 */
#ifndef CADT_STRING_H
#define CADT_STRING_H

#include "cadt/common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cadt_string cadt_string;

[[nodiscard]] cadt_string *cadt_string_create(const cadt_allocator *alloc);
[[nodiscard]] cadt_string *cadt_string_create_n(const char *data, size_t length,
                                  const cadt_allocator *alloc);
[[nodiscard]] cadt_string *cadt_string_create_cstr(const char *cstr, const cadt_allocator *alloc);
void cadt_string_destroy(cadt_string *str);

size_t cadt_string_size(const cadt_string *str);
size_t cadt_string_capacity(const cadt_string *str);
bool cadt_string_empty(const cadt_string *str);
const char *cadt_string_c_str(const cadt_string *str);
char *cadt_string_data(cadt_string *str);

[[nodiscard]] cadt_status cadt_string_reserve(cadt_string *str, size_t capacity);
[[nodiscard]] cadt_status cadt_string_shrink_to_fit(cadt_string *str);
void cadt_string_clear(cadt_string *str);

[[nodiscard]] cadt_status cadt_string_assign_n(cadt_string *str, const char *data, size_t length);
[[nodiscard]] cadt_status cadt_string_append_n(cadt_string *str, const char *data, size_t length);
[[nodiscard]] cadt_status cadt_string_append_cstr(cadt_string *str, const char *cstr);
[[nodiscard]] cadt_status cadt_string_append_char(cadt_string *str, char ch);
[[nodiscard]] cadt_status cadt_string_append(cadt_string *str, const cadt_string *other);
[[nodiscard]] cadt_status cadt_string_insert_n(cadt_string *str, size_t index, const char *data,
                                 size_t length);
[[nodiscard]] cadt_status cadt_string_erase(cadt_string *str, size_t index, size_t count);

int cadt_string_compare(const cadt_string *a, const cadt_string *b);
bool cadt_string_equals(const cadt_string *a, const cadt_string *b);
bool cadt_string_equals_cstr(const cadt_string *str, const char *cstr);
bool cadt_string_starts_with(const cadt_string *str, const char *prefix);
bool cadt_string_ends_with(const cadt_string *str, const char *suffix);

/* Index of the first match, or CADT_NPOS. */
size_t cadt_string_find_n(const cadt_string *str, const char *needle, size_t needle_len);
size_t cadt_string_find_cstr(const cadt_string *str, const char *needle);

/* nullptr when the range does not fit. */
[[nodiscard]] cadt_string *cadt_string_substr(const cadt_string *str, size_t index, size_t count);

#ifdef __cplusplus
}
#endif

#endif /* CADT_STRING_H */
