/*
 * MIT License
 *
 * Copyright (c) 2026 Bob Lowe <rlowe8@utm.edu>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * list-test.c – test program for the generic doubly linked list.
 */

#include <stdio.h>
#include <stdlib.h>

#include "../include/list.h"

static int g_failures = 0;

#define CHECK(cond, msg)                                            \
    do {                                                            \
        if (!(cond)) {                                              \
            fprintf(stderr, "FAIL: %s (line %d)\n", msg, __LINE__);\
            g_failures++;                                           \
        } else {                                                    \
            printf("ok: %s\n", msg);                                \
        }                                                           \
    } while (0)

/* Simple copy/free for integers allocated on the heap. */
static void *int_copy(const void *src)
{
    const int *i = (const int *)src;
    int *p = (int *)malloc(sizeof(int));
    if (!p) return NULL;
    *p = *i;
    return p;
}

static void int_free(void *obj)
{
    free(obj);
}

int main(void)
{
    /* 1. Allocate list. */
    list_t *lst = list_alloc(int_copy, int_free);
    CHECK(lst != NULL, "list_alloc returns non‑NULL");
    CHECK(list_size(lst) == 0, "size is zero after alloc");

    /* 2. Push back a sequence and pop front. */
    for (int i = 0; i < 10; ++i) {
        int val = i;
        CHECK(list_push_back(lst, &val) == 0, "list_push_back succeeded");
        CHECK(list_size(lst) == (size_t)(i + 1), "size grows correctly");
    }
    CHECK(list_size(lst) == 10, "size after pushes");
    for (int i = 0; i < 10; ++i) {
        void *out = NULL;
        CHECK(list_pop_front(lst, &out) == 0, "list_pop_front returns value");
        CHECK(out != NULL, "pop returned non‑NULL");
        CHECK(*(int *)out == i, "pop_front yields correct order");
        int_free(out);
    }
    CHECK(list_size(lst) == 0, "list empty after pops");

    /* 3. Push front and pop back. */
    for (int i = 0; i < 10; ++i) {
        int val = i;
        CHECK(list_push_front(lst, &val) == 0, "list_push_front succeeded");
    }
    CHECK(list_size(lst) == 10, "size after push_front");
    for (int i = 0; i < 10; ++i) {
        void *out = NULL;
        CHECK(list_pop_back(lst, &out) == 0, "list_pop_back returns value");
        CHECK(out != NULL, "pop_back non‑NULL");
        CHECK(*(int *)out == i, "pop_back yields reverse order of push_front");
        int_free(out);
    }
    CHECK(list_size(lst) == 0, "list empty after pop_back");

    /* 4. Interleaved operations and pointer stability. */
    int first_val = 42;
    CHECK(list_push_back(lst, &first_val) == 0, "push first element");
    void *peek = NULL;
    CHECK(list_pop_front(lst, &peek) == 0, "peek front");
    CHECK(peek != NULL && *(int *)peek == 42, "front returns original copy");
    int_free(peek);

    /* Re‑populate and cause many allocations. */
    for (int i = 0; i < 50; ++i) {
        CHECK(list_push_back(lst, &i) == 0, "fill list");
    }
    /* Ensure the first stored element (value 0) is still correct after many ops. */
    void *first_ptr = NULL;
    CHECK(list_pop_front(lst, &first_ptr) == 0, "pop first after many pushes");
    CHECK(first_ptr != NULL && *(int *)first_ptr == 0, "first element stable");
    int_free(first_ptr);

    /* Clean up remaining items. */
    while (list_size(lst) > 0) {
        void *tmp = NULL;
        list_pop_front(lst, &tmp);
        int_free(tmp);
    }
    list_free(lst);

    return g_failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
