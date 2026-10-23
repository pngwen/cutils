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
 * map.c – generic associative array backed by a self‑balancing (AVL) tree.
 *
 * The tree implementation is private to this translation unit. Users interact
 * only through the public map_* API declared in map.h.
 */

#include "../include/map.h"

#include <stdlib.h>

/* ------------------------------------------------------------------ */
/* Private tree node                                                  */
/* ------------------------------------------------------------------ */

/**
 * @brief One node of the private AVL tree.
 *
 * Each node owns a copy of the item inserted into the map.
 */
typedef struct map_node {
    void           *key;   /**< Owned copy of the stored item. */
    struct map_node *left;  /**< Left child. */
    struct map_node *right; /**< Right child. */
    int             height; /**< Height of the subtree rooted here. */
} map_node_t;

/* ------------------------------------------------------------------ */
/* Private helpers                                                    */
/* ------------------------------------------------------------------ */

/* Return the height of a node (NULL node has height 0). */
static int node_height(const map_node_t *n)
{
    return n ? n->height : 0;
}

/* Return the larger of two integers. */
static int max_int(int a, int b)
{
    return a > b ? a : b;
}

/* Balance factor of a node (left height minus right height). */
static int node_balance(const map_node_t *n)
{
    return n ? node_height(n->left) - node_height(n->right) : 0;
}

/* Allocate a node and store an owned copy of item. NULL on failure. */
static map_node_t *node_create(map_t *m, const void *item)
{
    map_node_t *n = (map_node_t *)malloc(sizeof(map_node_t));
    if (!n) return NULL;

    n->key = m->create(item);
    if (!n->key) {
        free(n);
        return NULL;
    }

    n->left = NULL;
    n->right = NULL;
    n->height = 1;
    return n;
}

/* Right‑rotate subtree rooted at y. Returns new root. */
static map_node_t *node_rotate_right(map_node_t *y)
{
    map_node_t *x = y->left;
    map_node_t *t2 = x->right;

    x->right = y;
    y->left = t2;

    y->height = 1 + max_int(node_height(y->left), node_height(y->right));
    x->height = 1 + max_int(node_height(x->left), node_height(x->right));
    return x;
}

/* Left‑rotate subtree rooted at x. Returns new root. */
static map_node_t *node_rotate_left(map_node_t *x)
{
    map_node_t *y = x->right;
    map_node_t *t2 = y->left;

    y->left = x;
    x->right = t2;

    x->height = 1 + max_int(node_height(x->left), node_height(x->right));
    y->height = 1 + max_int(node_height(y->left), node_height(y->right));
    return y;
}

/* Rebalance subtree rooted at node after insert/remove. Returns new root. */
static map_node_t *node_rebalance(map_node_t *node)
{
    int bal = node_balance(node);

    if (bal > 1) {
        if (node_balance(node->left) < 0) {
            node->left = node_rotate_left(node->left);
        }
        return node_rotate_right(node);
    }

    if (bal < -1) {
        if (node_balance(node->right) > 0) {
            node->right = node_rotate_right(node->right);
        }
        return node_rotate_left(node);
    }

    return node;
}

/* Insert item into subtree rooted at node.
 * *status: 0 inserted, 1 replaced, -1 allocation failure.
 * Returns new subtree root (NULL on failure). */
static map_node_t *node_insert(map_t *m, map_node_t *node, const void *item,
                               int *status)
{
    int cmp;

    if (!node) {
        map_node_t *n = node_create(m, item);
        if (!n) {
            *status = -1;
            return NULL;
        }
        *status = 0;
        return n;
    }

    cmp = m->cmp(item, node->key);

    if (cmp < 0) {
        map_node_t *res = node_insert(m, node->left, item, status);
        if (*status == -1) return NULL;
        node->left = res;
    } else if (cmp > 0) {
        map_node_t *res = node_insert(m, node->right, item, status);
        if (*status == -1) return NULL;
        node->right = res;
    } else {
        /* Duplicate key: replace the owned copy. */
        void *newkey = m->create(item);
        if (!newkey) {
            *status = -1;
            return NULL;
        }
        m->free(node->key);
        node->key = newkey;
        *status = 1;
        return node;
    }

    node->height = 1 + max_int(node_height(node->left), node_height(node->right));
    return node_rebalance(node);
}

/* Find the node with the minimum key in a subtree (leftmost node). */
static map_node_t *node_min(map_node_t *node)
{
    while (node && node->left) {
        node = node->left;
    }
    return node;
}

/* Remove the entry matching item from subtree rooted at node.
 * *found is set to 1 if removed, 0 otherwise. Returns new subtree root. */
static map_node_t *node_remove(map_t *m, map_node_t *node, const void *item,
                               int *found)
{
    int cmp;

    if (!node) {
        *found = 0;
        return NULL;
    }

    cmp = m->cmp(item, node->key);

    if (cmp < 0) {
        node->left = node_remove(m, node->left, item, found);
    } else if (cmp > 0) {
        node->right = node_remove(m, node->right, item, found);
    } else {
        /* Found the node to remove. */
        *found = 1;

        if (!node->left) {
            map_node_t *right = node->right;
            m->free(node->key);
            free(node);
            return right;
        }
        if (!node->right) {
            map_node_t *left = node->left;
            m->free(node->key);
            free(node);
            return left;
        }

        /* Two children: swap with inorder successor, then remove it.
         * The original key ends up on succ and is freed when succ is removed. */
        map_node_t *succ = node_min(node->right);
        void *tmp = node->key;
        node->key = succ->key;
        succ->key = tmp;
        node->right = node_remove(m, node->right, succ->key, found);
    }

    node->height = 1 + max_int(node_height(node->left), node_height(node->right));
    return node_rebalance(node);
}

/* Find the node matching key in a subtree. */
static map_node_t *node_find(map_node_t *node, const void *key, map_cmp_f cmp)
{
    while (node) {
        int c = cmp(key, node->key);
        if (c == 0) return node;
        node = (c < 0) ? node->left : node->right;
    }
    return NULL;
}

/* Recursively free every owned copy and every node in a subtree. */
static void node_destroy_all(map_t *m, map_node_t *node)
{
    if (!node) return;
    node_destroy_all(m, node->left);
    node_destroy_all(m, node->right);
    m->free(node->key);
    free(node);
}

struct map_iter {
    map_t         *map;
    map_node_t   **stack;
    int            top;
    int            capacity;
    map_node_t    *current;
};

/* Push every node along the left spine of root onto iter->stack. */
static void iter_push_left_spine(map_iter_t *iter, map_node_t *root)
{
    while (root) {
        if (iter->top == iter->capacity) {
            int new_cap = iter->capacity ? iter->capacity * 2 : 16;
            map_node_t **new_stack = (map_node_t **)realloc(iter->stack,
                (size_t)new_cap * sizeof(map_node_t *));
            if (!new_stack) return; /* Best effort; iteration may be short. */
            iter->stack = new_stack;
            iter->capacity = new_cap;
        }
        iter->stack[iter->top++] = root;
        root = root->left;
    }
}

/* ------------------------------------------------------------------ */
/* Public API                                                         */
/* ------------------------------------------------------------------ */

/* Create a new empty map. */
map_t *map_alloc(map_create_f create, map_cmp_f cmp, map_free_f free)
{
    map_t *m = (map_t *)malloc(sizeof(map_t));
    if (!m) return NULL;

    m->create = create;
    m->cmp    = cmp;
    m->free   = free;
    m->data   = NULL;
    return m;
}

/* Destroy a map and every owned copy it contains. */
void map_free(map_t *m)
{
    if (!m) return;
    node_destroy_all(m, (map_node_t *)m->data);
    free(m);
}

/* Insert an owned copy of item into the map. */
int map_insert(map_t *m, const void *item)
{
    if (!m || !item) return -1;
    int status = 0;
    m->data = node_insert(m, (map_node_t *)m->data, item, &status);
    if (status == -1) return -1;
    return 0;
}

/* Remove the entry matching item. */
int map_remove(map_t *m, const void *item)
{
    if (!m || !item) return -1;
    int found = 0;
    m->data = node_remove(m, (map_node_t *)m->data, item, &found);
    return found ? 0 : -1;
}

/* Borrow a pointer to the stored copy matching key. */
void *map_get(map_t *m, const void *key)
{
    if (!m || !key) return NULL;
    map_node_t *n = node_find((map_node_t *)m->data, key, m->cmp);
    return n ? n->key : NULL;
}

/* ------------------------------------------------------------------ */
/* Iterator                                                           */
/* ------------------------------------------------------------------ */

/* Create an in‑order iterator over m. */
map_iter_t *map_iter_create(map_t *m)
{
    if (!m) return NULL;

    map_iter_t *iter = (map_iter_t *)malloc(sizeof(map_iter_t));
    if (!iter) return NULL;

    iter->map = m;
    iter->stack = NULL;
    iter->top = 0;
    iter->capacity = 0;
    iter->current = NULL;

    iter->stack = (map_node_t **)malloc(16 * sizeof(map_node_t *));
    if (!iter->stack) {
        free(iter);
        return NULL;
    }
    iter->capacity = 16;

    iter_push_left_spine(iter, (map_node_t *)m->data);
    return iter;
}

/* Destroy an iterator. */
void map_iter_destroy(map_iter_t *iter)
{
    if (!iter) return;
    free(iter->stack);
    free(iter);
}

/* Advance the iterator and return the next stored item. */
int map_iter_next(map_iter_t *iter, void **out)
{
    if (!iter || iter->top == 0) return 0;

    map_node_t *n = iter->stack[--iter->top];
    if (out) *out = n->key;
    iter->current = n;

    iter_push_left_spine(iter, n->right);
    return 1;
}

/* Return the last item returned by map_iter_next without advancing. */
void *map_iter_get(map_iter_t *iter)
{
    if (!iter) return NULL;
    return iter->current ? iter->current->key : NULL;
}

/* ------------------------------------------------------------------ */
/* Convenience                                                        */
/* ------------------------------------------------------------------ */

/* Apply func to every item in the map, in ascending order. */
int map_foreach(map_t *m, int (*func)(void *item, void *ctx), void *ctx)
{
    if (!m || !func) return -1;

    map_iter_t *iter = map_iter_create(m);
    if (!iter) return -1;

    int result = 0;
    void *item;
    while (map_iter_next(iter, &item)) {
        result = func(item, ctx);
        if (result != 0) break;
    }

    map_iter_destroy(iter);
    return result;
}
