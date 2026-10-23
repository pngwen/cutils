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
 * deque.h – double‑ended queue (deque) implementation based on a circular buffer.
 */

#ifndef DEQUE_H
#define DEQUE_H

#include <stddef.h>

/**
 * @file deque.h
 * @brief Generic double‑ended queue storing opaque pointers.
 */

/** Opaque deque handle – actually points to the first stored element.
 * The hidden header lives immediately before this pointer.
 */
typedef void **deque_t;

/** Allocate an empty deque.
 * The returned pointer may be NULL on allocation failure.
 */
deque_t deque_alloc(void);

/** Push an element to the back of the deque.
 * May reallocate; the returned deque pointer must be used for further calls.
 * @param d Deque handle (may be NULL, treated as empty).
 * @param elem Pointer to the element to store.
 * @return Updated deque handle or NULL on failure.
 */
deque_t deque_push_back(deque_t d, void *elem);

/** Push an element to the front of the deque.
 * May reallocate; the returned deque pointer must be used for further calls.
 */
deque_t deque_push_front(deque_t d, void *elem);

/** Pop an element from the front of the deque.
 * @param d Deque handle.
 * @return Stored pointer, or NULL if the deque is empty.
 */
void *deque_pop_front(deque_t d);

/** Pop an element from the back of the deque.
 * @param d Deque handle.
 * @return Stored pointer, or NULL if the deque is empty.
 */
void *deque_pop_back(deque_t d);

/** Return a pointer to the element at the front without removing it.
 * @param d Deque handle.
 * @return Stored pointer, or NULL if empty.
 */
void *deque_front(deque_t d);

/** Return a pointer to the element at the back without removing it.
 * @param d Deque handle.
 * @return Stored pointer, or NULL if empty.
 */
void *deque_back(deque_t d);

/** Number of elements stored in the deque. */
size_t deque_size(deque_t d);

/** Current capacity of the deque (in elements). */
size_t deque_capacity(deque_t d);

/** Free all memory associated with the deque. */
void deque_free(deque_t d);

#endif // DEQUE_H
