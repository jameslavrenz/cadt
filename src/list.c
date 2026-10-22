/* cadt - C Abstract Data Types
 * SPDX-License-Identifier: MIT
 */
#include "cadt/list.h"

#include "internal.h"

#include <stdalign.h>

typedef struct cadt_dnode {
    struct cadt_dnode *prev;
    struct cadt_dnode *next;
    _Alignas(max_align_t) unsigned char data[];
} cadt_dnode;

struct cadt_list {
    const cadt_allocator *alloc;
    cadt_type type;
    cadt_dnode *nil;
    size_t size;
};

static cadt_dnode *node_of(cadt_list_iter it)
{
    return (cadt_dnode *)it.node;
}

static cadt_list_iter iter_of(const cadt_list *list, cadt_dnode *node)
{
    cadt_list_iter it;

    it.node = node;
    it.owner = list;
    return it;
}

static cadt_status insert_before(cadt_list *list, cadt_dnode *pos, const void *elem,
                                 cadt_dnode **out_node)
{
    size_t bytes;
    cadt_dnode *node;
    cadt_status status;

    if (!cadt_add_size(offsetof(cadt_dnode, data), list->type.size, &bytes)) {
        return CADT_ERR_OVERFLOW;
    }
    node = (cadt_dnode *)cadt_xalloc(list->alloc, bytes);
    if (node == NULL) {
        return CADT_ERR_NOMEM;
    }
    status = cadt_type_copy(&list->type, node->data, elem);
    if (status != CADT_OK) {
        cadt_xfree(list->alloc, node);
        return status;
    }
    node->next = pos;
    node->prev = pos->prev;
    pos->prev->next = node;
    pos->prev = node;
    list->size++;
    if (out_node != NULL) {
        *out_node = node;
    }
    return CADT_OK;
}

static cadt_status erase_node(cadt_list *list, cadt_dnode *node, void *out,
                              cadt_dnode **next)
{
    cadt_dnode *after;

    if (node == NULL || node == list->nil) {
        return CADT_ERR_RANGE;
    }
    after = node->next;
    node->prev->next = node->next;
    node->next->prev = node->prev;
    if (out != NULL) {
        memcpy(out, node->data, list->type.size);
    } else {
        cadt_type_destroy(&list->type, node->data);
    }
    cadt_xfree(list->alloc, node);
    list->size--;
    if (next != NULL) {
        *next = after;
    }
    return CADT_OK;
}

cadt_list *cadt_list_create(const cadt_type *type, const cadt_allocator *alloc)
{
    cadt_list *list;

    if (cadt_type_check(type) != CADT_OK) {
        return NULL;
    }
    alloc = cadt_alloc_or_default(alloc);
    list = (cadt_list *)cadt_xalloc(alloc, sizeof(*list));
    if (list == NULL) {
        return NULL;
    }
    list->nil = (cadt_dnode *)cadt_xalloc(alloc, sizeof(*list->nil));
    if (list->nil == NULL) {
        cadt_xfree(alloc, list);
        return NULL;
    }
    list->alloc = alloc;
    list->type = *type;
    list->nil->prev = list->nil;
    list->nil->next = list->nil;
    list->size = 0;
    return list;
}

void cadt_list_clear(cadt_list *list)
{
    if (list == NULL) {
        return;
    }
    while (list->nil->next != list->nil) {
        (void)erase_node(list, list->nil->next, NULL, NULL);
    }
}

void cadt_list_destroy(cadt_list *list)
{
    if (list == NULL) {
        return;
    }
    cadt_list_clear(list);
    cadt_xfree(list->alloc, list->nil);
    cadt_xfree(list->alloc, list);
}

size_t cadt_list_size(const cadt_list *list)
{
    return list == NULL ? 0 : list->size;
}

bool cadt_list_empty(const cadt_list *list)
{
    return cadt_list_size(list) == 0;
}

void *cadt_list_front(const cadt_list *list)
{
    if (list == NULL || list->size == 0) {
        return NULL;
    }
    return list->nil->next->data;
}

void *cadt_list_back(const cadt_list *list)
{
    if (list == NULL || list->size == 0) {
        return NULL;
    }
    return list->nil->prev->data;
}

cadt_list_iter cadt_list_begin(const cadt_list *list)
{
    if (list == NULL) {
        return iter_of(NULL, NULL);
    }
    return iter_of(list, list->nil->next);
}

cadt_list_iter cadt_list_end(const cadt_list *list)
{
    if (list == NULL) {
        return iter_of(NULL, NULL);
    }
    return iter_of(list, list->nil);
}

cadt_list_iter cadt_list_next(cadt_list_iter it)
{
    cadt_dnode *node = node_of(it);

    if (node == NULL) {
        return it;
    }
    return iter_of((const cadt_list *)it.owner, node->next);
}

cadt_list_iter cadt_list_prev(cadt_list_iter it)
{
    cadt_dnode *node = node_of(it);

    if (node == NULL) {
        return it;
    }
    return iter_of((const cadt_list *)it.owner, node->prev);
}

bool cadt_list_iter_eq(cadt_list_iter a, cadt_list_iter b)
{
    return a.node == b.node;
}

void *cadt_list_iter_data(cadt_list_iter it)
{
    const cadt_list *list = (const cadt_list *)it.owner;
    cadt_dnode *node = node_of(it);

    if (list == NULL || node == NULL || node == list->nil) {
        return NULL;
    }
    return node->data;
}

cadt_status cadt_list_push_back(cadt_list *list, const void *elem)
{
    if (list == NULL || elem == NULL) {
        return CADT_ERR_INVALID;
    }
    return insert_before(list, list->nil, elem, NULL);
}

cadt_status cadt_list_push_front(cadt_list *list, const void *elem)
{
    if (list == NULL || elem == NULL) {
        return CADT_ERR_INVALID;
    }
    return insert_before(list, list->nil->next, elem, NULL);
}

cadt_status cadt_list_pop_back(cadt_list *list, void *out)
{
    if (list == NULL) {
        return CADT_ERR_INVALID;
    }
    if (list->size == 0) {
        return CADT_ERR_EMPTY;
    }
    return erase_node(list, list->nil->prev, out, NULL);
}

cadt_status cadt_list_pop_front(cadt_list *list, void *out)
{
    if (list == NULL) {
        return CADT_ERR_INVALID;
    }
    if (list->size == 0) {
        return CADT_ERR_EMPTY;
    }
    return erase_node(list, list->nil->next, out, NULL);
}

cadt_status cadt_list_insert(cadt_list *list, cadt_list_iter pos, const void *elem,
                             cadt_list_iter *out_it)
{
    cadt_dnode *node = NULL;
    cadt_status status;

    if (list == NULL || elem == NULL || node_of(pos) == NULL) {
        return CADT_ERR_INVALID;
    }
    status = insert_before(list, node_of(pos), elem, &node);
    if (status == CADT_OK && out_it != NULL) {
        *out_it = iter_of(list, node);
    }
    return status;
}

cadt_status cadt_list_erase(cadt_list *list, cadt_list_iter pos, void *out,
                            cadt_list_iter *next)
{
    cadt_dnode *after = NULL;
    cadt_status status;

    if (list == NULL) {
        return CADT_ERR_INVALID;
    }
    status = erase_node(list, node_of(pos), out, &after);
    if (status == CADT_OK && next != NULL) {
        *next = iter_of(list, after);
    }
    return status;
}

void cadt_list_reverse(cadt_list *list)
{
    cadt_dnode *cur;

    if (list == NULL || list->size < 2u) {
        return;
    }
    cur = list->nil;
    do {
        cadt_dnode *old_next = cur->next;

        cur->next = cur->prev;
        cur->prev = old_next;
        cur = old_next;
    } while (cur != list->nil);
}

static cadt_dnode *merge_nodes(cadt_dnode *a, cadt_dnode *b, const cadt_type *type)
{
    cadt_dnode dummy;
    cadt_dnode *tail = &dummy;

    dummy.next = NULL;
    while (a != NULL && b != NULL) {
        if (cadt_type_compare(type, a->data, b->data) <= 0) {
            tail->next = a;
            a = a->next;
        } else {
            tail->next = b;
            b = b->next;
        }
        tail = tail->next;
    }
    tail->next = a != NULL ? a : b;
    return dummy.next;
}

static cadt_dnode *merge_sort_nodes(cadt_dnode *head, const cadt_type *type)
{
    cadt_dnode *slow;
    cadt_dnode *fast;
    cadt_dnode *mid;

    if (head == NULL || head->next == NULL) {
        return head;
    }
    slow = head;
    fast = head->next;
    while (fast != NULL && fast->next != NULL) {
        slow = slow->next;
        fast = fast->next->next;
    }
    mid = slow->next;
    slow->next = NULL;
    return merge_nodes(merge_sort_nodes(head, type), merge_sort_nodes(mid, type), type);
}

void cadt_list_sort(cadt_list *list)
{
    cadt_dnode *head;
    cadt_dnode *prev;
    cadt_dnode *cur;

    if (list == NULL || list->size < 2u) {
        return;
    }
    head = list->nil->next;
    list->nil->prev->next = NULL;
    head = merge_sort_nodes(head, &list->type);
    prev = list->nil;
    cur = head;
    list->nil->next = head;
    while (cur != NULL) {
        cur->prev = prev;
        prev = cur;
        cur = cur->next;
    }
    prev->next = list->nil;
    list->nil->prev = prev;
}

void *cadt_list_find(const cadt_list *list, const void *elem)
{
    cadt_dnode *cur;

    if (list == NULL || elem == NULL) {
        return NULL;
    }
    for (cur = list->nil->next; cur != list->nil; cur = cur->next) {
        if (cadt_type_compare(&list->type, cur->data, elem) == 0) {
            return cur->data;
        }
    }
    return NULL;
}
