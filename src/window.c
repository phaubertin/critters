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

#include <SDL/SDL.h>
#include <quatre/macros.h>
#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "window.h"


#define PIXELS_BORDER       10

#define PIXELS_MARGIN       20

#define COLOUR_BORDER       rgb(200, 200, 200)

#define COLOUR_WINDOW_BG    rgb(0, 0, 0)

#define COLOUR_SCENE_BG     rgb(30, 30, 30)

#define MILLISECONDS_PER_SECOND 1000


struct window_t {
    SDL_Surface *screen;
    SDL_Rect     scene_rect;
    SDL_Rect     border_rect;
    SDL_Rect     surface_rect;
    scene_t     *scene;
    int          ticks;
};

window_t *window_new(scene_t *scene) {
    window_t     *window;
    
    window = qrt_new(window_t);
    
    if(window != NULL) {
        window->scene  = scene;
        window->ticks  = SDL_GetTicks();
        
        SDL_WM_SetCaption("Critters", "Critters");
        
        window_resize(window, WINDOW_WIDTH, WINDOW_HEIGHT);
    }
    
    return window;
}

void window_free(window_t *window) {
    free(window);
}

void window_resize(window_t *window, int width, int height) {
    SDL_Surface *screen;
    
    screen = SDL_SetVideoMode(width, height, 32, SDL_SWSURFACE);
    
    if(screen == NULL) {
        fprintf(stderr, "Unable to set video mode (%i x %i): %s\n",
            width,
            height,
            SDL_GetError() );
        exit(EXIT_FAILURE);
    }
    else {
        window->screen          = screen;
        
        window->surface_rect.x  = 0;
        window->surface_rect.y  = 0;
        window->surface_rect.w  = width;
        window->surface_rect.h  = height;
        
        window->border_rect.x   = PIXELS_MARGIN;
        window->border_rect.y   = PIXELS_MARGIN;
        window->border_rect.w   = width  - 2 * PIXELS_MARGIN;
        window->border_rect.h   = height - 2 * PIXELS_MARGIN;
        
        window->scene_rect.x    = PIXELS_MARGIN + PIXELS_BORDER;
        window->scene_rect.y    = PIXELS_MARGIN + PIXELS_BORDER;
        window->scene_rect.w    = width  - 2 * (PIXELS_MARGIN + PIXELS_BORDER);
        window->scene_rect.h    = height - 2 * (PIXELS_MARGIN + PIXELS_BORDER);
        
        scene_resize(window->scene, window->scene_rect.w, window->scene_rect.h);
    }
}

void window_render(window_t *window) {
    SDL_Surface *screen;
    SDL_Rect     inner_border_rect;
    
    screen      = window->screen;
    
    if(SDL_MUSTLOCK(screen)) {
        if (SDL_LockSurface(screen) != 0) {
            return;
        }
    }
    
    /* clean scene */
    inner_border_rect.x = window->border_rect.x + 1;
    inner_border_rect.y = window->border_rect.y + 1;
    inner_border_rect.w = window->border_rect.w - 2;
    inner_border_rect.h = window->border_rect.h - 2;
    
    SDL_FillRect(window->screen, &window->surface_rect, COLOUR_WINDOW_BG);
    SDL_FillRect(window->screen, &window->border_rect,  COLOUR_BORDER);
    SDL_FillRect(window->screen, &inner_border_rect,    COLOUR_WINDOW_BG);
    SDL_FillRect(window->screen, &window->scene_rect,   COLOUR_SCENE_BG);
    
    /* render scene content */
    scene_render(window->scene, screen, window->scene_rect.x, window->scene_rect.y);
    
    if (SDL_MUSTLOCK(screen)) {
        SDL_UnlockSurface(screen);
    }

    SDL_UpdateRect(screen, window->surface_rect.x, window->surface_rect.y, window->surface_rect.w, window->surface_rect.h);
}

void window_update(window_t *window) {
    int ticks_now;
    int ticks_prev;
    float delta;
    
    ticks_prev    = window->ticks;
    ticks_now     = SDL_GetTicks();
    
    window->ticks = ticks_now;
    
    delta = (float)(ticks_now - ticks_prev) / (float)MILLISECONDS_PER_SECOND;
    
    scene_update(window->scene, delta);
}
