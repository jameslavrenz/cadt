/* cadt - C Abstract Data Types
 * SPDX-License-Identifier: MIT
 *
 * Doubly linked list (std::list).
 * Insert and erase do not invalidate iterators to other elements.
 */
#ifndef CADT_LIST_H
#define CADT_LIST_H

#include "cadt/common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cadt_list cadt_list;

typedef struct cadt_list_iter {
    void *node;
    const void *owner;
} cadt_list_iter;

cadt_list *cadt_list_create(const cadt_type *type, const cadt_allocator *alloc);
void cadt_list_destroy(cadt_list *list);

size_t cadt_list_size(const cadt_list *list);
bool cadt_list_empty(const cadt_list *list);
void cadt_list_clear(cadt_list *list);

void *cadt_list_front(const cadt_list *list);
void *cadt_list_back(const cadt_list *list);

cadt_list_iter cadt_list_begin(const cadt_list *list);
cadt_list_iter cadt_list_end(const cadt_list *list);
cadt_list_iter cadt_list_next(cadt_list_iter it);
cadt_list_iter cadt_list_prev(cadt_list_iter it);
bool cadt_list_iter_eq(cadt_list_iter a, cadt_list_iter b);
/* nullptr at end. */
void *cadt_list_iter_data(cadt_list_iter it);

cadt_status cadt_list_push_back(cadt_list *list, const void *elem);
cadt_status cadt_list_push_front(cadt_list *list, const void *elem);
cadt_status cadt_list_pop_back(cadt_list *list, void *out);
cadt_status cadt_list_pop_front(cadt_list *list, void *out);

/* Insert before pos. out_it may be nullptr. */
cadt_status cadt_list_insert(cadt_list *list, cadt_list_iter pos, const void *elem,
                             cadt_list_iter *out_it);
/* next may be nullptr. Erasing end returns CADT_ERR_RANGE. */
cadt_status cadt_list_erase(cadt_list *list, cadt_list_iter pos, void *out,
                            cadt_list_iter *next);

void cadt_list_reverse(cadt_list *list);
/* Stable merge sort. Equivalent elements keep their relative order. */
void cadt_list_sort(cadt_list *list);

void *cadt_list_find(const cadt_list *list, const void *elem);

#ifdef __cplusplus
}
#endif

#endif /* CADT_LIST_H */
