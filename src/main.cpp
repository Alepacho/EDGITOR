#include <iostream>
#include <vector>
#include <stack>
#include <algorithm>
#include <memory>
#include <cmath>
#include <string>

#ifdef __APPLE__
#include <SDL2/SDL.h>
#include <SDL2_image/SDL_image.h>
#include <SDL2_ttf/SDL_ttf.h>
#else
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#endif

#include "SYSTEM.h"
#include "VARIABLES.h"
#include "FUNCTIONS.h"
#include "UI_CONTROL.h"
#include "CANVAS.h"
#include "BRUSH.h"
#include "UNDO.h"
#include <filesystem>
#include <fstream>
#include "CUBE_VIEW.h"
namespace fs = std::filesystem;

  //
 //   MAIN LOOP   ///////////////////////////////////////////////// ///////  //////   /////    ////     ///      //       /
//

int main(int, char*[])
{
	//fs::create_directories("sandbox/a/b");
	//std::ofstream("sandbox/file1.txt");
	//std::ofstream("sandbox/file2.txt");
	//fs::temp_directory_path();
	//fs::current_path();
	/*for (auto& p : fs::recursive_directory_iterator(fs::current_path()))
		std::cout << p.path() << " : " << ((float)p.file_size())/1000.0f << "KB" << '\n';*/

	// MAIN INIT

	//GIF_Image* gif = GIF_LoadImage("my_animation.gif");

	if (!INIT_SDL())
	{
		PRINT("COULD NOT START");
	}
	auto WINDOW = INIT_WINDOW();
	RENDERER = INIT_RENDERER(WINDOW);
	FONTMAP = INIT_FONT();
	SDL_SetTextureBlendMode(FONTMAP, SDL_BLENDMODE_NONE);

	CUBE_VIEW_INIT();
	CUBE_VIEW_UPDATE();

	while (!QUIT) // MAIN LOOP
	{

        const char *error = SDL_GetError();
        if (*error) {
            std::cout << "SDL Error: " << error << std::endl;
            SDL_ClearError();
        }
        
		const Uint64 fps_start = SDL_GetPerformanceCounter(); // fps counter

		CUBE_VIEW_RENDER();

		BRUSH_UPDATE = false; // reset brush update

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

			for (int i = 0; i < UIBOXES.size(); i++)
			{
				if (UIBOXES[i]->snap) continue;

				// MOVE UIBOX IN RATIO TO WHERE IT WAS WITH THE NEW WINDOW SIZE
				UIBOXES[i]->x = (int)floor(((((float)UIBOXES[i]->x + ((float)(UIBOXES[i]->w) / 2)) / (float)t_win_w) * (float)WINDOW_W) - ((float)(UIBOXES[i]->w) / 2));
				UIBOXES[i]->y = (int)floor(((((float)UIBOXES[i]->y + ((float)(UIBOXES[i]->h) / 2)) / (float)t_win_h) * (float)WINDOW_H) - ((float)(UIBOXES[i]->h) / 2));
			}
		}

		if (CANVAS_PREVW != CANVAS_W || CANVAS_PREVH != CANVAS_H)
		{
			refresh_canvas();
		}

		///////////////////////////////////////////////// ///////  //////   /////    ////     ///      //       /

		SYSTEM_INPUT_UPDATE();

		if (!(MOUSE_X > 0 && MOUSE_X < WINDOW_W - 1 && MOUSE_Y > 0 && MOUSE_Y < WINDOW_H - 1))
		{
			MOUSEBUTTON_LEFT = false;
			MOUSEBUTTON_PRESSED_LEFT = false;
			MOUSEBUTTON_MIDDLE = false;
			MOUSEBUTTON_PRESSED_MIDDLE = false;
			MOUSEBUTTON_RIGHT = false;
			MOUSEBUTTON_PRESSED_RIGHT = false;
			MOUSEWHEEL_X = 0;
			MOUSEWHEEL_Y = 0;

			KEYBOARD_CTRL = false;
			KEYBOARD_PRESSED_CTRL = false;
			KEYBOARD_SHIFT = false;
			KEYBOARD_PRESSED_SHIFT = false;
			KEYBOARD_ALT = false;
			KEYBOARD_PRESSED_ALT = false;

			KEYBOARD_SPACE = false;
			KEYBOARD_PRESSED_SPACE = false;
			KEYBOARD_ESC = false;
			KEYBOARD_PRESSED_ESC = false;
		}
		else
		{
			SYSTEM_BRUSH_UPDATE();
			SYSTEM_LAYER_UPDATE();
			SYSTEM_CANVAS_UPDATE();
		}

		///////////////////////////////////////////////// ///////  //////   /////    ////     ///      //       /


		SDL_SetRenderDrawColor(RENDERER, 255, 255, 255, 255);

		// RENDER
		// smooth lerping animation to make things SLIGHTLY smooth when panning and zooming
		// the '4.0' can be any number, and will be a changeable option in Settings
		/*  WAS HAVING ISSUES WITH FLOATS AND DRAWING
		float anim_speed = 2.0f;
		CANVAS_X_ANIM = (reach_tween(CANVAS_X_ANIM, floorf(CANVAS_X), anim_speed));
		CANVAS_Y_ANIM = (reach_tween(CANVAS_Y_ANIM, floorf(CANVAS_Y), anim_speed));
		CANVAS_W_ANIM = (reach_tween(CANVAS_W_ANIM, floorf((float)CANVAS_W * CANVAS_ZOOM), anim_speed));
		CANVAS_H_ANIM = (reach_tween(CANVAS_H_ANIM, floorf((float)CANVAS_H * CANVAS_ZOOM), anim_speed));
		CELL_W_ANIM = (reach_tween(CELL_W_ANIM, floorf((float)CELL_W * CANVAS_ZOOM), anim_speed));
		CELL_H_ANIM = (reach_tween(CELL_H_ANIM, floorf((float)CELL_H * CANVAS_ZOOM), anim_speed));*/

		CANVAS_X_ANIM = floorf(CANVAS_X);
		CANVAS_Y_ANIM = floorf(CANVAS_Y);
		CANVAS_W_ANIM = floorf((float)CANVAS_W * CANVAS_ZOOM);
		CANVAS_H_ANIM = floorf((float)CANVAS_H * CANVAS_ZOOM);
		CELL_W_ANIM = floorf((float)CELL_W * CANVAS_ZOOM);
		CELL_H_ANIM = floorf((float)CELL_H * CANVAS_ZOOM);
		
		SDL_Rect _trect{};
		// transparent background grid
		float bg_w = floorf(ceilf(CANVAS_W_ANIM / CELL_W_ANIM) * CELL_W_ANIM);
		float bg_h = floorf(ceilf(CANVAS_H_ANIM / CELL_H_ANIM) * CELL_H_ANIM);
		_trect = { (int)CANVAS_X_ANIM, (int)CANVAS_Y_ANIM, (int)bg_w, (int)bg_h};
		SDL_SetTextureBlendMode(BG_GRID_TEXTURE, SDL_BLENDMODE_NONE);
		SDL_RenderCopy(RENDERER, BG_GRID_TEXTURE, nullptr, &_trect);

		// these 2 rects cover the overhang the background grid has beyond the canvas
		SDL_SetRenderDrawColor(RENDERER, 0, 0, 0, 255);
		_trect = {
			(int)(std::max(0.0f, CANVAS_X_ANIM)), (int)(std::max(0.0f,CANVAS_Y_ANIM + (CANVAS_H_ANIM))),
			(int)(std::min(float(WINDOW_W),bg_w)), (int)(CELL_H * CANVAS_ZOOM)
		};
		SDL_RenderFillRect(RENDERER, &_trect);
		_trect = {
			(int)(std::max(0.0f, CANVAS_X_ANIM + (CANVAS_W_ANIM))), (int)(std::max(0.0f, CANVAS_Y_ANIM)),
			(int)(CELL_W * CANVAS_ZOOM), (int)(std::min(float(WINDOW_H) , bg_h))
		};
		SDL_RenderFillRect(RENDERER, &_trect);
		
		_trect = {(int)CANVAS_X_ANIM, (int)CANVAS_Y_ANIM, (int)CANVAS_W_ANIM, (int)CANVAS_H_ANIM};
		// auto &&
		auto draw_layer = [](int a, auto r, int x, int y)
		{
			SDL_SetTextureBlendMode(CURRENT_FRAME_PTR->layers[a]->texture, (SDL_BlendMode)CURRENT_FRAME_PTR->layers[a]->blendmode);
			r = { (int)(CANVAS_X_ANIM + (x * CANVAS_W_ANIM)), (int)(CANVAS_Y_ANIM + (y * CANVAS_H_ANIM)), (int)CANVAS_W_ANIM, (int)CANVAS_H_ANIM };
			SDL_RenderCopy(RENDERER, CURRENT_FRAME_PTR->layers[a]->texture, nullptr, &r);
			if (a == CURRENT_LAYER)
			{
				if (BRUSH_UPDATE)
				{
					SDL_SetTextureBlendMode(BRUSH_TEXTURE, SDL_BLENDMODE_BLEND);
					SDL_RenderCopy(RENDERER, BRUSH_TEXTURE, nullptr, &r);
				}
			}
		};

		// RENDER THE LAYERS
		for (uint16_t i = 0; i < CURRENT_FRAME_PTR->layers.size(); i++)
		{
			//LAYER_INFO* layer = CURRENT_FRAME_PTR->layers[i];// LAYERS[i];
			/*SDL_SetTextureBlendMode(CURRENT_FRAME_PTR->layers[i]->texture, (SDL_BlendMode)CURRENT_FRAME_PTR->layers[i]->blendmode);
			_trect = { (int)CANVAS_X_ANIM, (int)CANVAS_Y_ANIM, (int)CANVAS_W_ANIM, (int)CANVAS_H_ANIM };
			SDL_RenderCopy(RENDERER, CURRENT_FRAME_PTR->layers[i]->texture, nullptr, &_trect);

			_trect = { (int)(CANVAS_X_ANIM - CANVAS_W_ANIM), (int)CANVAS_Y_ANIM, (int)CANVAS_W_ANIM, (int)CANVAS_H_ANIM };
			SDL_RenderCopy(RENDERER, CURRENT_FRAME_PTR->layers[i]->texture, nullptr, &_trect);
			_trect = { (int)(CANVAS_X_ANIM + CANVAS_W_ANIM), (int)CANVAS_Y_ANIM, (int)CANVAS_W_ANIM, (int)CANVAS_H_ANIM };
			SDL_RenderCopy(RENDERER, CURRENT_FRAME_PTR->layers[i]->texture, nullptr, &_trect);
			if (i == CURRENT_LAYER)
			{
				if (BRUSH_UPDATE)
				{
					SDL_SetTextureBlendMode(BRUSH_TEXTURE, SDL_BLENDMODE_BLEND);
					SDL_RenderCopy(RENDERER, BRUSH_TEXTURE, nullptr, &_trect);
				}
			}*/
			draw_layer(i, _trect, 0, 0);

			draw_layer(i, _trect, 1, 0);
			draw_layer(i, _trect, -1, 0);
			draw_layer(i, _trect, 0, 1);
			draw_layer(i, _trect, 0, -1);
			draw_layer(i, _trect, 1, 1);
			draw_layer(i, _trect, 1, -1);
			draw_layer(i, _trect, -1, 1);
			draw_layer(i, _trect, -1, -1);
		}

		// the grey box around the canvas
		SDL_SetRenderDrawColor(RENDERER, 64, 64, 64, 255);
		_trect = { (int)(CANVAS_X_ANIM - 2.0f), (int)(CANVAS_Y_ANIM - 2.0f), (int)(CANVAS_W_ANIM + 4.0f), (int)(CANVAS_H_ANIM + 4.0f) };
		SDL_RenderDrawRect(RENDERER, &_trect);

		SYSTEM_UIBOX_UPDATE();

		SDL_SetRenderDrawColor(RENDERER, 0, 0, 0, 255);

		// TEST HUE BAR
		//const SDL_Rect temp_rect{10,10,16,360};
		//SDL_RenderCopy(RENDERER, UI_TEXTURE_HUEBAR, nullptr, &temp_rect);
		
		SDL_SetRenderDrawColor(RENDERER, 0, 0, 0, 0);
		SDL_RenderPresent(RENDERER);

		CANVAS_PREVW = CANVAS_W;
		CANVAS_PREVH = CANVAS_H;
        
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
            //std::cout << MOUSEWHEEL_X << " " << MOUSEWHEEL_Y << "       " << '\r';
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
