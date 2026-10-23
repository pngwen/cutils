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
 * vector.h – generic dynamic array (vector) implementation.
 */

#ifndef VECTOR_H
#define VECTOR_H

/**
 * @file vector.h
 * @brief Simple generic dynamic array (vector) implementation.
 */

#include <stddef.h>

/**
 * @brief Hidden header stored before the first element of a vector.
 */
typedef struct {
    size_t size;          /**< Number of elements currently stored. */
    size_t capacity;      /**< Allocated capacity (in elements). */
    size_t element_size;  /**< Size of a single element in bytes. */
} vector_t;

/**
 * @brief Allocate a new empty vector.
 *
 * The returned pointer points to the first element (which does not exist yet).
 * The vector starts with a capacity of one element and size zero.
 *
 * @param element_size Size of each element in bytes.
 * @return Pointer to the first element of the allocated vector, or NULL on failure.
 */
void *vector_alloc(size_t element_size);

/**
 * @brief Ensure the vector has at least the given capacity.
 *
 * If the current capacity is less than @p capacity, the underlying storage
 * is reallocated using a doubling strategy until the required capacity is met or
 * exceeded.
 *
 * @param v Pointer to the first element of the vector (may be NULL).
 * @param capacity Desired minimum capacity (in elements).
 * @return Pointer to the (potentially reallocated) first element, or NULL on failure.
 */
void *vector_reserve(void *v, size_t capacity);

/**
 * @brief Append an element to the end of the vector.
 *
 * The vector may be reallocated; the returned pointer must be used for further
 * operations.
 *
 * @param v Pointer to the first element of the vector (may be NULL).
 * @param element Pointer to the element to copy into the vector. The data is
 *                copied, not stored by reference.
 * @return Pointer to the (potentially reallocated) first element, or NULL on failure.
 */
void *vector_append(void *v, void *element);

/**
 * @brief Get the number of elements stored in the vector.
 *
 * @param v Pointer to the first element of the vector.
 * @return Number of elements currently stored.
 */
size_t vector_size(void *v);

/**
 * @brief Get the current capacity of the vector (in elements).
 *
 * @param v Pointer to the first element of the vector.
 * @return Capacity of the vector.
 */
size_t vector_capacity(void *v);

/**
 * @brief Free all memory associated with the vector.
 *
 * @param v Pointer to the first element of the vector.
 */
void vector_free(void *v);

#endif // VECTOR_H
