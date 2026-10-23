# Pngwen's C Utils

Small, dependency-free data structures for C11: a generic dynamic array, a
dynamic string built on top of it, and an ordered associative array backed by
a self-balancing tree. Every module is a single `.c`/`.h` pair, small enough to
read in one sitting and easy to drop into an existing project.

## Motivation

The C standard library gives us arrays and NUL-terminated strings, but very
little else. Third-party container libraries are often large, opinionated, or
tied to a particular build system. Pngwen's C Utils takes the opposite
approach:

- **Small and readable.** Each module is a few hundred lines of straightforward
  C11 with no clever macros and no generated code.
- **No dependencies beyond libc.** No build system is required to use the code;
  the bundled `Makefile` is a convenience, not a requirement.
- **Explicit ownership.** Every allocation, copy, and borrowed pointer is
  documented. Nothing is hidden behind a garbage collector or an implicit cast.
- **Consistent API.** Functions are prefixed by module, use `snake_case`, and
  document their failure modes.
- **Header-first documentation.** Public declarations carry Doxygen comments, so
  the headers double as the reference manual.

The result is a small toolkit that is pleasant to learn from, easy to audit, and
practical for embedded, educational, and systems-adjacent work.

## What's included

| Module   | Header       | Handle      | Summary                                                                 |
|----------|--------------|-------------|-------------------------------------------------------------------------|
| `vector` | `vector.h`   | `void *`    | Generic dynamic array with doubling growth and an initial capacity of 1. |
| `cstr`   | `cstr.h`     | `char *`    | Growable NUL-terminated string: append, concat, `printf`, `scanf`, and helpers. |
| `map`    | `map.h`      | `map_t *`   | Ordered associative array over owned copies, implemented as an AVL tree. |
| `deque`  | `deque.h`    | `deque_t`   | Generic double-ended queue using a circular buffer with stable element pointers. |
| `list`   | `list.h`     | `list_t *`  | Generic doubly linked list with owned copies, exposing list and node structs. |
| `set`    | `set.h`      | `set_t *`   | Generic set backed by `map`, with union and intersection operations. |

All modules are built into a single library, `libcutils` (static `.a` and shared
`.so`).

## Requirements

- A C11 compiler (tested with GCC and Clang)
- GNU Make, for the bundled build
- A Unix-like environment

The library itself only needs a hosted C11 implementation and libc.

## Project layout

```
include/   public headers (vector.h, cstr.h, map.h, deque.h, list.h, set.h)
src/       implementations (vector.c, cstr.c, map.c, deque.c, list.c, set.c)
test/      single-file test programs
bin/       test executables        (generated)
lib/       libcutils.a / .so       (generated)
obj/       object files            (generated)
```

## Building

```sh
make              # build the static and shared libraries plus the test programs
make clean        # remove obj/, lib/, and bin/
make reinstall    # clean, then rebuild everything
```

The build produces:

```
lib/libcutils.a
lib/libcutils.so
bin/cstr-test
bin/deque-test
bin/list-test
bin/map-test
bin/set-test
```

Run the tests with:

```sh
./bin/cstr-test
./bin/deque-test
./bin/list-test
./bin/map-test
./bin/set-test
```

Test programs are statically linked against `lib/libcutils.a`.

## Using the library

### Copy the sources

Add `include/` to your include path and compile the `src/*.c` files you need.
This is the simplest option and keeps the build transparent.

### Link the library

```sh
gcc -std=c11 -Iinclude myprog.c lib/libcutils.a -o myprog
```

Or, after installing `libcutils` somewhere on your link path:

```sh
gcc -std=c11 -Iinclude myprog.c -lcutils -o myprog
```

## Quick start

### Vector: a generic dynamic array

```c
#include <stdio.h>

#include "vector.h"

int main(void)
{
    int *v = vector_alloc(sizeof *v);
    if (v == NULL) {
        return 1;
    }

    for (int i = 0; i < 5; ++i) {
        int *grown = vector_append(v, &i);
        if (grown == NULL) {
            vector_free(v);
            return 1;
        }
        v = grown;
    }

    printf("size = %zu, capacity = %zu\n", vector_size(v), vector_capacity(v));
    for (size_t i = 0; i < vector_size(v); ++i) {
        printf("%d\n", v[i]);
    }

    vector_free(v);
    return 0;
}
```

The public pointer refers directly to the first element, so ordinary indexing
(`v[i]`) works. Capacity starts at one element and doubles when it is exhausted.
**`vector_append` and `vector_reserve` may reallocate:** always assign the
returned pointer back to the vector before using it again.

### cstr: a growable string

```c
#include <stdio.h>

#include "cstr.h"

int main(void)
{
    char *s = string_alloc();
    if (s == NULL) {
        return 1;
    }

    s = string_append(s, "Hello");
    if (s == NULL) {
        return 1;
    }

    s = string_printf(s, ", %s! The answer is %d.", "world", 42);
    if (s == NULL) {
        return 1;
    }

    printf("%s (%zu bytes)\n", s, string_len(s));

    string_free(s);
    return 0;
}
```

The handle is a plain `char *`, so the result can be passed to any function that
expects a NUL-terminated string. Strings always carry a terminator. Like the
vector, functions that grow the string return a possibly reallocated pointer.
Free the result with `string_free`, never with `free`.

### map: an ordered associative array

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "map.h"

typedef struct {
    char *name;
    int   score;
} player_t;

static void *player_create(const void *src)
{
    const player_t *s = src;
    player_t *copy = malloc(sizeof *copy);
    if (copy == NULL) {
        return NULL;
    }
    size_t n = strlen(s->name) + 1;
    copy->name = malloc(n);
    if (copy->name == NULL) {
        free(copy);
        return NULL;
    }
    memcpy(copy->name, s->name, n);
    copy->score = s->score;
    return copy;
}

static int player_cmp(const void *a, const void *b)
{
    return strcmp(((const player_t *)a)->name, ((const player_t *)b)->name);
}

static void player_free(void *obj)
{
    player_t *p = obj;
    if (p != NULL) {
        free(p->name);
        free(p);
    }
}

int main(void)
{
    map_t *m = map_alloc(player_create, player_cmp, player_free);
    if (m == NULL) {
        return 1;
    }

    const player_t players[] = {
        { "ada",   100 },
        { "grace",  95 },
        { "linus",  88 },
    };

    for (size_t i = 0; i < sizeof players / sizeof players[0]; ++i) {
        if (map_insert(m, &players[i]) != 0) {
            map_free(m);
            return 1;
        }
    }

    player_t key = { "grace", 0 };
    player_t *found = map_get(m, &key);
    if (found != NULL) {
        printf("grace -> %d\n", found->score);
    }

    map_iter_t *it = map_iter_create(m);
    if (it != NULL) {
        void *item = NULL;
        while (map_iter_next(it, &item)) {
            player_t *p = item;
            printf("%s: %d\n", p->name, p->score);
        }
        map_iter_destroy(it);
    }

    map_free(m);
    return 0;
}
```

The map stores **owned copies** created by the caller-supplied `create`
function and released by the caller-supplied `free` function. Ordering and key
equality are defined entirely by the `cmp` function. Inserting a key that
already exists replaces the stored value. Iteration visits entries in ascending
comparison order, and pointers returned by `map_get` / `map_iter_next` are
borrowed: they remain valid only until the entry is removed or the map is
destroyed.

## API at a glance

### `vector.h`

```c
void   *vector_alloc(size_t element_size);
void   *vector_reserve(void *v, size_t capacity);
void   *vector_append(void *v, void *element);
size_t  vector_size(void *v);
size_t  vector_capacity(void *v);
void    vector_free(void *v);
```

### `cstr.h`

```c
char   *string_alloc(void);
void    string_free(char *s);
char   *string_append(char *s, const char *suffix);
char   *string_concat(const char *a, const char *b);
char   *string_printf(char *s, const char *fmt, ...);
int     string_scan(const char *s, const char *fmt, ...);
size_t  string_len(const char *s);
int     string_cmp(const char *a, const char *b);
char   *string_dup(const char *s);
```

### `map.h`

```c
map_t      *map_alloc(map_create_f create, map_cmp_f cmp, map_free_f free);
void        map_free(map_t *m);
int         map_insert(map_t *m, const void *item);
int         map_remove(map_t *m, const void *item);
void       *map_get(map_t *m, const void *key);

map_iter_t *map_iter_create(map_t *m);
void        map_iter_destroy(map_iter_t *iter);
int         map_iter_next(map_iter_t *iter, void **out);
void       *map_iter_get(map_iter_t *iter);

int         map_foreach(map_t *m, int (*func)(void *item, void *ctx), void *ctx);
```

### `set.h`

```c
set_t      *set_alloc(map_create_f create, map_cmp_f cmp, map_free_f free);
void        set_free(set_t *s);
int         set_add(set_t *s, const void *item);
int         set_remove(set_t *s, const void *item);
int         set_contains(set_t *s, const void *item);
size_t      set_size(const set_t *s);
int         set_foreach(set_t *s, int (*func)(void *item, void *ctx), void *ctx);
set_t      *set_union(set_t *a, set_t *b);
set_t      *set_intersect(set_t *a, set_t *b);
```

The headers are the authoritative reference; each declaration documents its
parameters, return values, and ownership rules.

## Design notes

**Hidden headers.** A `vector` (and therefore a string) is a pointer to its
first element. The bookkeeping struct (`size`, `capacity`, `element_size`) is
stored immediately *before* that pointer. This keeps the public type as a
natural `void *` / `char *` while avoiding a separate struct allocation.

**Reallocation contract.** Operations that can grow storage return the new
pointer. The old pointer is invalid after a successful reallocation, so callers
must reassign it. Each function that can fail returns `NULL` (or `-1` for
`map_insert` / `map_remove`) so failures are checked explicitly.

**Ownership.** Vectors copy the bytes of the element you append; they do not
take ownership of any memory the element points to. Strings own their character
buffer and must be released with `string_free`. Maps copy every item through
`create` and release it through `free`, so the caller's objects can be stack
values or temporary buffers.

**Thread safety.** None of the containers are synchronized. Use external
locking if a container is shared between threads.

**NULL handling.** Query helpers such as `vector_size`, `string_len`, and
`string_cmp` tolerate `NULL` and treat it as empty. Mutation functions follow
the contract documented in the headers.

## Testing

Each module has a corresponding single-file test program in `test/`:

```sh
make
./bin/cstr-test
./bin/deque-test
./bin/list-test
./bin/map-test
./bin/set-test
```

Tests are compiled with `-Wall -Wextra -Wpedantic -std=c11` and linked
statically against `lib/libcutils.a`.

## Contributing

The project follows the conventions documented in [`AGENTS.md`](AGENTS.md):
MIT headers on every source file, K&R braces, `snake_case` functions,
`PascalCase` types, module-prefixed symbols, and Doxygen comments on the public
API. New modules should live in `src/` with a matching header in `include/`, and
should ship with a test program in `test/`.

## Agents

This project was developed collaboratively with AI coding agents. The
following agents contributed to the codebase and its documentation:

| Agent                                | Contribution                                                     |
|--------------------------------------|------------------------------------------------------------------|
| gpt-oss:120b                         | Initial project scaffolding, `AGENTS.md`, and the vector module. |
| Nemotron 3.5 Super VL Free (opencode) | The map module (AVL tree) and updates to `AGENTS.md`.            |
| DeepSeek V4.1 Flash Free (opencode)   | README, LICENSE, and the packaging/documentation pass.           |

## License

Released under the [MIT License](LICENSE).

Copyright (c) 2026 Bob Lowe <rlowe8@utm.edu>
