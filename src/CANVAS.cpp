#include "CANVAS.h"
#include "COLOR.h"
#include "QUAD.h"
#include "VARIABLES.h"
#include "BRUSH.h"
#include "SUPERSTACK.h"
#include "SYSTEM.h"
#include "UNDO.h"
#include <stack>
#include <string>
#include <bitset>
#include <unordered_set>
#include <functional>
#include <memory>

// CANVAS
bool CANVAS_UPDATE = false;
float CANVAS_ZOOM = 1.0;
float CANVAS_X = 0.0;
float CANVAS_Y = 0.0;
uint16_t CANVAS_W = 16;
uint16_t CANVAS_H = 16;
uint16_t CANVAS_PREVW = 0;
uint16_t CANVAS_PREVH = 0;
int16_t CANVAS_MOUSE_X = 0;
int16_t CANVAS_MOUSE_Y = 0;
int16_t CANVAS_MOUSE_CLICKX = 0;
int16_t CANVAS_MOUSE_CLICKY = 0;
int16_t CANVAS_MOUSE_CLICKPREVX = 0;
int16_t CANVAS_MOUSE_CLICKPREVY = 0;
int16_t CANVAS_MOUSE_PREVX = 0;
int16_t CANVAS_MOUSE_PREVY = 0;
int16_t CANVAS_MOUSE_CELL_X = 0;
int16_t CANVAS_MOUSE_CELL_Y = 0;
int16_t CANVAS_PITCH = (sizeof(COLOR) * CANVAS_W);

SDL_Texture* BG_GRID_TEXTURE;
int16_t CELL_W = 16;
int16_t CELL_H = 16;
int16_t BG_GRID_W = 0;
int16_t BG_GRID_H = 0;

float CANVAS_PANX = 0.0;
float CANVAS_PANY = 0.0;

float CANVAS_X_ANIM = 0.0;
float CANVAS_Y_ANIM = 0.0;
float CANVAS_W_ANIM = 0.0;
float CANVAS_H_ANIM = 0.0;
float CELL_W_ANIM = 0.0;
float CELL_H_ANIM = 0.0;

std::vector<std::shared_ptr<FILE_INFO>> FILES;

// LAYER
uint16_t CURRENT_LAYER = 0;
std::shared_ptr<LAYER_INFO> CURRENT_LAYER_PTR = nullptr;
int16_t LAYER_UPDATE = 0;
QUAD LAYER_UPDATE_REGION = QUAD::empty();
//std::vector<LAYER_INFO> LAYERS;

// FRAME
uint16_t CURRENT_FRAME = 0;
std::shared_ptr<FRAME_INFO> CURRENT_FRAME_PTR = nullptr;

// FILE
uint16_t CURRENT_FILE = 0;
std::shared_ptr<FILE_INFO> CURRENT_FILE_PTR = nullptr;



/*void layer_new(SDL_Renderer* _renderer, int16_t _x, int16_t _y, int16_t _a, /*SDL_BlendMode* int _b)
{
	LAYER_INFO new_layer;
	new_layer.texture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, CANVAS_W, CANVAS_H);

	new_layer.pixels = std::make_unique<COLOR[]>(CANVAS_W * CANVAS_H);
	new_layer.x = _x;
	new_layer.y = _y;
	new_layer.alpha = _a;
	new_layer.blendmode = _b;
	LAYERS.push_back(std::move(new_layer));
	CANVAS_UPDATE = true;
	CURRENT_LAYER = 0;
	CURRENT_LAYER_PTR = LAYERS[CURRENT_LAYER].pixels.get();
}*/

bool MODE_TILEX = 1;
bool MODE_TILEY = 1;

bool in_canvas(const uint16_t x, const uint16_t y)
{
	return (x < CANVAS_W && y < CANVAS_H);
}

bool out_canvas(const uint16_t x, const uint16_t y)
{
	return (x >= CANVAS_W || y >= CANVAS_H);
}

void set_pixel(const int16_t x, const int16_t y, const COLOR c)
{
	if (out_canvas(x, y)) return;

	BRUSH_PIXELS[y * CANVAS_W + x] = c;
	BRUSH_UPDATE_REGION = BRUSH_UPDATE_REGION.include_point(x, y);
}

void set_pixel_brush(int x, int y, COLOR c)
{
	int _tx = x, _ty = y;
	for (uint16_t s = 0; s < BRUSH_SCATTER_N; s++)
	{
		if (BRUSH_SCATTER_X) _tx = x + (heptrand() % BRUSH_SCATTER_X) - (BRUSH_SCATTER_X / 2);
		if (BRUSH_SCATTER_Y) _ty = y + (heptrand() % BRUSH_SCATTER_Y) - (BRUSH_SCATTER_Y / 2);
		for (int i = 0; i < BRUSH_W; i++)
		{
			for (int j = 0; j < BRUSH_W; j++)
			{
				//_tx = ((_tx + BRUSH_X) + i);
				//_ty = ((_ty + BRUSH_Y) + j);
				//if (!in_canvas(_tx, _ty)) continue;
				uint8_t _a = (BRUSH_LIST[BRUSH_LIST_POS]->alpha[j * BRUSH_W + i]);
				if (!_a) continue; else {
					if (MODE_TILEX) _tx = (_tx + (CELL_W * 10)) % CELL_W;
					if (MODE_TILEY) _ty = (_ty + (CELL_H * 10)) % CELL_H;

					set_pixel((_tx + BRUSH_X) + i, (_ty + BRUSH_Y) + j, c);
				}

				//if (BRUSH_PIXELS[(_tx + BRUSH_X) + i, (_ty + BRUSH_Y) + j] != COL_EMPTY) continue;
			}
		}
	}
}

void set_pixel_layer(const int16_t x, const int16_t y, const COLOR c, std::shared_ptr<LAYER_INFO> l)
{
	if (out_canvas(x, y)) return;

	l->pixels[y * CANVAS_W + x] = c;

	BRUSH_UPDATE_REGION = BRUSH_UPDATE_REGION.include_point(x, y);
}

void set_pixel_line(int16_t x0, int16_t y0, const int16_t x1, const int16_t y1, COLOR c)
{
	int16_t dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
	int16_t dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
	int16_t err = dx + dy, e2;
	for (;;) {
		set_pixel_brush(x0, y0, c); // set_pixel_brush
		if (x0 == x1 && y0 == y1) break;
		e2 = err << 1;
		if (e2 >= dy) { err += dy; x0 += sx; }
		if (e2 <= dx) { err += dx; y0 += sy; }
	}
	BRUSH_UPDATE = true;
	LAYER_UPDATE = 2;
}

COLOR get_pixel(const int16_t x, const int16_t y)
{
	return BRUSH_PIXELS[y * CANVAS_W + x];
}

COLOR get_pixel_layer(const int16_t x, const int16_t y, uint16_t l)
{
	return CURRENT_FRAME_PTR->layers[l]->pixels[y * CANVAS_W + x];
}

bool floodfill_check(const uint16_t x, const uint16_t y, const COLOR dest_col, const COLOR fill_col)
{
	return ((CURRENT_LAYER_PTR->pixels[y * CANVAS_W + x] != dest_col) || !(BRUSH_PIXELS[y * CANVAS_W + x] != fill_col));
}

bool floodfill_check_not(const uint16_t x, const uint16_t y, const COLOR dest_col, const COLOR fill_col)
{
	return ((CURRENT_LAYER_PTR->pixels[y * CANVAS_W + x] == dest_col) && (BRUSH_PIXELS[y * CANVAS_W + x] != fill_col));
}

void floodfill(uint16_t x, uint16_t y, const uint16_t width, const uint16_t height, const COLOR dest_col, const COLOR fill_col)
{
	while (true)
	{
		int ox = x, oy = y;
		while (y != 0 && floodfill_check_not(x, y - 1, dest_col, fill_col)) y--;
		while (x != 0 && floodfill_check_not(x - 1, y, dest_col, fill_col)) x--;
		if (x == ox && y == oy) break;
	}
	floodfill_core(x, y, width, height, dest_col, fill_col);
	BRUSH_UPDATE = true;
	LAYER_UPDATE = 2;
}

void floodfill_core(uint16_t x, uint16_t y, const uint16_t width, const uint16_t height, const COLOR dest_col, const COLOR fill_col)
{
	int lastRowLength = 0;
	do
	{
		int rowLength = 0, sx = x;
		if (lastRowLength != 0 && floodfill_check(x, y, dest_col, fill_col))
		{
			do
			{
				if (--lastRowLength == 0) return;
				++x;
			} while (floodfill_check(x, y, dest_col, fill_col));
			sx = x;
		}
		else
		{
			for (; x != 0 && floodfill_check_not(x - 1, y, dest_col, fill_col); rowLength++, lastRowLength++)
			{
				set_pixel(--x, y, fill_col);
				if (y != 0 && floodfill_check_not(x, y - 1, dest_col, fill_col)) floodfill(x, y - 1, width, height, dest_col, fill_col);
			}
		}

		for (; sx < width && floodfill_check_not(sx, y, dest_col, fill_col); rowLength++, sx++) set_pixel(sx, y, fill_col);
		if (rowLength < lastRowLength)
		{
			for (int end = x + lastRowLength; ++sx < end; )
			{
				if (floodfill_check_not(sx, y, dest_col, fill_col)) floodfill_core(sx, y, width, height, dest_col, fill_col);
			}
		}
		else if (rowLength > lastRowLength && y != 0)
		{
			for (int ux = x + lastRowLength; ++ux < sx; )
			{
				if (floodfill_check_not(ux, y - 1, dest_col, fill_col)) floodfill(ux, y - 1, width, height, dest_col, fill_col);
			}
		}
		lastRowLength = rowLength;
	} while (lastRowLength != 0 && ++y < height);
}

void floodfill_test(uint16_t x, uint16_t y, COLOR dest_col, COLOR fill_col)
{
	MOUSEBUTTON_LEFT = 0;
	MOUSEBUTTON_PRESSED_LEFT = 0;
	if (dest_col == fill_col) return;

	struct t {
		uint16_t x;
		uint16_t y;
		uint8_t d;
	};

	t tp;
	uint16_t tx = x;
	uint16_t ty = y;
	uint8_t td = 0;

	faststack<uint16_t> changed;
	changed.reserve(CANVAS_W * CANVAS_H);
	changed.push(0);
	changed.push(y);
	changed.push(x);

	while (changed.size() > 2)
	{
		tx = changed.top();
		changed.pop();
		ty = changed.top();
		changed.pop();
		td = changed.top();
		changed.pop();

		set_pixel(tx, ty, fill_col);

		if (td != 1 && tx + 1 < CANVAS_W && (get_pixel_layer(tx + 1, ty, CURRENT_LAYER) == dest_col && get_pixel(tx + 1, ty) != fill_col))
		{
			changed.push(2);
			changed.push(ty);
			changed.push(tx + 1);
		}
		if (td != 2 && tx - 1 >= 0 && (get_pixel_layer(tx - 1, ty, CURRENT_LAYER) == dest_col && get_pixel(tx - 1, ty) != fill_col))
		{
			changed.push(1);
			changed.push(ty);
			changed.push(tx - 1);
		}
		if (td != 3 && ty + 1 < CANVAS_H && (get_pixel_layer(tx, ty + 1, CURRENT_LAYER) == dest_col && get_pixel(tx, ty + 1) != fill_col))
		{
			changed.push(4);
			changed.push(ty + 1);
			changed.push(tx);
		}
		if (td != 4 && ty - 1 >= 0 && (get_pixel_layer(tx, ty - 1, CURRENT_LAYER) == dest_col && get_pixel(tx, ty - 1) != fill_col))
		{
			changed.push(3);
			changed.push(ty - 1);
			changed.push(tx);
		}
	}

	BRUSH_UPDATE = true;
	LAYER_UPDATE = 2;
}

/*void floodfill(int x, int y, COLOR oldColor, COLOR newColor)
{
	if (oldColor == newColor) return;

	int x1;
	bool spanAbove, spanBelow;

	faststack<int> check;

	check.reserve(200);

	for (int i = 0; i < 100; ++i) { check.push(i); }
	for (int i = 0; i < 100; ++i) { check.pop(); }

	// when you uncomment this line, std::stack performance will magically rise about 18%
	std::vector<int> magicVector(10);

	check.push(y);
	check.push(x);

	while (check.size() > 0)
	{
		x = (check.top());
		check.pop();
		y = (check.top());
		check.pop();

		x1 = x;
		while (x1 >= 0 && ((get_pixel_layer(x1, y, CURRENT_LAYER) == oldColor) && (get_pixel(x1, y) == oldColor))) x1--;
		x1++;
		spanAbove = spanBelow = 0;
		while (x1 < CANVAS_W && ((get_pixel_layer(x1, y, CURRENT_LAYER) == oldColor) && (get_pixel(x1, y) == oldColor)))
		{
			//set_pixel_surface(x1, y, newColor, S);
			set_pixel(x1, y, newColor);
			//set_pixel_layer(x1, y, newColor, CURRENT_LAYER);
			if (!spanAbove && y > 0 && ((get_pixel_layer(x1, (y - 1), CURRENT_LAYER) == oldColor) && (get_pixel(x1, (y - 1)) == oldColor)))
			{
				check.push((y - 1));
				check.push(x1);
				spanAbove = 1;
			}
			else if (spanAbove && y > 0 && ((get_pixel_layer(x1, (y - 1), CURRENT_LAYER) != oldColor) || (get_pixel(x1, (y - 1)) != oldColor)))
			{
				spanAbove = 0;
			}
			if (!spanBelow && y < CANVAS_H - 1 && ((get_pixel_layer(x1, (y + 1), CURRENT_LAYER) == oldColor) && (get_pixel(x1, (y + 1)) == oldColor)))
			{
				check.push((y + 1));
				check.push(x1);
				spanBelow = 1;
			}
			else if (spanBelow && y < CANVAS_H - 1 && ((get_pixel_layer(x1, (y + 1), CURRENT_LAYER) != oldColor) || (get_pixel(x1, (y + 1)) != oldColor)))
			{
				spanBelow = 0;
			}
			x1++;
		}
	}
	BRUSH_UPDATE = true;
	LAYER_UPDATE = 2;
	CANVAS_UPDATE = true;
}*/

/*void floodfill(uint16_t x, uint16_t y, COLOR dest_col, COLOR fill_col)
{
	MOUSEBUTTON_LEFT = 0;
	MOUSEBUTTON_PRESSED_LEFT = 0;
	if (dest_col == fill_col) return;

	struct t {
		uint16_t x;
		uint16_t y;
		uint8_t d;
	};

	t tp;
	uint16_t tx = x;
	uint16_t ty = y;
	uint8_t td = 0;

	faststack<t> changed;
	changed.reserve(CANVAS_W * CANVAS_H);
	changed.push(t{ tx,ty,td });

	while (changed.size() > 0)
	{
		tp = changed.top();
		changed.pop();
		tx = tp.x;
		ty = tp.y;
		td = tp.d;

		set_pixel(tx, ty, fill_col);

		if ((!td || td != 1) && tx + 1 < CANVAS_W)
		{
			if (get_pixel_layer(tx + 1, ty, CURRENT_LAYER) == dest_col && get_pixel(tx + 1, ty) != fill_col)
			{
				changed.push(t{ (uint16_t)(tx + 1), ty, 2 });
			}
		}
		if ((!td || td != 2) && tx - 1 >= 0)
		{
			if (get_pixel_layer(tx - 1, ty, CURRENT_LAYER) == dest_col && get_pixel(tx - 1, ty) != fill_col)
			{
				changed.push(t{ (uint16_t)(tx - 1), ty, 1 });
			}
		}
		if ((!td || td != 3) && ty + 1 < CANVAS_H)
		{
			if (get_pixel_layer(tx, ty + 1, CURRENT_LAYER) == dest_col && get_pixel(tx, ty + 1) != fill_col)
			{
				changed.push(t{ tx, (uint16_t)(ty + 1), 4 });
			}
		}
		if ((!td || td != 4) && ty - 1 >= 0)
		{
			if (get_pixel_layer(tx, ty - 1, CURRENT_LAYER) == dest_col && get_pixel(tx, ty - 1) != fill_col)
			{
				changed.push(t{ tx, (uint16_t)(ty - 1), 3 });
			}
		}
	}

	std::unordered_set<int> changed;

	std::function<void(int,int,int,COLOR,COLOR)> fill = [&](int x, int y, int d, COLOR dest_col, COLOR fill_col)
	{
		if (changed.find(x + y * CANVAS_W) == changed.end())
		{
			set_pixel((int16_t)x, (int16_t)y, fill_col);
			changed.emplace(x + y * CANVAS_W);

			if ((!d || d != 1) && x + 1 < CANVAS_W)
			{
				if (get_pixel_layer(x + 1, y, CURRENT_LAYER) == dest_col && get_pixel(x + 1, y) != fill_col)
				{
					fill(x + 1, y, 2, dest_col, fill_col);
				}
			}
			if ((!d || d != 2) && x - 1 >= 0)
			{
				if (get_pixel_layer(x - 1, y, CURRENT_LAYER) == dest_col && get_pixel(x - 1, y) != fill_col)
				{
					fill(x - 1, y, 1, dest_col, fill_col);
				}
			}
			if ((!d || d != 3) && y + 1 < CANVAS_H)
			{
				if (get_pixel_layer(x, y + 1, CURRENT_LAYER) == dest_col && get_pixel(x, y + 1) != fill_col)
				{
					fill(x, y + 1, 4, dest_col, fill_col);
				}
			}
			if ((!d || d != 4) && y - 1 >= 0)
			{
				if (get_pixel_layer(x, y - 1, CURRENT_LAYER) == dest_col && get_pixel(x, y - 1) != fill_col)
				{
					fill(x, y - 1, 3, dest_col, fill_col);
				}
			}
		}
	};
	fill(x, y, 0, dest_col, fill_col);

	BRUSH_UPDATE = true;
	LAYER_UPDATE = 2;
}*/

/*void floodfill(uint16_t x, uint16_t y, COLOR dest_col, COLOR fill_col)
{
	MOUSEBUTTON_LEFT = 0;
	MOUSEBUTTON_PRESSED_LEFT = 0;
	if (dest_col == fill_col) return;

	int16_t fx = -1;
	int16_t fy = -1;
	int16_t tx = 0;
	int16_t ty = 0;
	bool x1 = 1;
	bool x2 = 1;
	bool y1 = 1;
	bool y2 = 1;

	for (int i = 0; i < 10; i++)
	{
		fx = -1;
		x1 = 1;
		x2 = 1;
		ty = y + ++fy;
		while (true)
		{
			tx = x + ++fx;
			if (x1) { if (get_pixel_layer(tx, ty, CURRENT_LAYER) == dest_col) set_pixel(tx, ty, fill_col); else x1 = 0; }
			else if (!x2) break;
			tx = x + ~fx;
			if (x2) { if (get_pixel_layer(tx, ty, CURRENT_LAYER) == dest_col) set_pixel(tx, ty, fill_col); else x2 = 0; }
			else if (!x1) break;
		}
		fx = -1;
		x1 = 1;
		x2 = 1;
		ty = y + ~fy;
		while (true)
		{
			tx = x + ++fx;
			if (x1) { if (get_pixel_layer(tx, ty, CURRENT_LAYER) == dest_col) set_pixel(tx, ty, fill_col); else x1 = 0; }
			else if (!x2) break;
			tx = x + ~fx;
			if (x2) { if (get_pixel_layer(tx, ty, CURRENT_LAYER) == dest_col) set_pixel(tx, ty, fill_col); else x2 = 0; }
			else if (!x1) break;
		}
	}

	BRUSH_UPDATE = true;
	LAYER_UPDATE = 2;
}*/

void refresh_canvas()
{
	CANVAS_W = CURRENT_FILE_PTR->canvas_w;
	CANVAS_H = CURRENT_FILE_PTR->canvas_h;

	//clear_undo_stack();

	CANVAS_PITCH = (sizeof(COLOR) * CANVAS_W);
	BRUSH_PIXELS = nullptr;
	BRUSH_PIXELS = std::make_unique<COLOR[]>(CANVAS_W * CANVAS_H);
	SDL_DestroyTexture(BRUSH_TEXTURE);
	BRUSH_TEXTURE = SDL_CreateTexture(RENDERER, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, CANVAS_W, CANVAS_H);

	if (CANVAS_PREVW && CANVAS_PREVH) SDL_DestroyTexture(BG_GRID_TEXTURE);
	// BACKGROUND GRID TEXTURE
	BG_GRID_W = ((int16_t)ceil((double)CANVAS_W / (double)CELL_W));
	BG_GRID_H = ((int16_t)ceil((double)CANVAS_H / (double)CELL_H));
	auto BG_GRID_PIXELS = std::make_unique<COLOR[]>(BG_GRID_W * BG_GRID_H);
	BG_GRID_TEXTURE = SDL_CreateTexture(RENDERER, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, BG_GRID_W, BG_GRID_H);
	for (int i = 0; i < BG_GRID_H; i++)
	{
		for (int j = 0; j < BG_GRID_W; j++)
		{
			const COLOR cell_colors[]{
				COLOR {0x0c, 0x0c, 0x0c, 0xff},
				COLOR {0x10, 0x10, 0x10, 0xff},
			};

			BG_GRID_PIXELS[i * BG_GRID_W + j] = cell_colors[(i + j) % 2];
		}
	}
	SDL_SetTextureBlendMode(BG_GRID_TEXTURE, SDL_BLENDMODE_NONE);
	SDL_UpdateTexture(BG_GRID_TEXTURE, nullptr, BG_GRID_PIXELS.get(), BG_GRID_W * sizeof(COLOR));
}

void file_add(std::string path, std::string filename, uint16_t canvas_w, uint16_t canvas_h)
{
	std::shared_ptr<FILE_INFO> _file = std::make_shared<FILE_INFO>();
	_file->path = path;
	_file->filename = filename;
	_file->canvas_w = canvas_w;
	_file->canvas_h = canvas_h;
	CANVAS_W = canvas_w;
	CANVAS_H = canvas_h;

	CANVAS_UPDATE = true;
	CURRENT_FILE = FILES.size();
	CURRENT_FILE_PTR = _file;

	_file->add_frame(1);

	FILES.push_back(std::move(_file));

	refresh_canvas();

	/*PRINT(CURRENT_FILE);
	PRINT((int)&CURRENT_FILE_PTR);
	PRINT(CURRENT_FRAME);
	PRINT((int)&CURRENT_FRAME_PTR);
	PRINT(CURRENT_LAYER);
	PRINT((int)&CURRENT_LAYER_PTR);*/
}