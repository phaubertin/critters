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

#ifndef _CRITTERS_GENOME_H_
#define _CRITTERS_GENOME_H_

#include <stdint.h>

/* Number of neurons with a sigmoid-like activation function in the hidden layer.
 * Must be a multiple of four. Can be zero. */
#define GENOME_HIDDEN_SIGMOID   4

/* Number of neurons with a gaussian-like activation function in the hidden layer.
 * Must be a multiple of four. Can be zero. */
#define GENOME_HIDDEN_GAUSSIAN  4

/* Number of neurons with a rectifier activation function (ReLU) in the hidden layer.
 * Must be a multiple of four. Can be zero. */
#define GENOME_HIDDEN_RELU      4

/* All weights are between plus or minus this value. */
#define GENOME_WEIGHT_AMPLITUDE 20.0


#define GENOME_HIDDEN_COUNT     (GENOME_HIDDEN_SIGMOID + GENOME_HIDDEN_GAUSSIAN + GENOME_HIDDEN_RELU)

#define GENOME_INPUT_COUNT       8

#define GENOME_OUTPUT_COUNT      2


#define GENOME_HIDDEN_GENES     (GENOME_HIDDEN_COUNT / 4)

#define GENOME_SIGMOID_GENES    (GENOME_HIDDEN_SIGMOID / 4)

#define GENOME_GAUSSIAN_GENES   (GENOME_HIDDEN_GAUSSIAN / 4)

#define GENOME_RELU_GENES       (GENOME_HIDDEN_RELU / 4)

#define GENOME_HIDDEN_WEIGHTS   (GENOME_INPUT_COUNT  + 1)   /* + 1 for bias */

#define GENOME_OUTPUT_WEIGHTS   (GENOME_HIDDEN_COUNT + 1)


typedef struct genome_t genome_t;

/* A vector of four 32-bit floating-point values */
typedef float genome_f4_t __attribute__ ((vector_size (16)));


typedef union {
    genome_f4_t v;
    float       f[4];
} gene_chunk_t __attribute__ ((aligned (16)));

typedef union gene_hidden_t {
    gene_chunk_t    chunk[GENOME_HIDDEN_WEIGHTS];
    float           f[4 * GENOME_HIDDEN_WEIGHTS];
} gene_hidden_t;

typedef union  {
    gene_chunk_t    chunk[GENOME_OUTPUT_WEIGHTS];
    float           f[4 * GENOME_OUTPUT_WEIGHTS];
} gene_output_t;

struct genome_t {
    gene_hidden_t    hidden[GENOME_HIDDEN_GENES];
    gene_output_t    output;
    uint32_t         colour;
    int              ref_count;
    genome_t        *next;
} __attribute__ ((aligned (16)));

genome_t *genome_new(void);

void genome_free(genome_t *genome);

genome_t *genome_clone(genome_t *genome);

void genome_make_random(genome_t *genome);

void genome_make_baby(genome_t *genome, const genome_t *mommy, const genome_t *daddy);

void genome_dump(const genome_t *genome);

#endif
