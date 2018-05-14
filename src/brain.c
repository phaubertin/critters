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

#include <math.h>
#include <xmmintrin.h>
#include "brain.h"


/* This function and the next few ones use compiler intrinsic functions for SSE2
 * instructions that act on vectors of four floating point values. The return
 * value and all arguments of these functions are vectors of four floating point
 * values. */
static inline genome_f4_t mux(genome_f4_t cond, genome_f4_t vthen, genome_f4_t velse) {
    /* mux(p, a, b) = p ? a : b
     *              = p & a | ~p & b */
    return _mm_or_ps(
                _mm_and_ps(cond, vthen),
                _mm_andnot_ps(cond, velse) );
}

static inline genome_f4_t mux_if_less(genome_f4_t op1, genome_f4_t op2, genome_f4_t vthen, genome_f4_t velse) {
    genome_f4_t cond;
    
    cond = _mm_cmplt_ps(op1, op2);
    
    return mux(cond, vthen, velse);
}

static inline genome_f4_t mux_if_between(genome_f4_t op, genome_f4_t low, genome_f4_t high, genome_f4_t vthen, genome_f4_t velse) {
    genome_f4_t cond;
    
    cond = _mm_and_ps(
        _mm_cmplt_ps(low, op),
        _mm_cmplt_ps(op,  high) );

    return mux(cond, vthen, velse);
}

/* Rectifier activation function (ReLU) */
static inline genome_f4_t relu(genome_f4_t t) {
    return mux_if_less(t, _mm_set1_ps(0.0), _mm_set1_ps(0.0), t);
}

/* Piecewise polynomial approximation of a sigmoid-like curve
 * 
 * The value of the function is zero for arguments under -5 and one for 
 * arguments over 5. Between -5 and 5, the value of the function is the value of
 * a degree 3 polynomial with the following characteristics:
 * 
 *  - The polynomial has value 0 at -5 and 1 at 5 so as not to have
 *    discontinuities.
 *  - The first derivative is zero at -5 and 5 to prevent discontinuities of
 *    that derivative.
 * 
 *  */
static inline genome_f4_t sigmoid(genome_f4_t t) {
    genome_f4_t poly;
    genome_f4_t mux1;
    genome_f4_t mux2;
    
    /* compute polynomial:
     *      poly(t) =  -0.002 * t^3 + 0.15 * t + 0.5
     *              = (-0.002 * t^2 + 0.15) * t + 0.5 */
    poly = (_mm_set1_ps(-0.002) * t*t + _mm_set1_ps(0.15)) * t + _mm_set1_ps(0.5);
    
    /* select poly if -5 < t < 5, 0 if t < -5, 1 otherwise (t > 5) */
    mux1 = mux_if_less(t, _mm_set1_ps(-5.0), _mm_set1_ps(0.0), poly);
    mux2 = mux_if_less(t, _mm_set1_ps( 5.0), mux1, _mm_set1_ps(1.0));
            
    return mux2;
}

/* Piecewise polynomial approximation of a gaussian-like curve
 * 
 * The value of the function is zero for arguments under -5 and over 5. Between
 * -5 and 0 the value of the function is the value of a degree 3 polynomial,
 * whereas between 0 and 5, it is the value of that same polynomial computed on
 * the inverse of the argument (i.e. p(-x)). The coefficients of the polynomial
 * have been computed with the following constraints in mind:
 * 
 *  - The polynomial has value 0 at -5 and 1 at 0 so as not to have
 *    discontinuities.
 *  - The first derivative is zero at -5 and 0 to prevent discontinuities of
 *    that derivative. 

 *  */
static inline genome_f4_t gaussian(genome_f4_t t) {
    genome_f4_t a;
    genome_f4_t poly;
    
    /* select coefficient: a = -0.016 if t < 0, 0.016 otherwise */
    a = mux_if_less(t, _mm_set1_ps(0.0), _mm_set1_ps(-0.016), _mm_set1_ps(0.016));
    
   /* compute polynomial:
     *      poly(t) =  +/-0.016 * t^3 - 0.12 * t^2 + 1.0
     *              =         a * t^3 - 0.12 * t^2 + 1.0
     *              =        (a * t - 0.12) * t^2 + 1.0 */
    poly = (a * t - _mm_set1_ps(0.12)) * t*t + _mm_set1_ps(1.0);
    
    /* select poly if -5 < t < 5, 0 otherwise */
    return mux_if_between(t, _mm_set1_ps(-5.0), _mm_set1_ps(5.0), poly, _mm_set1_ps(0.0));
}

bool brain_control_init(brain_control_t *control) {
    control->left_speed  = 0.0;
    control->right_speed = 0.0;
    
    return true;
}

void brain_control_compute(brain_control_t * restrict control, const genome_t * restrict genome, const stimuli_t * restrict stimuli) {
    const gene_chunk_t *weight;
    const float        *hidden;
    gene_chunk_t        hidden_layer[GENOME_HIDDEN_GENES];
    gene_chunk_t        input[GENOME_INPUT_COUNT];
    gene_chunk_t        acc;
    int                 idx, idy;
    
    input[0].v = _mm_load1_ps(&stimuli->food_intensity);
    input[1].v = _mm_load1_ps(&stimuli->food_angle);
    input[2].v = _mm_load1_ps(&stimuli->danger_intensity);
    input[3].v = _mm_load1_ps(&stimuli->danger_angle);
    input[4].v = _mm_load1_ps(&stimuli->wall_intensity);
    input[5].v = _mm_load1_ps(&stimuli->wall_angle);
    input[6].v = _mm_load1_ps(&stimuli->food_odour);
    input[7].v = _mm_load1_ps(&stimuli->danger_odour);

    for(idy = 0; idy < GENOME_HIDDEN_GENES; ++idy) {
        /* chunk 0 is bias, weight * 1 = weight */
        acc.v  =  genome->hidden[idy].chunk[0].v;
        weight = &genome->hidden[idy].chunk[1];
        
        for(idx = 0; idx < GENOME_INPUT_COUNT; ++idx) {
            acc.v += weight[idx].v * input[idx].v;
        }
        
        if(idy < GENOME_SIGMOID_GENES) {
            hidden_layer[idy].v = gaussian(acc.v);
        }
        else if(idy < GENOME_SIGMOID_GENES + GENOME_GAUSSIAN_GENES) {
            hidden_layer[idy].v = sigmoid(acc.v);
        }
        else {
            hidden_layer[idy].v = relu(acc.v);
        }
    }
    
    /* chunk 0 is bias */
    acc.v  =  genome->output.chunk[0].v;
    weight = &genome->output.chunk[1];
    hidden = (float *)hidden_layer;
    
    for(idx = 0; idx < GENOME_HIDDEN_COUNT; ++idx) {
        acc.v += weight[idx].v * _mm_load1_ps(&hidden[idx]);
    }
    
    acc.v = sigmoid(acc.v);
    
    control->left_speed  = acc.f[0];
    control->right_speed = acc.f[1];
}
