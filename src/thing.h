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

#ifndef CRITTER_THING_H_
#define CRITTER_THING_H_

#include <SDL/SDL.h>
#include <stdbool.h>
#include <stdint.h>

#define THING_KIND_CRITTER  0

#define THING_KIND_FOOD     1

#define THING_KIND_DANGER   2


typedef struct thing_t thing_t;

typedef bool (*thing_render_func_t)(void *this_ptr, int x, int y);

typedef void (*thing_pre_render_func_t)(void *this_ptr);

typedef void (*thing_update_func_t)(void *this_ptr, float delta, float w, float h);

typedef void (*thing_free_func_t)(void *this_ptr);


struct thing_t {
    int          kind;
    float       x;
    float       y;
    int          bound;
    uint32_t     colour;
    void        *this_ptr;
    thing_render_func_t      render_func;
    thing_pre_render_func_t  pre_func;
    thing_update_func_t      update_func;
    thing_free_func_t        free_func;
};

bool thing_init(
        thing_t                 *thing,
        int                      kind,
        float                   x,
        float                   y,
        int                      bound,
        uint32_t                 colour,
        void                    *this_ptr,
        thing_render_func_t      func,
        thing_pre_render_func_t  pre_func,
        thing_update_func_t      update_func,
        thing_free_func_t        free_func);

void thing_render(thing_t *thing, SDL_Surface *screen, int v_offset, int h_offset);

static inline void thing_update_position(thing_t *thing, float delta, float w, float h) {
    thing->update_func(thing->this_ptr, delta, w, h);
}

static inline void thing_free(thing_t *thing) {
    thing->free_func(thing->this_ptr);
}

static inline float thing_get_x(thing_t *thing) {
    return thing->x;
}

static inline float thing_get_y(thing_t *thing) {
    return thing->y;
}

static inline void thing_set_position(thing_t *thing, float x, float y) {
    thing->x = x;
    thing->y = y;
}

static inline int thing_get_kind(thing_t *thing) {
    return thing->kind;
}

#endif
