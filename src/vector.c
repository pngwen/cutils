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
 * vector.c – implementation of the generic vector functions.
 */

#include "../include/vector.h"
#include <stdlib.h>
#include <string.h>

/* Retrieve the hidden vector header from a public pointer */
static inline vector_t *vector_header(void *v) {
    return (vector_t *)v - 1;
}

/* Allocate a new empty vector with initial capacity of one element */
void *vector_alloc(size_t element_size) {
    if (element_size == 0) return NULL;
    size_t total = sizeof(vector_t) + element_size * 1;
    vector_t *hdr = (vector_t *)malloc(total);
    if (!hdr) return NULL;
    hdr->size = 0;
    hdr->capacity = 1;
    hdr->element_size = element_size;
    return (void *)((char *)hdr + sizeof(vector_t));
}

/* Ensure the vector can hold at least "capacity" elements, reallocating if necessary */
void *vector_reserve(void *v, size_t capacity) {
    if (!v) return NULL;
    vector_t *hdr = vector_header(v);
    if (capacity <= hdr->capacity) return v;
    size_t new_cap = hdr->capacity;
    while (new_cap < capacity) {
        new_cap *= 2;
    }
    size_t total = sizeof(vector_t) + new_cap * hdr->element_size;
    vector_t *new_hdr = (vector_t *)malloc(total);
    if (!new_hdr) return NULL;
    new_hdr->size = hdr->size;
    new_hdr->capacity = new_cap;
    new_hdr->element_size = hdr->element_size;
    memcpy((char *)new_hdr + sizeof(vector_t), (char *)v, hdr->size * hdr->element_size);
    free(hdr);
    return (void *)((char *)new_hdr + sizeof(vector_t));
}

/* Append a copy of "element" to the vector, resizing if needed */
void *vector_append(void *v, void *element) {
    if (!element) return v;
    if (!v) return NULL;
    vector_t *hdr = vector_header(v);
    if (hdr->size + 1 > hdr->capacity) {
        v = vector_reserve(v, hdr->size + 1);
        if (!v) return NULL;
        hdr = vector_header(v);
    }
    memcpy((char *)v + hdr->size * hdr->element_size, element, hdr->element_size);
    hdr->size += 1;
    return v;
}

/* Return the number of elements currently stored in the vector */
size_t vector_size(void *v) {
    if (!v) return 0;
    return vector_header(v)->size;
}

/* Return the current capacity of the vector (in elements) */
size_t vector_capacity(void *v) {
    if (!v) return 0;
    return vector_header(v)->capacity;
}

/* Free the memory allocated for the vector */
void vector_free(void *v) {
    if (!v) return;
    vector_t *hdr = vector_header(v);
    free(hdr);
}
