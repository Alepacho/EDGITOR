#include "BRUSH.h"
#include "COLOR.h"
#include "QUAD.h"

// manpat: really not a fan of this - this should be cmakes job :(
#ifdef __APPLE__
#include <SDL2/SDL.h>
#include <SDL2_image/SDL_image.h>
#include <SDL2_ttf/SDL_ttf.h>
#else
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#endif


SDL_Texture* BRUSH_CURSOR_TEXTURE;
SDL_Texture* BRUSH_TEXTURE;
bool BRUSH_UPDATE = false;

int16_t BRUSH_X = 0;
int16_t BRUSH_Y = 0;
uint16_t BRUSH_W = 0;
QUAD BRUSH_UPDATE_REGION = QUAD::empty();
std::unique_ptr<COLOR[]> BRUSH_PIXELS;
COLOR BRUSH_COLOR {255, 255, 255, 128};
COLOR UNDO_COLOR{255, 0, 64, 192};
COLOR* BRUSH_CURSOR_PIXELS;
COLOR* BRUSH_CURSOR_PIXELS_CLEAR;
uint16_t BRUSH_SCATTER_X = 0;
uint16_t BRUSH_SCATTER_Y = 0;
uint16_t BRUSH_SCATTER_N = 1;


std::vector<std::unique_ptr<BRUSH_DATA>> BRUSH_LIST;
uint16_t BRUSH_LIST_POS = 0;


void brush_new(uint8_t* a, uint16_t w)
{
	std::unique_ptr<BRUSH_DATA> _u(new BRUSH_DATA(w));
	
	for (int _i = 0; _i < (w * w); _i++)
	{
		_u->set(_i, a[_i]);
	}
	BRUSH_LIST.push_back(std::move(_u));
}


void brush_set(uint16_t sel)
{
	uint16_t w = BRUSH_LIST[sel]->w;
	BRUSH_X = -w / 2;
	BRUSH_Y = -w / 2;
	BRUSH_W = w;
}