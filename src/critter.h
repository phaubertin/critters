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

#ifndef CRITTERS_CRITTER_H_
#define CRITTERS_CRITTER_H_

#include <SDL/SDL.h>
#include "brain.h"
#include "thing.h"
#include "genome.h"
#include "stimuli.h"


typedef struct critter_t critter_t;

struct critter_t {
    thing_t          thing;
    genome_t        *genome;
    brain_control_t  brain_control;
    critter_t       *next;
    float            angle;
    int              food_count;
    int              danger_count;
            
    float           cx1;
    float           cy1;
    float           cx2;
    float           cy2;
};

critter_t *critter_new(genome_t *genome);

static inline void critter_free(critter_t *critter) {
    thing_free(&critter->thing);
}

static inline void critter_render(critter_t *critter, SDL_Surface *screen, int v_offset, int h_offset) {
    thing_render(&critter->thing, screen, v_offset, h_offset);
}

static inline void critter_update_position(critter_t *critter, float delta, float w, float h) {
    thing_update_position(&critter->thing, delta, w, h);
}

static inline void critter_update_brain(critter_t *critter, const stimuli_t *stimuli) {
    brain_control_compute(&critter->brain_control, critter->genome, stimuli);    
}

static inline void critter_genome_transplant(critter_t *critter, genome_t *genome) {
     genome_free(critter->genome);
     critter->genome = genome_clone(genome);
}

static inline void critter_set_position(critter_t *critter, float x, float y) {
    thing_set_position(&critter->thing, x, y);
}

static inline float critter_get_x(critter_t *critter) {
    return thing_get_x(&critter->thing);
}

static inline float critter_get_y(critter_t *critter) {
    return thing_get_y(&critter->thing);
}

static inline thing_t *critter_get_thing(critter_t *critter) {
    return &critter->thing;
}

#endif
