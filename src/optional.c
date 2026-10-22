/* cadt - C Abstract Data Types
 * SPDX-License-Identifier: MIT
 */
#include "cadt/optional.h"

#include "internal.h"

struct cadt_optional {
    const cadt_allocator *alloc;
    cadt_type type;
    void *value;
};

cadt_optional *cadt_optional_create(const cadt_type *type, const cadt_allocator *alloc)
{
    cadt_optional *opt;

    if (cadt_type_check(type) != CADT_OK) {
        return nullptr;
    }
    alloc = cadt_alloc_or_default(alloc);
    opt = (cadt_optional *)cadt_xalloc(alloc, sizeof(*opt));
    if (opt == nullptr) {
        return nullptr;
    }
    opt->alloc = alloc;
    opt->type = *type;
    opt->value = nullptr;
    return opt;
}

void cadt_optional_reset(cadt_optional *opt)
{
    if (opt == nullptr || opt->value == nullptr) {
        return;
    }
    cadt_type_destroy(&opt->type, opt->value);
    cadt_xfree(opt->alloc, opt->value);
    opt->value = nullptr;
}

void cadt_optional_destroy(cadt_optional *opt)
{
    if (opt == nullptr) {
        return;
    }
    cadt_optional_reset(opt);
    cadt_xfree(opt->alloc, opt);
}

bool cadt_optional_has_value(const cadt_optional *opt)
{
    return opt != nullptr && opt->value != nullptr;
}

void *cadt_optional_value(const cadt_optional *opt)
{
    return opt == nullptr ? nullptr : opt->value;
}

cadt_status cadt_optional_set(cadt_optional *opt, const void *elem)
{
    void *fresh;
    cadt_status status;

    if (opt == nullptr || elem == nullptr) {
        return CADT_ERR_INVALID;
    }
    fresh = cadt_xalloc(opt->alloc, opt->type.size);
    if (fresh == nullptr) {
        return CADT_ERR_NOMEM;
    }
    status = cadt_type_copy(&opt->type, fresh, elem);
    if (status != CADT_OK) {
        cadt_xfree(opt->alloc, fresh);
        return status;
    }
    cadt_optional_reset(opt);
    opt->value = fresh;
    return CADT_OK;
}

cadt_status cadt_optional_take(cadt_optional *opt, void *out)
{
    if (opt == nullptr || out == nullptr) {
        return CADT_ERR_INVALID;
    }
    if (opt->value == nullptr) {
        return CADT_ERR_EMPTY;
    }
    memcpy(out, opt->value, opt->type.size);
    cadt_xfree(opt->alloc, opt->value);
    opt->value = nullptr;
    return CADT_OK;
}
