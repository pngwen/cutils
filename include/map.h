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
 * map.h – generic associative array backed by a self‑balancing binary tree.
 *
 * The self‑balancing tree is completely hidden behind the opaque map_t handle.
 * Items are stored as owned copies: the map allocates and frees every item it
 * holds via the caller‑supplied function pointers.
 */

#ifndef MAP_H
#define MAP_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct map map_t;

/* ------------------------------------------------------------------ */
/* Function pointer types                                             */
/* ------------------------------------------------------------------ */

/**
 * @brief Allocate and initialize a new owned object from @p src.
 *
 * The map calls this to create every copy it stores. The returned object is
 * owned by the map and will be passed to @p free when removed or destroyed.
 *
 * @param src Source object to copy (must not be NULL).
 * @return Pointer to the newly allocated copy, or NULL on failure.
 */
typedef void *(*map_create_f)(const void *src);

/**
 * @brief Compare two objects for ordering.
 *
 * @param a First object.
 * @param b Second object.
 * @return Negative if a<b, zero if a==b, positive if a>b.
 */
typedef int (*map_cmp_f)(const void *a, const void *b);

/**
 * @brief Deallocate an object previously created by map_create_f.
 *
 * @param obj Object to free (may be NULL).
 */
typedef void (*map_free_f)(void *obj);

/* ------------------------------------------------------------------ */
/* Map type                                                           */
/* ------------------------------------------------------------------ */

/**
 * @brief Opaque associative‑array handle.
 *
 * The @c data member holds the private root of the self‑balancing tree and
 * must only be accessed by the map implementation.
 */
struct map {
    map_create_f create;  /**< Allocate/copy a stored object.   */
    map_cmp_f    cmp;     /**< Compare two stored objects.      */
    map_free_f   free;    /**< Free a stored object.            */
    void        *data;    /**< Private: tree root (opaque).     */
};

/* ------------------------------------------------------------------ */
/* Lifecycle                                                          */
/* ------------------------------------------------------------------ */

/**
 * @brief Create a new empty map.
 *
 * @param create Function to allocate/copy a stored object.
 * @param cmp    Function to compare two stored objects.
 * @param free   Function to free a stored object.
 * @return Pointer to the new map, or NULL on allocation failure.
 */
map_t *map_alloc(map_create_f create, map_cmp_f cmp, map_free_f free);

/**
 * @brief Destroy a map and every owned copy it contains.
 *
 * After this call, @p m must not be used. Passing NULL is a no‑op.
 *
 * @param m Map to destroy (may be NULL).
 */
void map_free(map_t *m);

/* ------------------------------------------------------------------ */
/* Operations                                                         */
/* ------------------------------------------------------------------ */

/**
 * @brief Insert an owned copy of @p item into the map.
 *
 * The item is copied via the map's create function; the map owns the copy.
 * If an entry comparing equal to @p item already exists, its copy is freed
 * and replaced by the new copy (associative‑array semantics).
 *
 * @param m    Map to insert into (must not be NULL).
 * @param item Item to copy and store (must not be NULL).
 * @return 0 on success (inserted or replaced), -1 on allocation failure.
 */
int map_insert(map_t *m, const void *item);

/**
 * @brief Remove the entry matching @p item.
 *
 * The stored copy is freed via the map's free function. If no matching entry
 * exists, nothing happens.
 *
 * @param m    Map to remove from (must not be NULL).
 * @param item Item whose matching entry is removed (must not be NULL).
 * @return 0 if an entry was removed, -1 if no matching entry exists.
 */
int map_remove(map_t *m, const void *item);

/**
 * @brief Borrow a pointer to the stored copy matching @p key.
 *
 * The returned pointer points to internal storage owned by the map. The
 * caller must not free it and must not use it after the matching entry is
 * removed or the map is destroyed.
 *
 * @param m   Map to query (must not be NULL).
 * @param key Key to search for (must not be NULL).
 * @return Borrowed pointer to the stored copy, or NULL if not found.
 */
void *map_get(map_t *m, const void *key);

/* ------------------------------------------------------------------ */
/* Iterator                                                           */
/* ------------------------------------------------------------------ */

/**
 * @brief Opaque in‑order iterator over a map.
 */
typedef struct map_iter map_iter_t;

/**
 * @brief Create an in‑order iterator over @p m.
 *
 * @param m Map to iterate (must not be NULL).
 * @return Pointer to a new iterator, or NULL on allocation failure.
 */
map_iter_t *map_iter_create(map_t *m);

/**
 * @brief Destroy an iterator.
 *
 * Passing NULL is a no‑op.
 *
 * @param iter Iterator to destroy (may be NULL).
 */
void map_iter_destroy(map_iter_t *iter);

/**
 * @brief Advance the iterator and return the next stored item.
 *
 * Items are visited in ascending order according to the map's comparison
 * function. The returned pointer is borrowed from the map.
 *
 * @param iter Iterator (must not be NULL).
 * @param out  Receives the borrowed pointer to the next item.
 * @return 1 if an item was returned, 0 at the end of the map.
 */
int map_iter_next(map_iter_t *iter, void **out);

/**
 * @brief Return the last item returned by map_iter_next without advancing.
 *
 * @param iter Iterator (must not be NULL).
 * @return Borrowed pointer to the current item, or NULL if none has been
 *         returned yet or the iteration is exhausted.
 */
void *map_iter_get(map_iter_t *iter);

/* ------------------------------------------------------------------ */
/* Convenience                                                        */
/* ------------------------------------------------------------------ */

/**
 * @brief Apply @p func to every item in the map, in ascending order.
 *
 * Iteration stops early if @p func returns a non‑zero value.
 *
 * @param m    Map to traverse (must not be NULL).
 * @param func Callback invoked with each borrowed item and @p ctx.
 * @param ctx  Context pointer passed unchanged to @p func.
 * @return 0 if all items were visited, or the non‑zero value returned by
 *         @p func that stopped iteration.
 */
int map_foreach(map_t *m, int (*func)(void *item, void *ctx), void *ctx);

#ifdef __cplusplus
}
#endif

#endif // MAP_H
