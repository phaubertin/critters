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
#include <quatre/tree.h>
#include <quatre/macros.h>
#include <quatre-test/report.h>

#define LEFT  true

#define RIGHT false

/* This structure represents a test subtree which can be rotated.
 * It has a left form with A being the root and B being its left child
 * When this tree is rotated right, B becomes the root with A its right
 * child.
 * 
 * r is the right-most node (lowest key) and A's right child. l is the
 * left-most node (highest key) and B's left child. m is the middle
 * node, alternatively B's right child and A's left child. x and y are
 * m's left and right children, respectively.
 * 
 *                 A (6)                                B (2) 
 *                / \                                  / \       
 *               /   \          rotate right          /   \      
 *              /     \          --------->          /     \     
 *         (2) B       r (7)    <---------      (1) l       A (6)
 *            / \               rotate left                / \          
 *           /   \                                        /   \         
 *          /     \                                      /     \        
 *     (1) l      m (4)                             (4) m       r (7)
 *               / \                                   / \      
 *              /   \                                 /   \     
 *             /     \                               /     \    
 *        (3) x       y (5)                   (3)   x       y (5)
 * 
 * In addition, when testing the tree validation function, we need to make the
 * tree AVL-balanced. To that end, there is a "dummy" node which can be added
 * as a child of l of r.
 * 
 * */
typedef struct {
    qrt_tree_node_t *root;
    qrt_tree_node_t  A;
    qrt_tree_node_t  B;
    qrt_tree_node_t  l;
    qrt_tree_node_t  r;
    qrt_tree_node_t  m;
    qrt_tree_node_t  x;
    qrt_tree_node_t  y;
    qrt_tree_node_t  dummy;
} test_tree_t;

static void link_nodes(qrt_tree_node_t *parent, qrt_tree_node_t *child, bool left) {
    if(child != NULL) {
        qrt_tree_node_parent_lvalue(child) = parent;
    }
    
    if(parent != NULL) {
        if(left) {
            qrt_tree_node_left_lvalue(parent) = child;
        }
        else {
            qrt_tree_node_right_lvalue(parent) = child;
        }
    }
}

static bool check_link(qrt_tree_node_t *parent, qrt_tree_node_t *child, bool left) {
    if(child != NULL) {
        if(qrt_tree_node_parent(child) != parent) {
            return false;
        }
    }
    
    if(parent != NULL) {
        if(left) {
            if(qrt_tree_node_left(parent) != child) {
                return false;
            }
        }
        else {
            if(qrt_tree_node_right(parent) != child) {
                return false;
            }
        }
    }
    
    return true;
}

static void initialize_tree(test_tree_t *tree, int root_balance, int subroot_balance, int m_balance) {
    qrt_tree_node_t *A;
    qrt_tree_node_t *B;
    qrt_tree_node_t *m;
    qrt_tree_node_t *l;
    qrt_tree_node_t *r;
    qrt_tree_node_t *x;
    qrt_tree_node_t *y;

    A = &tree->A;
    B = &tree->B;
    l = &tree->l;
    r = &tree->r;
    m = &tree->m;
    x = &tree->x;
    y = &tree->y;
    
    /* node keys */
    qrt_tree_node_key_lvalue(l) = 1;
    qrt_tree_node_key_lvalue(B) = 2;
    qrt_tree_node_key_lvalue(x) = 3;
    qrt_tree_node_key_lvalue(m) = 4;
    qrt_tree_node_key_lvalue(y) = 5;
    qrt_tree_node_key_lvalue(A) = 6;
    qrt_tree_node_key_lvalue(r) = 7;
    
    /* node values */
    qrt_tree_node_set_value(A, (qrt_tree_value_t)0xA421);
    qrt_tree_node_set_value(B, (qrt_tree_value_t)0xA422);
    qrt_tree_node_set_value(l, (qrt_tree_value_t)0xA423);
    qrt_tree_node_set_value(r, (qrt_tree_value_t)0xA424);
    qrt_tree_node_set_value(m, (qrt_tree_value_t)0xA425);
    qrt_tree_node_set_value(x, (qrt_tree_value_t)0xA426);
    qrt_tree_node_set_value(y, (qrt_tree_value_t)0xA427);
    
    /* These are irrelevant and should (in the sense of must) have no
     * effect on any test. */
    qrt_tree_node_balance_lvalue(l) = 0xBEE1;
    qrt_tree_node_balance_lvalue(r) = 0xBEE2;
    qrt_tree_node_balance_lvalue(x) = 0xBEE3;
    qrt_tree_node_balance_lvalue(y) = 0xBEE4;
    
    /* link structure */
    link_nodes(B, l, LEFT);
    link_nodes(A, r, RIGHT);
    link_nodes(m, x, LEFT);
    link_nodes(m, y, RIGHT);
    
    link_nodes(r, NULL, LEFT);
    link_nodes(r, NULL, RIGHT);
    link_nodes(l, NULL, LEFT);
    link_nodes(l, NULL, RIGHT);
    link_nodes(x, NULL, LEFT);
    link_nodes(x, NULL, RIGHT);
    link_nodes(y, NULL, LEFT);
    link_nodes(y, NULL, RIGHT);
    
    assert(root_balance != 0);
    
    if(root_balance > 0) {
        tree->root = A;
        link_nodes(NULL, A, false);
        
        link_nodes(A,    B, LEFT);
        link_nodes(B,    m, RIGHT);
    }
    else {
        tree->root = B;
        link_nodes(NULL, B, false);
        
        link_nodes(B,    A, RIGHT);
        link_nodes(A,    m, LEFT);
    }
    
    /* set specified balance factors */
    qrt_tree_node_balance_lvalue(m) = m_balance;
    
    if(root_balance > 0) {
        A->balance = root_balance;
        B->balance = subroot_balance;
    }
    else {
        A->balance = subroot_balance;
        B->balance = root_balance;
    }
    
    /* unused dummy node */
    qrt_tree_node_set_value(&tree->dummy, 0);
    qrt_tree_node_key_lvalue(&tree->dummy)      = 0;
    qrt_tree_node_balance_lvalue(&tree->dummy)  = 0;
    qrt_tree_node_left_lvalue(&tree->dummy)     = NULL;
    qrt_tree_node_right_lvalue(&tree->dummy)    = NULL;
    qrt_tree_node_parent_lvalue(&tree->dummy)   = NULL;
}

static void initialize_tree_simple(test_tree_t *tree, int root_balance, int subroot_balance) {
    initialize_tree(tree, root_balance, subroot_balance, 0);
}

static void initialize_tree_valid(test_tree_t *tree, bool left) {
    if(left) {    
        initialize_tree_simple(tree,  1, -1);
        
        link_nodes(&tree->r, &tree->dummy, RIGHT);
        qrt_tree_node_key_lvalue(&tree->dummy) = 8;
        
        qrt_tree_node_balance_lvalue(&tree->l) =  0;
        qrt_tree_node_balance_lvalue(&tree->r) = -1;
    }
    else {
        initialize_tree_simple(tree, -1, 1);
        
        link_nodes(&tree->l, &tree->dummy, LEFT);
        qrt_tree_node_key_lvalue(&tree->dummy) = 0;
        
        qrt_tree_node_balance_lvalue(&tree->l) = 1;
        qrt_tree_node_balance_lvalue(&tree->r) = 0;
    }
    
    qrt_tree_node_balance_lvalue(&tree->x) = 0;
    qrt_tree_node_balance_lvalue(&tree->y) = 0;
}

#define assert_return(cond) \
    do { \
        if( ! (cond) ) { \
            return false; \
        } \
    } while (0)

static bool check_tree_invariant(test_tree_t *tree) {
    qrt_tree_node_t *A;
    qrt_tree_node_t *B;
    qrt_tree_node_t *l;
    qrt_tree_node_t *r;
    qrt_tree_node_t *m;
    qrt_tree_node_t *x;
    qrt_tree_node_t *y;

    A = &tree->A;
    B = &tree->B;
    l = &tree->l;
    r = &tree->r;
    m = &tree->m;
    x = &tree->x;
    y = &tree->y;
    
    assert_return( qrt_tree_node_value(A) == (qrt_tree_value_t)0xA421 );
    assert_return( qrt_tree_node_value(B) == (qrt_tree_value_t)0xA422 );
    assert_return( qrt_tree_node_value(l) == (qrt_tree_value_t)0xA423 );
    assert_return( qrt_tree_node_value(r) == (qrt_tree_value_t)0xA424 );
    assert_return( qrt_tree_node_value(m) == (qrt_tree_value_t)0xA425 );
    assert_return( qrt_tree_node_value(x) == (qrt_tree_value_t)0xA426 );
    assert_return( qrt_tree_node_value(y) == (qrt_tree_value_t)0xA427 );

    assert_return( qrt_tree_node_balance(l) == 0xBEE1 );
    assert_return( qrt_tree_node_balance(r) == 0xBEE2 );
    assert_return( qrt_tree_node_balance(x) == 0xBEE3 );
    assert_return( qrt_tree_node_balance(y) == 0xBEE4 );
    
    assert_return( check_link(B, l, LEFT) );    
    assert_return( check_link(A, r, RIGHT) );  
      
    assert_return( check_link(r, NULL, LEFT) );    
    assert_return( check_link(r, NULL, RIGHT) );    
    assert_return( check_link(l, NULL, LEFT) );    
    assert_return( check_link(l, NULL, RIGHT) );    
    assert_return( check_link(x, NULL, LEFT) );    
    assert_return( check_link(x, NULL, RIGHT) );    
    assert_return( check_link(y, NULL, LEFT) );    
    assert_return( check_link(y, NULL, RIGHT) );
    
    return true;
}

static bool check_tree(test_tree_t *tree, bool left) {
    qrt_tree_node_t *A;
    qrt_tree_node_t *B;
    qrt_tree_node_t *m;
    qrt_tree_node_t *l;
    qrt_tree_node_t *r;
    qrt_tree_node_t *x;
    qrt_tree_node_t *y;

    A = &tree->A;
    B = &tree->B;
    l = &tree->l;
    r = &tree->r;
    m = &tree->m;
    x = &tree->x;
    y = &tree->y;
    
    assert_return( check_tree_invariant(tree) );
    
    assert_return( check_link(m, x, LEFT) );    
    assert_return( check_link(m, y, RIGHT) );
    
    if(left) {
        assert_return(tree->root == A);        
        assert_return( check_link(NULL, A, false) );
        
        assert_return( check_link(A, B, LEFT) );        
        assert_return( check_link(B, m, RIGHT) );
    }
    else {
        assert_return(tree->root == B);        
        assert_return( check_link(NULL, B, false) );
        
        assert_return( check_link(B, A, RIGHT) );        
        assert_return( check_link(A, m, LEFT) );
    }
    
    return true;

}

/* This is what the tree looks after a double rotation for cases where it
 * is required (left-right and right-left cases):
 * 
 *                 m (4)
 *                / \ 
 *               /   \
 *              /     \
 *         (2) B       A (6)
 *            / \     / \
 *           /   \   /   \
 *          /     x y     \
 *     (1) l    (3) (5)    r (7)
 */
static bool check_tree_double(test_tree_t *tree) {
    qrt_tree_node_t *A;
    qrt_tree_node_t *B;
    qrt_tree_node_t *m;
    qrt_tree_node_t *l;
    qrt_tree_node_t *r;
    qrt_tree_node_t *x;
    qrt_tree_node_t *y;

    A = &tree->A;
    B = &tree->B;
    l = &tree->l;
    r = &tree->r;
    m = &tree->m;
    x = &tree->x;
    y = &tree->y;
    
    assert_return( check_tree_invariant(tree) );
    
    assert_return( check_link(m, B, LEFT) );    
    assert_return( check_link(m, A, RIGHT) );
    
    assert_return( check_link(B, l, LEFT) );    
    assert_return( check_link(B, x, RIGHT) );
    
    assert_return( check_link(A, y, LEFT) );
    
    return true;
}

void test_tree_node_accessor(void) {
    test_tree_t tree;
    
    report_start();
    
    /* left-leaning tree */
    initialize_tree_simple(&tree, 1, 0);
    
    assert( check_tree(&tree, LEFT) );
    assert( qrt_tree_node_balance(tree.root) == 1 );
    assert( qrt_tree_node_balance(&tree.B)   == 0 );
    
    /* right-leaning tree */initialize_tree(&tree, -2, 1, 1);
    initialize_tree_simple(&tree, -1, 0);
    
    assert( check_tree(&tree, RIGHT) );
    assert( qrt_tree_node_balance(tree.root) == -1 );
    assert( qrt_tree_node_balance(&tree.A)   ==  0 );
    
    /* left-leaning tree, full */
    initialize_tree(&tree, 1, -1, 0);
    
    assert( check_tree(&tree, LEFT) );
    assert( qrt_tree_node_balance(tree.root) ==  1 );
    assert( qrt_tree_node_balance(&tree.B)   == -1 );
    assert( qrt_tree_node_balance(&tree.m)   ==  0 );
    
    /* right-leaning tree, full */
    initialize_tree(&tree, -1, 1, 0);
    
    assert( check_tree(&tree, RIGHT) );
    assert( qrt_tree_node_balance(tree.root) == -1 );
    assert( qrt_tree_node_balance(&tree.A)   ==  1 );
    assert( qrt_tree_node_balance(&tree.m)   ==  0 );
}

void test_tree_sub_rotate_right(void) {
    test_tree_t tree;
    
    report_start();
    
    /* Balance factors:  root: 1 subroot: 1 */
    initialize_tree_simple(&tree, 1, 1);
    tree.root = qrt_tree_sub_rotate(tree.root);
    
    assert( check_tree(&tree, RIGHT) );
    assert( qrt_tree_node_balance(tree.root) == -1 );
    assert( qrt_tree_node_balance(&tree.A)   == -1 );
    
    /* Balance factors:  root: 1 subroot: 0 */
    initialize_tree_simple(&tree, 1, 0);
    tree.root = qrt_tree_sub_rotate(tree.root);
    
    assert( check_tree(&tree, RIGHT) );
    assert( qrt_tree_node_balance(tree.root) == -1 );
    assert( qrt_tree_node_balance(&tree.A)   ==  0 );
    
    /* Balance factors:  root: 1 subroot: -1 */
    initialize_tree_simple(&tree, 1, -1);
    tree.root = qrt_tree_sub_rotate(tree.root);
    
    assert( check_tree(&tree, RIGHT) );
    assert( qrt_tree_node_balance(tree.root) == -2 );
    assert( qrt_tree_node_balance(&tree.A)   ==  0 );
    
    /* Balance factors:  root: 2 subroot: 0 */
    initialize_tree_simple(&tree, 2, 0);
    tree.root = qrt_tree_sub_rotate(tree.root);
    
    assert( check_tree(&tree, RIGHT) );
    assert( qrt_tree_node_balance(tree.root) == -1 );
    assert( qrt_tree_node_balance(&tree.A)   ==  1 );
    
    /* Balance factors:  root: 2 subroot: 1 */
    initialize_tree_simple(&tree, 2, 1);
    tree.root = qrt_tree_sub_rotate(tree.root);
    
    assert( check_tree(&tree, RIGHT) );
    assert( qrt_tree_node_balance(tree.root) == 0 );
    assert( qrt_tree_node_balance(&tree.A)   == 0 );
    
    /* Balance factors:  root: 2 subroot: 2 */
    initialize_tree_simple(&tree, 2, 2);
    tree.root = qrt_tree_sub_rotate(tree.root);
    
    assert( check_tree(&tree, RIGHT) );
    assert( qrt_tree_node_balance(tree.root) ==  0 );
    assert( qrt_tree_node_balance(&tree.A)   == -1 );
}

void test_tree_sub_rotate_left(void) {
    test_tree_t tree;
    
    report_start();
    
    /* Balance factors:  root: -1 subroot: -1 */
    initialize_tree_simple(&tree, -1, -1);
    tree.root = qrt_tree_sub_rotate(tree.root);
    
    assert( check_tree(&tree, LEFT) );
    assert( qrt_tree_node_balance(tree.root) == 1 );
    assert( qrt_tree_node_balance(&tree.B)   == 1 );
    
    /* Balance factors:  root: -1 subroot: 0 */
    initialize_tree_simple(&tree, -1, 0);
    tree.root = qrt_tree_sub_rotate(tree.root);
    
    assert( check_tree(&tree, LEFT) );
    assert( qrt_tree_node_balance(tree.root) == 1 );
    assert( qrt_tree_node_balance(&tree.B)   ==  0 );
    
    /* Balance factors:  root: -1 subroot: 1 */
    initialize_tree_simple(&tree, -1, 1);
    tree.root = qrt_tree_sub_rotate(tree.root);
    
    assert( check_tree(&tree, LEFT) );
    assert( qrt_tree_node_balance(tree.root) == 2 );
    assert( qrt_tree_node_balance(&tree.B)   == 0 );
    
    /* Balance factors:  root: -2 subroot: 0 */
    initialize_tree_simple(&tree, -2, 0);
    tree.root = qrt_tree_sub_rotate(tree.root);
    
    assert( check_tree(&tree, LEFT) );
    assert( qrt_tree_node_balance(tree.root) ==  1 );
    assert( qrt_tree_node_balance(&tree.B)   == -1 );
    
    /* Balance factors:  root: -2 subroot: -1 */
    initialize_tree_simple(&tree, -2, -1);
    tree.root = qrt_tree_sub_rotate(tree.root);
    
    assert( check_tree(&tree, LEFT) );
    assert( qrt_tree_node_balance(tree.root) == 0 );
    assert( qrt_tree_node_balance(&tree.B)   == 0 );
    
    /* Balance factors:  root: -2 subroot: -2 */
    initialize_tree_simple(&tree, -2, -2);
    tree.root = qrt_tree_sub_rotate(tree.root);
    
    assert( check_tree(&tree, LEFT) );
    assert( qrt_tree_node_balance(tree.root) == 0 );
    assert( qrt_tree_node_balance(&tree.B)   == 1 );
}

void test_tree_sub_rotate_right_double(void) {
    test_tree_t tree;
    
    report_start();
    
    /* Balance factors:  root: 2 subroot: -1 middle: -1 */
    initialize_tree(&tree, 2, -1, -1);
    tree.root = qrt_tree_sub_rotate(tree.root);
    
    assert( check_tree_double(&tree) );
    assert( qrt_tree_node_balance(&tree.m) ==  0 );
    assert( qrt_tree_node_balance(&tree.A) ==  0 );
    assert( qrt_tree_node_balance(&tree.B) ==  1 );
    
    /* Balance factors:  root: 2 subroot: -1 middle: 0 */
    initialize_tree(&tree, 2, -1, 0);
    tree.root = qrt_tree_sub_rotate(tree.root);
    
    assert( check_tree_double(&tree) );
    assert( qrt_tree_node_balance(&tree.m) ==  0 );
    assert( qrt_tree_node_balance(&tree.A) ==  0 );
    assert( qrt_tree_node_balance(&tree.B) ==  0 );
    
    /* Balance factors:  root: 2 subroot: -1 middle: 1 */
    initialize_tree(&tree, 2, -1, 1);
    tree.root = qrt_tree_sub_rotate(tree.root);
    
    assert( check_tree_double(&tree) );
    assert( qrt_tree_node_balance(&tree.m) ==  0 );
    assert( qrt_tree_node_balance(&tree.A) == -1 );
    assert( qrt_tree_node_balance(&tree.B) ==  0 );
}

void test_tree_sub_rotate_left_double(void) {
    test_tree_t tree;
    
    report_start();
    
    /* Balance factors:  root: -2 subroot: 1 middle: 1 */
    initialize_tree(&tree, -2, 1, 1);
    tree.root = qrt_tree_sub_rotate(tree.root);
    
    assert( check_tree_double(&tree) );
    assert( qrt_tree_node_balance(&tree.m) ==  0 );
    assert( qrt_tree_node_balance(&tree.B) ==  0 );
    assert( qrt_tree_node_balance(&tree.A) == -1 );
    
    /* Balance factors:  root: -2 subroot: 1 middle: 0 */
    initialize_tree(&tree, -2, 1, 0);
    tree.root = qrt_tree_sub_rotate(tree.root);
    
    assert( check_tree_double(&tree) );
    assert( qrt_tree_node_balance(&tree.m) ==  0 );
    assert( qrt_tree_node_balance(&tree.B) ==  0 );
    assert( qrt_tree_node_balance(&tree.A) ==  0 );
    
    /* Balance factors:  root: -2 subroot: 1 middle: -1 */
    initialize_tree(&tree, -2, 1, -1);
    tree.root = qrt_tree_sub_rotate(tree.root);
    
    assert( check_tree_double(&tree) );
    assert( qrt_tree_node_balance(&tree.m) ==  0 );
    assert( qrt_tree_node_balance(&tree.B) ==  1 );
    assert( qrt_tree_node_balance(&tree.A) ==  0 );
}

void test_tree_node_next(void) {
    test_tree_t tree;
    
    report_start();
    
    initialize_tree_simple(&tree, 1, 0);
    
    assert( qrt_tree_node_next(&tree.l) ==  &tree.B);
    assert( qrt_tree_node_next(&tree.B) ==  &tree.x);
    assert( qrt_tree_node_next(&tree.x) ==  &tree.m);
    assert( qrt_tree_node_next(&tree.m) ==  &tree.y);
    assert( qrt_tree_node_next(&tree.y) ==  &tree.A);
    assert( qrt_tree_node_next(&tree.A) ==  &tree.r);
    assert( qrt_tree_node_next(&tree.r) ==  NULL);
    
    initialize_tree_simple(&tree, -1, 0);
    
    assert( qrt_tree_node_next(&tree.l) ==  &tree.B);
    assert( qrt_tree_node_next(&tree.B) ==  &tree.x);
    assert( qrt_tree_node_next(&tree.x) ==  &tree.m);
    assert( qrt_tree_node_next(&tree.m) ==  &tree.y);
    assert( qrt_tree_node_next(&tree.y) ==  &tree.A);
    assert( qrt_tree_node_next(&tree.A) ==  &tree.r);
    assert( qrt_tree_node_next(&tree.r) ==  NULL);
}

void test_tree_node_prev(void) {
    test_tree_t tree;
    
    report_start();
    
    initialize_tree_simple(&tree, 1, 0);
    
    assert( qrt_tree_node_prev(&tree.r) ==  &tree.A);    
    assert( qrt_tree_node_prev(&tree.A) ==  &tree.y);
    assert( qrt_tree_node_prev(&tree.y) ==  &tree.m);
    assert( qrt_tree_node_prev(&tree.m) ==  &tree.x);
    assert( qrt_tree_node_prev(&tree.x) ==  &tree.B);
    assert( qrt_tree_node_prev(&tree.B) ==  &tree.l);
    assert( qrt_tree_node_prev(&tree.l) ==  NULL);
    
    initialize_tree_simple(&tree, -1, 0);
    
    assert( qrt_tree_node_prev(&tree.r) ==  &tree.A);    
    assert( qrt_tree_node_prev(&tree.A) ==  &tree.y);
    assert( qrt_tree_node_prev(&tree.y) ==  &tree.m);
    assert( qrt_tree_node_prev(&tree.m) ==  &tree.x);
    assert( qrt_tree_node_prev(&tree.x) ==  &tree.B);
    assert( qrt_tree_node_prev(&tree.B) ==  &tree.l);
    assert( qrt_tree_node_prev(&tree.l) ==  NULL);
}

void test_tree_sub_validate(void) {
    test_tree_t tree;
    
    report_start();
    
    /* valid tree, left-leaning */
    initialize_tree_valid(&tree, LEFT);
    
    assert(qrt_tree_sub_validate(tree.root) == QRT_SUCCESS);
    
    /* valid tree, right-leaning */
    initialize_tree_valid(&tree, RIGHT);
    
    assert(qrt_tree_sub_validate(tree.root)  == QRT_SUCCESS);
    
    /* invalid tree: search tree property */
    initialize_tree_valid(&tree, LEFT);
    
    qrt_tree_node_key_lvalue(&tree.A) = 1;
    
    assert(qrt_tree_sub_validate(tree.root) != QRT_SUCCESS);
    
    /* invalid tree: balance factor */
    initialize_tree_valid(&tree, LEFT);
    
    qrt_tree_node_balance_lvalue(&tree.A) = 0;
    
    assert(qrt_tree_sub_validate(tree.root) != QRT_SUCCESS);
    
    initialize_tree_valid(&tree, RIGHT);
    
    qrt_tree_node_balance_lvalue(&tree.m) = -1;
    
    assert(qrt_tree_sub_validate(tree.root) != QRT_SUCCESS);
    
    /* invalid tree: AVL property */
    initialize_tree_valid(&tree, LEFT);
    
    link_nodes(&tree.r, NULL, LEFT);
    link_nodes(&tree.r, NULL, RIGHT);
    
    qrt_tree_node_balance_lvalue(&tree.r) = 0;
    qrt_tree_node_balance_lvalue(&tree.A) = 2;
    
    assert(qrt_tree_sub_validate(tree.root) != QRT_SUCCESS);
    
    /* invalid tree: parent pointer */
    initialize_tree_valid(&tree, LEFT);
    
    qrt_tree_node_parent_lvalue(&tree.B) = NULL;
    
    assert(qrt_tree_sub_validate(tree.root) != QRT_SUCCESS);
    
    initialize_tree_valid(&tree, RIGHT);
    
    qrt_tree_node_parent_lvalue(&tree.m) = &tree.r;
    
    assert(qrt_tree_sub_validate(tree.root) != QRT_SUCCESS);
}

void test_tree_sub_count(void) {
    test_tree_t tree;
    
    report_start();
    
    assert(qrt_tree_sub_count(NULL) == 0);
    
    initialize_tree_simple(&tree, 1, 0);
    
    assert(qrt_tree_sub_count(tree.root) == 7);
}

void test_tree_sub_height(void) {
    test_tree_t tree;
    
    report_start();
    
    assert(qrt_tree_sub_height(NULL) == 0);
    
    initialize_tree_simple(&tree, 1, 0);
    
    assert(qrt_tree_sub_height(tree.root) == 4);
}

void test_tree_node_depth(void) {
    test_tree_t tree;
    
    report_start();
    
    assert(qrt_tree_sub_height(NULL) == 0);
    
    initialize_tree_simple(&tree, 1, 0);
    
    assert(qrt_tree_node_depth(&tree.A) == 1);
    assert(qrt_tree_node_depth(&tree.B) == 2);
    assert(qrt_tree_node_depth(&tree.r) == 2);
    assert(qrt_tree_node_depth(&tree.l) == 3);
    assert(qrt_tree_node_depth(&tree.m) == 3);
    assert(qrt_tree_node_depth(&tree.x) == 4);
    assert(qrt_tree_node_depth(&tree.y) == 4);
}

int main(int argc, char *argv[]) {
    test_tree_node_accessor();
    test_tree_sub_rotate_right();
    test_tree_sub_rotate_left();
    test_tree_sub_rotate_right_double();
    test_tree_sub_rotate_left_double();
    test_tree_node_next();
    test_tree_node_prev();
    test_tree_sub_validate();
    test_tree_sub_count();
    test_tree_sub_height();
    test_tree_node_depth();
    
    return EXIT_SUCCESS;
}
