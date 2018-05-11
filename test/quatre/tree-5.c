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
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <quatre/macros.h>
#include <quatre/tree.h>
#include <quatre-test/report.h>


#define TEST_RANDOM_COUNT   1000000

static void finalizer(void *param, qrt_tree_value_t value) {
    volatile int a;
    a = 42;
}

static void iterate_tree(qrt_tree_t *tree) {
    qrt_tree_iterator_t *iter;
    qrt_tree_node_t     *node;
#ifdef DUMP_TREE    
    int                  idx;
    int                  depth;
    int                  depth_max;
    int                  depths[100];
    int                  acc, n;

    for(idx = 0; idx < 100; ++idx) {
        depths[idx] = 0;
    }
    
    depth_max = 0;
    acc       = 0;
    n         = 0;
#endif
    
    iter = qrt_tree_iterator_new(tree);
    assert(iter != NULL);
    
    node = qrt_tree_iterator_node(iter);
    
    while(node != NULL) {
#ifdef DUMP_TREE
        depth = qrt_tree_node_depth(node);
        
        if(depth < 99) {
            depths[depth] += 1;
        }
        else {
            depths[99] += 1;
        }
        
        if(depth > depth_max) {
            depth_max = depth;
        }
        
        acc += depth;
        n   += 1;
#endif
        node  = qrt_tree_iterator_next(iter);
    }

#ifdef DUMP_TREE    
    printf("average: %f max: %u\n", (double)acc/(double)n, depth_max);
    
    if(depth_max >= 99) {
        depth_max = 98;
    }
    
    for(idx = 0; idx <= depth_max; ++idx) {
        printf("%2u,%8u\n", idx, depths[idx]);
    }
#endif
}

void test_tree_sequential_prof(void) {
    qrt_tree_t          *tree;
    qrt_tree_iterator_t *iter;
    qrt_tree_node_t     *node;
    qrt_tree_key_t       key;
    int                  idx;

    report_start();
    
    srand(123);
    
    /* create and delete empty trees repeatedly */
    for(idx = 0; idx < TEST_RANDOM_COUNT; ++idx) {
        tree = qrt_tree_new();
        
        assert(tree != NULL);
        
        qrt_tree_free(tree, finalizer, NULL);
    }
        
    tree = qrt_tree_new();
    
    assert(tree != NULL);
    
    /* insert many keys sequential */
    for(key = 0; key < TEST_RANDOM_COUNT; ++key) {
        node = qrt_tree_add_node(tree, key);
        
        assert(node != NULL);
    }
    
    /* perform lookups */
    for(idx = 0; idx < TEST_RANDOM_COUNT; ++idx) {
        /* key (probably) not in the map */
        key = (qrt_tree_key_t)rand();
        
        node = qrt_tree_lookup_node(tree, key);
        
        /* key in the map */
        key = idx;
        
        node = qrt_tree_lookup_node(tree, key);
        assert(node != NULL);
    }
    
    /* create and delete iterators repeatedly */
    for(idx = 0; idx < TEST_RANDOM_COUNT; ++idx) {
        iter = qrt_tree_iterator_new(tree);
        
        assert(node != NULL);
        
        qrt_tree_iterator_free(iter);
    }
    
    /* iterate on tree, dump statistics if DUMP_TREE is defined */
    iterate_tree(tree);
    
    /* remove half the keys */
    for(key = 0; key < TEST_RANDOM_COUNT/2; ++key) {
        (void)qrt_tree_remove_key(tree, key, finalizer, NULL);
    }
    
    /* destroy the tree, freeing the other half of the nodes */
    qrt_tree_free(tree, finalizer, NULL);
}


int main(int argc, char *argv[]) {
    test_tree_sequential_prof();
    
    return EXIT_SUCCESS;
}
