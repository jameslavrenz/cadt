/* cadt - C Abstract Data Types
 * SPDX-License-Identifier: MIT
 *
 * A value that may be absent (std::optional).
 */
#ifndef CADT_OPTIONAL_H
#define CADT_OPTIONAL_H

#include "cadt/common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cadt_optional cadt_optional;

cadt_optional *cadt_optional_create(const cadt_type *type, const cadt_allocator *alloc);
void cadt_optional_destroy(cadt_optional *opt);

bool cadt_optional_has_value(const cadt_optional *opt);
/* nullptr when empty. Invalidated by set, reset, and destroy. */
void *cadt_optional_value(const cadt_optional *opt);

cadt_status cadt_optional_set(cadt_optional *opt, const void *elem);
void cadt_optional_reset(cadt_optional *opt);
/* Move the value out. CADT_ERR_EMPTY when absent. */
cadt_status cadt_optional_take(cadt_optional *opt, void *out);

#ifdef __cplusplus
}
#endif

#endif /* CADT_OPTIONAL_H */
