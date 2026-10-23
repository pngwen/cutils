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
 * set.c – implementation of a generic set backed by the map container.
 */

#include <set.h>
#include <stdlib.h>

/* Set structure – hidden from users. */
struct set {
    map_t *map;
    size_t size;
};

/** Allocate a new empty set. */
set_t *set_alloc(map_create_f create, map_cmp_f cmp, map_free_f free)
{
    if (!create || !cmp) return NULL;

    set_t *s = (set_t *)malloc(sizeof(set_t));
    if (!s) return NULL;

    s->map = map_alloc(create, cmp, free);
    if (!s->map) {
        free(s);
        return NULL;
    }
    s->size = 0;
    return s;
}

/** Destroy a set and every item it contains. */
void set_free(set_t *s)
{
    if (!s) return;
    map_free(s->map);
    free(s);
}

/** Add a copy of @p item to the set. */
int set_add(set_t *s, const void *item)
{
    if (!s || !item) return -1;

    if (map_get(s->map, item) != NULL) {
        return 0; /* already present */
    }

    if (map_insert(s->map, item) != 0) {
        return -1;
    }
    s->size += 1;
    return 0;
}

/** Remove the item equivalent to @p item. */
int set_remove(set_t *s, const void *item)
{
    if (!s || !item) return -1;

    if (map_remove(s->map, item) != 0) {
        return -1;
    }
    s->size -= 1;
    return 0;
}

/** Test whether an equivalent item is present. */
int set_contains(set_t *s, const void *item)
{
    if (!s || !item) return 0;
    return map_get(s->map, item) != NULL ? 1 : 0;
}

/** Return the number of elements in the set. */
size_t set_size(const set_t *s)
{
    return s ? s->size : 0;
}

/** Apply @p func to every item in ascending order. */
int set_foreach(set_t *s, int (*func)(void *item, void *ctx), void *ctx)
{
    if (!s || !func) return -1;
    return map_foreach(s->map, func, ctx);
}

/* Callback used by set_union to add an item to the destination set. */
static int set_union_cb(void *item, void *ctx)
{
    return set_add((set_t *)ctx, item);
}

/* Context for set_intersect traversal. */
typedef struct {
    set_t *other;
    set_t *result;
} set_intersect_ctx_t;

/* Callback used by set_intersect; adds item only if present in the other set. */
static int set_intersect_cb(void *item, void *ctx)
{
    set_intersect_ctx_t *c = (set_intersect_ctx_t *)ctx;
    if (set_contains(c->other, item)) {
        return set_add(c->result, item);
    }
    return 0;
}

/** Create a new set containing every element from both sets. */
set_t *set_union(set_t *a, set_t *b)
{
    if (!a || !b) return NULL;

    set_t *result = set_alloc(a->map->create, a->map->cmp, a->map->free);
    if (!result) return NULL;

    int rc = set_foreach(a, set_union_cb, result);
    if (rc == 0) {
        rc = set_foreach(b, set_union_cb, result);
    }
    if (rc != 0) {
        set_free(result);
        return NULL;
    }
    return result;
}

/** Create a new set containing only elements present in both sets. */
set_t *set_intersect(set_t *a, set_t *b)
{
    if (!a || !b) return NULL;

    set_t *result = set_alloc(a->map->create, a->map->cmp, a->map->free);
    if (!result) return NULL;

    set_intersect_ctx_t ctx = { b, result };
    int rc = set_foreach(a, set_intersect_cb, &ctx);
    if (rc != 0) {
        set_free(result);
        return NULL;
    }
    return result;
}
