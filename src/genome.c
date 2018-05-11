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

#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "genome.h"
#include "util.h"


static inline float random_weight(void) {
    return 2.0 * GENOME_WEIGHT_AMPLITUDE * ((float)rand() / (float)RAND_MAX - 0.5);
}

static inline uint32_t random_colour(void) {
    return rgb(
        50 + rand() % 200,
        50 + rand() % 200,
        50 + rand() % 200 );
}

genome_t *genome_new(void) {
    genome_t *genome;
    
    genome = memalign(16, sizeof(genome_t));
    
    if(genome != NULL) {
        genome->ref_count   = 1;
        genome->next        = NULL;
    }
    
    return genome;
}

void genome_free(genome_t *genome) {
    if(genome != NULL) {
        genome->ref_count -= 1;
        
        if(genome->ref_count == 0) {
            free(genome);
        }
    }
}

genome_t *genome_clone(genome_t *genome) {
    genome->ref_count += 1;
    
    return genome;
}

void genome_make_random(genome_t *genome) {
    int idx, idy;
    
    genome->colour = random_colour();
        
    for(idy = 0; idy < GENOME_HIDDEN_GENES; ++idy) {
        for(idx = 0; idx < GENOME_HIDDEN_WEIGHTS; ++idx) {
            genome->hidden[idy].chunk[idx].f[0] = random_weight();
            genome->hidden[idy].chunk[idx].f[1] = random_weight();
            genome->hidden[idy].chunk[idx].f[2] = random_weight();
            genome->hidden[idy].chunk[idx].f[3] = random_weight();
        }
    }
    
    for(idx = 0; idx < GENOME_OUTPUT_WEIGHTS; ++idx) {
        genome->output.chunk[idx].f[0] = random_weight();
        genome->output.chunk[idx].f[1] = random_weight();
        genome->output.chunk[idx].f[2] = 0.0;
        genome->output.chunk[idx].f[3] = 0.0;
    }
}

void genome_make_baby(genome_t *genome, const genome_t *mommy, const genome_t *daddy) {
    int       idx, idy;
    int       who;
    int       step;
    
    for(idy = 0; idy < GENOME_HIDDEN_GENES; ++idy) {
        who = rand() % 2;
    
        if(who == 0) {
            for(idx = 0; idx < GENOME_HIDDEN_WEIGHTS; ++idx) {
                genome->hidden[idy].chunk[idx].v = mommy->hidden[idy].chunk[idx].v;
            }
        }
        else {
            for(idx = 0; idx < GENOME_HIDDEN_WEIGHTS; ++idx) {
                genome->hidden[idy].chunk[idx].v = daddy->hidden[idy].chunk[idx].v;
            }
        }
    }
    
    who = rand() % 2;
    
    if(who == 0) {
        for(idx = 0; idx < GENOME_OUTPUT_WEIGHTS; ++idx) {
            genome->output.chunk[idx].v = mommy->output.chunk[idx].v;
        }
    }
    else {
        for(idx = 0; idx < GENOME_OUTPUT_WEIGHTS; ++idx) {
            genome->output.chunk[idx].v = daddy->output.chunk[idx].v;
        }
    }
    
    /* mutation */
    for(step = 0; step < 10; ++step) {
        who = rand();
        
        if(who % 2 != 0) {
            break;
        }
        
        who >>= 2;
        
        if(who % 32 == 0) {
            idx = rand() % (4 * GENOME_OUTPUT_WEIGHTS);
            
            genome->output.f[idx] = random_weight();
        }
        else {
            idy = rand() % GENOME_HIDDEN_GENES;
            idx = rand() % (4 * GENOME_HIDDEN_WEIGHTS);
            
            genome->hidden[idy].f[idx] = random_weight();
        }
    }
    
    /* colour */
    who = rand() % 2;
    
    if(who == 0) {
        genome->colour = mommy->colour;
    }
    else {
        genome->colour = daddy->colour;
    }
}

void genome_dump(const genome_t *genome) {
    char name[32];
    int idx, idy;
    
    printf("\n");
    printf("Hidden layer:\n");
    printf("--------------------------------------------------------------------------\n");
    
    for(idx = 0; idx < GENOME_HIDDEN_GENES; ++idx) {        
        for(idy = 0; idy < 4; ++idy) {
            if(idx < GENOME_GAUSSIAN_GENES) {
                sprintf(name, "[G%u]", 4 * idx + idy);
            }
            else {
                sprintf(name, "[H%u]", 4 * idx + idy);
            }
            
            printf("    %-16s --> %10.5f --> neuron %s\n",   "[1]",                genome->hidden[idx].chunk[0].f[idy], name);
            
            printf("    %-16s --> %10.5f\n",                 "food_intensity",     genome->hidden[idx].chunk[1].f[idy]);
            printf("    %-16s --> %10.5f\n",                 "food_angle",         genome->hidden[idx].chunk[2].f[idy]);
            printf("    %-16s --> %10.5f\n",                 "danger_intensity",   genome->hidden[idx].chunk[3].f[idy]);
            printf("    %-16s --> %10.5f\n",                 "danger_angle",       genome->hidden[idx].chunk[4].f[idy]);
            printf("    %-16s --> %10.5f\n",                 "wall_intensity",     genome->hidden[idx].chunk[5].f[idy]);
            printf("    %-16s --> %10.5f\n",                 "wall_angle",         genome->hidden[idx].chunk[6].f[idy]);
            printf("    %-16s --> %10.5f\n",                 "food_odour",         genome->hidden[idx].chunk[7].f[idy]);
            printf("    %-16s --> %10.5f\n",                 "danger_odour",       genome->hidden[idx].chunk[8].f[idy]);
            
            printf("--------------------------------------------------------------------------\n");
        }
    }
    
    printf("\n");
    printf("Output layer:\n");
    printf("--------------------------------------------------------------------------\n");
    
    for(idy = 0; idy < GENOME_OUTPUT_COUNT; ++idy) {
        printf("    %-16s --> %10.5f --> neuron [Y%u]\n", "[1]", genome->output.chunk[0].f[idy], idy);
        
        for(idx = 1; idx < GENOME_OUTPUT_WEIGHTS; ++idx) {
            if(idx < GENOME_HIDDEN_GAUSSIAN + 1) {
                sprintf(name, "[G%u]", idx - 1);
            }
            else {
                sprintf(name, "[H%u]", idx - 1);
            }
            
            printf("    %-16s --> %10.5f\n", name, genome->output.chunk[idx].f[idy]);
        }
        
        printf("--------------------------------------------------------------------------\n");
    }
}
