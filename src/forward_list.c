/* cadt - C Abstract Data Types
 * SPDX-License-Identifier: MIT
 */
#include "cadt/forward_list.h"

#include "internal.h"

#include <stdalign.h>

typedef struct cadt_snode {
    struct cadt_snode *next;
    _Alignas(max_align_t) unsigned char data[];
} cadt_snode;

struct cadt_flist {
    const cadt_allocator *alloc;
    cadt_type type;
    cadt_snode *head;
    size_t size;
};

static cadt_snode *node_of(cadt_flist_iter it)
{
    return (cadt_snode *)it.node;
}

static cadt_flist_iter make_iter(cadt_snode *node, cadt_flist_kind kind)
{
    cadt_flist_iter it;

    it.node = node;
    it.kind = kind;
    return it;
}

static cadt_status make_node(cadt_flist *list, const void *elem, cadt_snode **out)
{
    size_t bytes;
    cadt_snode *node;
    cadt_status status;

    if (!cadt_add_size(offsetof(cadt_snode, data), list->type.size, &bytes)) {
        return CADT_ERR_OVERFLOW;
    }
    node = (cadt_snode *)cadt_xalloc(list->alloc, bytes);
    if (node == NULL) {
        return CADT_ERR_NOMEM;
    }
    status = cadt_type_copy(&list->type, node->data, elem);
    if (status != CADT_OK) {
        cadt_xfree(list->alloc, node);
        return status;
    }
    node->next = NULL;
    *out = node;
    return CADT_OK;
}

static void destroy_node(cadt_flist *list, cadt_snode *node, void *out)
{
    if (out != NULL) {
        memcpy(out, node->data, list->type.size);
    } else {
        cadt_type_destroy(&list->type, node->data);
    }
    cadt_xfree(list->alloc, node);
}

cadt_flist *cadt_flist_create(const cadt_type *type, const cadt_allocator *alloc)
{
    cadt_flist *list;

    if (cadt_type_check(type) != CADT_OK) {
        return NULL;
    }
    alloc = cadt_alloc_or_default(alloc);
    list = (cadt_flist *)cadt_xalloc(alloc, sizeof(*list));
    if (list == NULL) {
        return NULL;
    }
    list->alloc = alloc;
    list->type = *type;
    list->head = NULL;
    list->size = 0;
    return list;
}

void cadt_flist_clear(cadt_flist *list)
{
    cadt_snode *cur;

    if (list == NULL) {
        return;
    }
    cur = list->head;
    while (cur != NULL) {
        cadt_snode *next = cur->next;

        destroy_node(list, cur, NULL);
        cur = next;
    }
    list->head = NULL;
    list->size = 0;
}

void cadt_flist_destroy(cadt_flist *list)
{
    if (list == NULL) {
        return;
    }
    cadt_flist_clear(list);
    cadt_xfree(list->alloc, list);
}

size_t cadt_flist_size(const cadt_flist *list)
{
    return list == NULL ? 0 : list->size;
}

bool cadt_flist_empty(const cadt_flist *list)
{
    return cadt_flist_size(list) == 0;
}

void *cadt_flist_front(const cadt_flist *list)
{
    if (list == NULL || list->head == NULL) {
        return NULL;
    }
    return list->head->data;
}

cadt_flist_iter cadt_flist_before_begin(const cadt_flist *list)
{
    (void)list;
    return make_iter(NULL, CADT_FLIST_BEFORE);
}

cadt_flist_iter cadt_flist_begin(const cadt_flist *list)
{
    if (list == NULL || list->head == NULL) {
        return make_iter(NULL, CADT_FLIST_END);
    }
    return make_iter(list->head, CADT_FLIST_NODE);
}

cadt_flist_iter cadt_flist_end(const cadt_flist *list)
{
    (void)list;
    return make_iter(NULL, CADT_FLIST_END);
}

cadt_flist_iter cadt_flist_next(const cadt_flist *list, cadt_flist_iter it)
{
    if (it.kind == CADT_FLIST_BEFORE) {
        return cadt_flist_begin(list);
    }
    if (it.kind == CADT_FLIST_END || node_of(it) == NULL || node_of(it)->next == NULL) {
        return cadt_flist_end(list);
    }
    return make_iter(node_of(it)->next, CADT_FLIST_NODE);
}

bool cadt_flist_iter_eq(cadt_flist_iter a, cadt_flist_iter b)
{
    return a.kind == b.kind && a.node == b.node;
}

void *cadt_flist_iter_data(cadt_flist_iter it)
{
    if (it.kind != CADT_FLIST_NODE || it.node == NULL) {
        return NULL;
    }
    return node_of(it)->data;
}

cadt_status cadt_flist_push_front(cadt_flist *list, const void *elem)
{
    cadt_snode *node;
    cadt_status status;

    if (list == NULL || elem == NULL) {
        return CADT_ERR_INVALID;
    }
    status = make_node(list, elem, &node);
    if (status != CADT_OK) {
        return status;
    }
    node->next = list->head;
    list->head = node;
    list->size++;
    return CADT_OK;
}

cadt_status cadt_flist_pop_front(cadt_flist *list, void *out)
{
    cadt_snode *node;

    if (list == NULL) {
        return CADT_ERR_INVALID;
    }
    if (list->head == NULL) {
        return CADT_ERR_EMPTY;
    }
    node = list->head;
    list->head = node->next;
    list->size--;
    destroy_node(list, node, out);
    return CADT_OK;
}

cadt_status cadt_flist_insert_after(cadt_flist *list, cadt_flist_iter pos,
                                    const void *elem, cadt_flist_iter *out_it)
{
    cadt_snode *node;
    cadt_snode *after;
    cadt_status status;

    if (list == NULL || elem == NULL || pos.kind == CADT_FLIST_END) {
        return CADT_ERR_INVALID;
    }
    if (pos.kind == CADT_FLIST_BEFORE) {
        status = cadt_flist_push_front(list, elem);
        if (status == CADT_OK && out_it != NULL) {
            *out_it = cadt_flist_begin(list);
        }
        return status;
    }
    if (pos.node == NULL) {
        return CADT_ERR_INVALID;
    }
    status = make_node(list, elem, &node);
    if (status != CADT_OK) {
        return status;
    }
    after = node_of(pos);
    node->next = after->next;
    after->next = node;
    list->size++;
    if (out_it != NULL) {
        *out_it = make_iter(node, CADT_FLIST_NODE);
    }
    return CADT_OK;
}

cadt_status cadt_flist_erase_after(cadt_flist *list, cadt_flist_iter pos, void *out)
{
    cadt_snode *prev;
    cadt_snode *doomed;

    if (list == NULL || pos.kind == CADT_FLIST_END) {
        return CADT_ERR_INVALID;
    }
    if (pos.kind == CADT_FLIST_BEFORE) {
        return cadt_flist_pop_front(list, out);
    }
    prev = node_of(pos);
    if (prev == NULL || prev->next == NULL) {
        return CADT_ERR_RANGE;
    }
    doomed = prev->next;
    prev->next = doomed->next;
    list->size--;
    destroy_node(list, doomed, out);
    return CADT_OK;
}

void cadt_flist_reverse(cadt_flist *list)
{
    cadt_snode *prev = NULL;
    cadt_snode *cur;

    if (list == NULL) {
        return;
    }
    cur = list->head;
    while (cur != NULL) {
        cadt_snode *next = cur->next;

        cur->next = prev;
        prev = cur;
        cur = next;
    }
    list->head = prev;
}

static cadt_snode *merge_nodes(cadt_snode *a, cadt_snode *b, const cadt_type *type)
{
    cadt_snode dummy;
    cadt_snode *tail = &dummy;

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

static cadt_snode *merge_sort_nodes(cadt_snode *head, const cadt_type *type)
{
    cadt_snode *slow;
    cadt_snode *fast;
    cadt_snode *mid;

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

void cadt_flist_sort(cadt_flist *list)
{
    if (list == NULL || list->size < 2u) {
        return;
    }
    list->head = merge_sort_nodes(list->head, &list->type);
}

void *cadt_flist_find(const cadt_flist *list, const void *elem)
{
    cadt_snode *cur;

    if (list == NULL || elem == NULL) {
        return NULL;
    }
    for (cur = list->head; cur != NULL; cur = cur->next) {
        if (cadt_type_compare(&list->type, cur->data, elem) == 0) {
            return cur->data;
        }
    }
    return NULL;
}
