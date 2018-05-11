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

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <quatre/tree.h>
#include <quatre-test/report.h>

#define TEST_RANDOM_COUNT   600

#define TEST_RANDOM_LOOPS   3


qrt_tree_value_t finalized_node_value;

uintptr_t finalized_node_param;

int finalized_node_count;

#define TEST_VECTOR_SIZE 5

typedef struct {
    qrt_tree_key_t   key;
    char            *value;
} key_val_t;

const key_val_t test_vector[TEST_VECTOR_SIZE] = {
    {55,    "first"},
    {72,    "second"},
    {104,   "third"},
    {11,    "fourth"},
    {30110, "fifth"}
};

const int order_vector[TEST_VECTOR_SIZE] = {3, 0, 1, 2, 4};

const key_val_t not_in_tree = {17, "not_in_tree"};

typedef enum {IN_TREE, DELETED} key_status_t;

qrt_tree_key_t keys[TEST_RANDOM_COUNT];


static void finalizer(void *param, qrt_tree_value_t value) {
    finalized_node_value = value;
    finalized_node_param = (uintptr_t)param;
    ++finalized_node_count;
}

static void reset_finalized(void) {
    finalized_node_value = NULL;
    finalized_node_param = 0;
    finalized_node_count = 0;
}

static int search_value(qrt_tree_value_t value) {
    int idx;
    
    for(idx = 0; idx < TEST_VECTOR_SIZE; ++idx) {
        if((qrt_tree_value_t)test_vector[idx].value == value) {
            return idx;
        }
    }
    
    return -1;
}


void test_tree_add_validate(void) {
    qrt_tree_t      *tree;
    qrt_tree_node_t *node;
    int              idx;
    
    report_start();
    
    tree = qrt_tree_new();
    assert(tree != NULL);
    
    for(idx = 0; idx < TEST_VECTOR_SIZE; ++idx) {
        node = qrt_tree_add_node(tree, test_vector[idx].key);
        
        assert(node != NULL);
        assert(qrt_tree_node_key(node)   == test_vector[idx].key);
        assert(qrt_tree_node_value(node) == NULL);
    }
    
    assert(qrt_tree_count(tree) == TEST_VECTOR_SIZE);
    assert(qrt_tree_validate(tree) == QRT_SUCCESS);
    
    qrt_tree_free(tree, NULL, NULL);
}

void test_tree_add_lookup(void) {
    qrt_tree_t          *tree;
    qrt_tree_value_t     value;
    int                  idx;
    int                  ret;
    
    report_start();
    
    tree = qrt_tree_new();
    assert(tree != NULL);
    
    /* add new values */
    for(idx = 0; idx < TEST_VECTOR_SIZE; ++idx) {
        ret = qrt_tree_add_value(tree, test_vector[idx].key, (qrt_tree_value_t)idx);
        
        assert(ret == QRT_SUCCESS);
    }
    
    /* set new values for existing nodes */
    for(idx = 0; idx < TEST_VECTOR_SIZE; ++idx) {
        ret = qrt_tree_add_value(tree, test_vector[idx].key, test_vector[idx].value);
        
        assert(ret == QRT_SUCCESS);
    }
    
    for(idx = 0; idx < TEST_VECTOR_SIZE; ++idx) {
        value = qrt_tree_lookup_value(tree, test_vector[idx].key);
        
        assert(value == test_vector[idx].value);
    }
    
    value = qrt_tree_lookup_value(tree, not_in_tree.key);
    assert(value == NULL);
    
    qrt_tree_free(tree, NULL, NULL);
}

void test_tree_add_lookup_node(void) {
    qrt_tree_t      *tree;
    qrt_tree_node_t *node;
    int              idx;
    
    report_start();
    
    tree = qrt_tree_new();
    assert(tree != NULL);
    
    /* add new nodes */
    for(idx = 0; idx < TEST_VECTOR_SIZE; ++idx) {
        node = qrt_tree_add_node(tree, test_vector[idx].key);
        
        assert(node != NULL);
        assert(qrt_tree_node_key(node)   == test_vector[idx].key);
        assert(qrt_tree_node_value(node) == NULL);
        
        qrt_tree_node_set_value(node, (qrt_tree_value_t)idx);
        assert(qrt_tree_node_value(node) == (qrt_tree_value_t)idx);
    }
    
    /* set new values for existing nodes */
    for(idx = 0; idx < TEST_VECTOR_SIZE; ++idx) {
        node = qrt_tree_add_node(tree, test_vector[idx].key);
        
        assert(node != NULL);
        assert(qrt_tree_node_key(node)   == test_vector[idx].key);
        assert(qrt_tree_node_value(node) == (qrt_tree_value_t)idx);
        
        qrt_tree_node_set_value(node, test_vector[idx].value);
        assert(qrt_tree_node_value(node) == test_vector[idx].value);
    }
    
    for(idx = 0; idx < TEST_VECTOR_SIZE; ++idx) {
        node = qrt_tree_lookup_node(tree, test_vector[idx].key);
        
        assert(node != NULL);
        assert(qrt_tree_node_key(node)   == test_vector[idx].key);
        assert(qrt_tree_node_value(node) == test_vector[idx].value);
    }
    
    node = qrt_tree_lookup_node(tree, not_in_tree.key);
    assert(node == NULL);
    
    qrt_tree_free(tree, NULL, NULL);
}

void test_tree_add_remove_key(void) {
    key_status_t     status_vector[TEST_VECTOR_SIZE];
    qrt_tree_t      *tree;
    qrt_tree_node_t *node;
    int              idx;
    int              count;
    int              ret_add;
    int              ret;
    
    report_start();
    
    tree = qrt_tree_new();
    assert(tree != NULL);
    
    assert( qrt_tree_is_empty(tree) );
    
    /* add items */
    for(idx = 0; idx < TEST_VECTOR_SIZE; ++idx) {
        ret_add = qrt_tree_add_value(tree, test_vector[idx].key, test_vector[idx].value);
        
        assert(ret_add == QRT_SUCCESS);
        
        status_vector[idx] = IN_TREE;
    }
    
    count = qrt_tree_count(tree);
    assert(count == TEST_VECTOR_SIZE);
    
    /* remove an item not in the list */
    reset_finalized();
    
    ret = qrt_tree_remove_key(tree, not_in_tree.key, finalizer, NULL);
    assert(! ret);
    
    assert(qrt_tree_count(tree) == count);
    assert(finalized_node_count == 0);
    
    /* remove a few items */
    reset_finalized();
    
    ret = qrt_tree_remove_key(tree, test_vector[2].key, finalizer, (void *)42);
    assert(ret);
    assert(finalized_node_value == test_vector[2].value);
    assert(finalized_node_param == 42);
    
    status_vector[2] = DELETED;
    
    ret = qrt_tree_remove_key(tree, test_vector[3].key, finalizer, (void *)56);
    assert(ret);
    assert(finalized_node_value == test_vector[3].value);
    assert(finalized_node_param == 56);
    
    status_vector[3] = DELETED;
    
    ret = qrt_tree_remove_key(tree, test_vector[0].key, finalizer, NULL);
    assert(ret);
    assert(finalized_node_value == test_vector[0].value);
    assert(finalized_node_param == (uintptr_t)NULL);
    
    status_vector[0] = DELETED;
    
    assert(qrt_tree_count(tree) == count - 3);
    assert(finalized_node_count == 3);
    assert(qrt_tree_validate(tree) == QRT_SUCCESS);
    
    assert( ! qrt_tree_is_empty(tree) );
    
    /* check which items are still in the tree */    
    for(idx = 0; idx < TEST_VECTOR_SIZE; ++idx) {
        node = qrt_tree_lookup_node(tree, test_vector[idx].key);
        
        if(status_vector[idx] == DELETED) {
            assert(node == NULL);
        }
        else {
            assert(node != NULL);
            assert(qrt_tree_node_key(node)   == test_vector[idx].key);
            assert(qrt_tree_node_value(node) == test_vector[idx].value);
        }
    }
    
    /* remove the rest */
    reset_finalized();
    
    ret = qrt_tree_remove_key(tree, test_vector[1].key, finalizer, NULL);
    assert(ret);
    ret = qrt_tree_remove_key(tree, test_vector[4].key, finalizer, NULL);
    assert(ret);
    
    assert( qrt_tree_is_empty(tree) );
    assert(finalized_node_count == 2);

    /* done */
    qrt_tree_free(tree, NULL, NULL);
}

void test_tree_add_remove_node(void) {
    key_status_t     status_vector[TEST_VECTOR_SIZE];
    qrt_tree_t      *tree;
    qrt_tree_node_t *node;
    int              idx;
    int              count;
    int              ret_add;

    report_start();
    
    tree = qrt_tree_new();
    assert(tree != NULL);
    
    assert( qrt_tree_is_empty(tree) );
    
    /* add items */
    for(idx = 0; idx < TEST_VECTOR_SIZE; ++idx) {
        ret_add = qrt_tree_add_value(tree, test_vector[idx].key, test_vector[idx].value);
        
        assert(ret_add == QRT_SUCCESS);
        
        status_vector[idx] = IN_TREE;
    }
    
    count = qrt_tree_count(tree);
    assert(count == TEST_VECTOR_SIZE);
    
    /* remove a few items */
    reset_finalized();
    
    node = qrt_tree_lookup_node(tree, test_vector[1].key);
    assert(node != NULL);
    
    qrt_tree_remove_node(tree, node, finalizer, (void *)42);
    assert(finalized_node_value == test_vector[1].value);
    assert(finalized_node_param == 42);
    
    status_vector[1] = DELETED;
    
    node = qrt_tree_lookup_node(tree, test_vector[4].key);
    assert(node != NULL);
    
    qrt_tree_remove_node(tree, node, finalizer, (void *)66);
    assert(finalized_node_value == test_vector[4].value);
    assert(finalized_node_param == 66);
    
    status_vector[4] = DELETED;
    
    assert(qrt_tree_count(tree) == count - 2);
    assert(finalized_node_count == 2);
    assert(qrt_tree_validate(tree) == QRT_SUCCESS);
    
    assert( ! qrt_tree_is_empty(tree) );
    
    /* check which items are still in the tree */    
    for(idx = 0; idx < TEST_VECTOR_SIZE; ++idx) {
        node = qrt_tree_lookup_node(tree, test_vector[idx].key);
        
        if(status_vector[idx] == DELETED) {
            assert(node == NULL);
        }
        else {
            assert(node != NULL);
            assert(qrt_tree_node_key(node)   == test_vector[idx].key);
            assert(qrt_tree_node_value(node) == test_vector[idx].value);
        }
    }
    
    /* remove the rest */
    reset_finalized();
    
    node = qrt_tree_lookup_node(tree, test_vector[0].key);
    assert(node != NULL);
    qrt_tree_remove_node(tree, node, finalizer, NULL);
    
    node = qrt_tree_lookup_node(tree, test_vector[2].key);
    assert(node != NULL);
    qrt_tree_remove_node(tree, node, finalizer, NULL);
    
    node = qrt_tree_lookup_node(tree, test_vector[3].key);
    assert(node != NULL);
    qrt_tree_remove_node(tree, node, finalizer, NULL);
    
    assert( qrt_tree_is_empty(tree) );
    assert(finalized_node_count == 3);

    /* done */
    qrt_tree_free(tree, NULL, NULL);
}

void test_tree_add_pop_min(void) {
    key_status_t         status_vector[TEST_VECTOR_SIZE];
    qrt_tree_t          *tree;
    qrt_tree_node_t     *node;
    qrt_tree_value_t     value;
    int                  ret;
    int                  idx, idy, idz;
    
    report_start();
    
    tree = qrt_tree_new();
    assert(tree != NULL);
    
    for(idx = 0; idx < TEST_VECTOR_SIZE; ++idx) {
        ret = qrt_tree_add_value(tree, test_vector[idx].key, test_vector[idx].value);
        
        assert(ret == QRT_SUCCESS);
        
        status_vector[idx] = IN_TREE;
    }
    
    for(idy = 0; idy < TEST_VECTOR_SIZE; ++idy) {
        idx = order_vector[idy];
        
        value = qrt_tree_pop_min(tree);
        assert(value == test_vector[idx].value);
        
        status_vector[idx] = DELETED;
        
        for(idz = 0; idz < TEST_VECTOR_SIZE; ++idz) {
            node = qrt_tree_lookup_node(tree, test_vector[idz].key);
            
            if(status_vector[idz] == IN_TREE) {
                assert(node != NULL);
                assert(qrt_tree_node_key(node)   == test_vector[idz].key);
                assert(qrt_tree_node_value(node) == test_vector[idz].value);
            }
            else {
                assert(node == NULL);
            }
        }
    }
    
    assert( qrt_tree_is_empty(tree) ); 
    
    qrt_tree_free(tree, NULL, NULL);
}

void test_tree_add_pop_max(void) {
    key_status_t         status_vector[TEST_VECTOR_SIZE];
    qrt_tree_t          *tree;
    qrt_tree_node_t     *node;
    qrt_tree_value_t     value;
    int                  ret;
    int                  idx, idy, idz;
    
    report_start();
    
    tree = qrt_tree_new();
    assert(tree != NULL);
    
    for(idx = 0; idx < TEST_VECTOR_SIZE; ++idx) {
        ret = qrt_tree_add_value(tree, test_vector[idx].key, test_vector[idx].value);
        
        assert(ret == QRT_SUCCESS);
        
        status_vector[idx] = IN_TREE;
    }
    
    for(idy = TEST_VECTOR_SIZE - 1; idy >= 0; --idy) {
        idx = order_vector[idy];
        
        value = qrt_tree_pop_max(tree);
        assert(value == test_vector[idx].value);
        
        status_vector[idx] = DELETED;
        
        for(idz = 0; idz < TEST_VECTOR_SIZE; ++idz) {
            node = qrt_tree_lookup_node(tree, test_vector[idz].key);
            
            if(status_vector[idz] == IN_TREE) {
                assert(node != NULL);
                assert(qrt_tree_node_key(node)   == test_vector[idz].key);
                assert(qrt_tree_node_value(node) == test_vector[idz].value);
            }
            else {
                assert(node == NULL);
            }
        }
    }
    
    assert( qrt_tree_is_empty(tree) ); 
    
    qrt_tree_free(tree, NULL, NULL);
}

void test_tree_add_pop_random(void) {
    key_status_t         status_vector[TEST_VECTOR_SIZE];
    int                  sequence1[TEST_VECTOR_SIZE];
    int                  sequence2[TEST_VECTOR_SIZE];
    int                 *sequence;
    qrt_tree_t          *tree;
    qrt_tree_value_t     value;
    int                  ret;
    int                  step;
    int                  round;
    int                  idx;
    
    report_start();
    
    tree = qrt_tree_new();
    assert(tree != NULL);
    
    for(round = 0; round < TEST_VECTOR_SIZE; ++round) {        
        for(idx = 0; idx < TEST_VECTOR_SIZE; ++idx) {
            ret = qrt_tree_add_value(tree, test_vector[idx].key, test_vector[idx].value);
            
            assert(ret == QRT_SUCCESS);
            
            status_vector[idx] = IN_TREE;
        }
        
        if(round == 0) {
            sequence = sequence1;
            srand(10);
        }
        else {
            sequence = sequence2;
            srand(13);
        }
        
        for(step = 0; step < TEST_VECTOR_SIZE; ++step) {
            value = qrt_tree_pop_random(tree);
            idx = search_value(value);
            
            assert(idx >= 0);
            assert(status_vector[idx] == IN_TREE);
            
            status_vector[idx] = DELETED;
            
            sequence[step] = idx;
        }
        
        assert( qrt_tree_is_empty(tree) ); 
    }
    
    for(idx = 0; idx < TEST_VECTOR_SIZE; ++idx) {
        if(sequence1[idx] != sequence2[idx]) {
            break;
        }
    }
    
    assert(idx < TEST_VECTOR_SIZE);
    
    qrt_tree_free(tree, NULL, NULL);
}

void test_tree_add_clear(void) {
    qrt_tree_t      *tree;
    int              count;
    int              idx;
    int              ret;
    
    report_start();
    
    tree = qrt_tree_new();
    assert(tree != NULL);
    
    for(idx = 0; idx < TEST_VECTOR_SIZE; ++idx) {
        ret = qrt_tree_add_value(tree, test_vector[idx].key, NULL);
        
        assert(ret == QRT_SUCCESS);
    }
    
    count = qrt_tree_count(tree);
    assert(count == TEST_VECTOR_SIZE);
    
    reset_finalized();    
    qrt_tree_clear(tree, finalizer, (void *)10);
        
    assert(finalized_node_count == count);
    assert(finalized_node_param == 10);
    assert(qrt_tree_is_empty(tree));
}

void test_tree_add_free(void) {
    qrt_tree_t      *tree;
    int              count;
    int              idx;
    int              ret;
    
    report_start();
    
    tree = qrt_tree_new();
    assert(tree != NULL);
    
    for(idx = 0; idx < TEST_VECTOR_SIZE; ++idx) {
        ret = qrt_tree_add_value(tree, test_vector[idx].key, NULL);
        
        assert(ret == QRT_SUCCESS);
    }
    
    count = qrt_tree_count(tree);
    assert(count == TEST_VECTOR_SIZE);
    
    reset_finalized();    
    qrt_tree_free(tree, finalizer, (void *)1001);
        
    assert(finalized_node_count == count);
    assert(finalized_node_param == 1001);
}

void test_tree_add_finalize_static(void) {
    qrt_tree_t       tree;
    int              count;
    int              idx;
    int              ret;
    bool             status;
    
    report_start();
    
    status = qrt_tree_init(&tree);
    assert(status);
    
    for(idx = 0; idx < TEST_VECTOR_SIZE; ++idx) {
        ret = qrt_tree_add_value(&tree, test_vector[idx].key, NULL);
        
        assert(ret == QRT_SUCCESS);
    }
    
    count = qrt_tree_count(&tree);
    assert(count == TEST_VECTOR_SIZE);
    
    reset_finalized();
    qrt_tree_finalize(&tree, finalizer, (void *)202);
        
    assert(finalized_node_count == count);
    assert(finalized_node_param == 202);
}

void test_tree_random(void) {
    qrt_tree_t      *tree;
    qrt_tree_node_t *node;
    qrt_tree_key_t   key;
    bool             ret;
    int              idx, idy;
    int              cx;
    bool             duplicate;
    unsigned int     height;
    double           height_limit;
    
    report_start();
    
    /* a few loops with different seeds to try to catch bugs which depend on
     * the insertion order */
    for(cx = 0; cx < TEST_RANDOM_LOOPS; ++cx) {
        /* repeatable seeds to that failures can be reproduced */
        srand(42 + cx);
        
        tree = qrt_tree_new();
        assert(tree != NULL);
        
        assert(qrt_tree_count(tree) == 0);
        
        /* insert many random keys */
        for(idx = 0; idx < TEST_RANDOM_COUNT; ++idx) {
            key = (qrt_tree_key_t)rand();
            
            /* check for duplicate key */
            duplicate = false;
            for(idy = 0; idy < idx; ++idy) {
                if(key == keys[idy]) {
                    duplicate = true;
                    break;
                }
            }
            
            if(duplicate) {
                /* duplicate key, try again */
                --idx;
                continue;
            }
            
            node = qrt_tree_add_node(tree, key);
            keys[idx] = key;
            
            assert(node != NULL);
            assert(qrt_tree_node_key(node) == key);
        }
        
        assert(qrt_tree_count(tree) == TEST_RANDOM_COUNT);
        assert(qrt_tree_validate(tree) == QRT_SUCCESS);
        
        /* check balancing by comparing height to AVL tree height limit        
         * height < 3/2 * log2(N + 1) where log2(x) = log(x)/log(2) */
        height_limit = 1.5 * log((double)TEST_RANDOM_COUNT + 1.0) / log(2);
        height       = qrt_tree_height(tree);
        
        assert(height > 0);
        assert((double)height <= height_limit);
        
        /* Remove the keys one by one in the order they were inserted, after
         * each removal, check the content to ensure all keys that should
         * still be there are, and those that should no longer be there aren't. */
        for(idx = 0; idx < TEST_RANDOM_COUNT; ++idx) {
            key = keys[idx];
            
            node = qrt_tree_lookup_node(tree, key);
            
            assert(node != NULL);
            assert(qrt_tree_node_key(node) == key);
            
            ret = qrt_tree_remove_key(tree, key, NULL, NULL);
            assert(ret);
            
            for(idy = 0; idy < TEST_RANDOM_COUNT; ++idy) {
                key  = keys[idy];
                node = qrt_tree_lookup_node(tree, key);
               
                if(idy > idx) {
                    assert(node != NULL);
                    assert(qrt_tree_node_key(node) == key);
                }
                else {
                    assert(node == NULL);
                }
            }
            
            assert(qrt_tree_validate(tree) == QRT_SUCCESS);
        }
            
        assert(qrt_tree_count(tree) == 0);
        
        qrt_tree_free(tree, NULL, NULL);
    }
}

void test_tree_iterator_iterate(void) {
    qrt_tree_t          *tree;
    qrt_tree_iterator_t *iter;
    qrt_tree_node_t     *node;
    qrt_tree_key_t       key;
    qrt_tree_value_t     value;
    int                  ret;
    int                  idy, idx;
    
    report_start();
    
    tree = qrt_tree_new();
    assert(tree != NULL);
    
    /* add values */
    for(idx = 0; idx < TEST_VECTOR_SIZE; ++idx) {
        ret = qrt_tree_add_value(tree, test_vector[idx].key, (qrt_tree_value_t)idx);
        
        assert(ret == QRT_SUCCESS);
    }
    
    /* set new values for existing nodes */
    for(idx = 0; idx < TEST_VECTOR_SIZE; ++idx) {
        ret = qrt_tree_add_value(tree, test_vector[idx].key, test_vector[idx].value);
        
        assert(ret == QRT_SUCCESS);
    }
    
    iter = qrt_tree_iterator_new(tree);
    assert(tree != NULL);
    
    node = qrt_tree_iterator_node(iter);
    
    for(idy = 0; idy < TEST_VECTOR_SIZE; ++idy) {
        idx = order_vector[idy];
        
        key   = test_vector[idx].key;
        value = test_vector[idx].value;
        
        assert(qrt_tree_node_key(node)       == key  );
        assert(qrt_tree_node_value(node)     == value);
        assert(qrt_tree_iterator_key(iter)   == key  );
        assert(qrt_tree_iterator_value(iter) == value);
        
        node = qrt_tree_iterator_next(iter);
    }
    
    assert(node == NULL);
    /* key is undefined */
    assert(qrt_tree_iterator_value(iter) == NULL);
    
    node = qrt_tree_iterator_next(iter);
    
    assert(node == NULL);
    /* key is undefined */
    assert(qrt_tree_iterator_value(iter) == NULL);
    
    qrt_tree_free(tree, NULL, NULL);
}

void test_tree_iterator_iterate_backwards(void) {
    qrt_tree_t          *tree;
    qrt_tree_iterator_t *iter;
    qrt_tree_node_t     *node;
    qrt_tree_key_t       key;
    qrt_tree_value_t     value;
    int                  ret;
    int                  idy, idx;
    
    report_start();
    
    tree = qrt_tree_new();
    assert(tree != NULL);
    
    for(idx = 0; idx < TEST_VECTOR_SIZE; ++idx) {
        ret = qrt_tree_add_value(tree, test_vector[idx].key, test_vector[idx].value);
        
        assert(ret == QRT_SUCCESS);
    }
    
    iter = qrt_tree_iterator_new_from_end(tree);
    assert(tree != NULL);
    
    node = qrt_tree_iterator_node(iter);
    
    for(idy = TEST_VECTOR_SIZE - 1; idy >= 0; --idy) {
        idx = order_vector[idy];
        
        key   = test_vector[idx].key;
        value = test_vector[idx].value;
        
        assert(qrt_tree_node_key(node)       == key  );
        assert(qrt_tree_node_value(node)     == value);
        assert(qrt_tree_iterator_key(iter)   == key  );
        assert(qrt_tree_iterator_value(iter) == value);
        
        node = qrt_tree_iterator_prev(iter);
    }
    
    assert(node == NULL);
    /* key is undefined */
    assert(qrt_tree_iterator_value(iter) == NULL);
    
    node = qrt_tree_iterator_prev(iter);
    
    assert(node == NULL);
    /* key is undefined */
    assert(qrt_tree_iterator_value(iter) == NULL);
    
    qrt_tree_free(tree, NULL, NULL);
}

void test_tree_iterator_add_duplicate(void) {
    qrt_tree_t          *tree;
    qrt_tree_iterator_t *iter;
    qrt_tree_node_t     *node;
    qrt_tree_key_t       key;
    qrt_tree_value_t     value;
    int                  ret;
    int                  idy, idx;
    
    report_start();
    
    tree = qrt_tree_new();
    assert(tree != NULL);
    
    /* add values */
    for(idx = 0; idx < TEST_VECTOR_SIZE; ++idx) {
        ret = qrt_tree_add_value(tree, test_vector[idx].key, (qrt_tree_value_t)idx);
        
        assert(ret == QRT_SUCCESS);
    }
    
    /* add duplicate nodes with different values */
    for(idx = 0; idx < TEST_VECTOR_SIZE; ++idx) {
        ret = qrt_tree_add_value_duplicate(tree, test_vector[idx].key, test_vector[idx].value);
        
        assert(ret == QRT_SUCCESS);
    }
    
    iter = qrt_tree_iterator_new(tree);
    assert(tree != NULL);
    
    node = qrt_tree_iterator_node(iter);
    
    for(idy = 0; idy < TEST_VECTOR_SIZE; ++idy) {
        idx = order_vector[idy];
        
        key   = test_vector[idx].key;
        value = (qrt_tree_value_t)idx;
        
        assert(qrt_tree_node_key(node)       == key  );
        assert(qrt_tree_node_value(node)     == value);
        assert(qrt_tree_iterator_key(iter)   == key  );
        assert(qrt_tree_iterator_value(iter) == value);
        
        node = qrt_tree_iterator_next(iter);
        
        value = test_vector[idx].value;
        
        assert(qrt_tree_node_key(node)       == key  );
        assert(qrt_tree_node_value(node)     == value);
        assert(qrt_tree_iterator_key(iter)   == key  );
        assert(qrt_tree_iterator_value(iter) == value);
        
        node = qrt_tree_iterator_next(iter);
    }
    
    assert(node == NULL);

    qrt_tree_free(tree, NULL, NULL);
}

void test_tree_iterator_remove(void) {
    key_status_t         status_vector[TEST_VECTOR_SIZE];
    qrt_tree_t          *tree;
    qrt_tree_iterator_t *iter;
    qrt_tree_node_t     *node;
    int                  ret;
    int                  idx;
    
    report_start();
    
    tree = qrt_tree_new();
    assert(tree != NULL);
    
    for(idx = 0; idx < TEST_VECTOR_SIZE; ++idx) {
        ret = qrt_tree_add_value(tree, test_vector[idx].key, test_vector[idx].value);
        
        assert(ret == QRT_SUCCESS);
        
        status_vector[idx] = IN_TREE;
    }
    
    iter = qrt_tree_iterator_new(tree);
    assert(tree != NULL);
    
    /* first node */
    node = qrt_tree_iterator_node(iter);
    idx  = order_vector[0];
    
    assert(qrt_tree_node_key(node)   == test_vector[idx].key);
    assert(qrt_tree_node_value(node) == test_vector[idx].value);
    
    /* delete first node, iterator now on second node */
    reset_finalized();
    
    node = qrt_tree_iterator_remove(iter, finalizer, (void *)99);
    status_vector[idx] = DELETED;
    
    assert(finalized_node_count ==  1);
    assert(finalized_node_param == 99);
    assert(finalized_node_value == test_vector[idx].value);
    
    idx  = order_vector[1];
    assert(qrt_tree_node_key(node)   == test_vector[idx].key);
    assert(qrt_tree_node_value(node) == test_vector[idx].value);
    
    /* skip two nodes */
    node = qrt_tree_iterator_next(iter);
    idx  = order_vector[2];
    assert(qrt_tree_node_key(node)   == test_vector[idx].key);
    assert(qrt_tree_node_value(node) == test_vector[idx].value);
    
    node = qrt_tree_iterator_next(iter);
    idx  = order_vector[3];
    assert(qrt_tree_node_key(node)   == test_vector[idx].key);
    assert(qrt_tree_node_value(node) == test_vector[idx].value);
    
    /* delete second to last node, iterator now on last node */
    reset_finalized();
    
    node = qrt_tree_iterator_remove(iter, finalizer, NULL);
    status_vector[idx] = DELETED;
    
    assert(finalized_node_count ==  1);
    assert(finalized_node_param == (uintptr_t)NULL);
    assert(finalized_node_value == test_vector[idx].value);
    
    idx  = order_vector[4];
    assert(qrt_tree_node_key(node)   == test_vector[idx].key);
    assert(qrt_tree_node_value(node) == test_vector[idx].value);
    
    /* delete last node */
    reset_finalized();
    
    node = qrt_tree_iterator_remove(iter, finalizer, NULL);
    status_vector[idx] = DELETED;
    
    assert(finalized_node_count ==  1);
    assert(finalized_node_param == (uintptr_t)NULL);
    assert(finalized_node_value == test_vector[idx].value);
    
    assert(node == NULL);
    
    /* one more */
    reset_finalized();
    
    node = qrt_tree_iterator_remove(iter, finalizer, NULL);
    assert(node == NULL);
    assert(finalized_node_count == 0);
    
    /* validate tree to ensure nothing wrong happened */
    qrt_tree_validate(tree);
    
    /* back to start and ensure we find the two nodes we expect */
    node = qrt_tree_iterator_to_start(iter);    
    idx  = order_vector[1];
    assert(qrt_tree_node_key(node)   == test_vector[idx].key);
    assert(qrt_tree_node_value(node) == test_vector[idx].value);
    
    node = qrt_tree_iterator_next(iter);
    idx  = order_vector[2];
    assert(qrt_tree_node_key(node)   == test_vector[idx].key);
    assert(qrt_tree_node_value(node) == test_vector[idx].value);
    
    node = qrt_tree_iterator_next(iter);
    assert(node == NULL);
    
    node = qrt_tree_iterator_next(iter);
    assert(node == NULL);
    
    qrt_tree_free(tree, NULL, NULL);
}

void test_tree_iterator_random(void) {
    qrt_tree_t          *tree;
    qrt_tree_iterator_t *rand_iter;
    qrt_tree_node_t     *node;
    qrt_tree_key_t       key;
    qrt_tree_key_t       prev;
    int                  idx, idy;
    int                  cx;
    bool                 duplicate;
    
    report_start();
    
    srand(103);
        
    tree = qrt_tree_new();
    assert(tree != NULL);
    
    /* insert many random keys */
    for(idx = 0; idx < TEST_RANDOM_COUNT; ++idx) {
        key = (qrt_tree_key_t)rand();
        
        /* check for duplicate key */
        duplicate = false;
        for(idy = 0; idy < idx; ++idy) {
            if(key == keys[idy]) {
                duplicate = true;
                break;
            }
        }
        
        if(duplicate) {
            /* duplicate key, try again */
            --idx;
            continue;
        }
        
        node = qrt_tree_add_node(tree, key);
        keys[idx] = key;
        
        assert(node != NULL);
        assert(qrt_tree_node_key(node) == key);
    }
    
    assert(qrt_tree_count(tree) == TEST_RANDOM_COUNT);
    
    rand_iter = qrt_tree_iterator_new(tree);
    assert(rand_iter != NULL);
    
    cx   = 0;
    prev = 0;
    
    node = qrt_tree_iterator_node(rand_iter);
    
    while(node != NULL) {
        key = qrt_tree_node_key(node);
        
        assert(key > prev || idx == 0);
        
        for(idy = 0; idx < TEST_RANDOM_COUNT; ++idy) {
            if(key == keys[idy]) {
                break;
            }
        }
        
        assert(idy < TEST_RANDOM_COUNT);
        
        prev = key;
        node = qrt_tree_iterator_next(rand_iter);
        ++cx;
    }
    
    assert(cx == TEST_RANDOM_COUNT);
    
    qrt_tree_iterator_free(rand_iter);
    qrt_tree_free(tree, NULL, NULL);
}

int main(int argc, char *argv[]) {
    test_tree_add_validate();
    test_tree_add_lookup();
    test_tree_add_lookup_node();
    test_tree_add_remove_key();
    test_tree_add_remove_node();
    test_tree_add_pop_min();
    test_tree_add_pop_max();
    test_tree_add_pop_random();
    test_tree_add_clear();
    test_tree_add_free();
    test_tree_random();
    test_tree_add_finalize_static();
    
    test_tree_iterator_iterate();
    test_tree_iterator_iterate_backwards();
    test_tree_iterator_add_duplicate();
    test_tree_iterator_remove();
    test_tree_iterator_random();
    
    return EXIT_SUCCESS;
}
