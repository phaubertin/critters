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

#ifndef QUATRE_TYPES_H_
#define QUATRE_TYPES_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


#define QRT_SUCCESS     0

#define QRT_ERROR       1


#define QRT_CMP_LESS_THAN   (-1)

#define QRT_CMP_EQUAL       0

#define QRT_CMP_MORE_THAN   1

#define QRT_CMP_NOT_EQUAL   2

#define QRT_CMP_ERROR       3


/* inline functions are only supported starting with C99 */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define QRT_INLINE static inline
#elif defined(__GNUC__)
/* When a standard earlier than C99 is specified, GCC does not inline, but
 * still defines __inline__ which has the same effect. */
#define QRT_INLINE static __inline__
#else
#define QRT_INLINE static
#endif

#endif
