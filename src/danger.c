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

#include <quatre/macros.h>
#include <stdbool.h>
#include <stdlib.h>
#include "danger.h"
#include "util.h"


/* in pixels per second */
#define SPEED 40.0

#define BOUND 8

static bool render_func(void *this_ptr, int x, int y) {
    danger_t *danger;
    int       bound;
    int       abs_x;
    
    if(x < 0) {
        abs_x = -x;
    }
    else {
        abs_x = x;
    }
    
    danger = (danger_t *)this_ptr;
    bound  = danger->thing.bound;
    
    if(y > bound - abs_x) {
        return false;
    }
    
    if(y < abs_x - bound) {
        return false;
    }
    
    return true;
}

static void update_func(void *this_ptr, float delta, float w, float h) {
    danger_t *danger;
    
    danger = (danger_t *)this_ptr;
    
    boing_update_thing_position(&danger->boing, &danger->thing, delta, w, h);
}

danger_t *danger_new(float x, float y, int dir) {
    danger_t *danger;
    bool      ret;
    
    danger = qrt_new(danger_t);
    
    if(danger != NULL) {
        ret  = boing_init(&danger->boing, SPEED, dir); 
        ret &= thing_init(
                &danger->thing,     /* object to initialize */
                THING_KIND_DANGER,  /* kind */
                x, y,               /* position */
                BOUND,              /* bounding box size */
                rgb(200, 0, 0),     /* colour */
                danger,             /* this (self) pointer */
                render_func,        /* rendering function */
                NULL,               /* pre-rendering function */
                update_func,        /* position update function */
                free );             /* finalizer */
        
        if(! ret) {
            free(danger);
            return NULL;
        }
    }
    
    return danger;
}
