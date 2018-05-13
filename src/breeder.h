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

#ifndef _CRITTERS_BREEDER_H_
#define _CRITTERS_BREEDER_H_

#include <stdbool.h>
#include "genome.h"

/* Selection procedure: First, the genomes with the lowest fitness score are
 * discarded. Then, a pool of genomes is created by picking the genomes with
 * top fitness score, then picking a few other genomes randomly, and finally
 * adding a few randomly-generated novel genomes. Genomes with top fitness score
 * are added multiple times to increase the probability they get chosen. Once
 * the pool has been created, pairs of genomes are selected randomly with
 * uniform distribution within that pool for recombination. */

/* Number of genomes in a generation */
#define BREEDER_POPULATION_SIZE     200

/* Number of selected genomes with top fitness score */
#define BREEDER_BEST_KEEP             9

/* Number of randomly-selected genomes */
#define BREEDER_RAND_KEEP            48

/* Number of novel randomly-generated genomes */
#define BREEDER_RAND_NEW              6

/* Weight of top fitness score genomes (i.e. how many time each is added to the pool) */
#define BREEDER_BEST_PRIORITY         4

#define BREEDER_POOL_SIZE           (BREEDER_BEST_KEEP * BREEDER_BEST_PRIORITY + BREEDER_RAND_KEEP + BREEDER_RAND_NEW)

/* Number of genomes with the lowest fitness score that are discarded */
#define BREEDER_WORST_DISCARD        50

#define BREEDER_SIM_TIME             40 /* in seconds */

#define BREEDER_TIME_STEP            200 /* in milliseconds */

#define BREEDER_SIM_STEPS           (BREEDER_SIM_TIME * 1000 / BREEDER_TIME_STEP)

/* Fitness score: number of points gained each time food is captured */
#define BREEDER_FOOD_COST           1.0

/* Fitness score: number of points gained (negative for loss) each time the critter is captured */
#define BREEDER_DANGER_COST         -50.0


typedef struct breeder_t breeder_t;

typedef struct breeder_iterator_t breeder_iterator_t;


breeder_t *breeder_new(int thread_n);

void breeder_free(breeder_t *breeder);

int breeder_lock(breeder_t *breeder);

int breeder_unlock(breeder_t *breeder);

bool breeder_next_generation(breeder_t *breeder);

float breeder_fitness(breeder_t *breeder);

float breeder_fitness_n(breeder_t *breeder, int n);

int breeder_start_loop(breeder_t *breeder);

void breeder_dump_population(breeder_t *breeder);


breeder_iterator_t *breeder_iterator_new(breeder_t *breeder);

void breeder_iterator_free(breeder_iterator_t *iter);

genome_t *breeder_iterator_current(breeder_iterator_t *iter);

genome_t *breeder_iterator_next(breeder_iterator_t *iter);

float breeder_iterator_fitness(breeder_iterator_t *iter);


#endif
