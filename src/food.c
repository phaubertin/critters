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
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include "food.h"
#include "util.h"


/* in pixels per second */
#define SPEED 10.0

#define BOUND 6

static bool render_func(void *this_ptr, int x, int y) {
    food_t *food;
    
    food = (food_t *)this_ptr;
    
    return sqrtf( (float)(x*x + y*y) ) <= food->thing.bound;
}

static void update_func(void *this_ptr, float delta, float w, float h) {
    food_t *food;
    
    food = (food_t *)this_ptr;
    
    boing_update_thing_position(&food->boing, &food->thing, delta, w, h);
}

food_t *food_new(float x, float y, int dir) {
    food_t *food;
    bool    ret;
    
    food = qrt_new(food_t);
    
    if(food != NULL) {
        ret  = boing_init(&food->boing, SPEED, dir); 
        ret &= thing_init(
                &food->thing,       /* object to initialize */
                THING_KIND_FOOD,    /* kind */
                x, y,               /* position */
                BOUND,              /* bounding box size */
                rgb(0, 200, 0),     /* colour */
                food,               /* this (self) pointer */
                render_func,        /* rendering function */
                NULL,               /* pre-rendering function */
                update_func,        /* position update function */
                free );             /* finalizer */

        if(! ret) {
            free(food);
            return NULL;
        }
    }
    
    return food;
}
