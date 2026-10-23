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
 * list.h – generic doubly linked list implementation.
 */

#ifndef LIST_H
#define LIST_H

#include <stddef.h>

/**
 * @file list.h
 * @brief Generic doubly linked list that owns copies of inserted items.
 */

/** Function type to copy an item. Must allocate a new copy and return it.
 * @param src Pointer to source item (must not be NULL).
 * @return Pointer to a newly allocated copy, or NULL on failure.
 */
typedef void *(*list_copy_f)(const void *src);

/** Function type to free a previously copied item.
 * @param obj Pointer to the object to free (may be NULL).
 */
typedef void (*list_free_f)(void *obj);

/** Opaque list handle. */
typedef struct list list_t;

/** Allocate a new empty list.
 * @param copy Function that copies an item for storage.
 * @param free Function that frees a stored item.
 * @return Pointer to the new list, or NULL on allocation failure.
 */
list_t *list_alloc(list_copy_f copy, list_free_f free);

/** Destroy a list and free all stored items.
 * @param l List to destroy (may be NULL).
 */
void list_free(list_t *l);

/** Push a copy of @p item to the back of the list.
 * @param l List handle (must not be NULL).
 * @param item Pointer to the source item.
 * @return 0 on success, -1 on allocation failure.
 */
int list_push_back(list_t *l, const void *item);

/** Push a copy of @p item to the front of the list.
 * @param l List handle (must not be NULL).
 * @param item Pointer to the source item.
 * @return 0 on success, -1 on allocation failure.
 */
int list_push_front(list_t *l, const void *item);

/** Pop the front element, returning the owned pointer.
 * @param l List handle (must not be NULL).
 * @param out Receives the stored pointer (ownership transferred to caller).
 * @return 0 on success, -1 if the list is empty.
 */
int list_pop_front(list_t *l, void **out);

/** Pop the back element, returning the owned pointer.
 * @param l List handle (must not be NULL).
 * @param out Receives the stored pointer (ownership transferred to caller).
 * @return 0 on success, -1 if the list is empty.
 */
int list_pop_back(list_t *l, void **out);

/** Return the number of elements stored in the list. */
size_t list_size(const list_t *l);

#endif // LIST_H
