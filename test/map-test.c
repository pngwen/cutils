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
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
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
 * map-test.c – test program for the generic map (associative array).
 *
 * Statically linked against libcutils.a.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <map.h>

/* ------------------------------------------------------------------ */
/* Key‑value integer type helpers                                     */
/* ------------------------------------------------------------------ */

typedef struct {
    int key;
    int value;
} kv_t;

static void *kv_create(const void *src)
{
    const kv_t *s = (const kv_t *)src;
    kv_t *copy = (kv_t *)malloc(sizeof(kv_t));
    if (!copy) return NULL;
    *copy = *s;
    return copy;
}

static int kv_cmp(const void *a, const void *b)
{
    int ka = ((const kv_t *)a)->key;
    int kb = ((const kv_t *)b)->key;
    return (ka > kb) - (ka < kb);
}

static void kv_free(void *obj)
{
    free(obj);
}

/* ------------------------------------------------------------------ */
/* String type helpers                                                */
/* ------------------------------------------------------------------ */

static void *str_create(const void *src)
{
    const char *s = (const char *)src;
    char *copy = (char *)malloc(strlen(s) + 1);
    if (!copy) return NULL;
    strcpy(copy, s);
    return copy;
}

static int str_cmp(const void *a, const void *b)
{
    return strcmp((const char *)a, (const char *)b);
}

static void str_free(void *obj)
{
    free(obj);
}

/* ------------------------------------------------------------------ */
/* Test state                                                         */
/* ------------------------------------------------------------------ */

static int g_failures = 0;

#define CHECK(cond, msg)                                            \
    do {                                                            \
        if (!(cond)) {                                              \
            fprintf(stderr, "FAIL: %s (line %d)\n", msg, __LINE__); \
            g_failures++;                                           \
        } else {                                                    \
            printf("ok: %s\n", msg);                                \
        }                                                           \
    } while (0)

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
    kv_t *kv = (kv_t *)item;
    return list_append(list, kv->key);
}

int main(void)
{
    /* ------------------------------------------------------------------ */
    /* Integer key‑value map                                              */
    /* ------------------------------------------------------------------ */
    map_t *m = map_alloc(kv_create, kv_cmp, kv_free);
    CHECK(m != NULL, "map_alloc returns non‑NULL");

    kv_t entries[] = {
        {5, 50}, {3, 30}, {8, 80}, {1, 10},
        {4, 40}, {7, 70}, {9, 90}
    };
    size_t nentries = sizeof(entries) / sizeof(entries[0]);

    for (size_t i = 0; i < nentries; i++) {
        int rc = map_insert(m, &entries[i]);
        CHECK(rc == 0, "map_insert success");
    }

    /* get each inserted entry */
    for (size_t i = 0; i < nentries; i++) {
        kv_t probe = {entries[i].key, 0};
        void *got = map_get(m, &probe);
        CHECK(got != NULL && ((kv_t *)got)->value == entries[i].value,
              "map_get returns inserted value");
    }

    /* get missing key */
    kv_t missing = {42, 0};
    CHECK(map_get(m, &missing) == NULL, "map_get returns NULL for missing key");

    /* iteration yields keys in ascending order */
    int expected[] = {1, 3, 4, 5, 7, 8, 9};
    size_t nexpected = sizeof(expected) / sizeof(expected[0]);

    int_list list = {NULL, 0, 0};
    map_iter_t *it = map_iter_create(m);
    CHECK(it != NULL, "map_iter_create non‑NULL");

    size_t ei = 0;
    void *item;
    while (map_iter_next(it, &item)) {
        CHECK(ei < nexpected, "iteration does not overrun expected");
        if (ei < nexpected) {
            CHECK(((kv_t *)item)->key == expected[ei],
                  "iteration yields sorted keys");
        }
        ei++;
    }
    CHECK(ei == nexpected, "iteration visits all items");
    {
        void *last = map_iter_get(it);
        CHECK(last != NULL && ((kv_t *)last)->key == 9,
              "map_iter_get returns last item after exhaustion");
    }
    map_iter_destroy(it);

    /* foreach yields the same sorted keys */
    int_list flist = {NULL, 0, 0};
    int rc = map_foreach(m, collect_cb, &flist);
    CHECK(rc == 0, "map_foreach returns 0");
    CHECK(flist.count == nexpected, "foreach visits all items");
    for (size_t i = 0; i < flist.count; i++) {
        CHECK(flist.keys[i] == expected[i], "foreach yields sorted keys");
    }

    /* replace: same key, new value */
    kv_t replacement = {5, 500};
    rc = map_insert(m, &replacement);
    CHECK(rc == 0, "map_insert replace success");
    kv_t probe5 = {5, 0};
    void *got5 = map_get(m, &probe5);
    CHECK(got5 != NULL && ((kv_t *)got5)->value == 500,
          "map_get returns replaced value");

    /* remove an interior key */
    rc = map_remove(m, &(kv_t){3, 0});
    CHECK(rc == 0, "map_remove success");
    CHECK(map_get(m, &(kv_t){3, 0}) == NULL, "removed key not found");

    /* remove missing key */
    CHECK(map_remove(m, &missing) == -1, "map_remove missing returns -1");

    /* remove the key 5 (replaced value) */
    rc = map_remove(m, &probe5);
    CHECK(rc == 0, "map_remove replaced key success");
    CHECK(map_get(m, &probe5) == NULL, "removed key not found");

    /* verify remaining keys via iteration */
    int_list rlist = {NULL, 0, 0};
    map_foreach(m, collect_cb, &rlist);
    int remain[] = {1, 4, 7, 8, 9};
    size_t nremain = sizeof(remain) / sizeof(remain[0]);
    CHECK(rlist.count == nremain, "correct count after removals");
    for (size_t i = 0; i < rlist.count; i++) {
        CHECK(rlist.keys[i] == remain[i], "remaining keys sorted");
    }

    free(list.keys);
    free(flist.keys);
    free(rlist.keys);

    map_free(m);
    map_free(NULL); /* no‑op */

    /* ------------------------------------------------------------------ */
    /* String map (genericity check)                                      */
    /* ------------------------------------------------------------------ */
    map_t *sm = map_alloc(str_create, str_cmp, str_free);
    CHECK(sm != NULL, "string map_alloc non‑NULL");

    CHECK(map_insert(sm, "banana") == 0, "string insert banana");
    CHECK(map_insert(sm, "apple") == 0, "string insert apple");
    CHECK(map_insert(sm, "cherry") == 0, "string insert cherry");

    CHECK(strcmp((char *)map_get(sm, "apple"), "apple") == 0,
          "string get apple");
    CHECK(strcmp((char *)map_get(sm, "banana"), "banana") == 0,
          "string get banana");
    CHECK(strcmp((char *)map_get(sm, "cherry"), "cherry") == 0,
          "string get cherry");
    CHECK(map_get(sm, "durian") == NULL, "string get missing NULL");

    /* string iteration in alphabetical order */
    map_iter_t *sit = map_iter_create(sm);
    char *order[3] = {NULL, NULL, NULL};
    size_t oi = 0;
    void *sitem;
    while (map_iter_next(sit, &sitem) && oi < 3) {
        order[oi++] = (char *)sitem;
    }
    CHECK(oi == 3, "string iteration visits 3 items");
    if (oi == 3) {
        CHECK(strcmp(order[0], "apple") == 0, "string first is apple");
        CHECK(strcmp(order[1], "banana") == 0, "string second is banana");
        CHECK(strcmp(order[2], "cherry") == 0, "string third is cherry");
    }
    map_iter_destroy(sit);

    /* replace string */
    CHECK(map_insert(sm, "apple") == 0, "string insert duplicate apple (replace)");
    CHECK(strcmp((char *)map_get(sm, "apple"), "apple") == 0,
          "string get apple after replace");

    CHECK(map_remove(sm, "banana") == 0, "string remove banana");
    CHECK(map_get(sm, "banana") == NULL, "string removed banana not found");

    map_free(sm);

    if (g_failures) {
        fprintf(stderr, "%d test(s) failed\n", g_failures);
        return EXIT_FAILURE;
    }
    printf("All map tests passed.\n");
    return EXIT_SUCCESS;
}
