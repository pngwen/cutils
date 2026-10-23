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
 * deque-test.c – test program for the generic double‑ended queue.
 */

#include <stdio.h>
#include <stdlib.h>

#include "../include/deque.h"

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

/* Helper to allocate an int on the heap. */
static int *make_int(int v)
{
    int *p = malloc(sizeof(int));
    if (p) *p = v;
    return p;
}

int main(void)
{
    /* 1. Allocate an empty deque. */
    deque_t d = deque_alloc();
    CHECK(d != NULL, "deque_alloc returns non‑NULL");
    CHECK(deque_size(d) == 0, "size is zero after alloc");

    /* 2. Push back a sequence of integers and verify order on pop front. */
    for (int i = 0; i < 10; ++i) {
        int *p = make_int(i);
        CHECK(p != NULL, "malloc int");
        d = deque_push_back(d, p);
        CHECK(d != NULL, "deque_push_back succeeded");
        CHECK(deque_size(d) == (size_t)(i + 1), "size grows correctly");
    }
    CHECK(deque_size(d) == 10, "size after pushes");

    for (int i = 0; i < 10; ++i) {
        int *p = deque_pop_front(d);
        CHECK(p != NULL, "deque_pop_front returns value");
        CHECK(*p == i, "pop_front yields correct order");
        free(p);
    }
    CHECK(deque_size(d) == 0, "deque empty after pops");

    /* 3. Push front and pop back. */
    for (int i = 0; i < 10; ++i) {
        int *p = make_int(i);
        d = deque_push_front(d, p);
    }
    CHECK(deque_size(d) == 10, "size after push_front");
    for (int i = 0; i < 10; ++i) {
        int *p = deque_pop_back(d);
        CHECK(p != NULL, "deque_pop_back returns value");
        CHECK(*p == i, "pop_back yields reverse order of push_front");
        free(p);
    }
    CHECK(deque_size(d) == 0, "deque empty after pop_back");

    /* 4. Interleaved operations and pointer stability test. */
    int *first = make_int(42);
    d = deque_push_back(d, first);
    int *peek = deque_front(d);
    CHECK(peek == first, "deque_front returns original pointer");

    /* cause a resize by adding many elements */
    for (int i = 0; i < 20; ++i) {
        int *p = make_int(i);
        d = deque_push_back(d, p);
    }
    /* the original pointer should still be the same value */
    CHECK(deque_front(d) == first, "pointer stability after resize");

    /* clean up remaining elements */
    while ((peek = deque_pop_front(d)) != NULL) {
        free(peek);
    }

    deque_free(d);

    return g_failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
