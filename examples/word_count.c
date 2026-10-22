#include "cadt.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_live;

static cadt_status copy_cstr(void *dst, const void *src, [[maybe_unused]] void *ctx)
{
    const char *text = *(const char *const *)src;
    size_t length = strlen(text) + 1u;
    char *copy = (char *)malloc(length);
    if (copy == nullptr) {
        return CADT_ERR_NOMEM;
    }
    memcpy(copy, text, length);
    memcpy(dst, &copy, sizeof(copy));
    g_live++;
    return CADT_OK;
}

static void destroy_cstr(void *elem, [[maybe_unused]] void *ctx)
{
    free(*(char **)elem);
    g_live--;
}

static int cmp_cstr(const void *a, const void *b, [[maybe_unused]] void *ctx)
{
    return strcmp(*(const char *const *)a, *(const char *const *)b);
}

static uint64_t hash_cstr(const void *elem, [[maybe_unused]] void *ctx)
{
    const char *text = *(const char *const *)elem;
    return cadt_hash_bytes(text, strlen(text));
}

static int cmp_cstr_ptr(const void *a, const void *b, [[maybe_unused]] void *ctx)
{
    return strcmp(*(const char *const *)a, *(const char *const *)b);
}

static int note(cadt_umap *counts, const char *word)
{
    int *slot = (int *)cadt_umap_get(counts, &word);
    int one = 1;

    if (slot != nullptr) {
        (*slot)++;
        return 1;
    }
    return cadt_umap_insert(counts, &word, &one) == CADT_OK;
}

int main(void)
{
    cadt_type key_type;
    cadt_type value_type;
    cadt_type pointer_type;
    cadt_umap *counts;
    cadt_string *word;
    cadt_vector *keys;
    cadt_umap_iter it;
    int ch;

    memset(&key_type, 0, sizeof(key_type));
    key_type.size = sizeof(char *);
    key_type.copy = copy_cstr;
    key_type.destroy = destroy_cstr;
    key_type.compare = cmp_cstr;
    key_type.hash = hash_cstr;
    memset(&value_type, 0, sizeof(value_type));
    value_type.size = sizeof(int);
    memset(&pointer_type, 0, sizeof(pointer_type));
    pointer_type.size = sizeof(char *);

    counts = cadt_umap_create(&key_type, &value_type, nullptr);
    word = cadt_string_create(nullptr);
    keys = cadt_vector_create(&pointer_type, nullptr);
    if (counts == nullptr || word == nullptr || keys == nullptr) {
        fprintf(stderr, "out of memory\n");
        return 1;
    }

    while ((ch = fgetc(stdin)) != EOF) {
        if (isspace((unsigned char)ch)) {
            if (!cadt_string_empty(word) && !note(counts, cadt_string_c_str(word))) {
                fprintf(stderr, "out of memory\n");
                return 1;
            }
            cadt_string_clear(word);
        } else if (cadt_string_append_char(word, (char)ch) != CADT_OK) {
            fprintf(stderr, "out of memory\n");
            return 1;
        }
    }
    if (!cadt_string_empty(word) && !note(counts, cadt_string_c_str(word))) {
        fprintf(stderr, "out of memory\n");
        return 1;
    }

    for (it = cadt_umap_begin(counts); !cadt_umap_iter_eq(it, cadt_umap_end(counts));
         it = cadt_umap_next(it)) {
        char *text = *(char **)cadt_umap_iter_key(counts, it);
        if (cadt_vector_push_back(keys, &text) != CADT_OK) {
            fprintf(stderr, "out of memory\n");
            return 1;
        }
    }
    if (cadt_sort(cadt_vector_data(keys), cadt_vector_size(keys), sizeof(char *),
                  cmp_cstr_ptr, nullptr) != CADT_OK) {
        fprintf(stderr, "sort failed\n");
        return 1;
    }
    for (size_t i = 0; i < cadt_vector_size(keys); i++) {
        char *text = *(char **)cadt_vector_at(keys, i);
        int *count = (int *)cadt_umap_get(counts, &text);
        printf("%s %d\n", text, count == nullptr ? 0 : *count);
    }

    cadt_vector_destroy(keys);
    cadt_string_destroy(word);
    cadt_umap_destroy(counts);
    return g_live == 0 ? 0 : 1;
}
