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
#include <quatre/macros.h>
#include <quatre/tree.h>


                    /* ----- tree node definitions ----- */

qrt_tree_node_t *qrt_tree_node_next(qrt_tree_node_t *node) {
    qrt_tree_node_t *child;
    
    if(node == NULL) {
        return NULL;
    }
    
    /* If node has a right child, its in-order successor is the left-most child 
     * of its right child. */
    if(qrt_tree_node_right(node) != NULL) {
        node = qrt_tree_node_right(node);

        while(qrt_tree_node_left(node) != NULL) {
            node = qrt_tree_node_left(node);
        }
    }
    else {
        while(1) {
            /* No right child of node, so lets go back up to its parent. */
            child = node;
            node  = qrt_tree_node_parent(node);
            
            /* If the parent is NULL, node has no successor. */
            if(node == NULL) {
                break;
            }
            
            /* If we just came back up from the left, the parent is the
             * successor we are looking for. */
            if(qrt_tree_node_left(node) == child) {
                break;
            }
        }
    }
    
    return node;
}

qrt_tree_node_t *qrt_tree_node_prev(qrt_tree_node_t *node) {
    qrt_tree_node_t *child;
    
    if(node == NULL) {
        return NULL;
    }
    
    /* If node has a left child, its in-order predecessor is the right-most
     * child of its left child. */
    if(qrt_tree_node_left(node) != NULL) {
        node = qrt_tree_node_left(node);

        while(qrt_tree_node_right(node) != NULL) {
            node = qrt_tree_node_right(node);
        }
    }
    else {
        while(1) {
            /* No left child of node, so lets go back up to its parent. */
            child = node;
            node  = qrt_tree_node_parent(node);
            
            /* If the parent is NULL, node has no predecessor. */
            if(node == NULL) {
                break;
            }
            
            /* If we just came back up from the right, the parent is the
             * predecessor we are looking for. */
            if(qrt_tree_node_right(node) == child) {
                break;
            }
        }
    }
    
    return node;
}

unsigned int qrt_tree_node_depth(qrt_tree_node_t *node) {
    unsigned int depth;
    
    depth = 0;
    
    while(node != NULL) {
        node = qrt_tree_node_parent(node);
        ++depth;
    }
    
    return depth;    
}


                    /* ----- tree definitions ----- */


bool qrt_tree_init(qrt_tree_t *tree) {
    tree->root = NULL;
    
    return true;
}

void qrt_tree_finalize(qrt_tree_t *tree, qrt_tree_finalize_func_t finalizer, void *param) {
    qrt_tree_sub_destroy(tree->root, finalizer, param);
}

qrt_tree_t *qrt_tree_new(void) {
    qrt_tree_t *tree;
    
    tree = qrt_new(qrt_tree_t);
    
    if(tree != NULL) {
        if( ! qrt_tree_init(tree) ) {
            free(tree);
            return NULL;
        }
    }
    
    return tree;
}

void qrt_tree_free(qrt_tree_t *tree, qrt_tree_finalize_func_t finalizer, void *param) {
    if(tree != NULL) {
        qrt_tree_finalize(tree, finalizer, param);
    }
    
    free(tree);
}

QRT_INLINE qrt_tree_node_t *lookup_node(qrt_tree_t *tree, qrt_tree_key_t key, qrt_tree_node_t **pparent, bool *pleft) {
    qrt_tree_node_t *node;
    qrt_tree_node_t *parent;
    bool             left;
    
    node    = qrt_tree_root(tree);
    parent  = NULL;
    left    = true; /* arbitrary */
    
    while(1) {
        if(node == NULL) {
            break;
        }
        
        if( key == qrt_tree_node_key(node) ) {
            break;
        }
        
        parent = node;
        
        if( key < qrt_tree_node_key(node) ) {
            left = true;
            node = qrt_tree_node_left(node);
        }
        else {
            left = false;
            node = qrt_tree_node_right(node);
        }
    }
    
    if(pparent != NULL) {
        *pparent = parent;
    }
    
    if(pleft != NULL) {
        *pleft = left;
    }
    
    return node;
}

qrt_tree_node_t *qrt_tree_lookup_node(qrt_tree_t *tree, qrt_tree_key_t key) {
    return lookup_node(tree, key, NULL, NULL);
}

qrt_tree_value_t qrt_tree_lookup_value(qrt_tree_t *tree, qrt_tree_key_t key) {
    qrt_tree_node_t     *node;
    
    node = lookup_node(tree, key, NULL, NULL);
    
    if(node == NULL) {
        return NULL;
    }
    
    return qrt_tree_node_value(node);
}

static void rebalance_insert(qrt_tree_t *tree, qrt_tree_node_t *node) {
    qrt_tree_node_t *child;
    qrt_tree_node_t *parent;
    qrt_tree_node_t *old_node;
    int              balance;
    
    if( qrt_tree_node_balance(node) == 0) {
        return;
    }
    
    child = node;
    node  = qrt_tree_node_parent(node);
    
    while(node != NULL) {
        if(child == qrt_tree_node_left(node)) {
            qrt_tree_node_balance_lvalue(node) += 1;
        }
        else {
            assert(child == qrt_tree_node_right(node));
            
            qrt_tree_node_balance_lvalue(node) -= 1;
        }
        
        balance = qrt_tree_node_balance(node);
        
        if(balance == 0) {
            break;
        }
        
        if(balance < -1 || balance > 1) {
            parent   = qrt_tree_node_parent(node);
            old_node = node;
            node     = qrt_tree_sub_rotate(node);
            
            if(parent == NULL) {
                tree->root = node;
                break;
            }
            
            if(qrt_tree_node_left(parent) == old_node) {
                qrt_tree_node_left_lvalue(parent) = node;
            }
            else {
                assert(qrt_tree_node_right(parent) == old_node);
                
                qrt_tree_node_right_lvalue(parent) = node;
            }
            
            break;
        }

        child = node;
        node  = qrt_tree_node_parent(node);
    }
}

static void rebalance_remove(qrt_tree_t *tree, qrt_tree_node_t *node) {
    qrt_tree_node_t *child;
    qrt_tree_node_t *parent;
    qrt_tree_node_t *old_node;
    int              balance;
    
    if(node == NULL) {
        return;
    }
    
    balance = qrt_tree_node_balance(node);
    
    if(balance < -1 || balance > 1) {
        parent   = qrt_tree_node_parent(node);
        old_node = node;
        node     = qrt_tree_sub_rotate(node);
        
        if(parent == NULL) {
            tree->root = node;
            return;
        }
        
        if(qrt_tree_node_left(parent) == old_node) {
            qrt_tree_node_left_lvalue(parent) = node;
        }
        else {
            assert(qrt_tree_node_right(parent) == old_node);
            
            qrt_tree_node_right_lvalue(parent) = node;
        }
        
        balance = qrt_tree_node_balance(node);
    }
    
    if( balance != 0) {
        return;
    }
    
    child = node;
    node  = qrt_tree_node_parent(node);
    
    while(node != NULL) {        
        if(child == qrt_tree_node_left(node)) {
            qrt_tree_node_balance_lvalue(node) -= 1;
        }
        else {
            assert(child == qrt_tree_node_right(node));
            
            qrt_tree_node_balance_lvalue(node) += 1;
        }
        
        balance = qrt_tree_node_balance(node);
        
        if(balance < -1 || balance > 1) {
            parent   = qrt_tree_node_parent(node);
            old_node = node;
            node     = qrt_tree_sub_rotate(node);
            
            if(parent == NULL) {
                tree->root = node;
                break;
            }
            
            if(qrt_tree_node_left(parent) == old_node) {
                qrt_tree_node_left_lvalue(parent) = node;
            }
            else {
                assert(qrt_tree_node_right(parent) == old_node);
                
                qrt_tree_node_right_lvalue(parent) = node;
            }
            
            balance = qrt_tree_node_balance(node);
        }
        
        if(balance != 0) {
            break;
        }

        child = node;
        node  = qrt_tree_node_parent(node);
    }
}

QRT_INLINE qrt_tree_node_t *add_node(qrt_tree_t *tree, qrt_tree_key_t key, qrt_tree_node_t *parent, bool left) {
    qrt_tree_node_t     *node;

    /* create new node */
    node = qrt_new(qrt_tree_node_t);
    
    if(node == NULL) {
        return NULL;
    }
    
    qrt_tree_node_key_lvalue(node)      = key;
    qrt_tree_node_value_lvalue(node)    = NULL;
    qrt_tree_node_left_lvalue(node)     = NULL;
    qrt_tree_node_right_lvalue(node)    = NULL;
    qrt_tree_node_parent_lvalue(node)   = parent;
    qrt_tree_node_balance_lvalue(node)  = 0;
    
    /* link new node in tree */    
    if(parent == NULL) {
        tree->root = node;
    }
    else {
        if(left) {
            qrt_tree_node_left_lvalue(parent)     = node;
            qrt_tree_node_balance_lvalue(parent) += 1;
        }
        else {
            qrt_tree_node_right_lvalue(parent)    = node;
            qrt_tree_node_balance_lvalue(parent) -= 1;
        }
    }
    
    /* re-balance tree */
    rebalance_insert(tree, parent);
    
    return node;
}

qrt_tree_node_t *qrt_tree_add_node(qrt_tree_t *tree, qrt_tree_key_t key) {
    qrt_tree_node_t     *node;
    qrt_tree_node_t     *parent;
    bool                 left;
    
    /* check if node is already in the tree, and return it if it is */
    node = lookup_node(tree, key, &parent, &left);
    
    if(node != NULL) {
        return node;
    }
    
    /* create new node */
    return add_node(tree, key, parent, left);
}

int qrt_tree_add_value(qrt_tree_t *tree, qrt_tree_key_t key, qrt_tree_value_t value) {
    qrt_tree_node_t *node;
    
    node = qrt_tree_add_node(tree, key);
    
    if(node == NULL) {
        return QRT_ERROR;
    }
    
    qrt_tree_node_set_value(node, value);
    
    return QRT_SUCCESS;
}

int qrt_tree_add_value_duplicate(qrt_tree_t *tree, qrt_tree_key_t key, qrt_tree_value_t value) {
    qrt_tree_node_t *node;
    qrt_tree_node_t *parent;
    bool             left;
    
    node    = qrt_tree_root(tree);
    parent  = NULL;
    left    = true; /* arbitrary */
    
    while(1) {
        if(node == NULL) {
            break;
        }

        parent = node;
        
        /* if we find an equal node, we go right */
        if( key < qrt_tree_node_key(node) ) {
            left = true;
            node = qrt_tree_node_left(node);
        }
        else {
            left = false;
            node = qrt_tree_node_right(node);
        }
    }
    
    node = add_node(tree, key, parent, left);
    
    if(node == NULL) {
        return QRT_ERROR;
    }
    
    qrt_tree_node_set_value(node, value);
    
    return QRT_SUCCESS;
}

QRT_INLINE qrt_tree_node_t *remove_node(qrt_tree_t *tree, qrt_tree_node_t *node, qrt_tree_finalize_func_t finalizer, void *param) {
    qrt_tree_node_t     *parent;
    qrt_tree_node_t     *victim;
    qrt_tree_node_t     *child;
    qrt_tree_node_t     *next;
    
    if(node == NULL) {
        return NULL;
    }
    
    /* figure out which node to remove */    
    if(qrt_tree_node_left(node) == NULL || qrt_tree_node_right(node) == NULL) {
        /* we remove node */
        victim = node;
        next   = qrt_tree_node_next(node);
        
        /* finalize value */
        if(finalizer != NULL) {
            finalizer(param, qrt_tree_node_value(node));
        }
    }
    else {
        /* We swap node with its in-order sucessor, and delete the sucessor
         * instead. */
        victim = qrt_tree_node_next(node);
        next   = node;
        
        /* finalize value and copy node content */
        if(finalizer != NULL) {
            finalizer(param, qrt_tree_node_value(node));
        }
        
        qrt_tree_node_key_lvalue(node)      = qrt_tree_node_key(victim);
        qrt_tree_node_value_lvalue(node)    = qrt_tree_node_value(victim);
    }
    
    /* Replace victim by its child, if any. victim is guaranteed to have at
     * most one child. */
    assert(qrt_tree_node_left(victim) == NULL || qrt_tree_node_right(victim) == NULL);
    
    if(qrt_tree_node_left(victim) != NULL) {
        child = qrt_tree_node_left(victim);
    }
    else if(qrt_tree_node_right(victim) != NULL) {
        child = qrt_tree_node_right(victim);
    }
    else {
        child = NULL;
    }
    
    if(child != NULL) {
        child->parent = victim->parent;
    }
    
    if(victim == tree->root) {
        tree->root = child;
        parent     = NULL;
    }
    else {
        parent = qrt_tree_node_parent(victim);
        
        if( victim == qrt_tree_node_left(parent) ) {
            qrt_tree_node_left_lvalue(parent)     = child;
            qrt_tree_node_balance_lvalue(parent) -= 1;
        }
        else {
            qrt_tree_node_right_lvalue(parent)    = child;
            qrt_tree_node_balance_lvalue(parent) += 1;
        }
    }
    
    /* remove node */
    free(victim);
    
    /* re-balance tree */
    rebalance_remove(tree, parent);
    
    return next;
}

bool qrt_tree_remove_key(qrt_tree_t *tree, qrt_tree_key_t key, qrt_tree_finalize_func_t finalizer, void *param) {
    qrt_tree_node_t      *node;
    
    node = lookup_node(tree, key, NULL, NULL);
    
    if(node == NULL) {
        return false;
    }
    
    (void)remove_node(tree, node, finalizer, param);
    
    return true;
}

void qrt_tree_remove_node(qrt_tree_t *tree, qrt_tree_node_t *node, qrt_tree_finalize_func_t finalizer, void *param) {
    (void)remove_node(tree, node, finalizer, param);
}

QRT_INLINE qrt_tree_value_t pop_min_max(qrt_tree_t *tree, bool go_left) {
    qrt_tree_value_t     value;
    qrt_tree_node_t     *node;
    
    node = qrt_tree_root(tree);
    
    if(node == NULL) {
        return NULL;
    }
    
    if(go_left) {
        while(qrt_tree_node_left(node) != NULL) {
            node = qrt_tree_node_left(node);
        }
    }
    else {
        while(qrt_tree_node_right(node) != NULL) {
            node = qrt_tree_node_right(node);
        }
    }
    
    value = qrt_tree_node_value(node);
    
    qrt_tree_remove_node(tree, node, NULL, NULL);
    
    return value;
}

qrt_tree_value_t qrt_tree_pop_min(qrt_tree_t *tree) {
    return pop_min_max(tree, true);
}

qrt_tree_value_t qrt_tree_pop_max(qrt_tree_t *tree) {
    return pop_min_max(tree, false);
}

qrt_tree_value_t qrt_tree_pop_random(qrt_tree_t *tree) {
    qrt_tree_value_t     value;
    qrt_tree_node_t     *node;
    qrt_tree_node_t     *child;
    qrt_tree_node_t     *parent;
    int                  step;
    bool                 done;
    int                  whereto;
    
    node = qrt_tree_root(tree);
    
    if(node == NULL) {
        return NULL;
    }
    
    /* walk randomly until we hit a wall */
    while(1) {
        whereto = rand();
        
        /* RAND_MAX is guaranteed to be at least 32767 (i.e. 15 bits) by the
         * C standard */
        for(step = 0; step < 15; ++step) {
            done  = true;
            
            if((whereto & 1) == 0) {
                child = qrt_tree_node_left(node);
            }
            else {
                child = qrt_tree_node_right(node);
            }
            
            if(child == NULL) {
                break;
            }
            
            node = child;
            whereto >>= 1;
            
            done  = false;
        }

        if(done) {
            break;
        }
    }
    
    /* go back up a random number of steps */
    while(1) {
        whereto = rand();
        done  = false;

        for(step = 0; step < 15; ++step) {
            done  = true;
            
            if((whereto & 1) == 0) {
                break;
            }
            
            parent = qrt_tree_node_parent(node);
            
            if(parent == NULL) {
                break;
            }
            
            node = parent;
            whereto >>= 1;
            
            done  = false;
        }
        
        if(done) {
            break;
        }
    }
    
    value = qrt_tree_node_value(node);
    
    qrt_tree_remove_node(tree, node, NULL, NULL);
    
    return value;
}

void qrt_tree_clear(qrt_tree_t *tree, qrt_tree_finalize_func_t finalizer, void *param) {
    qrt_tree_sub_destroy(tree->root, finalizer, param);
    tree->root = NULL;
}

bool qrt_tree_is_empty(qrt_tree_t *tree) {
    return tree->root == NULL;
}

int qrt_tree_validate(qrt_tree_t *tree) {
    return qrt_tree_sub_validate(tree->root);
}

unsigned int qrt_tree_count(qrt_tree_t *tree) {
    return qrt_tree_sub_count(tree->root);
}

unsigned int qrt_tree_height(qrt_tree_t *tree) {
    return qrt_tree_sub_height(tree->root);
}


                    /* ----- subtree functions ----- */

void qrt_tree_sub_destroy(qrt_tree_node_t *node, qrt_tree_finalize_func_t finalizer, void *param) {
    if(node == NULL) {
        return;
    }
    
    qrt_tree_sub_destroy(qrt_tree_node_left(node),  finalizer, param);
    qrt_tree_sub_destroy(qrt_tree_node_right(node), finalizer, param);
    
    if(finalizer != NULL) {
        finalizer(param,  qrt_tree_node_value(node));
    }
    
    free(node);
}

qrt_tree_node_t *qrt_tree_sub_rotate(qrt_tree_node_t *node) {
    qrt_tree_node_t* pivot;
    qrt_tree_node_t* child;
    qrt_tree_node_t* parent;
    
    assert(qrt_tree_node_balance(node) != 0);
    
    if(qrt_tree_node_balance(node) < 0) {
        /* rotate left */
        pivot = qrt_tree_node_right(node);
        
        assert(pivot != NULL);
        
        /* right-left case */
        if(qrt_tree_node_balance(node) == -2 && qrt_tree_node_balance(pivot) == 1) {
            pivot = qrt_tree_sub_rotate(pivot);
            assert(pivot != NULL);
        }
        
        child = qrt_tree_node_left(pivot);
        
        qrt_tree_node_right_lvalue(node) = child;
        qrt_tree_node_left_lvalue(pivot) = node;
        
        /* update balance factors */
        if(qrt_tree_node_balance(node) == -2) {
            qrt_tree_node_balance_lvalue(node)  =  -1 - qrt_tree_node_balance(pivot);
            
            if(qrt_tree_node_balance(pivot) == 0) {
                qrt_tree_node_balance_lvalue(pivot) = 1;
            }
            else {
                qrt_tree_node_balance_lvalue(pivot) = 0;
            }
        }
        else {
            assert(qrt_tree_node_balance(node) == -1);
            
            if(qrt_tree_node_balance(pivot) == 1) {
                qrt_tree_node_balance_lvalue(node)  = 0;
                qrt_tree_node_balance_lvalue(pivot) = 2;
            }
            else {
                qrt_tree_node_balance_lvalue(node)  = 0 - qrt_tree_node_balance(pivot);
                qrt_tree_node_balance_lvalue(pivot) = 1;
            }
        }
    }
    else {
        /* rotate right */
        pivot = qrt_tree_node_left(node);
    
        assert(pivot != NULL);
        
        /* left-right case */
        if(qrt_tree_node_balance(node) == 2 && qrt_tree_node_balance(pivot) == -1) {
            pivot = qrt_tree_sub_rotate(pivot);
            assert(pivot != NULL);
        }
        
        child = qrt_tree_node_right(pivot);
        
        qrt_tree_node_left_lvalue(node)   = child;
        qrt_tree_node_right_lvalue(pivot) = node;
        
        /* update balance factors */
        if(qrt_tree_node_balance(node) == 2) {
            qrt_tree_node_balance_lvalue(node)  =  1 - qrt_tree_node_balance(pivot);
            
            if(qrt_tree_node_balance(pivot) == 0) {
                qrt_tree_node_balance_lvalue(pivot) = -1;
            }
            else {
                qrt_tree_node_balance_lvalue(pivot) =  0;
            }
        }
        else {
            assert(qrt_tree_node_balance(node) == 1);
            
            if(qrt_tree_node_balance(pivot) == -1) {
                qrt_tree_node_balance_lvalue(node)  =  0;
                qrt_tree_node_balance_lvalue(pivot) = -2;
            }
            else {
                qrt_tree_node_balance_lvalue(node)  =  0 - qrt_tree_node_balance(pivot);
                qrt_tree_node_balance_lvalue(pivot) = -1;
            }
        }
    }
    
    /* update parent pointers */
    parent = qrt_tree_node_parent(node);
    
    qrt_tree_node_parent_lvalue(pivot) = parent;
    qrt_tree_node_parent_lvalue(node)  = pivot;
    
    if(child != NULL) {
        qrt_tree_node_parent_lvalue(child) = node;
    }
    
    return pivot;
}

static int validate_recursive(qrt_tree_node_t *node, qrt_tree_node_t *parent, int *height) {
    qrt_tree_node_t *left;
    qrt_tree_node_t *right;
    int              height_left;
    int              height_right;
    int              balance;
    int              status;
    
    if(node == NULL) {
        return QRT_SUCCESS;
    }
    
    /* check parent pointer */
    if(qrt_tree_node_parent(node) != parent) {
        return __LINE__;
    }
    
    left  = qrt_tree_node_left(node);
    right = qrt_tree_node_right(node);
    
    /* check search tree property */
    if(left != NULL) {
        if(qrt_tree_node_key(left) >= qrt_tree_node_key(node)) {
            return __LINE__;
        }
    }
    
    if(right != NULL) {
        if(qrt_tree_node_key(right) <= qrt_tree_node_key(node)) {
            return __LINE__;
        }
    }
    
    /* validate children recursively, get heights */
    height_left  = 0;
    height_right = 0;
    
    status = validate_recursive(left, node, &height_left);
    
    if(status != QRT_SUCCESS) {
        return status;
    }
    
    status = validate_recursive(right, node, &height_right);
    
    if(status != QRT_SUCCESS) {
        return status;
    }
    
    if(height != NULL) {
        *height = 1 + qrt_max(height_left, height_right);
    }
    
    /* check balance field */
    balance = height_left - height_right;
    
    if(qrt_tree_node_balance(node) != balance) {
        return __LINE__;
    }
    
    /* check AVL property */
    if(balance < -1 || balance > 1) {
        return __LINE__;
    }
    
    return QRT_SUCCESS;
}

int qrt_tree_sub_validate(qrt_tree_node_t *node) {
    return validate_recursive(node, NULL, NULL);
}

unsigned int qrt_tree_sub_count(qrt_tree_node_t *node) {
    if(node == NULL) {
        return 0;
    }
    
    return 1 + qrt_tree_sub_count( qrt_tree_node_left(node) )
             + qrt_tree_sub_count( qrt_tree_node_right(node) );
}

unsigned int qrt_tree_sub_height(qrt_tree_node_t *node) {
    if(node == NULL) {
        return 0;
    }
    
    return 1 + qrt_max(
        qrt_tree_sub_height( qrt_tree_node_left(node)  ) ,
        qrt_tree_sub_height( qrt_tree_node_right(node) ) );
}


                    /* ----- tree iterator definitions ----- */

struct qrt_tree_iterator_t {
    qrt_tree_node_t *node;
    qrt_tree_t      *tree;
};

QRT_INLINE qrt_tree_iterator_t *new_iterator(qrt_tree_t *tree) {
    qrt_tree_iterator_t *iter;
    
    iter = qrt_new(qrt_tree_iterator_t);
    
    if(iter != NULL) {
        iter->tree = tree;
    }
    
    return iter;
}

qrt_tree_iterator_t *qrt_tree_iterator_new(qrt_tree_t *tree) {
    qrt_tree_iterator_t *iter;
    
    iter = new_iterator(tree);
    
    if(iter != NULL) {
        (void)qrt_tree_iterator_to_start(iter);
    }
        
    return iter;
}

qrt_tree_iterator_t *qrt_tree_iterator_new_from_end(qrt_tree_t *tree) {
    qrt_tree_iterator_t *iter;
    
    iter = new_iterator(tree);
    
    if(iter != NULL) {
        (void)qrt_tree_iterator_to_end(iter);
    }
        
    return iter;
}

void qrt_tree_iterator_free(qrt_tree_iterator_t *iter) {
    free(iter);
}

qrt_tree_node_t *qrt_tree_iterator_node(qrt_tree_iterator_t *iter) {
    return iter->node;
}

qrt_tree_key_t qrt_tree_iterator_key(qrt_tree_iterator_t *iter) {
    if(iter->node == NULL) {
        return (qrt_tree_key_t)0;
    }
    
    return qrt_tree_node_key(iter->node);
}

qrt_tree_value_t qrt_tree_iterator_value(qrt_tree_iterator_t *iter) {
    if(iter->node == NULL) {
        return NULL;
    }
    
    return qrt_tree_node_value(iter->node);
}

qrt_tree_node_t *qrt_tree_iterator_to_start(qrt_tree_iterator_t *iter) {
    qrt_tree_node_t *node;
    
    node = qrt_tree_root(iter->tree);
    
    while(qrt_tree_node_left(node) != NULL) {
        node = qrt_tree_node_left(node);
    }
    
    iter->node = node;
    
    return iter->node;
}

qrt_tree_node_t *qrt_tree_iterator_to_end(qrt_tree_iterator_t *iter) {
    qrt_tree_node_t *node;
    
    node = qrt_tree_root(iter->tree);
    
    while(qrt_tree_node_right(node) != NULL) {
        node = qrt_tree_node_right(node);
    }
    
    iter->node = node;
    
    return iter->node;
}

qrt_tree_node_t *qrt_tree_iterator_next(qrt_tree_iterator_t *iter) {
    iter->node = qrt_tree_node_next(iter->node);
    
    return iter->node;
}

qrt_tree_node_t *qrt_tree_iterator_prev(qrt_tree_iterator_t *iter) {
    iter->node = qrt_tree_node_prev(iter->node);
    
    return iter->node;
}

qrt_tree_node_t *qrt_tree_iterator_remove(qrt_tree_iterator_t *iter, qrt_tree_finalize_func_t finalizer, void *param) {
    iter->node = remove_node(iter->tree, iter->node, finalizer, param);
    
    return iter->node;
}
