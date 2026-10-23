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
 * set-test.c – test program for the generic set wrapper.
 */

#include <stdio.h>
#include <stdlib.h>

#include <set.h>

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

/* Copy an int into a newly allocated object. */
static void *int_copy(const void *src)
{
    const int *i = (const int *)src;
    int *p = (int *)malloc(sizeof(int));
    if (!p) return NULL;
    *p = *i;
    return p;
}

/* Compare two ints. */
static int int_cmp(const void *a, const void *b)
{
    int ia = *(const int *)a;
    int ib = *(const int *)b;
    return (ia > ib) - (ia < ib);
}

/* Free an int. */
static void int_free(void *obj)
{
    free(obj);
}

/* Collect visited keys in order. */
typedef struct {
    int *keys;
    size_t count;
    size_t cap;
} int_list;

static int list_append(int_list *list, int value)
{
    if (list->count == list->cap) {
        size_t new_cap = list->cap ? list->cap * 2 : 8;
        int *tmp = (int *)realloc(list->keys, new_cap * sizeof(int));
        if (!tmp) return -1;
        list->keys = tmp;
        list->cap = new_cap;
    }
    list->keys[list->count++] = value;
    return 0;
}

static int collect_cb(void *item, void *ctx)
{
    int_list *list = (int_list *)ctx;
    int *value = (int *)item;
    return list_append(list, *value);
}

int main(void)
{
    set_t *s = set_alloc(int_copy, int_cmp, int_free);
    CHECK(s != NULL, "set_alloc returns non‑NULL");
    CHECK(set_size(s) == 0, "size is zero after alloc");

    int values[] = {5, 3, 8, 1, 4, 7, 9};
    size_t nvalues = sizeof(values) / sizeof(values[0]);

    for (size_t i = 0; i < nvalues; ++i) {
        CHECK(set_add(s, &values[i]) == 0, "set_add succeeds");
    }
    CHECK(set_size(s) == nvalues, "size after adding all values");

    for (size_t i = 0; i < nvalues; ++i) {
        CHECK(set_contains(s, &values[i]) == 1, "set_contains finds present item");
    }
    int missing = 42;
    CHECK(set_contains(s, &missing) == 0, "set_contains returns 0 for missing item");

    /* Duplicate add should not change size. */
    int duplicate = 5;
    CHECK(set_add(s, &duplicate) == 0, "duplicate set_add succeeds");
    CHECK(set_size(s) == nvalues, "size unchanged after duplicate add");

    /* Remove an existing item. */
    int to_remove = 5;
    CHECK(set_remove(s, &to_remove) == 0, "set_remove existing item");
    CHECK(set_size(s) == nvalues - 1, "size decreases after remove");
    CHECK(set_contains(s, &to_remove) == 0, "removed item is absent");

    /* Remove a missing item. */
    CHECK(set_remove(s, &missing) == -1, "set_remove missing item returns -1");

    /* Add it back. */
    CHECK(set_add(s, &to_remove) == 0, "re-add removed item");
    CHECK(set_size(s) == nvalues, "size restored after re-add");

    /* Foreach yields ascending order. */
    int expected[] = {1, 3, 4, 5, 7, 8, 9};
    size_t nexpected = sizeof(expected) / sizeof(expected[0]);
    int_list list = {NULL, 0, 0};
    int rc = set_foreach(s, collect_cb, &list);
    CHECK(rc == 0, "set_foreach returns 0");
    CHECK(list.count == nexpected, "foreach visits all items");
    for (size_t i = 0; i < list.count; ++i) {
        CHECK(list.keys[i] == expected[i], "foreach yields sorted items");
    }
    free(list.keys);

    /* Test set_union and set_intersect. */
    int avals[] = {1, 3, 5, 7, 9};
    int bvals[] = {3, 4, 5, 6, 7};
    int uvals[] = {1, 3, 4, 5, 6, 7, 9};
    int ivals[] = {3, 5, 7};
    size_t navals = sizeof(avals) / sizeof(avals[0]);
    size_t nbvals = sizeof(bvals) / sizeof(bvals[0]);
    size_t nuvals = sizeof(uvals) / sizeof(uvals[0]);
    size_t nivals = sizeof(ivals) / sizeof(ivals[0]);

    set_t *a = set_alloc(int_copy, int_cmp, int_free);
    set_t *b = set_alloc(int_copy, int_cmp, int_free);
    CHECK(a != NULL && b != NULL, "alloc union/intersection operands");

    for (size_t i = 0; i < navals; ++i) {
        CHECK(set_add(a, &avals[i]) == 0, "add operand A item");
    }
    for (size_t i = 0; i < nbvals; ++i) {
        CHECK(set_add(b, &bvals[i]) == 0, "add operand B item");
    }

    set_t *u = set_union(a, b);
    CHECK(u != NULL, "set_union returns non-NULL");
    CHECK(set_size(u) == nuvals, "union has expected size");
    for (size_t i = 0; i < nuvals; ++i) {
        CHECK(set_contains(u, &uvals[i]) == 1, "union contains expected item");
    }
    int absent = 2;
    CHECK(set_contains(u, &absent) == 0, "union excludes absent item");

    set_t *inter = set_intersect(a, b);
    CHECK(inter != NULL, "set_intersect returns non-NULL");
    CHECK(set_size(inter) == nivals, "intersection has expected size");
    for (size_t i = 0; i < nivals; ++i) {
        CHECK(set_contains(inter, &ivals[i]) == 1, "intersection contains expected item");
    }
    int only_a = 1;
    CHECK(set_contains(inter, &only_a) == 0, "intersection excludes item not in both");

    /* Results are independent copies of their operands. */
    int three = 3;
    CHECK(set_remove(a, &three) == 0, "remove item from operand after set ops");
    CHECK(set_contains(u, &three) == 1, "union remains independent of operands");
    CHECK(set_contains(inter, &three) == 1, "intersection remains independent of operands");

    set_free(u);
    set_free(inter);
    set_free(a);
    set_free(b);

    set_free(s);
    return g_failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
