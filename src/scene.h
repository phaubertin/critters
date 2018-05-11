/*
 * Copyright (C) 2014-2018 Philippe Aubertin.
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

#ifndef CRITTERS_SCENE_H_
#define CRITTERS_SCENE_H_

#include <SDL/SDL.h>
#include "critter.h"


#define SCENE_WIDTH    800

#define SCENE_HEIGHT   500

#define SCENE_FOODS      4

#define SCENE_DANGERS    2


typedef struct scene_t scene_t;

scene_t *scene_new(void);

void scene_free(scene_t *scene);

void scene_render(scene_t *scene, SDL_Surface *screen, int v_offset, int h_offset);

void scene_update(scene_t *scene, float delta);

void scene_resize(scene_t *scene, int width, int height);

void scene_shake(scene_t *scene);

void scene_add_critter(scene_t *scene, critter_t *critter);

critter_t *scene_harvest_critter(scene_t *scene);

critter_t *scene_first_critter(scene_t *scene);

critter_t *scene_next_critter(scene_t *scene, critter_t *critter);

#endif
