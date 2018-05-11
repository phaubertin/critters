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

#include <SDL/SDL.h>
#include "thing.h"

bool thing_init(
        thing_t                 *thing,
        int                      kind,
        float                   x,
        float                   y,
        int                      bound,
        uint32_t                 colour,
        void                    *this_ptr,
        thing_render_func_t      func,
        thing_pre_render_func_t  pre_func,
        thing_update_func_t      update_func,
        thing_free_func_t        free_func) {
            
    thing->kind         = kind;
    thing->x            = x;
    thing->y            = y;
    thing->bound        = bound;
    thing->colour       = colour;
    thing->this_ptr     = this_ptr;
    thing->render_func  = func;
    thing->pre_func     = pre_func;
    thing->update_func  = update_func;
    thing->free_func    = free_func;
    
    return true;
}

void thing_render(thing_t *thing, SDL_Surface *screen, int v_offset, int h_offset) {
    uint32_t    *pixel_array;
    uint32_t    *pixel_ptr;
    uint32_t    *line_start;
    int          v_corner, h_corner;
    int          x, y;
    
    /* prepare state for rendering */
    if(thing->pre_func != NULL) {
        thing->pre_func(thing->this_ptr);
    }
    
    pixel_array = (uint32_t *)screen->pixels;    
    v_corner    = v_offset + (int)thing->y - thing->bound;
    h_corner    = h_offset + (int)thing->x - thing->bound;
    
    line_start  = &pixel_array[v_corner * screen->pitch / sizeof(uint32_t)];
    
    for(y = -thing->bound; y < thing->bound; ++y) {
        pixel_ptr = line_start + h_corner;
        
        for(x = -thing->bound; x < thing->bound; ++x) {
            if( thing->render_func(thing->this_ptr, x, y) ) {
                *pixel_ptr = thing->colour;
            }
            ++pixel_ptr;
        }
        
        line_start += screen->pitch / sizeof(uint32_t);
    }
}
