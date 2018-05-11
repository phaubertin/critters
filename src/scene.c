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

#define _BSD_SOURCE /* for M_* constants in math.h */
#include <quatre/macros.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include "critter.h"
#include "danger.h"
#include "food.h"
#include "scene.h"
#include "stimuli.h"


#define VISION_DISTANCE_LIMIT   600.0

#define VISION_ANGLE_LIMIT      0.7 * M_PI_2

#define SCENT_DISTANCE_LIMIT    250.0


#define SCENE_THINGS    (SCENE_FOODS + SCENE_DANGERS)


typedef void (*render_func_t)(scene_t *, int, int);


struct scene_t {
    int          width;
    int          height;
    critter_t   *critter;
    thing_t     *thing[SCENE_THINGS];
};

static inline int random_horizontal_position(const scene_t *scene) {
    return rand() % scene->width;
}

static inline int random_vertical_position(const scene_t *scene) {
    return rand() % scene->height;
}

scene_t *scene_new(void) {
    scene_t      *scene;
    food_t       *food;
    danger_t     *danger;
    thing_t     **thing_ptr;
    bool         something_null;
    int          idx;
   
    scene = qrt_new(scene_t);
    
    if(scene != NULL) {
        scene->width    = SCENE_WIDTH;
        scene->height   = SCENE_HEIGHT;
        something_null  = false;
        
        thing_ptr = scene->thing;
        
        for(idx = 0; idx < SCENE_FOODS; ++idx) {
            food = food_new(
                    random_horizontal_position(scene),
                    random_vertical_position(scene),
                    rand());
            
            if(food == NULL) {
                something_null = true;
            }
            
            *(thing_ptr++) = food_get_thing(food);
        }
        
        for(idx = 0; idx < SCENE_DANGERS; ++idx) {
            danger = danger_new(
                    random_horizontal_position(scene),
                    random_vertical_position(scene),
                    rand());
            
            if(danger == NULL) {
                something_null = true;
            }

            *(thing_ptr++) = danger_get_thing(danger);
        }
        
        if(something_null) {
            scene_free(scene);
            return NULL;
        }
        
        scene->critter = NULL;
    }
    
    return scene;
}

void scene_free(scene_t *scene) {
    critter_t   *critter;
    critter_t   *victim;
    int          idx;
    
    if(scene != NULL) {
        for(idx = 0; idx < SCENE_THINGS; ++idx) {
            thing_free(scene->thing[idx]);
        }
        
        critter = scene->critter;
    
        while(critter != NULL) {
            victim  = critter;
            critter = critter->next;
            
            critter_free(victim);
        }
    }
        
    free(scene);
}

void scene_render(scene_t *scene, SDL_Surface *screen, int v_offset, int h_offset) {
    critter_t   *critter;
    int          idx;
    
    for(idx = 0; idx < SCENE_THINGS; ++idx) {
        thing_render(scene->thing[idx], screen, v_offset, h_offset);
    }
    
    critter = scene->critter;
    
    while(critter != NULL) {
        critter_render(critter, screen, v_offset, h_offset);
        
        critter = critter->next;
    }
}

static bool compute_stimuli(stimuli_t*stimuli, critter_t *critter, scene_t *scene) {
    thing_t             *thing;
    int                  kind;
    float                critter_angle;
    float                target_angle;
    float                view_angle;
    float                distance;
    float                distance2;
    float                bound2;
    bool                 zero_two_pi;    
    float                x, y;
    float                intensity;
    int                  idx;
        
    critter_angle = critter->angle;

    /* We store angles in the range -pi..pi. If the critter is looking 
     * in a direction close to -pi or pi, we convert the range to 0..2*pi
     * so we don't have to deal with the discontinuity. */
    if(critter_angle > M_PI_2) {
        zero_two_pi = true;
    }
    else if(critter_angle < -M_PI_2) {
        critter_angle += 2 * M_PI;
        zero_two_pi = true;
    }
    else {
        zero_two_pi = false;
    }
    
    /* compute food and danger stimuli */
    stimuli->food_intensity     = 0.0;
    stimuli->food_angle         = 0.0;
    stimuli->food_odour         = 0.0;
    
    stimuli->danger_intensity   = 0.0;
    stimuli->danger_angle       = 0.0;
    stimuli->danger_odour       = 0.0;
    
    bound2  = critter_get_thing(critter)->bound;
    bound2 *= bound2;
    
    for(idx = 0; idx < SCENE_THINGS; ++idx) {
        thing = scene->thing[idx];
        kind  = thing_get_kind(thing);
        
        x     = thing_get_x(thing) - critter_get_x(critter);
        y     = thing_get_y(thing) - critter_get_y(critter);
        
        distance2 = x*x + y*y;
        
        /* distance < bound */
        if(__builtin_expect (distance2 < bound2, 0)) {
            if(kind == THING_KIND_FOOD) {
                critter->food_count += 1;
                
                /* simulate deleting the thing and adding a new one by changing its
                 * position */
                thing_set_position(
                        thing,
                        random_horizontal_position(scene),
                        random_vertical_position(scene));
                
                continue;
            }            
            else if(kind == THING_KIND_DANGER) {
                critter->danger_count += 1;
                
                /* "dead" for this round  */
                critter_set_position(
                        critter,
                        random_horizontal_position(scene),
                        random_vertical_position(scene));
                
                return false;
            }
        }
        
        /* distance < VISION_DISTANCE_LIMIT */
        if(distance2 < VISION_DISTANCE_LIMIT * VISION_DISTANCE_LIMIT) {            
            target_angle = atan2f(-y, x);
            distance     = sqrtf(distance2);
            
            if(zero_two_pi && target_angle < 0.0) {
                target_angle += 2 * M_PI;
            }
            
            view_angle = critter_angle - target_angle;
            
            if(view_angle < VISION_ANGLE_LIMIT && view_angle > -VISION_ANGLE_LIMIT) {
                intensity = (VISION_DISTANCE_LIMIT - distance) * (1.0 / VISION_DISTANCE_LIMIT);
                
                if(kind == THING_KIND_FOOD) {
                    if(intensity > stimuli->food_intensity) {
                        stimuli->food_intensity = intensity;
                        stimuli->food_angle     = view_angle * (1.0 / VISION_ANGLE_LIMIT);
                    }
                }
                else if(kind == THING_KIND_DANGER) {
                    if(intensity > stimuli->danger_intensity) {
                        stimuli->danger_intensity = intensity;
                        stimuli->danger_angle     = view_angle * (1.0 / VISION_ANGLE_LIMIT);
                    }
                }
            }
            
            /* SCENT_DISTANCE_LIMIT < VISION_DISTANCE_LIMIT, which means
             * distance < SCENT_DISTANCE_LIMIT implies distance < SCENT_VISION_LIMIT */
            if(distance < SCENT_DISTANCE_LIMIT) {
                intensity = (SCENT_DISTANCE_LIMIT - distance) * (1.0 / SCENT_DISTANCE_LIMIT);            
                
                if(kind == THING_KIND_FOOD) {
                    if(intensity > stimuli->food_odour) {
                        stimuli->food_odour += intensity;
                    }
                }
                else if(kind == THING_KIND_DANGER) {
                    if(intensity > stimuli->danger_odour) {
                        stimuli->danger_odour += intensity;
                    }
                }
            }
        }
    }
    
    /* compute wall stimuli */
    stimuli->wall_intensity   = 0.0;
    stimuli->wall_angle       = 0.0;
    
    if(critter_angle > 0.0) {
        /* top wall */
        distance  = critter_get_y(critter) / sinf(critter_angle);
        
        if(distance < VISION_DISTANCE_LIMIT) {
            intensity = (VISION_DISTANCE_LIMIT - distance) * (1.0 / VISION_DISTANCE_LIMIT);
            
            if(intensity > stimuli->wall_intensity) {
                stimuli->wall_intensity = intensity;
                stimuli->wall_angle     = (critter_angle - M_PI_2) * (1.0 / M_PI_2);
            }
        }
    }
    else if(critter_angle < -0.0) {
        /* bottom wall */
        distance  = (critter_get_y(critter) - (float)scene->height) / sinf(critter_angle);
        
        if(distance < VISION_DISTANCE_LIMIT) {
            intensity = (VISION_DISTANCE_LIMIT - distance) * (1.0 / VISION_DISTANCE_LIMIT);
            
            if(intensity > stimuli->wall_intensity) {
                stimuli->wall_intensity = intensity;
                stimuli->wall_angle     = (M_PI_2 + critter_angle) * (1.0 / M_PI_2);
            }
        }
    }
    
    if (critter_angle > -M_PI_2 && critter_angle < M_PI_2) {
        /* right wall */
        distance  = ((float)scene->width - critter_get_x(critter)) / cosf(critter_angle);
        
        if(distance < VISION_DISTANCE_LIMIT) {
            intensity = (VISION_DISTANCE_LIMIT - distance) * (1.0 / VISION_DISTANCE_LIMIT);
            
            if(intensity > stimuli->wall_intensity) {
                stimuli->wall_intensity = intensity;
                stimuli->wall_angle     = critter_angle * (1.0 / M_PI_2);
            }
        }
    }
    else if(critter_angle < -M_PI_2) {
        /* left wall */
        distance  = -critter_get_x(critter) / cosf(critter_angle);
        
        if(distance < VISION_DISTANCE_LIMIT) {
            intensity = (VISION_DISTANCE_LIMIT - distance) * (1.0 / VISION_DISTANCE_LIMIT);
            
            if(intensity > stimuli->wall_intensity) {
                stimuli->wall_intensity = intensity;
                stimuli->wall_angle     = (critter_angle - M_PI) * (1.0 / M_PI_2);
            }
        }
    }
    else if(critter_angle > M_PI_2) {
        /* left wall */
        distance  = -critter_get_x(critter) / cosf(critter_angle);
        
        if(distance < VISION_DISTANCE_LIMIT) {
            intensity = (VISION_DISTANCE_LIMIT - distance) * (1.0 / VISION_DISTANCE_LIMIT);
            
            if(intensity > stimuli->wall_intensity) {
                stimuli->wall_intensity = intensity;
                stimuli->wall_angle     = (critter_angle + M_PI) * (1.0 / M_PI_2);
            }
        }
    }
    
    return true;
}

void scene_update(scene_t *scene, float delta) {
    critter_t   *critter;
    stimuli_t    stimuli;
    int          idx;
    
    for(idx = 0; idx < SCENE_THINGS; ++idx) {
        thing_update_position(scene->thing[idx], delta, scene->width, scene->height);
    }
    
    idx = 0;
    critter = scene->critter;
    
    while(critter != NULL) {
        critter_update_position(critter, delta, scene->width, scene->height);
        
        critter = critter->next;
        ++idx;
    }
    
    critter = scene->critter;
    
    while(critter != NULL) {
        if( compute_stimuli(&stimuli, critter, scene) ) {
            critter_update_brain(critter, &stimuli);
        }
        
        critter = critter->next;
    }
}

void scene_resize(scene_t *scene, int width, int height) {
    scene->width    = width;
    scene->height   = height;
    
    /* This ensures all coordinates are within bounds. */
    scene_shake(scene);
}

void scene_shake(scene_t *scene) {
    thing_t     *thing;
    critter_t   *critter;
    int          idx;
    
    critter = scene->critter;
    
    while(critter != NULL) {
        critter_set_position(
                critter,
                random_horizontal_position(scene),
                random_vertical_position(scene));
        
        critter = critter->next;
    }
    
    for(idx = 0; idx < SCENE_THINGS; ++idx) {
        thing = scene->thing[idx];
        
        thing_set_position(
                thing,
                random_horizontal_position(scene),
                random_vertical_position(scene));
    }
}

void scene_add_critter(scene_t *scene, critter_t *critter) {
    critter_set_position(
            critter,
            random_horizontal_position(scene),
            random_vertical_position(scene));
    
    critter->next  = scene->critter;
    scene->critter = critter;
}

critter_t *scene_harvest_critter(scene_t *scene) {
    critter_t   *critter;
    
    critter = scene->critter;
    
    if(critter != NULL) {
        scene->critter = critter->next;
    }
    
    return critter;
}

critter_t *scene_first_critter(scene_t *scene) {
    return scene->critter;
}

critter_t *scene_next_critter(scene_t *scene, critter_t *critter) {
    return critter->next;
}
