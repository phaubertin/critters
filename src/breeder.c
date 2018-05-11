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

#include <quatre/macros.h>
#include <quatre/tree.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "breeder.h"
#include "critter.h"
#include "genome.h"
#include "scene.h"
#include "util.h"


#define CRITTERS_PER_SCENE 5

#define MILLISECONDS_PER_SECOND 1000

typedef struct {
    scene_t     *scene;
    int          status;
    pthread_t    thread;
    critter_t   *critters_in;
    critter_t   *critters_out;
} thread_state_t;

struct breeder_t {
    int              generation;
    qrt_tree_t      *population;
    int              thread_n;
    thread_state_t  *threads;
    pthread_mutex_t  mutex;
    pthread_t        loop_thread;
};

static void tree_finalizer(void *param, void *genome) {
    genome_free(genome);
}

breeder_t *breeder_new(int thread_n) {
    breeder_t        *breeder;
    genome_t         *genome;
    qrt_tree_t       *population;
    thread_state_t   *threads;
    int               idx, idy;
    
    breeder = qrt_new(breeder_t);
    
    if(breeder != NULL) {
        population = qrt_tree_new();
        
        if(population == NULL) {
            free(breeder);
            return NULL;
        }
        
        if(thread_n < 1) {
            thread_n = 1;
        }
        
        threads = qrt_new_array(thread_state_t, thread_n);
        
        if(threads == NULL) {
            qrt_tree_free(population, NULL, NULL);
            free(breeder);
            return NULL;
        }
        
        for(idx = 0; idx < thread_n; ++idx) {
            threads[idx].scene = scene_new();
            
            if(threads[idx].scene == NULL) {
                for(idy = 0; idy < idx; ++idy) {
                    scene_free(threads[idy].scene);
                }
                
                qrt_tree_free(population, NULL, NULL);
                free(threads);
                free(breeder);
            }
        }
        
        breeder->generation = 0;
        breeder->thread_n   = thread_n;
        breeder->threads    = threads;
        breeder->population = population;
        pthread_mutex_init(&breeder->mutex, NULL);
        
        for(idx = 0; idx < BREEDER_POPULATION_SIZE; ++idx) {
            genome = genome_new();
            
            if(genome != NULL) {
                genome_make_random(genome);
                (void)qrt_tree_add_value_duplicate(population, 0.0, genome);
            }
        }
    }
    
    return breeder;
}

void breeder_free(breeder_t *breeder) {
    int idx;
    
    if(breeder != NULL) {
        for(idx = 0; idx < breeder->thread_n; ++idx) {
            scene_free(breeder->threads[idx].scene);
        }
        free(breeder->threads);
        pthread_mutex_destroy(&breeder->mutex);
        qrt_tree_free(breeder->population, tree_finalizer, NULL);
    }
    
    free(breeder);
}

static void simulate_work(thread_state_t *thread) {
    critter_t   *critter;
    scene_t     *scene;
    float        delta;
    int          step;
    int          idx;
    
    scene = thread->scene;
    delta = (float)(BREEDER_TIME_STEP) / (float)MILLISECONDS_PER_SECOND;
    
    thread->critters_out = NULL;

    while(thread->critters_in != NULL) {
        /* add critters to scene */
        for(idx = 0; idx < CRITTERS_PER_SCENE; ++idx) {
            /* take a critter from input list */
            critter             = thread->critters_in;
            thread->critters_in = critter->next;
            
            scene_add_critter(scene, critter);
            
            if(thread->critters_in == NULL) {
                break;
            }
        }
        
        /* simulate scene */
        for(step = 0; step < BREEDER_SIM_STEPS; ++step) {
            scene_update(scene, delta);
        }
        
        /* harvest time */
        critter = scene_harvest_critter(scene);
        
        while(critter != NULL) {
            /* add critter to output list */
            critter->next        = thread->critters_out;
            thread->critters_out = critter;
            
            critter = scene_harvest_critter(scene);
        }
    }
}

static void *simulate_thread(void *param) {
    thread_state_t  *thread;
    
    thread = (thread_state_t *)param;
    simulate_work(thread);
    
    return NULL;
}

static void simulate_in_thread(breeder_t *breeder, int thread_index) {
    thread_state_t  *thread;
    pthread_attr_t   attr;
    
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    
    thread = &breeder->threads[thread_index];
    
    thread->status = pthread_create(&thread->thread, &attr, simulate_thread, thread);
    
    if(thread->status != 0) {
        /* if thread creation failed, let's do the job in this thread instead */
        simulate_work(thread);
    }
}

int breeder_lock(breeder_t *breeder) {
    return pthread_mutex_lock(&breeder->mutex);
}

int breeder_unlock(breeder_t *breeder) {
    return pthread_mutex_unlock(&breeder->mutex);
}


bool breeder_next_generation(breeder_t *breeder) {
    genome_t         *gene_pool[BREEDER_POOL_SIZE];
    genome_t        **gene_ptr;
    genome_t         *genome;
    critter_t        *critter;
    thread_state_t   *thread;    
    int               idx, idy;
    int               thread_idx;
    float             fitness;
    
    gene_ptr = &gene_pool[0];
    
    /* discard the worst */
    for(idx = 0; idx < BREEDER_WORST_DISCARD; ++idx) {
        genome_free( qrt_tree_pop_min(breeder->population) );
    }
    
    /* build gene pool */
    for(idx = 0; idx < BREEDER_BEST_KEEP; ++idx) {
        genome = qrt_tree_pop_max(breeder->population);
        
        for(idy = 0; idy < BREEDER_BEST_PRIORITY; ++idy) {
            *(gene_ptr++) = genome_clone(genome);
        }
        
        genome_free(genome);
    }
    
    for(idx = 0; idx < BREEDER_RAND_KEEP; ++idx) {
        genome = qrt_tree_pop_random(breeder->population);
        
        *(gene_ptr++) = genome;
    }
    
    for(idx = 0; idx < BREEDER_RAND_NEW; ++idx) {
        genome = genome_new();
        
        if(genome == NULL) {
            return false;
        }
        
        genome_make_random(genome);
        *(gene_ptr++) = genome;
    }
    
    /* simulate genomes */
    for(thread_idx = 0; thread_idx < breeder->thread_n; ++thread_idx) {
        thread = &breeder->threads[thread_idx];
        
        /* This will prevent joining threads which we do not actually create. */
        thread->status = EAGAIN;
        
        thread->critters_in = NULL;
        
        for(idx = 0; idx < BREEDER_POPULATION_SIZE / breeder->thread_n; ++idx) {
            genome = genome_new();
            
            if(genome != NULL) {
                genome_make_baby(genome, gene_pool[rand() % BREEDER_POOL_SIZE], gene_pool[rand() % BREEDER_POOL_SIZE]);
                
                critter = critter_new(genome);
                
                genome_free(genome);
                
                if(critter != NULL) {
                    /* add to list */
                    critter->next       = thread->critters_in;
                    thread->critters_in = critter;
                }
            }
        }
        
        /* We simulate the first scene in this thread last, because we want
         * to start the other threads first. */
        if(thread_idx > 0) {
            simulate_in_thread(breeder, thread_idx);
        }
    }
    
    /* simulate first scene in this thread */
    simulate_work(&breeder->threads[0]);
    
    /* critter harvest */
    breeder_lock(breeder);
    
    qrt_tree_clear(breeder->population, tree_finalizer, NULL);
    
    for(thread_idx = 0; thread_idx < breeder->thread_n; ++thread_idx) {
        thread = &breeder->threads[thread_idx];
        
        /* wait for work to complete */
        if(thread->status == 0) {
            (void)pthread_join(thread->thread, NULL);
        }
        
        while(thread->critters_out != NULL) {
            critter              = thread->critters_out;
            thread->critters_out = critter->next;
            
            genome  = genome_clone(critter->genome);
            fitness = BREEDER_FOOD_COST * critter->food_count + BREEDER_DANGER_COST * critter->danger_count;
            
            critter_free(critter);
            
            qrt_tree_add_value_duplicate(breeder->population, fitness, genome);
        }
    }
    
    breeder_unlock(breeder);
    
    /* free gene pool */
    for(idx = 0; idx < BREEDER_POOL_SIZE; ++idx) {
        genome_free(gene_pool[idx]);
    }
    
    return true;
}

float breeder_fitness_n(breeder_t *breeder, int n) {
    breeder_iterator_t   *iter;
    int                   count;
    float                fitness;
    genome_t             *genome;
    
    fitness = 0.0;
    iter    = breeder_iterator_new(breeder);
    
    if(iter == NULL) {
        return fitness;
    }
    
    genome = breeder_iterator_current(iter);
    count  = 0;
    
    while(genome != NULL && count < n) {
        fitness += breeder_iterator_fitness(iter);
        genome   = breeder_iterator_next(iter);
        ++count;
    }

    return fitness / (float)count;
}

float breeder_fitness(breeder_t *breeder) {
    return breeder_fitness_n(breeder, BREEDER_BEST_KEEP);
}

static void *loop_thread(void *param) {
    struct timeval       generation_start;
    struct timeval       ticks;
    
    breeder_t *breeder  = param;
    
    while(1) {
        /* compute a new generation */
        gettimeofday(&generation_start, NULL);
        breeder_next_generation(breeder);
        gettimeofday(&ticks, NULL);
        
        if(breeder->generation % 50 == 0) {
            breeder_lock(breeder);
            
            printf(
                    "generation: %6u duration (ms): %4u fitness: %10.3f\n",
                    breeder->generation,
                    interval_milliseconds(&generation_start, &ticks),
                    breeder_fitness(breeder));
                    
            breeder_unlock(breeder);
        }
        
        ++breeder->generation;
    }
    
    return NULL;
}

int breeder_start_loop(breeder_t *breeder) {
    pthread_attr_t   attr;
    
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    
    return pthread_create(&breeder->loop_thread, &attr, loop_thread, breeder);
}


struct breeder_iterator_t {
    qrt_tree_iterator_t *qrt_tree_iter;
};


breeder_iterator_t *breeder_iterator_new(breeder_t *breeder) {
    breeder_iterator_t *iter;
    
    iter = qrt_new(breeder_iterator_t);
    
    if(iter != NULL) {
        iter->qrt_tree_iter = qrt_tree_iterator_new_from_end(breeder->population);
        
        if(iter->qrt_tree_iter == NULL) {
            free(iter);
            return NULL;
        }
    }
    
    return iter;
}

void breeder_iterator_free(breeder_iterator_t *iter) {
    if(iter != NULL) {
        qrt_tree_iterator_free(iter->qrt_tree_iter);
    }
    
    free(iter);
}

genome_t *breeder_iterator_current(breeder_iterator_t *iter) {    
    return qrt_tree_iterator_value(iter->qrt_tree_iter);
}

genome_t *breeder_iterator_next(breeder_iterator_t *iter) {
    qrt_tree_node_t *node;
    
    node = qrt_tree_iterator_prev(iter->qrt_tree_iter);
    
    return qrt_tree_node_value(node);
}

float breeder_iterator_fitness(breeder_iterator_t *iter) {
    return qrt_tree_iterator_key(iter->qrt_tree_iter);
}
