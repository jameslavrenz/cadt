/* cadt - C Abstract Data Types
 * SPDX-License-Identifier: MIT
 *
 * Singly linked list (std::forward_list).
 */
#ifndef CADT_FORWARD_LIST_H
#define CADT_FORWARD_LIST_H

#include "cadt/common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cadt_flist cadt_flist;

typedef enum cadt_flist_kind {
    CADT_FLIST_NODE = 0,
    CADT_FLIST_BEFORE = 1,
    CADT_FLIST_END = 2
} cadt_flist_kind;

typedef struct cadt_flist_iter {
    void *node;
    cadt_flist_kind kind;
} cadt_flist_iter;

cadt_flist *cadt_flist_create(const cadt_type *type, const cadt_allocator *alloc);
void cadt_flist_destroy(cadt_flist *list);

size_t cadt_flist_size(const cadt_flist *list);
bool cadt_flist_empty(const cadt_flist *list);
void cadt_flist_clear(cadt_flist *list);

void *cadt_flist_front(const cadt_flist *list);

cadt_flist_iter cadt_flist_before_begin(const cadt_flist *list);
cadt_flist_iter cadt_flist_begin(const cadt_flist *list);
cadt_flist_iter cadt_flist_end(const cadt_flist *list);
cadt_flist_iter cadt_flist_next(const cadt_flist *list, cadt_flist_iter it);
bool cadt_flist_iter_eq(cadt_flist_iter a, cadt_flist_iter b);
void *cadt_flist_iter_data(cadt_flist_iter it);

cadt_status cadt_flist_push_front(cadt_flist *list, const void *elem);
cadt_status cadt_flist_pop_front(cadt_flist *list, void *out);
cadt_status cadt_flist_insert_after(cadt_flist *list, cadt_flist_iter pos,
                                    const void *elem, cadt_flist_iter *out_it);
cadt_status cadt_flist_erase_after(cadt_flist *list, cadt_flist_iter pos, void *out);

void cadt_flist_reverse(cadt_flist *list);
void cadt_flist_sort(cadt_flist *list);
void *cadt_flist_find(const cadt_flist *list, const void *elem);

#ifdef __cplusplus
}
#endif

#endif /* CADT_FORWARD_LIST_H */
