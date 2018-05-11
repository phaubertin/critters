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

#include <sys/time.h>
#include <SDL/SDL.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "breeder.h"
#include "critter.h"
#include "genome.h"
#include "scene.h"
#include "window.h"
#include "util.h"

#ifdef _SC_NPROCESSORS_ONLN
#define NUMBER_OF_CORES   (sysconf( _SC_NPROCESSORS_ONLN ))
#else
#define NUMBER_OF_CORES   0
#endif

static void graphics_initialize(void) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    atexit(SDL_Quit);
}

int main(void) {
    SDL_Event            event;
    breeder_t           *breeder;
    critter_t           *scene_critter;
    breeder_iterator_t  *iter;
    genome_t            *genome;
    scene_t             *scene;
    window_t            *window;
    struct timeval       ticks;
    struct timeval       round_start;
    int                  idx;
    int                  count;
    
    srand( time(NULL) );
    
    graphics_initialize();
    
    scene = scene_new();
    
    if(scene == NULL) {
        fprintf(stderr, "Cannot create scene\n");
        return EXIT_FAILURE;
    }
    
    for(idx = 0; idx < 5; ++idx) {
        genome = genome_new();
        genome_make_random(genome);
        
        if(genome != NULL) {
            scene_critter = critter_new(genome);
            
            if(scene_critter != NULL) {
                scene_add_critter(scene, scene_critter);
            }
            
            genome_free(genome);
        }
    }
    
    window = window_new(scene);
    
    if(window == NULL) {
        fprintf(stderr, "Cannot create window\n");
        scene_free(scene);
        return EXIT_FAILURE;
    }
    
    breeder = breeder_new(NUMBER_OF_CORES - 1);
    
    if(breeder == NULL) {
        fprintf(stderr, "Cannot create breeder\n");
        window_free(window);
        scene_free(scene);
        return EXIT_FAILURE;
    }
    
    breeder_start_loop(breeder);
    
    gettimeofday(&round_start, NULL);
    
    while(1) {
        /* process SDL events (keyboard, etc.) */
        while( SDL_PollEvent(&event) ) {
            switch (event.type) {
            case SDL_QUIT:
                exit(EXIT_SUCCESS);
            case SDL_KEYUP:
                switch(event.key.keysym.sym) {
                
                case SDLK_ESCAPE:
                case SDLK_q:
                    exit(EXIT_SUCCESS);
                
                case SDLK_d:
                    breeder_lock(breeder);
                    scene_critter = scene_first_critter(scene);
                    genome_dump(scene_critter->genome);
                    breeder_unlock(breeder);
                    break;
                    
                case SDLK_r:
                    scene_shake(scene);
                    break;
                    
                default:
                    break;
                }
            }
        }
        
        /* update and render window content */
        window_update(window);
        window_render(window);
        
        /* update view every 20 seconds */
        gettimeofday(&ticks, NULL);
        
        if(interval_milliseconds(&round_start, &ticks) >= 20 * 1000) {
            breeder_lock(breeder);
            
            gettimeofday(&round_start, NULL);
            
            scene_critter = scene_first_critter(scene);
            iter = breeder_iterator_new(breeder);
            
            if(iter == NULL) {
                break;
            }
            
            genome = breeder_iterator_current(iter);
            count  = 0;
            
            while(genome != NULL && scene_critter != NULL) {
                critter_genome_transplant(scene_critter, genome);
                
                scene_critter   = scene_next_critter(scene, scene_critter);
                genome          = breeder_iterator_next(iter);
                ++count;
            }
            
            breeder_iterator_free(iter);
            
            printf("update fitness: %10.3f\n", breeder_fitness_n(breeder, count));
            
            breeder_unlock(breeder);
        }
    }
    
    window_free(window);
    scene_free(scene);
    breeder_free(breeder);
    
    return EXIT_SUCCESS;
}
