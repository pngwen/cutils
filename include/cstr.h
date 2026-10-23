/*
 * MIT License
 *
 * Copyright (c) 2024 Bob Lowe <rlowe8@utm.edu>
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
 * string.h – dynamic string implementation based on the vector container.
 */

#ifndef STRING_H
#define STRING_H

/**
 * @file
 * @brief Dynamic string built on the generic vector container.
 *
 * The public type is `char *`. Internally the pointer references the first
 * character of a vector whose hidden header lives immediately before the data.
 * All API functions ensure a NUL‑terminator is present after the last character.
 */

#include <stddef.h>
#include "vector.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Allocation / deallocation ------------------------------------------------*/
/**
 * @brief Allocate an empty dynamic string.
 *
 * The returned pointer may be `NULL` on allocation failure. The string is
 * NUL‑terminated and has length zero.
 */
char *string_alloc(void);

/**
 * @brief Free a dynamic string previously allocated.
 *
 * @param s Pointer returned by any string‑creating function; may be `NULL`.
 */
void string_free(char *s);

/* Append / concatenate ------------------------------------------------------*/
/**
 * @brief Append a NUL‑terminated C string to a dynamic string.
 *
 * The function may reallocate the underlying storage; the returned pointer must
 * be used for subsequent operations.
 *
 * @param s      Existing string (may be `NULL`, which is treated as empty).
 * @param suffix NUL‑terminated C string to append; must not be `NULL`.
 * @return Updated string pointer, or `NULL` on allocation failure.
 */
char *string_append(char *s, const char *suffix);

/**
 * @brief Create a new string containing the concatenation of two strings.
 *
 * Neither input string is modified. The returned string must be freed with
 * `string_free`.
 *
 * @param a First operand; may be `NULL` (treated as empty).
 * @param b Second operand; may be `NULL`.
 * @return New dynamically allocated string, or `NULL` on failure.
 */
char *string_concat(const char *a, const char *b);

/* printf / scanf style I/O ---------------------------------------------------*/
/**
 * @brief Append formatted data to a dynamic string (printf‑style).
 *
 * The function formats the data according to `fmt` and appends the result to the
 * existing string, reallocating as necessary.
 *
 * @param s   Existing string (may be `NULL`).
 * @param fmt `printf`‑style format string.
 * @param ... Formatting arguments.
 * @return Updated string pointer, or `NULL` on allocation failure.
 */
char *string_printf(char *s, const char *fmt, ...);

/**
 * @brief Scan formatted data from a dynamic string (scanf‑style).
 *
 * This is a thin wrapper around `vsscanf`. The string is not modified.
 *
 * @param s   Dynamic string to read from (must be non‑NULL).
 * @param fmt `scanf`‑style format string.
 * @param ... Pointers that receive the extracted values.
 * @return Number of input items successfully matched.
 */
int string_scan(const char *s, const char *fmt, ...);

/* Query helpers -------------------------------------------------------------*/
/**
 * @brief Return the length of the dynamic string (excluding the terminating NUL).
 *
 * @param s String to query; `NULL` is treated as length 0.
 * @return Length in bytes.
 */
size_t string_len(const char *s);

/**
 * @brief Compare two strings, equivalent to `strcmp`.
 *
 * `NULL` pointers are treated as empty strings.
 *
 * @param a First string; may be `NULL`.
 * @param b Second string; may be `NULL`.
 * @return <0 if a<b, 0 if a==b, >0 if a>b.
 */
int string_cmp(const char *a, const char *b);

/* Miscellaneous -------------------------------------------------------------*/
/**
 * @brief Duplicate a dynamic string (similar to `strdup`).
 *
 * The returned string must be freed with `string_free`.
 *
 * @param s Source string; may be `NULL`.
 * @return New copy, or `NULL` on allocation failure.
 */
char *string_dup(const char *s);

#ifdef __cplusplus
}
#endif

#endif /* STRING_H */
