/*
 * Copyright (c) 2018, 2019 Amine Ben Hassouna <amine.benhassouna@gmail.com>
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any
 * person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the
 * Software without restriction, including without
 * limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software
 * is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice
 * shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF
 * ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
 * SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 * IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#include <stdio.h>
#include <stdbool.h>

#define SDL_DISABLE_IMMINTRIN_H
#include <SDL2/SDL.h>
#link "SDL2"
//#include "../lib/SDL2_macos/SDL.h"
// needed for SDL.h to compile on Ubuntu 22.10 for some reason
#library "/usr/lib/x86_64-linux-gnu/pulseaudio/"

#product "bin/test_sdl2"

// Define MAX and MIN macros
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

// Define screen dimensions
#define SCREEN_WIDTH    1280
#define SCREEN_HEIGHT   720

int main(int argc, char* argv[])
{

    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not be initialized!\n"
               "SDL_Error: %s\n", SDL_GetError());
        return 0;
    }

    if(!SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0"))
        printf("Warning: SDL can not disable compositor bypass\n");

    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");
    SDL_Window *window = SDL_CreateWindow("Basic C SDL project",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT,
                                          SDL_WINDOW_SHOWN);
    if(!window)
    {
        printf("Window could not be created!\n"
               "SDL_Error: %s\n", SDL_GetError());
        return 0;
    }

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if(!renderer)
	{
		printf("Renderer could not be created!\n"
			   "SDL_Error: %s\n", SDL_GetError());
        return 0;
	}

	SDL_Rect squareRect;

	squareRect.w = MIN(SCREEN_WIDTH, SCREEN_HEIGHT) / 2;
	squareRect.h = MIN(SCREEN_WIDTH, SCREEN_HEIGHT) / 2;
	squareRect.x = SCREEN_WIDTH / 2 - squareRect.w / 2;
	squareRect.y = SCREEN_HEIGHT / 2 - squareRect.h / 2;

	bool quit = false;
	f32 tick = 0;

	while(!quit)
	{
		tick += 1.0/60.0;
		SDL_Event e;

		while (SDL_PollEvent(&e))
		{
			if(e.type == SDL_QUIT)
				quit = true;
		}

		SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(renderer);
		SDL_SetRenderDrawColor(renderer, 0xFF, 0x40+0x40*sin(1.21*tick), 0x80-0x80*sin(2.24*tick), 0xFF);

		squareRect.w = 200 + 100*sin(6.28*tick);
		squareRect.x = SCREEN_WIDTH / 2 - squareRect.w / 2;
		SDL_RenderFillRect(renderer, &squareRect);

		SDL_RenderPresent(renderer);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
