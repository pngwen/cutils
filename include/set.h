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
 * set.h – generic set wrapper around the map container.
 */

#ifndef SET_H
#define SET_H

#include <stddef.h>
#include <map.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Opaque set handle. */
typedef struct set set_t;

/** Allocate a new empty set.
 * @param create Function to allocate/copy an item.
 * @param cmp    Function to compare two items.
 * @param free   Function to free an item.
 * @return Pointer to the new set, or NULL on allocation failure.
 */
set_t *set_alloc(map_create_f create, map_cmp_f cmp, map_free_f free);

/** Destroy a set and every item it contains.
 * @param s Set to destroy (may be NULL).
 */
void set_free(set_t *s);

/** Add a copy of @p item to the set.
 * If an equivalent item already exists, the set is left unchanged.
 * @param s    Set to modify (must not be NULL).
 * @param item Item to add (must not be NULL).
 * @return 0 on success (including already present), -1 on allocation failure.
 */
int set_add(set_t *s, const void *item);

/** Remove the item equivalent to @p item.
 * @param s    Set to modify (must not be NULL).
 * @param item Item to remove (must not be NULL).
 * @return 0 if removed, -1 if not found.
 */
int set_remove(set_t *s, const void *item);

/** Test whether an equivalent item is present.
 * @param s    Set to query (must not be NULL).
 * @param item Item to test (must not be NULL).
 * @return 1 if present, 0 otherwise.
 */
int set_contains(set_t *s, const void *item);

/** Return the number of elements in the set. */
size_t set_size(const set_t *s);

/** Apply @p func to every item in ascending order.
 * @param s    Set to traverse (must not be NULL).
 * @param func Callback invoked with each borrowed item and @p ctx.
 * @param ctx  Context pointer passed unchanged to @p func.
 * @return 0 if all items were visited, or the non-zero value returned by
 *         @p func that stopped iteration.
 */
int set_foreach(set_t *s, int (*func)(void *item, void *ctx), void *ctx);

/** Create a new set containing every element from both sets.
 * Both sets must use compatible copy, comparison, and free functions.
 * @param a First set (must not be NULL).
 * @param b Second set (must not be NULL).
 * @return Newly allocated union set, or NULL on failure.
 */
set_t *set_union(set_t *a, set_t *b);

/** Create a new set containing only elements present in both sets.
 * Both sets must use compatible copy, comparison, and free functions.
 * @param a First set (must not be NULL).
 * @param b Second set (must not be NULL).
 * @return Newly allocated intersection set, or NULL on failure.
 */
set_t *set_intersect(set_t *a, set_t *b);

#ifdef __cplusplus
}
#endif

#endif // SET_H
