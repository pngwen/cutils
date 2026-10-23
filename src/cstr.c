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
 * string.c – implementation of the dynamic string API based on the vector.
 */

#include <cstr.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* Helper to retrieve the hidden vector header from a public char* pointer */
static inline vector_t *string_header(char *s)
{
    return (vector_t *)s - 1;
}

/* Allocation / deallocation ------------------------------------------------ */
/* Allocate an empty dynamic string. The vector holds char elements and starts
 * with capacity 1.  The string is NUL‑terminated and has length 0.
 */
char *string_alloc(void)
{
    char *s = (char *)vector_alloc(sizeof(char));
    if (!s)
        return NULL;
    /* Ensure the first byte is the terminating NUL.
     * The vector header size field is already 0.
     */
    s[0] = '\0';
    return s;
}

/* Free a dynamic string allocated by any string API function. */
void string_free(char *s)
{
    if (s)
        vector_free(s);
}

/* Append / concatenate ------------------------------------------------------ */
/* Append a NUL‑terminated C string to an existing dynamic string.
 * Returns the possibly reallocated pointer, or NULL on failure.
 */
char *string_append(char *s, const char *suffix)
{
    if (!suffix)
        return s; /* nothing to append */

    size_t add_len = strlen(suffix);
    if (add_len == 0)
        return s; /* no change */

    if (!s)
        s = string_alloc();
    if (!s)
        return NULL;

    vector_t *hdr = string_header(s);
    size_t cur_len = hdr->size;               /* current logical length */
    size_t needed = cur_len + add_len + 1;     /* +1 for terminating NUL */

    if (hdr->capacity < needed) {
        s = (char *)vector_reserve(s, needed);
        if (!s)
            return NULL;
        hdr = string_header(s);
    }

    /* Copy the new characters, including the terminating NUL. */
    memcpy(s + cur_len, suffix, add_len + 1);
    hdr->size = cur_len + add_len; /* update logical length, NUL not counted */
    return s;
}

/* Create a new string that is the concatenation of two existing strings.
 * Neither input is modified. The result must be freed with string_free.
 */
char *string_concat(const char *a, const char *b)
{
    char *result = string_alloc();
    if (!result)
        return NULL;

    if (a) {
        result = string_append(result, a);
        if (!result)
            return NULL;
    }
    if (b) {
        result = string_append(result, b);
        if (!result)
            return NULL;
    }
    return result;
}

/* printf / scanf style I/O --------------------------------------------------- */
/* Append formatted data to a dynamic string, similar to asprintf.
 * The function returns the possibly reallocated string pointer.
 */
char *string_printf(char *s, const char *fmt, ...)
{
    if (!fmt)
        return s;

    va_list args;
    va_start(args, fmt);
    va_list args_copy;
    va_copy(args_copy, args);

    /* Determine needed length without writing. */
    int needed = vsnprintf(NULL, 0, fmt, args_copy);
    va_end(args_copy);
    if (needed < 0) {
        va_end(args);
        return NULL;
    }

    if (!s)
        s = string_alloc();
    if (!s) {
        va_end(args);
        return NULL;
    }

    vector_t *hdr = string_header(s);
    size_t cur_len = hdr->size;
    size_t total_needed = cur_len + (size_t)needed + 1; /* +1 for NUL */

    if (hdr->capacity < total_needed) {
        s = (char *)vector_reserve(s, total_needed);
        if (!s) {
            va_end(args);
            return NULL;
        }
        hdr = string_header(s);
    }

    /* Write formatted output directly into the buffer after existing data. */
    int written = vsnprintf(s + cur_len, (size_t)needed + 1, fmt, args);
    va_end(args);
    if (written < 0)
        return NULL;

    hdr->size = cur_len + (size_t)written;
    return s;
}

/* Scan formatted data from a dynamic string using vsscanf. */
int string_scan(const char *s, const char *fmt, ...)
{
    if (!s || !fmt)
        return 0;

    va_list args;
    va_start(args, fmt);
    int ret = vsscanf(s, fmt, args);
    va_end(args);
    return ret;
}

/* Query helpers ------------------------------------------------------------- */
/* Return the length of the string (excluding the terminating NUL). */
size_t string_len(const char *s)
{
    if (!s)
        return 0;
    return vector_size((void *)s);
}

/* Compare two strings, treating NULL as the empty string. */
int string_cmp(const char *a, const char *b)
{
    if (!a)
        a = "";
    if (!b)
        b = "";
    return strcmp(a, b);
}

/* Miscellaneous ------------------------------------------------------------- */
/* Duplicate a string using the vector allocator. */
char *string_dup(const char *s)
{
    if (!s)
        return string_alloc();
    char *copy = string_alloc();
    if (!copy)
        return NULL;
    copy = string_append(copy, s);
    return copy;
}
