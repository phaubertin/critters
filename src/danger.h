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

#ifndef CRITTERS_DANGER_H_
#define CRITTERS_DANGER_H_

#include <SDL/SDL.h>
#include "boing.h"
#include "thing.h"


typedef struct danger_t danger_t;

struct danger_t {
    thing_t thing;
    boing_t boing;
};


danger_t *danger_new(float x, float y, int dir);

static inline void danger_free(danger_t *danger) {
    thing_free(&danger->thing);
}

static inline void danger_render(danger_t *danger, SDL_Surface *screen, int v_offset, int h_offset) {
    thing_render(&danger->thing, screen, v_offset, h_offset);
}

static inline void danger_update_position(danger_t *danger, float delta, float w, float h) {
    thing_update_position(&danger->thing, delta, w, h);
}

static inline void danger_set_position(danger_t *danger, float x, float y) {
    thing_set_position(&danger->thing, x, y);
}

static inline float danger_get_x(danger_t *danger) {
    return thing_get_x(&danger->thing);
}

static inline float danger_get_y(danger_t *danger) {
    return thing_get_y(&danger->thing);
}

static inline thing_t *danger_get_thing(danger_t *danger) {
    return &danger->thing;
}

#endif
