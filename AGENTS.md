# Agent Configuration – Style & Preferences

## Licensing & Header
- All source files start with an **MIT license header**.
- Header includes:
  - Copyright line with your name and email: `Bob Lowe <rlowe8@utm.edu>`
  - Full MIT license text.
  - A brief one‑line description of the file’s purpose placed **after** the license block.

## File Structure
- Use **blank‑line paragraph style**: separate logical sections with an empty line.
- Header guards for `.h` files (`#ifndef … #define … … #endif`).
- Include only the necessary standard headers.
- Header files (*.h) belong in the `include/` directory.
- Implementation files (*.c) belong in the `src/` directory.
- Test programs (single‑file executables) belong in a `test/` directory.
- Compiled programs (executables) belong in the `bin/` directory.
- Combined library files (both static `.a` and dynamic `.so`) belong in the `lib/` directory.
- All test programs are to be statically linked.

## Formatting
- **K&R brace style** (brace on the same line as the function/statement).
- Always include braces, even for single‑statement `if`, `while`, etc.
- One‑line comment **above each function** summarising what it does.
- Public API functions receive full **Doxygen** documentation ( `/** … */` ) detailing purpose, parameters, and return values.
- Private/internal helpers receive a simple `/* … */` comment.

## Naming & Types
- Functions use **snake_case**.
- Types use **PascalCase** (e.g., `vector_t`).
- All symbols are prefixed with the module name when appropriate (e.g., `vector_alloc`).

## Vector Implementation Details (example reference)
- The vector hides a header struct (`vector_t`) **just before** the first element, so the public pointer points directly to the data array.
- The header stores `size`, `capacity`, and `element_size` in **element units**.
- Functions:
  - `void *vector_alloc(size_t element_size);`
  - `void *vector_reserve(void *v, size_t capacity);`
  - `void *vector_append(void *v, void *element);`
  - `size_t vector_size(void *v);`
  - `size_t vector_capacity(void *v);`
  - `void vector_free(void *v);`
- Allocation starts with capacity **1** and uses a **doubling strategy**.

## Commenting Conventions
- **Doxygen** for public API (multi‑line `/** … */`).
- Simple `/* … */` one‑line comments for internal functions and code blocks.
- Avoid trailing whitespace.
- Keep comments concise but informative.

## General Preferences
- Keep code **readable** and **consistent** across files.
- Separate logical sections with blank lines.
- Prefer explicit `if` checks over implicit truthiness.
- Use `static inline` for small helper functions accessed only within the translation unit.
- Ensure all public symbols are documented.

---

*This file documents the coding style and preferences to be followed when developing with the OpenCode agent.*

## Repository‑specific Guidance

- **Hidden header**: Each vector returned by the API points directly to the first element; a `vector_t` header is stored immediately *before* this pointer. Accessing size, capacity, or element size must be done via the provided API functions, not by casting the pointer.
- **Allocation contract**: `vector_alloc` creates a vector with an initial capacity of **1** element. The caller must treat the returned pointer as possibly `NULL` on allocation failure.
- **Reallocation semantics**: Functions that may reallocate (`vector_reserve`, `vector_append`) return a **new pointer**. The caller must assign the return value to the vector variable; otherwise subsequent operations will use a stale pointer.
- **Appending elements**: `vector_append` copies the data pointed to by `element` into the vector. The caller must ensure `element` points to a valid object of the correct size.
- **Memory management**: Always pair each successful `vector_alloc` (or any reallocation that returns non‑NULL) with a matching `vector_free` when the vector is no longer needed.
- **Typical compile command**:
  ```sh
  gcc -Wall -Wextra -Wpedantic -std=c11 -c vector.c -o vector.o
  ar rcs libvector.a vector.o
  # Use the static library when linking your program:
  gcc -Wall -Wextra -Wpedantic -std=c11 main.c -L. -lvector -o myprog
  ```
  Replace `main.c` with your source file that includes `vector.h`.
- **Testing tip**: After any operation that may reallocate, verify the returned pointer is not `NULL` before using it.
- **Naming**: Keep the `vector_` prefix for all public functions to avoid clashes with other libraries.

These points address behaviours that are easy to overlook without explicit guidance.

## Git Policies

- **Do not commit** any changes until I explicitly instruct you to do so.
- For every commit you *do* make, add yourself as a co‑author using the format:
  `Co-authored-by: [insert model name here]` (appropriate information for model and service replaces [insert model name here])
- Keep each commit **focused**: only group directly related files (e.g., a `.h` and its matching `.c`). If multiple unrelated pairs are changed, create separate commits, each with a concise message describing the change.
- I will handle all branching, merging, pulling, and pushing operations. You must never run those git commands.

## Retrieval‑augmented generation (RAG)

**If a top‑level `knowledge/` directory exists**, treat it as a curated store of reference material (Markdown, plain‑text, JSON, etc.). When a user asks a question that could be answered by that material, the agent should:

1. **Search** the `knowledge/` tree first (e.g., `grep` with `include: "knowledge/**"`).
2. **Quote** the exact snippet(s) that answer the question.
3. **Cite** each snippet using the form `knowledge/<path>:<start‑line>‑<end‑line>`.

*Optional:* Large generated files can be ignored by listing glob patterns in `knowledge/.opencode‑ignore`.
*Optional helper:* A small script `./tools/search‑knowledge.sh "<pattern>"` can be invoked via the shell tool to perform the same focused search.


