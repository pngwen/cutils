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
 * deque.c – implementation of a generic double‑ended queue using a circular buffer.
 */

#include <deque.h>
#include <stdlib.h>

/* Hidden header placed directly before the public pointer array. */
typedef struct {
    size_t size;          /* Number of stored elements. */
    size_t capacity;      /* Allocated capacity (in elements). */
    size_t head;          /* Index of the first element in the buffer. */
    size_t element_size;  /* Size of each stored element – always sizeof(void*). */
} deque_hdr_t;

/* Retrieve the hidden header from a public deque pointer. */
static inline deque_hdr_t *deque_header(deque_t d)
{
    return (deque_hdr_t *)d - 1;
}

/* Allocate a new empty deque with initial capacity of one element. */
deque_t deque_alloc(void)
{
    size_t cap = 1;
    size_t total = sizeof(deque_hdr_t) + cap * sizeof(void *);
    deque_hdr_t *hdr = (deque_hdr_t *)malloc(total);
    if (!hdr) return NULL;
    hdr->size = 0;
    hdr->capacity = cap;
    hdr->head = 0;
    hdr->element_size = sizeof(void *);
    return (deque_t)((char *)hdr + sizeof(deque_hdr_t));
}

/* Ensure the deque can hold at least "need" elements; may reallocate. */
static deque_t deque_reserve(deque_t d, size_t need)
{
    if (!d) return NULL;
    deque_hdr_t *hdr = deque_header(d);
    if (need <= hdr->capacity) return d;

    size_t new_cap = hdr->capacity;
    while (new_cap < need) new_cap *= 2;
    size_t total = sizeof(deque_hdr_t) + new_cap * sizeof(void *);
    deque_hdr_t *new_hdr = (deque_hdr_t *)malloc(total);
    if (!new_hdr) return NULL;

    /* Copy existing elements in order starting from hdr->head. */
    /* Copy existing elements in order starting from hdr->head */
    void **new_data = (void **)((char *)new_hdr + sizeof(deque_hdr_t));
    void **old_data = (void **)d;
    for (size_t i = 0; i < hdr->size; ++i) {
        size_t src_idx = (hdr->head + i) % hdr->capacity;
        new_data[i] = old_data[src_idx];
    }
    new_hdr->size = hdr->size;
    new_hdr->capacity = new_cap;
    new_hdr->head = 0;
    new_hdr->element_size = sizeof(void *);
    free(hdr);
    return (deque_t)((char *)new_hdr + sizeof(deque_hdr_t));
}

/* Push element to the back of the deque. */
deque_t deque_push_back(deque_t d, void *elem)
{
    if (!elem) return d; /* nothing to store */
    if (!d) d = deque_alloc();
    if (!d) return NULL;
    deque_hdr_t *hdr = deque_header(d);
    if (hdr->size + 1 > hdr->capacity) {
        d = deque_reserve(d, hdr->size + 1);
        if (!d) return NULL;
        hdr = deque_header(d);
    }
    size_t idx = (hdr->head + hdr->size) % hdr->capacity;
    ((void **)d)[idx] = elem;
    hdr->size += 1;
    return d;
}

/* Push element to the front of the deque. */
deque_t deque_push_front(deque_t d, void *elem)
{
    if (!elem) return d;
    if (!d) d = deque_alloc();
    if (!d) return NULL;
    deque_hdr_t *hdr = deque_header(d);
    if (hdr->size + 1 > hdr->capacity) {
        d = deque_reserve(d, hdr->size + 1);
        if (!d) return NULL;
        hdr = deque_header(d);
    }
    hdr->head = (hdr->head == 0) ? hdr->capacity - 1 : hdr->head - 1;
    ((void **)d)[hdr->head] = elem;
    hdr->size += 1;
    return d;
}

/* Pop element from the front; returns stored pointer or NULL if empty. */
void *deque_pop_front(deque_t d)
{
    if (!d) return NULL;
    deque_hdr_t *hdr = deque_header(d);
    if (hdr->size == 0) return NULL;
    void *elem = ((void **)d)[hdr->head];
    hdr->head = (hdr->head + 1) % hdr->capacity;
    hdr->size -= 1;
    if (hdr->size == 0) hdr->head = 0; /* reset head for simplicity */
    return elem;
}

/* Pop element from the back; returns stored pointer or NULL if empty. */
void *deque_pop_back(deque_t d)
{
    if (!d) return NULL;
    deque_hdr_t *hdr = deque_header(d);
    if (hdr->size == 0) return NULL;
    size_t idx = (hdr->head + hdr->size - 1) % hdr->capacity;
    void *elem = ((void **)d)[idx];
    hdr->size -= 1;
    if (hdr->size == 0) hdr->head = 0;
    return elem;
}

/* Peek at the front element without removing it. */
void *deque_front(deque_t d)
{
    if (!d) return NULL;
    deque_hdr_t *hdr = deque_header(d);
    return (hdr->size == 0) ? NULL : ((void **)d)[hdr->head];
}

/* Peek at the back element without removing it. */
void *deque_back(deque_t d)
{
    if (!d) return NULL;
    deque_hdr_t *hdr = deque_header(d);
    if (hdr->size == 0) return NULL;
    size_t idx = (hdr->head + hdr->size - 1) % hdr->capacity;
    return ((void **)d)[idx];
}

size_t deque_size(deque_t d)
{
    if (!d) return 0;
    return deque_header(d)->size;
}

size_t deque_capacity(deque_t d)
{
    if (!d) return 0;
    return deque_header(d)->capacity;
}

void deque_free(deque_t d)
{
    if (!d) return;
    deque_hdr_t *hdr = deque_header(d);
    free(hdr);
}
