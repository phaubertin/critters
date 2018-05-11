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

#define _BSD_SOURCE /* for M_SQRT2 in math.h */
#include <math.h>
#include "boing.h"


bool boing_init(boing_t *boing, float speed, int dir) {
    boing->speed_mult = speed * (1.0 / M_SQRT2);
    
    if((dir & 1) == 0) {    
        boing->go_left = true;
    }
    else {
        boing->go_left = false;
    }
    
    dir >>= 1;
    
    if((dir & 1) == 0) {    
        boing->go_down = true;
    }
    else {
        boing->go_down = false;
    }
    
    return true;
}

void boing_update_thing_position(boing_t *boing, thing_t* thing, float delta, float w, float h) {
    float   delta_s;
    float   x, y;
    
    delta_s = delta * boing->speed_mult;
    
    x = thing_get_x(thing);
    y = thing_get_y(thing);
    
    if(boing->go_left) {
        x += delta_s;
    }
    else {
        x -= delta_s;
    }
    
    if(boing->go_down) {
        y += delta_s;
    }
    else {
        y -= delta_s;
    }
    
    if(x >= w) {
        x = w - 1.0;
        boing->go_left = false;
    }
    else if(x < 0) {
        x = 0;
        boing->go_left = true;
    }
    
    if(y >= h) {
        y = h - 1.0;
        boing->go_down = false;
    }
    else if(y < 0) {
        y = 0;
        boing->go_down = true;
    }
    
    thing_set_position(thing, x, y);
}
