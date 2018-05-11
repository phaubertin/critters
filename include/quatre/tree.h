/*
 * Copyright (C) 2014 Philippe Aubertin.
 * All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of other contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef QUATRE_TREE_H_
#define QUATRE_TREE_H_

#include <quatre/types.h>


                    /* ----- tree node declarations ----- */
                    
#ifdef QRT_CONFIG_TREE_KEY_TYPE
typedef QRT_CONFIG_TREE_KEY_TYPE qrt_tree_key_t;
#else
typedef uintptr_t qrt_tree_key_t;
#endif

#ifdef QRT_CONFIG_TREE_VALUE_TYPE
typedef QRT_CONFIG_TREE_VALUE_TYPE qrt_tree_value_t;
#else
typedef void *qrt_tree_value_t;
#endif


typedef struct qrt_tree_node_t qrt_tree_node_t;

struct qrt_tree_node_t {
    qrt_tree_key_t       key;
    qrt_tree_node_t     *left;
    qrt_tree_node_t     *right;
    qrt_tree_node_t     *parent;
    qrt_tree_value_t     value;
    int                  balance;
};


#define qrt_tree_node_key_lvalue(n)     ((n)->key)

#define qrt_tree_node_value_lvalue(n)   ((n)->value)

#define qrt_tree_node_left_lvalue(n)    ((n)->left)

#define qrt_tree_node_right_lvalue(n)   ((n)->right)

#define qrt_tree_node_parent_lvalue(n)  ((n)->parent)

#define qrt_tree_node_balance_lvalue(n) ((n)->balance)


QRT_INLINE qrt_tree_node_t *qrt_tree_node_left(qrt_tree_node_t *node) {
    return qrt_tree_node_left_lvalue(node);
}

QRT_INLINE qrt_tree_node_t *qrt_tree_node_right(qrt_tree_node_t *node) {
    return qrt_tree_node_right_lvalue(node);
}

QRT_INLINE qrt_tree_node_t *qrt_tree_node_parent(qrt_tree_node_t *node) {
    return qrt_tree_node_parent_lvalue(node);
}

QRT_INLINE qrt_tree_key_t qrt_tree_node_key(qrt_tree_node_t *node) {
    return qrt_tree_node_key_lvalue(node);
}

QRT_INLINE qrt_tree_value_t qrt_tree_node_value(qrt_tree_node_t *node) {
    return qrt_tree_node_value_lvalue(node);
}

QRT_INLINE void qrt_tree_node_set_value(qrt_tree_node_t *node, qrt_tree_value_t value) {
    qrt_tree_node_value_lvalue(node) = value;
}

QRT_INLINE qrt_tree_value_t *qrt_tree_node_value_ptr(qrt_tree_node_t *node) {
    return &qrt_tree_node_value_lvalue(node);
}

QRT_INLINE int qrt_tree_node_balance(qrt_tree_node_t *node) {
    if(node == NULL) {
        return 0;
    }
    return qrt_tree_node_balance_lvalue(node);
}

qrt_tree_node_t *qrt_tree_node_next(qrt_tree_node_t *node);

qrt_tree_node_t *qrt_tree_node_prev(qrt_tree_node_t *node);

unsigned int qrt_tree_node_depth(qrt_tree_node_t *node);


                    /* ----- tree declarations ----- */

typedef struct qrt_tree_t qrt_tree_t;

typedef void (*qrt_tree_finalize_func_t)(void *, qrt_tree_value_t);

struct qrt_tree_t {
    qrt_tree_node_t *root;
};

#define qrt_tree_root_lvalue(t) ((t)->root)

QRT_INLINE qrt_tree_node_t *qrt_tree_root(qrt_tree_t *tree) {
    return qrt_tree_root_lvalue(tree);
}

bool qrt_tree_init(qrt_tree_t *tree);

void qrt_tree_finalize(qrt_tree_t *tree, qrt_tree_finalize_func_t finalizer, void *param);

qrt_tree_t *qrt_tree_new(void);

void qrt_tree_free(qrt_tree_t *tree, qrt_tree_finalize_func_t finalizer, void *param);


qrt_tree_node_t *qrt_tree_lookup_node(qrt_tree_t *tree, qrt_tree_key_t key);

qrt_tree_value_t qrt_tree_lookup_value(qrt_tree_t *tree, qrt_tree_key_t key);

qrt_tree_node_t *qrt_tree_add_node(qrt_tree_t *tree, qrt_tree_key_t key);

int qrt_tree_add_value(qrt_tree_t *tree, qrt_tree_key_t key, qrt_tree_value_t value);

int qrt_tree_add_value_duplicate(qrt_tree_t *tree, qrt_tree_key_t key, qrt_tree_value_t value);

bool qrt_tree_remove_key(qrt_tree_t *tree, qrt_tree_key_t key, qrt_tree_finalize_func_t finalizer, void *param);

void qrt_tree_remove_node(qrt_tree_t *tree, qrt_tree_node_t *node, qrt_tree_finalize_func_t finalizer, void *param);

qrt_tree_value_t qrt_tree_pop_min(qrt_tree_t *tree);

qrt_tree_value_t qrt_tree_pop_max(qrt_tree_t *tree);

qrt_tree_value_t qrt_tree_pop_random(qrt_tree_t *tree);

void qrt_tree_clear(qrt_tree_t *tree, qrt_tree_finalize_func_t finalizer, void *param);


bool qrt_tree_is_empty(qrt_tree_t *tree);

int qrt_tree_validate(qrt_tree_t *tree);

unsigned int qrt_tree_count(qrt_tree_t *tree);

unsigned int qrt_tree_height(qrt_tree_t *tree);


                    /* ----- subtree functions ----- */

void qrt_tree_sub_destroy(qrt_tree_node_t *node, qrt_tree_finalize_func_t finalizer, void *param);

qrt_tree_node_t *qrt_tree_sub_rotate(qrt_tree_node_t *node);

int qrt_tree_sub_validate(qrt_tree_node_t *node);

unsigned int qrt_tree_sub_count(qrt_tree_node_t *node);

unsigned int qrt_tree_sub_height(qrt_tree_node_t *node);

                   
                    /* ----- tree iterator declarations ----- */

typedef struct qrt_tree_iterator_t qrt_tree_iterator_t;


qrt_tree_iterator_t *qrt_tree_iterator_new(qrt_tree_t *tree);

qrt_tree_iterator_t *qrt_tree_iterator_new_from_end(qrt_tree_t *tree);

void qrt_tree_iterator_free(qrt_tree_iterator_t *iter);

qrt_tree_node_t *qrt_tree_iterator_node(qrt_tree_iterator_t *iter);

qrt_tree_key_t qrt_tree_iterator_key(qrt_tree_iterator_t *iter);

qrt_tree_value_t qrt_tree_iterator_value(qrt_tree_iterator_t *iter);

qrt_tree_node_t *qrt_tree_iterator_to_start(qrt_tree_iterator_t *iter);

qrt_tree_node_t *qrt_tree_iterator_to_end(qrt_tree_iterator_t *iter);

qrt_tree_node_t *qrt_tree_iterator_next(qrt_tree_iterator_t *iter);

qrt_tree_node_t *qrt_tree_iterator_prev(qrt_tree_iterator_t *prev);

qrt_tree_node_t *qrt_tree_iterator_remove(qrt_tree_iterator_t *iter, qrt_tree_finalize_func_t finalizer, void *param);


#endif
