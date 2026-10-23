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
 * cstr-test.c – simple test program exercising the dynamic‑string API.
 *
 * This file resides in the `test/` directory.  All test programs are
 * statically linked; the build system should compile it with `-static` and link
 * against the static library containing the vector and cstr implementations.
 */

#include <stdio.h>
#include <stdlib.h>

/* Include the public string header.  The test directory is a sibling of `include/`. */
#include "../include/cstr.h"

int main(void)
{
    /* 1. Allocate an empty string. */
    char *s = string_alloc();
    if (!s) {
        fprintf(stderr, "string_alloc failed\n");
        return EXIT_FAILURE;
    }
    printf("Allocated empty string: \"%s\" (len=%zu)\n", s, string_len(s));

    /* 2. Append literal strings. */
    s = string_append(s, "Hello");
    s = string_append(s, ", World");
    printf("After appends: \"%s\" (len=%zu)\n", s, string_len(s));

    /* 3. Append formatted data. */
    s = string_printf(s, " %d", 2026);
    printf("After printf: \"%s\" (len=%zu)\n", s, string_len(s));

    /* 4. Duplicate the string and compare. */
    char *dup = string_dup(s);
    printf("Duplicate: \"%s\" (len=%zu)\n", dup, string_len(dup));
    printf("Comparison (original vs dup): %d\n", string_cmp(s, dup));

    /* 5. Concatenate two strings into a new one. */
    char *c = string_concat("Prefix-", s);
    printf("Concatenated: \"%s\" (len=%zu)\n", c, string_len(c));

    /* 6. Scan formatted data from the string. */
    int year = 0;
    char word[32] = {0};
    int matched = string_scan(s, "%[A-Za-z], %d", word, &year);
    printf("Scanned %d items: word=\"%s\", year=%d\n", matched, word, year);

    /* 7. Clean up all allocated strings. */
    string_free(s);
    string_free(dup);
    string_free(c);

    return EXIT_SUCCESS;
}
