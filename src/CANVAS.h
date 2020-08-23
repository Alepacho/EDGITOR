#pragma once

#include "VARIABLES.h"
#include "UNDO.h"
#include <cstdint>
#include <memory>
#include <vector>
#include <algorithm>
#include <string>

struct COLOR;
struct QUAD;
struct SDL_Renderer;
struct SDL_Texture;

// CANVAS
extern bool CANVAS_UPDATE;
extern float CANVAS_ZOOM;
extern float CANVAS_X;
extern float CANVAS_Y;
extern uint16_t CANVAS_W;
extern uint16_t CANVAS_H;
extern uint16_t CANVAS_PREVW;
extern uint16_t CANVAS_PREVH;
extern int16_t CANVAS_MOUSE_X;
extern int16_t CANVAS_MOUSE_Y;
extern int16_t CANVAS_MOUSE_CLICKX;
extern int16_t CANVAS_MOUSE_CLICKY;
extern int16_t CANVAS_MOUSE_CLICKPREVX;
extern int16_t CANVAS_MOUSE_CLICKPREVY;
extern int16_t CANVAS_MOUSE_PREVX;
extern int16_t CANVAS_MOUSE_PREVY;
extern int16_t CANVAS_MOUSE_CELL_X;
extern int16_t CANVAS_MOUSE_CELL_Y;
extern int16_t CANVAS_PITCH;

extern SDL_Texture* BG_GRID_TEXTURE;
extern int16_t CELL_W;
extern int16_t CELL_H;
extern int16_t BG_GRID_W;
extern int16_t BG_GRID_H;

extern float CANVAS_PANX;
extern float CANVAS_PANY;

extern float CANVAS_X_ANIM;
extern float CANVAS_Y_ANIM;
extern float CANVAS_W_ANIM;
extern float CANVAS_H_ANIM;
extern float CELL_W_ANIM;
extern float CELL_H_ANIM;

struct LAYER_INFO;
struct FRAME_INFO;
struct FILE_INFO;

extern std::vector<std::shared_ptr<FILE_INFO>> FILES;

// LAYER
extern uint16_t CURRENT_LAYER;
extern std::shared_ptr<LAYER_INFO> CURRENT_LAYER_PTR;
extern int16_t LAYER_UPDATE;
extern QUAD LAYER_UPDATE_REGION;

// FRAME
extern uint16_t CURRENT_FRAME;
extern std::shared_ptr<FRAME_INFO> CURRENT_FRAME_PTR;

// FILE
extern uint16_t CURRENT_FILE;
extern std::shared_ptr<FILE_INFO> CURRENT_FILE_PTR;

enum FILE_TYPE {
	EMPTY,
	IMAGE,
	MAP,
	TEXT
};

struct LAYER_INFO {
	SDL_Texture* texture;
	std::shared_ptr<COLOR[]> pixels;
	int16_t x;
	int16_t y;
	uint8_t alpha;
	int blendmode; /* SDL_BlendMode */
};

struct FRAME_INFO {
	int time = 1;
	std::vector<std::shared_ptr<LAYER_INFO>> layers;

	void add_layer(int16_t _x, int16_t _y, int16_t _a, /*SDL_BlendMode*/ int _b)
	{
		std::shared_ptr<LAYER_INFO> new_layer = std::make_shared<LAYER_INFO>();
		new_layer->texture = SDL_CreateTexture(RENDERER, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, CANVAS_W, CANVAS_H);

		new_layer->pixels = std::make_unique<COLOR[]>(CANVAS_W * CANVAS_H);
		new_layer->x = _x;
		new_layer->y = _y;
		new_layer->alpha = _a;
		new_layer->blendmode = _b;
		
		CANVAS_UPDATE = true;
		CURRENT_LAYER = this->layers.size();
		CURRENT_LAYER_PTR = new_layer;

		this->layers.push_back(std::move(new_layer));
	}
};

struct FILE_INFO {
	int type = FILE_TYPE::EMPTY;
	std::string path = "";
	std::string filename = "";
	std::vector<std::shared_ptr<FRAME_INFO>> frames;
	int canvas_w = 0;
	int canvas_h = 0;
	std::vector<UNDO_ENTRY> undo_stack;
	uint16_t undo_pos = 0;

	void add_frame(int frame_time)
	{
		std::shared_ptr<FRAME_INFO> new_frame = std::make_shared<FRAME_INFO>();
		new_frame->add_layer(0, 0, 255, SDL_BLENDMODE_BLEND);

		CANVAS_UPDATE = true;
		CURRENT_FRAME = this->frames.size();
		CURRENT_FRAME_PTR = new_frame;

		this->frames.push_back(std::move(new_frame));
	}
};

extern bool MODE_TILEX;
extern bool MODE_TILEY;

bool in_canvas(const uint16_t x, const uint16_t y);
bool out_canvas(const uint16_t x, const uint16_t y);


//void layer_new(SDL_Renderer* _renderer, int16_t _x, int16_t _y, int16_t _a, /*SDL_BlendMode*/ int _b);


void set_pixel(const int16_t x, const int16_t y, const COLOR c);
void set_pixel_brush(int x, int y, COLOR c);
void set_pixel_layer(const int16_t x, const int16_t y, const COLOR c, std::shared_ptr<LAYER_INFO> l);
void set_pixel_line(int16_t x0, int16_t y0, const int16_t x1, const int16_t y1, COLOR c);

COLOR get_pixel(const int16_t x, const int16_t y);
COLOR get_pixel_layer(const int16_t x, const int16_t y, uint16_t l);


bool floodfill_check(const uint16_t x, const uint16_t y, const COLOR dest_col, const COLOR fill_col);
bool floodfill_check_not(const uint16_t x, const uint16_t y, const COLOR dest_col, const COLOR fill_col);
void floodfill(uint16_t x, uint16_t y, const uint16_t width, const uint16_t height, const COLOR dest_col, const COLOR fill_col);
void floodfill_core(uint16_t x, uint16_t y, const uint16_t width, const uint16_t height, const COLOR dest_col, const COLOR fill_col);

void floodfill_test(uint16_t x, uint16_t y, COLOR dest_col, COLOR fill_col);

void refresh_canvas();
void file_add(std::string path, std::string filename, uint16_t canvas_w, uint16_t canvas_h);