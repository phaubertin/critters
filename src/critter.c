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

#define _BSD_SOURCE /* for M_* constants in math.h */
#define _GNU_SOURCE /* for sincosf() in math.h */
#include <quatre/macros.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include "critter.h"
#include "util.h"

/* in pixels per second */
#define BASE_SPEED_FORWARD 100.0

/* in radian per second */
#define BASE_SPEED_ANGULAR 0.2 * M_PI

#define BOUND 10


static bool render_func(void *this_ptr, int x, int y) {
    critter_t *critter;
    float      bound;
    float      distance;
    float      dx, dy;
    
    critter = (critter_t *)this_ptr;
    bound   = (float)critter->thing.bound;
    
    dx = critter->cx2 - (float)x;
    dy = critter->cy2 - (float)y;
    
    distance = sqrtf( (dx*dx + dy*dy) );
    
    if(distance < 0.7 * bound) {
        critter->thing.colour = rgb(100, 100, 200);
        return true;
    }
    
    dx = critter->cx1 - (float)x;
    dy = critter->cy1 - (float)y;
    
    distance = sqrtf( (dx*dx + dy*dy) );
    
    if(distance < 0.4 * bound) {
        critter->thing.colour = critter->genome->colour;
        return true;
    }
    
    return false;
}

static void pre_render_func(void *this_ptr) {
    critter_t *critter;
    float cx, cy;
    
    critter = (critter_t *)this_ptr;
    
    cx =  (int)((float)critter->thing.bound * cosf(critter->angle));
    cy = -(int)((float)critter->thing.bound * sinf(critter->angle));
    
    critter->cx1 = 0.6 * cx;
    critter->cy1 = 0.6 * cy;
    critter->cx2 = 0.3 * (-cx);
    critter->cy2 = 0.3 * (-cy);
}

static void update_func(void *this_ptr, float delta, float w, float h) {
    critter_t   *critter;
    float        left_speed;
    float        right_speed;
    float        delta_s;
    float        delta_angle;
    float        speed;
    float        omega;
    float        x, y;
    float        ux, uy;
    
    critter = (critter_t *)this_ptr;
    
    left_speed  = critter->brain_control.left_speed;
    right_speed = critter->brain_control.right_speed;
    
    /* update position */
    speed    = BASE_SPEED_FORWARD * (right_speed + left_speed) * 0.5;
    delta_s  = (float)delta * speed;
    
    sincosf(critter->angle, &uy, &ux);
    
    x = critter_get_x(critter) + ux * delta_s;
    y = critter_get_y(critter) - uy * delta_s;
    
    if(x < 0.0) {
        x  = 0.0;
    }
    else if(x >= w) {
        x  = w - 1;
    }
    
    if(y < 0.0) {
        y  = 0.0;
    }
    else if(y >= h) {
        y  = h - 1;
    }
    
    critter_set_position(critter, x, y);
    
    /* update angle */
    omega        = BASE_SPEED_ANGULAR * (right_speed - left_speed);
    delta_angle  = (float)delta * omega;
    
    critter->angle += delta_angle;
        
    while(critter->angle < -M_PI) {
        critter->angle += 2 * M_PI;
    }
    
    while(critter->angle > M_PI) {
        critter->angle -= 2 * M_PI;
    }
}

static void free_func(void *this_ptr) {
    critter_t *critter;
    
    critter = (critter_t *)this_ptr;
    
    if(critter != NULL) {
        genome_free(critter->genome);
    }
    
    free(critter);
}

critter_t *critter_new(genome_t *genome) {
    critter_t *critter;
    bool       ret;
    
    critter = qrt_new(critter_t);
    
    if(critter != NULL) {
        critter->genome = genome_clone(genome);
        critter->angle          = 0.0;
        critter->food_count     = 0;
        critter->danger_count   = 0;
        
        ret  = brain_control_init(&critter->brain_control);
        ret &= thing_init(
                &critter->thing,    /* object to initialize */
                THING_KIND_CRITTER, /* kind */
                0.0, 0.0,           /* position */
                BOUND,              /* bounding box size */
                rgb(100, 100, 200), /* colour */
                critter,            /* this (self) pointer */
                render_func,        /* rendering function */
                pre_render_func,    /* pre-rendering function */
                update_func,        /* position update function */
                free_func );        /* finalizer */
            
        if(! ret) {
            free(critter);
            return NULL;
        }
    }
    
    return critter;
}
