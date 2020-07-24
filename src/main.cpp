#include <iostream>
#include <vector>
#include <stack>
#include <algorithm>
#include <memory>
#include <cmath>
#include <string>

#ifdef __APPLE__
#include <SDL2/SDL.h>
#include <SDL2_ttf/SDL_ttf.h>
#else
#include <SDL.h>
#include <SDL_ttf.h>
#endif

#include "SYSTEM.h"
#include "VARIABLES.h"
#include "FUNCTIONS.h"
#include "UI_CONTROL.h"
#include "CANVAS.h"
#include "BRUSH.h"

  //
 //   MAIN LOOP   ///////////////////////////////////////////////// ///////  //////   /////    ////     ///      //       /
//

int main(int, char*[])
{
#if __APPLE__
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
#endif
    
	// MAIN INIT
	INIT_SDL();
	auto WINDOW = INIT_WINDOW();
	RENDERER = INIT_RENDERER(WINDOW);
	FONTMAP = INIT_FONT();
	SDL_SetTextureBlendMode(FONTMAP, SDL_BLENDMODE_NONE);

	while (!QUIT) // MAIN LOOP
	{
        const char *error = SDL_GetError();
        if (*error) {
            std::cout << "SDL Error: " << error << std::endl;
            SDL_ClearError();
        }
        
        
		const Uint64 fps_start = SDL_GetPerformanceCounter(); // fps counter

		BRUSH_UPDATE = 0; // reset brush update

		float t_win_w = (float)WINDOW_W, t_win_h = (float)WINDOW_H; // temporary window size

		// SET WINDOW X, Y, W, H
		// CLEAR RENDER TARGET
		SDL_GetWindowSize(WINDOW, &WINDOW_W, &WINDOW_H);
		SDL_GetWindowPosition(WINDOW, &WINDOW_X, &WINDOW_Y);
		SDL_SetRenderTarget(RENDERER, nullptr);
		SDL_SetRenderDrawColor(RENDERER, 0, 0, 0, 255);
		SDL_RenderClear(RENDERER);

		// recenter the canvas if the window changes size
		if ((WINDOW_W != t_win_w) || (WINDOW_H != t_win_h))
		{
			CANVAS_X = (((float)WINDOW_W * .5f) - ((t_win_w * .5f) - CANVAS_X));
			CANVAS_Y = (((float)WINDOW_H * .5f) - ((t_win_h * .5f) - CANVAS_Y));
		}

		///////////////////////////////////////////////// ///////  //////   /////    ////     ///      //       /

		SYSTEM_INPUT_UPDATE();

		SYSTEM_BRUSH_UPDATE();
		SYSTEM_LAYER_UPDATE();
		SYSTEM_CANVAS_UPDATE();

		///////////////////////////////////////////////// ///////  //////   /////    ////     ///      //       /


		SDL_SetRenderDrawColor(RENDERER, 255, 255, 255, 255);

		// RENDER
		// smooth lerping animation to make things SLIGHTLY smooth when panning and zooming
		// the '4.0' can be any number, and will be a changeable option in Settings
		float anim_speed = 4.0f;
		CANVAS_X_ANIM = (reach_tween(CANVAS_X_ANIM, floor(CANVAS_X), anim_speed));
		CANVAS_Y_ANIM = (reach_tween(CANVAS_Y_ANIM, floor(CANVAS_Y), anim_speed));
		CANVAS_W_ANIM = (reach_tween(CANVAS_W_ANIM, floor((float)CANVAS_W * CANVAS_ZOOM), anim_speed));
		CANVAS_H_ANIM = (reach_tween(CANVAS_H_ANIM, floor((float)CANVAS_H * CANVAS_ZOOM), anim_speed));
		CELL_W_ANIM = (reach_tween(CELL_W_ANIM, floor((float)CELL_W * CANVAS_ZOOM), anim_speed));
		CELL_H_ANIM = (reach_tween(CELL_H_ANIM, floor((float)CELL_H * CANVAS_ZOOM), anim_speed));
		
		SDL_FRect F_RECT {};

		// transparent background grid
		float bg_w = (ceil(CANVAS_W_ANIM / CELL_W_ANIM) * CELL_W_ANIM);
		float bg_h = (ceil(CANVAS_H_ANIM / CELL_H_ANIM) * CELL_H_ANIM);
		F_RECT = SDL_FRect {CANVAS_X_ANIM, CANVAS_Y_ANIM, bg_w, bg_h};
		SDL_SetTextureBlendMode(BG_GRID_TEXTURE, SDL_BLENDMODE_NONE);
		SDL_RenderCopyF(RENDERER, BG_GRID_TEXTURE, nullptr, &F_RECT);

		// these 2 rects cover the overhang the background grid has beyond the canvas
		SDL_SetRenderDrawColor(RENDERER, 0, 0, 0, 255);
		F_RECT = SDL_FRect {
			std::max(0.0f, CANVAS_X_ANIM), std::max(0.0f,CANVAS_Y_ANIM + (CANVAS_H_ANIM)),
			std::min(float(WINDOW_W),bg_w), CELL_H * CANVAS_ZOOM
		};
		SDL_RenderFillRectF(RENDERER, &F_RECT);
		F_RECT = SDL_FRect {
			std::max(0.0f, CANVAS_X_ANIM + (CANVAS_W_ANIM)), std::max(0.0f, CANVAS_Y_ANIM),
			CELL_W * CANVAS_ZOOM, std::min(float(WINDOW_H),bg_h)
		};
		SDL_RenderFillRectF(RENDERER, &F_RECT);
		
		F_RECT = {CANVAS_X_ANIM, CANVAS_Y_ANIM, CANVAS_W_ANIM, CANVAS_H_ANIM};

		// RENDER THE LAYERS
		for (uint16_t i = 0; i < LAYERS.size(); i++)
		{
			const LAYER_INFO& layer = LAYERS[i];
			SDL_SetTextureBlendMode(layer.texture, (SDL_BlendMode) layer.blendmode);
			SDL_RenderCopyF(RENDERER, layer.texture, nullptr, &F_RECT);
			if (i == CURRENT_LAYER)
			{
				if (BRUSH_UPDATE)
				{
					SDL_SetTextureBlendMode(BRUSH_TEXTURE, SDL_BLENDMODE_BLEND);
					SDL_RenderCopyF(RENDERER, BRUSH_TEXTURE, nullptr, &F_RECT);
				}
			}
		}

		// the grey box around the canvas
		SDL_SetRenderDrawColor(RENDERER, 64, 64, 64, 255);
		F_RECT = { CANVAS_X_ANIM - 2.0f, CANVAS_Y_ANIM - 2.0f, CANVAS_W_ANIM + 4.0f, CANVAS_H_ANIM + 4.0f };
		SDL_RenderDrawRectF(RENDERER, &F_RECT);

		SYSTEM_UIBOX_UPDATE();

		SDL_SetRenderDrawColor(RENDERER, 0, 0, 0, 255);

		// TEST HUE BAR
		//const SDL_Rect temp_rect{10,10,16,360};
		//SDL_RenderCopy(RENDERER, UI_TEXTURE_HUEBAR, nullptr, &temp_rect);
		
		SDL_SetRenderDrawColor(RENDERER, 0, 0, 0, 0);
		SDL_RenderPresent(RENDERER);
        
#if __APPLE__
        SDL_Delay(1);
#endif
		//

		// SET FPS
		static int fps_rate = 0;
        const Uint64 fps_end = SDL_GetPerformanceCounter();
        const static Uint64 fps_freq = SDL_GetPerformanceFrequency();
        const double fps_seconds = (fps_end - fps_start) / static_cast<double>(fps_freq);
        FPS = reach_tween(FPS, 1 / (float)fps_seconds, 100.0);
        if (fps_rate <= 0) {
            std::cout << " FPS: " << (int)floor(FPS) << "       " << '\r';
            fps_rate = 60 * 4;
        } else fps_rate--;
	}

	SDL_Delay(10);

	SYSTEM_SHUTDOWN(WINDOW);

	return 0;
}

  //
 //   END   ///////////////////////////////////////////////// ///////  //////   /////    ////     ///      //       /
//
