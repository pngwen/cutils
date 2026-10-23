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
 * list.c – implementation of a generic doubly linked list that owns copies of
 * its elements.
 */

#include <list.h>
#include <stdlib.h>

/* Internal node structure. */
typedef struct list_node {
    void *data;
    struct list_node *prev;
    struct list_node *next;
} list_node_t;

/* List structure – hidden from users. */
struct list {
    list_node_t *head;
    list_node_t *tail;
    list_copy_f copy;
    list_free_f free;
    size_t size;
};

/** Allocate a new empty list. */
list_t *list_alloc(list_copy_f copy, list_free_f free)
{
    if (!copy) return NULL;
    list_t *l = (list_t *)malloc(sizeof(list_t));
    if (!l) return NULL;
    l->head = l->tail = NULL;
    l->copy = copy;
    l->free = free;
    l->size = 0;
    return l;
}

/** Destroy a list and free all stored items. */
void list_free(list_t *l)
{
    if (!l) return;
    list_node_t *cur = l->head;
    while (cur) {
        list_node_t *next = cur->next;
        if (l->free && cur->data) l->free(cur->data);
        free(cur);
        cur = next;
    }
    free(l);
}

static int list_insert_node(list_t *l, list_node_t *node, int front)
{
    if (!l || !node) return -1;
    node->prev = node->next = NULL;
    if (l->size == 0) {
        l->head = l->tail = node;
    } else if (front) {
        node->next = l->head;
        l->head->prev = node;
        l->head = node;
    } else {
        node->prev = l->tail;
        l->tail->next = node;
        l->tail = node;
    }
    l->size += 1;
    return 0;
}

/** Push a copy of @p item to the back of the list. */
int list_push_back(list_t *l, const void *item)
{
    if (!l) return -1;
    void *copy = l->copy(item);
    if (!copy) return -1;
    list_node_t *node = (list_node_t *)malloc(sizeof(list_node_t));
    if (!node) { if (l->free) l->free(copy); return -1; }
    node->data = copy;
    return list_insert_node(l, node, 0);
}

/** Push a copy of @p item to the front of the list. */
int list_push_front(list_t *l, const void *item)
{
    if (!l) return -1;
    void *copy = l->copy(item);
    if (!copy) return -1;
    list_node_t *node = (list_node_t *)malloc(sizeof(list_node_t));
    if (!node) { if (l->free) l->free(copy); return -1; }
    node->data = copy;
    return list_insert_node(l, node, 1);
}

static int list_pop_node(list_t *l, list_node_t *node, void **out)
{
    if (!l || !node) return -1;
    if (out) *out = node->data; else if (l->free) l->free(node->data);
    if (node->prev) node->prev->next = node->next; else l->head = node->next;
    if (node->next) node->next->prev = node->prev; else l->tail = node->prev;
    free(node);
    l->size -= 1;
    return 0;
}

/** Pop the front element, returning the owned pointer. */
int list_pop_front(list_t *l, void **out)
{
    if (!l || l->size == 0) return -1;
    return list_pop_node(l, l->head, out);
}

/** Pop the back element, returning the owned pointer. */
int list_pop_back(list_t *l, void **out)
{
    if (!l || l->size == 0) return -1;
    return list_pop_node(l, l->tail, out);
}

/** Return the number of elements stored in the list. */
size_t list_size(const list_t *l)
{
    return l ? l->size : 0;
}
