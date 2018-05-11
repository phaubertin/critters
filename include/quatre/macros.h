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
 
#ifndef QUATRE_MACROS_H_
#define QUATRE_MACROS_H_

#include <quatre/types.h>

/* Let's not include stdlib.h and all its declarations for just these
 * three. The following are specified in ISO 9899:1999 §§ 7.20.3.2-4 */
void *malloc(size_t size);
void free(void *ptr);
void *realloc(void *ptr, size_t size);

#define qrt_new(t)                  ( (t *)malloc(sizeof(t)) )

#define qrt_new_array(t, n)         ( (t *)malloc((n) * sizeof(t)) )

#define qrt_realloc_array(p, t, n)  ( (t *)realloc((void *)(p), (n) * sizeof(t)) )

QRT_INLINE int qrt_max(int a, int b) {
    if(b > a) {
        return b;
    }
    
    return a;
}

QRT_INLINE int qrt_min(int a, int b) {
    if(b < a) {
        return b;
    }
    
    return a;
}

#endif
