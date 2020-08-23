#pragma once

#include <string>
#include <vector>
#include <memory>
#include <deque>
#include <iostream>
#include <filesystem>
#include <algorithm>

#include "COLOR.h"
#include "CANVAS.h"
#include "BRUSH.h"
#include "SYSTEM.h"
#include "VARIABLES.h"
#include "SDL_image.h"
#include "SUPERSTACK.h"
#include "CUBE_VIEW.h"

//struct UIBOX_INFO;
struct UIBOX_ELEMENT_MAIN;

struct SDL_Texture;

constexpr uint16_t UIBOX_UPDATE_TICK = 0; // 0=go as fast as possible, >0=frame delay between character updates

extern int16_t UIBOX_IN;
extern int16_t UIBOX_PREVIN;
extern int16_t UIBOX_CLICKED_IN;
extern int16_t UIBOX_PANX;
extern int16_t UIBOX_PANY;

extern int16_t ELEMENT_IN;
extern int16_t ELEMENT_CLICKED_IN;
extern bool ELEMENT_TOGGLE_BOOL;

struct UIBOX_INFO;

extern std::vector<std::unique_ptr<UIBOX_INFO>> UIBOXES;
extern std::vector<std::pair<std::string, bool>> PATH_FILES;

struct UIBOX_CHAR {
	bool update = 0;
	uint8_t chr;
	COLOR col;
	COLOR bg_col = { 0,0,0,1 };
};

struct UIBOX_ELEMENT {
	std::string text;
	std::string over_text;
	bool over = false;
	bool sel = false;
	uint8_t type = 0; // 0 = button, >0 anything else
	bool* input_bool = nullptr;
	uint16_t* input_int = nullptr;
	uint16_t input_int_var = 0;
	bool is_pos = false;
	uint16_t px = 0;
	uint16_t py = 0;
};

void uibox_set_string(UIBOX_INFO* uibox, std::string _charlist, uint16_t x, uint16_t y, COLOR col, bool update);
void uibox_set_char(UIBOX_INFO* ui, uint16_t char_pos, uint8_t _CHR, COLOR _COL, COLOR _BG_COL, bool update);

struct UIBOX_INFO {
	bool creation_update = true;
	bool update = true;
	bool element_update = true;
	bool can_scroll = false;

	bool can_grab = true;
	bool in_grab = false;
	bool grab = false;

	bool can_shrink = true;
	bool in_shrink = false;
	bool shrink = false;

	std::vector<UIBOX_CHAR> charinfo;
	std::vector<UIBOX_ELEMENT> element;

	std::vector<std::shared_ptr<UIBOX_ELEMENT_MAIN>> element_list;

	faststack<uint16_t> update_stack;
	std::string title = "";
	SDL_Texture* texture;
	uint16_t tex_w;
	uint16_t tex_h;
	uint16_t chr_w;
	uint16_t chr_h;
	int x;
	int y;
	uint16_t w;
	uint16_t h;
	uint8_t alpha;
	uint16_t update_tick = 0;
	uint16_t scroll_x = 0;
	uint16_t scroll_y = 0;
	std::vector<uint16_t> scroll_element_list;
	bool scroll_element_list_create = true;
	uint16_t scroll_element_list_size = 0;

	bool snap = 0;
	bool has_drawloop = 0;

	virtual void construct()
	{
		if (this->texture != nullptr)
		{
			SDL_DestroyTexture(this->texture);
			this->texture = nullptr;
		}

		// FILL WINDOW WITH EMPTY CHARS
		for (int j = 0; j < this->chr_w * this->chr_h; j++)
		{
			UIBOX_CHAR _chr;
			_chr.col = COL_WHITE;
			_chr.chr = ' ';
			//_chr.bg_col = COL_BGUPDATE;
			this->charinfo.push_back(std::move(_chr));
			this->update_stack.push(j);// .insert(this->update_stack.begin() + (rand() % (this->update_stack.size() + 1)), j);
		}

		// ADD BORDER
		for (int j = 0; j < this->chr_h; j++)
		{
			for (int i = 0; i < this->chr_w; i++)
			{
				if ((j > 0 && j < this->chr_h - 1) && (i > 0 && i < this->chr_w - 1))
				{
					continue;
				}
				this->charinfo[j * this->chr_w + i].chr = (j == 0) ? ((i == 0) ? CHAR_BOXTL : ((i == this->chr_w - 1) ? CHAR_BOXTR : CHAR_BOXH)) :
					((j == this->chr_h - 1) ? ((i == 0) ? CHAR_BOXBL : ((i == this->chr_w - 1) ? CHAR_BOXBR : CHAR_BOXH)) : CHAR_BOXV);
			}
		}

		// TITLE
		uibox_set_string(this, STR_NBSP + title + STR_NBSP, 1, 0, COL_WHITE, false);

		// SHRINK BUTTON
		uibox_set_string(this, STR_NBSP STR_ARWD STR_NBSP, this->chr_w - 4, 0, COL_WHITE, false);
	}

	virtual void init(uint16_t _x, uint16_t _y, uint16_t _w, uint16_t _h, bool can_grab, std::string title)
	{
		this->title = title;
		this->can_grab = can_grab;
		this->update = true;
		this->x = _x;
		this->y = _y;

		// GET CHARS THAT FIT WINDOW
		this->chr_w = (int)floor(((float)_w / (float)FONT_CHRW) + 0.5f);
		this->chr_h = (int)floor(((float)_h / (float)FONT_CHRH) + 0.5f);
		this->update_stack.reserve(this->chr_w * this->chr_h);

		// MAKE WINDOW THE NEW ROUNDED CHAR SIZE
		this->w = (this->chr_w * FONT_CHRW);
		this->h = (this->chr_h * FONT_CHRH);

		this->construct();
	}

	virtual void update_elements(void)
	{
		//
	}

	virtual void drawloop()
	{
		//
	}
};

//struct UIBOX_ELEMENT_MAIN;

//void uibox_set_string(UIBOX_INFO* uibox, std::string _charlist, uint16_t x, uint16_t y, COLOR col, bool update);
//void uibox_set_char(UIBOX_INFO* ui, uint16_t char_pos, uint8_t _CHR, COLOR _COL, COLOR _BG_COL, bool update);
void UPDATE_PATH_FILES();

void SYSTEM_UIBOX_UPDATE();

// returns if the mouse input was claimed by a uibox
void uibox_shrink(UIBOX_INFO* uibox, bool s);
bool SYSTEM_UIBOX_HANDLE_MOUSE_DOWN(bool is_click, int mouse_x, int mouse_y);
void SYSTEM_UIBOX_HANDLE_MOUSE_UP();

// UIBOX_ELEMENT_MAIN
struct UIBOX_ELEMENT_MAIN {
	bool over = false;
	bool highlight = true;
	bool prev_sel = false;
	bool prev_over = false;
	bool const_update = false;

	std::string text = "";
	std::string sel_text = "";
	uint16_t x = 0;
	uint16_t y = 0;
	// for W & H: 0=text size, -1=window size
	int16_t w = 0;
	int16_t h = 0;
	virtual void create(UIBOX_INFO*) = 0;
	virtual void set(void)
	{
		//
	}
	virtual void update(UIBOX_INFO* uibox)
	{
		for (uint16_t iy = 0; iy < h; iy++)
		{
			for (uint16_t ix = 0; ix < w; ix++)
			{
				uibox_set_char(uibox, x + ix + ((y + iy) * uibox->chr_w),
					prev_sel ? (ix < sel_text.size() ? (sel_text.c_str())[ix] : 32) : (ix < text.size() ? (text.c_str())[ix] : 32),
					COL_EMPTY,
					(prev_over || prev_sel) ? COL_ACCENT : COL_BGUPDATE,
					1);
			}
		}
	}
	virtual bool is_sel()
	{
		return false;
	}
};

void uibox_add_element_textbox(UIBOX_INFO* uibox, uint16_t x, uint16_t y, std::string text);
void uibox_add_element_varbox(UIBOX_INFO* uibox, uint16_t x, uint16_t y, std::string text, uint16_t* input_var, uint16_t var);
void uibox_add_element_varbox_u8(UIBOX_INFO* uibox, uint16_t x, uint16_t y, std::string text, uint8_t* input_var, uint8_t var);
void uibox_add_element_varbox_s16(UIBOX_INFO* uibox, uint16_t x, uint16_t y, std::string text, int16_t* input_var, int16_t var);
void uibox_add_element_varbox_f(UIBOX_INFO* uibox, uint16_t x, uint16_t y, std::string text, float* input_var, float var);
void uibox_add_element_button(UIBOX_INFO* uibox, uint16_t x, uint16_t y, int16_t w, int16_t h, std::string text, std::string sel_text, uint16_t* input_var, uint16_t button_var);
void uibox_add_element_button_u8(UIBOX_INFO* uibox, uint16_t x, uint16_t y, int16_t w, int16_t h, std::string text, std::string sel_text, uint8_t* input_var, uint8_t button_var);
void uibox_add_element_button_color(UIBOX_INFO* uibox, uint16_t x, uint16_t y, int16_t w, int16_t h, std::string text, std::string sel_text, uint8_t* input_var, uint8_t button_var);
void uibox_add_element_button_string(UIBOX_INFO* uibox, uint16_t x, uint16_t y, int16_t w, int16_t h, std::string text, std::string sel_text, std::string* input_var, std::string button_var);
void uibox_add_element_button_files_goto(UIBOX_INFO* uibox, uint16_t x, uint16_t y, int16_t w, int16_t h, std::string text, std::string* input_var, std::string button_var);
void uibox_add_element_button_files_load(UIBOX_INFO* uibox, uint16_t x, uint16_t y, int16_t w, int16_t h, std::string text, std::string path, std::string file);
void uibox_add_element_button_files_open(UIBOX_INFO* uibox, uint16_t x, uint16_t y, int16_t w, int16_t h, std::string text, std::string file);
void uibox_add_element_button_cube_face(UIBOX_INFO* uibox, uint16_t x, uint16_t y, int16_t w, int16_t h, std::string text, uint16_t file_n, uint16_t faces_n);
void uibox_add_element_toggle(UIBOX_INFO* uibox, uint16_t x, uint16_t y, int16_t w, int16_t h, std::string text, std::string sel_text, bool* input_var);
void uibox_add_element_slider(UIBOX_INFO* uibox, uint16_t x, uint16_t y, std::string text, uint16_t* input_var);
void uibox_add_element_textinput(UIBOX_INFO* uibox, uint16_t x, uint16_t y, std::string text);
void uibox_add_element_numinput(UIBOX_INFO* uibox, uint16_t x, uint16_t y, std::string text);

void uibox_update_element(int16_t uibox_in, int16_t element_in);

struct UIBOX_INFO_COLOR : public UIBOX_INFO {
	uint8_t r1 = 0x00;
	uint8_t r2 = 0x00;
	uint8_t g1 = 0x00;
	uint8_t g2 = 0x00;
	uint8_t b1 = 0x00;
	uint8_t b2 = 0x00;
	uint8_t a1 = 0x00;
	uint8_t a2 = 0x00;

	uint8_t tr1 = 0x00;
	uint8_t tr2 = 0x00;
	uint8_t tg1 = 0x00;
	uint8_t tg2 = 0x00;
	uint8_t tb1 = 0x00;
	uint8_t tb2 = 0x00;
	uint8_t ta1 = 0x00;
	uint8_t ta2 = 0x00;
	uint16_t _ti = 1;

	COLOR col = COL_EMPTY;

	bool _init = 1;
	bool _tb = 0;

	bool color_update = 0;

	bool _test = 1;
	
	void update_elements() override
	{
		//this->element_list.clear();
		if (this->_init)
		{
			uibox_add_element_textbox(this, 2, 2, "R:");
			uibox_add_element_varbox_u8(this, 5, 2, "", &(BRUSH_COLOR.r), 0);

			uibox_add_element_textbox(this, 2, 6, "G:");
			uibox_add_element_varbox_u8(this, 5, 6, "", &(BRUSH_COLOR.g), 0);

			uibox_add_element_textbox(this, 2, 10, "B:");
			uibox_add_element_varbox_u8(this, 5, 10, "", &(BRUSH_COLOR.b), 0);

			uibox_add_element_textbox(this, 2, 14, "A:");
			uibox_add_element_varbox_u8(this, 5, 14, "", &(BRUSH_COLOR.a), 0);

			std::string _sp = "";
			for (int i = 0; i < 16; i++)
			{
				//if (i == 1 || i == 15 || i == 8) _sp = "+"; else _sp = "-";
				switch (i)
				{
				case 0:
					_sp = "0";
					break;

				case 1:
					_sp = "1";
					break;

				case 2:
					_sp = "2";
					break;

				case 3:
					_sp = "3";
					break;

				case 4:
					_sp = "4";
					break;

				case 5:
					_sp = "5";
					break;

				case 6:
					_sp = "6";
					break;

				case 7:
					_sp = "7";
					break;

				case 8:
					_sp = "8";
					break;

				case 9:
					_sp = "9";
					break;

				case 10:
					_sp = "A";
					break;

				case 11:
					_sp = "B";
					break;

				case 12:
					_sp = "C";
					break;

				case 13:
					_sp = "D";
					break;

				case 14:
					_sp = "E";
					break;

				case 15:
					_sp = "F";
					break;
				}
				uibox_add_element_button_color(this, 2 + i, 3, 1, 1, _sp, _sp, &(this->r1), (uint8_t)(i * 17));
				uibox_add_element_button_color(this, 2 + i, 4, 1, 1, _sp, _sp, &(this->r2), (uint8_t)(i * 17));
				uibox_add_element_button_color(this, 2 + i, 7, 1, 1, _sp, _sp, &(this->g1), (uint8_t)(i * 17));
				uibox_add_element_button_color(this, 2 + i, 8, 1, 1, _sp, _sp, &(this->g2), (uint8_t)(i * 17));
				uibox_add_element_button_color(this, 2 + i, 11, 1, 1, _sp, _sp, &(this->b1), (uint8_t)(i * 17));
				uibox_add_element_button_color(this, 2 + i, 12, 1, 1, _sp, _sp, &(this->b2), (uint8_t)(i * 17));
				uibox_add_element_button_color(this, 2 + i, 15, 1, 1, _sp, _sp, &(this->a1), (uint8_t)(i * 17));
				uibox_add_element_button_color(this, 2 + i, 16, 1, 1, _sp, _sp, &(this->a2), (uint8_t)(i * 17));
			}

			r1 = (uint8_t)(HI_NIBBLE(BRUSH_COLOR.r) | (HI_NIBBLE(BRUSH_COLOR.r) >> 4));
			r2 = (uint8_t)(LO_NIBBLE(BRUSH_COLOR.r) | (LO_NIBBLE(BRUSH_COLOR.r) << 4));
			g1 = (uint8_t)(HI_NIBBLE(BRUSH_COLOR.g) | (HI_NIBBLE(BRUSH_COLOR.g) >> 4));
			g2 = (uint8_t)(LO_NIBBLE(BRUSH_COLOR.g) | (LO_NIBBLE(BRUSH_COLOR.g) << 4));
			b1 = (uint8_t)(HI_NIBBLE(BRUSH_COLOR.b) | (HI_NIBBLE(BRUSH_COLOR.b) >> 4));
			b2 = (uint8_t)(LO_NIBBLE(BRUSH_COLOR.b) | (LO_NIBBLE(BRUSH_COLOR.b) << 4));
			a1 = (uint8_t)(HI_NIBBLE(BRUSH_COLOR.a) | (HI_NIBBLE(BRUSH_COLOR.a) >> 4));
			a2 = (uint8_t)(LO_NIBBLE(BRUSH_COLOR.a) | (LO_NIBBLE(BRUSH_COLOR.a) << 4));

			this->_init = 0;
		}
		
		if (col != BRUSH_COLOR)
		{
			col = BRUSH_COLOR;
		}
		else
		if (
			tr1 != r1 ||
			tr2 != r2 ||
			tg1 != g1 ||
			tg2 != g2 ||
			tb1 != b1 ||
			tb2 != b2 ||
			ta1 != a1 ||
			ta2 != a2
			)
		{
			BRUSH_COLOR = COLOR{ (uint8_t)(HI_NIBBLE(r1) | LO_NIBBLE(r2)),(uint8_t)(HI_NIBBLE(g1) | LO_NIBBLE(g2)),(uint8_t)(HI_NIBBLE(b1) | LO_NIBBLE(b2)),(uint8_t)(HI_NIBBLE(a1) | LO_NIBBLE(a2)) };
			col = BRUSH_COLOR;

			tr1 = r1;
			tr2 = r2;
			tg1 = g1;
			tg2 = g2;
			tb1 = b1;
			tb2 = b2;
			ta1 = a1;
			ta2 = a2;
		}
	}

	void drawloop() override
	{
		if (col != BRUSH_COLOR)
		{
			col = BRUSH_COLOR;

			r1 = (uint8_t)(HI_NIBBLE(BRUSH_COLOR.r) | (HI_NIBBLE(BRUSH_COLOR.r) >> 4));
			r2 = (uint8_t)(LO_NIBBLE(BRUSH_COLOR.r) | (LO_NIBBLE(BRUSH_COLOR.r) << 4));
			g1 = (uint8_t)(HI_NIBBLE(BRUSH_COLOR.g) | (HI_NIBBLE(BRUSH_COLOR.g) >> 4));
			g2 = (uint8_t)(LO_NIBBLE(BRUSH_COLOR.g) | (LO_NIBBLE(BRUSH_COLOR.g) << 4));
			b1 = (uint8_t)(HI_NIBBLE(BRUSH_COLOR.b) | (HI_NIBBLE(BRUSH_COLOR.b) >> 4));
			b2 = (uint8_t)(LO_NIBBLE(BRUSH_COLOR.b) | (LO_NIBBLE(BRUSH_COLOR.b) << 4));
			a1 = (uint8_t)(HI_NIBBLE(BRUSH_COLOR.a) | (HI_NIBBLE(BRUSH_COLOR.a) >> 4));
			a2 = (uint8_t)(LO_NIBBLE(BRUSH_COLOR.a) | (LO_NIBBLE(BRUSH_COLOR.a) << 4));

			tr1 = r1;
			tr2 = r2;
			tg1 = g1;
			tg2 = g2;
			tb1 = b1;
			tb2 = b2;
			ta1 = a1;
			ta2 = a2;

			this->update_elements();
		}

		SDL_SetRenderTarget(RENDERER, texture);
		SDL_SetRenderDrawBlendMode(RENDERER, SDL_BLENDMODE_NONE);

		SDL_Rect _trect = { this->w - (FONT_CHRW * 11) ,FONT_CHRH * 2,(FONT_CHRW * 8),(FONT_CHRH * 4) };
		SDL_SetRenderDrawColor(RENDERER, 0,0,0,255);
		SDL_RenderFillRect(RENDERER, &_trect);
		SDL_SetRenderDrawColor(RENDERER, 255, 255, 255, 255);
		SDL_RenderDrawRect(RENDERER, &_trect);
		SDL_RenderDrawLine(RENDERER, _trect.x, _trect.y, _trect.x + _trect.w - 1, _trect.y + _trect.h - 1);
		SDL_SetRenderDrawColor(RENDERER, BRUSH_COLOR.r, BRUSH_COLOR.g, BRUSH_COLOR.b, BRUSH_COLOR.a);
		SDL_SetRenderDrawBlendMode(RENDERER, SDL_BLENDMODE_BLEND);
		SDL_RenderFillRect(RENDERER, &_trect);
		SDL_SetRenderDrawBlendMode(RENDERER, SDL_BLENDMODE_NONE);

		SDL_SetRenderTarget(RENDERER, NULL);
	}
};

struct UIBOX_INFO_FILE_EXPLORER : public UIBOX_INFO {
	void update_elements() override
	{
		if (!this->scroll_element_list.empty() && !this->element_list.empty())
		{
			int _offset = 0;
			for (int i = 0; i < this->scroll_element_list.size(); i++)
			{
				this->element_list.erase(this->element_list.begin() + this->scroll_element_list[i - _offset]);
				++_offset;
			}
		}
		this->scroll_element_list.clear();

		if (this->scroll_element_list_create)
		{
			this->element_list.clear();
			if (CURRENT_PATH.back() != PATH_SEP)
			{
				CURRENT_PATH += PATH_SEP;
				std::filesystem::current_path(CURRENT_PATH);
			}
			std::string _currentpath = CURRENT_PATH;
			std::string _name;
			int _tt = 0;
			for (auto _htap = _currentpath.crbegin(); _htap != _currentpath.crend(); ++_htap)
			{
				if (*_htap == PATH_SEP && _tt)
				{
					uibox_add_element_button_files_goto(this, (_currentpath.size() - _tt) + 2, 2, 0, 1, _name, &CURRENT_PATH, CURRENT_PATH.substr(0, (_currentpath.size() - _tt) + _name.size()));
					_name.clear();
				}
				_tt++;
				_name.insert(_name.begin(), *_htap);
			}

			uibox_add_element_button_files_goto(this, (_currentpath.size() - _tt) + 2, 2, 0, 1, _name, &CURRENT_PATH, CURRENT_PATH.substr(0, (_currentpath.size() - _tt) + _name.size()));
			uibox_add_element_textbox(this, 2, 3, STR_LINEV);
			UPDATE_PATH_FILES();
		}
		//PRINT(this->scroll_element_list_size);
		this->scroll_element_list_size = (uint16_t)PATH_FILES.size();
		//PRINT(this->scroll_element_list_size);
		for (int _file = 0; _file < PATH_FILES.size(); ++_file)
		{
			if (!((_file - this->scroll_y) >= 0 && ((4 + _file) - this->scroll_y) < (this->chr_h - 1))) continue;

			if (PATH_FILES[_file].first.size() > (this->chr_w - 6)) // shorten name if it's wider than uibox
			{
				PATH_FILES[_file].first.erase(this->chr_w - 10);
				PATH_FILES[_file].first += "...";
			}

			if (PATH_FILES[_file].second)
			{
				uibox_add_element_button_files_goto(this, 2, (4 + _file) - this->scroll_y, 0, 1, (_file < (PATH_FILES.size() - 1) ? STR_LINEBL STR_ARWR " " : STR_LINEBL STR_ARWR " ") + PATH_FILES[_file].first + PATH_SEP, &CURRENT_PATH, CURRENT_PATH + PATH_FILES[_file].first);
			}
			else
			{
				//std::string _tstr = PATH_FILES[_file].first.substr(PATH_FILES[_file].first.size() - 3, 3);
				//if (_tstr == "png" || _tstr == "PNG")
				//{
					uibox_add_element_button_files_load(this, 2, (4 + _file) - this->scroll_y, 0, 1, (_file < (PATH_FILES.size() - 1) ? "\xc5 " : "\xc1 ") + PATH_FILES[_file].first, CURRENT_PATH, PATH_FILES[_file].first);
				//}
				//else
					//uibox_add_element_textbox(this, 2, (4 + _file) - this->scroll_y, "\xb3 " + PATH_FILES[_file].first);
			}
			this->scroll_element_list.push_back(this->element_list.size() - 1);
		}
		this->scroll_element_list_create = 0;
	}
};

struct UIBOX_INFO_OPEN_FILES : public UIBOX_INFO {
	void update_elements() override
	{
		this->element_update = 1;
		this->update = 1;

		int _tx, _ty;
		std::string _tchar;
		for (int el = 0; el < this->element_list.size(); el++)
		{
			_tx = this->element_list[el]->x;
			_ty = this->element_list[el]->y;
			_tchar = this->element_list[el]->text;
			for (uint16_t sj = 0; sj < _tchar.size(); sj++)
			{
				if (((sj + (_ty * this->chr_w + _tx)) % this->chr_w) > (this->chr_w - 4)) break;
				uibox_set_char(this, sj + (_ty * this->chr_w + _tx), 32, COL_EMPTY, COL_BGUPDATE, 1);
			}
		}

		this->element_list.clear();

		for (int i = 0; i < FILES.size(); i++)
		{
			uibox_add_element_button_files_open(this, 2, 2 + i, 0, 1, FILES[i]->filename, FILES[i]->filename);
		}
	}
};

struct UIBOX_INFO_FRAMES_LAYERS : public UIBOX_INFO {
	void update_elements() override
	{
		this->element_update = 1;
		this->update = 1;

		int _tx, _ty;
		std::string _tchar;
		for (int el = 0; el < this->element_list.size(); el++)
		{
			_tx = this->element_list[el]->x;
			_ty = this->element_list[el]->y;
			_tchar = this->element_list[el]->text;
			for (uint16_t sj = 0; sj < _tchar.size(); sj++)
			{
				if (((sj + (_ty * this->chr_w + _tx)) % this->chr_w) > (this->chr_w - 4)) break;
				uibox_set_char(this, sj + (_ty * this->chr_w + _tx), 32, COL_EMPTY, COL_BGUPDATE, 1);
			}
		}

		this->element_list.clear();

		for (int f = 0; f < CURRENT_FILE_PTR->frames.size(); f++)
		{
			for (int l= 0; l < CURRENT_FILE_PTR->frames[f]->layers.size(); l++)
			{
				uibox_add_element_textbox(this, 2 + (f*2), 2 + l, (CURRENT_FRAME == f) ? ((CURRENT_LAYER == l) ? "\x08" : "\x09") : "\x07");
			}
		}
	}
};

UIBOX_INFO* uibox_new(uint16_t _x, uint16_t _y, uint16_t _w, uint16_t _h, bool can_grab, std::string title);
UIBOX_INFO_COLOR* uibox_new_color(uint16_t _x, uint16_t _y, uint16_t _w, uint16_t _h, bool can_grab, std::string title);
UIBOX_INFO_FILE_EXPLORER* uibox_new_file_explorer(uint16_t _x, uint16_t _y, uint16_t _w, uint16_t _h, bool can_grab, std::string title);
UIBOX_INFO_OPEN_FILES* uibox_new_open_files(uint16_t _x, uint16_t _y, uint16_t _w, uint16_t _h, bool can_grab, std::string title);
UIBOX_INFO_FRAMES_LAYERS* uibox_new_frames_layers(uint16_t _x, uint16_t _y, uint16_t _w, uint16_t _h, bool can_grab, std::string title);

/*
	Finds UIBOX though the title
	RETURN: ID of that UIBOX or -1 if it does not exist
*/
int16_t uibox_get_uibox_by_title(std::string title);

/*
	Finds ELEMENT though the text
	RETURN: ID of that UIBOX or -1 if it does not exist
*/
int16_t uibox_get_element_by_text(int16_t uibox_in, std::string text);

extern SDL_Texture* UI_TEXTURE_HUEBAR;
extern COLOR* UI_PIXELS_HUEBAR;

  //
 //   ELEMENTS   ///////////////////////////////////////////////// ///////  //////   /////    ////     ///      //       /
//

// UIBOX_ELEMENT_MAIN
/*struct UIBOX_ELEMENT_MAIN {
	bool over = false;
	bool highlight = true;
	bool prev_sel = false;
	bool prev_over = false;
	bool const_update = false;
	
	std::string text = "";
	std::string sel_text = "";
	uint16_t x = 0;
	uint16_t y = 0;
	// for W & H: 0=text size, -1=window size
	int16_t w = 0;
	int16_t h = 0;
	virtual void create(UIBOX_INFO*) = 0;
	virtual void set(void)
	{
		//
	}
	virtual void update(UIBOX_INFO* uibox)
	{
		for (uint16_t iy = 0; iy < h; iy++)
		{
			for (uint16_t ix = 0; ix < w; ix++)
			{
				uibox_set_char(uibox, x + ix + ((y + iy) * uibox->chr_w),
					prev_sel ? (ix < sel_text.size() ? (sel_text.c_str())[ix] : 32) : (ix < text.size() ? (text.c_str())[ix] : 32),
					COL_EMPTY,
					(prev_over || prev_sel) ? COL_ACCENT : COL_BGUPDATE,
					1);
			}
		}
	}
	virtual bool is_sel()
	{
		return false;
	}
};*/

// TEXTBOX
struct UIBOX_ELEMENT_TEXTBOX : public UIBOX_ELEMENT_MAIN {
	void create(UIBOX_INFO* uibox) override
	{
		uibox_set_string(uibox, text, x, y, COL_WHITE, uibox->element_update);
	}
};

// VARBOX
struct UIBOX_ELEMENT_VARBOX : public UIBOX_ELEMENT_MAIN {
	uint16_t* input_var = nullptr;
	uint16_t var = 0;
	void create(UIBOX_INFO* uibox) override
	{
		uibox_set_string(uibox, text, x, y, COL_WHITE, uibox->element_update);
	}
	void update(UIBOX_INFO* uibox) override
	{
		char buffer[16];
		sprintf(buffer, "%i", *input_var);
		std::string var_text(buffer);

		for (uint16_t ix = 0; ix < 5; ix++)
		{
			uibox_set_char(uibox, x + ix + (y * uibox->chr_w),
				ix < var_text.size() ? (var_text.c_str())[ix] : 32,
				COL_EMPTY,
				COL_BGUPDATE,
				1);
		}
	}
	bool is_sel() override
	{
		bool _c = *input_var != var;
		if (_c) var = *input_var;
		return _c;
	}
};

struct UIBOX_ELEMENT_VARBOX_U8 : public UIBOX_ELEMENT_MAIN {
	uint8_t* input_var = nullptr;
	uint8_t var = 0;
	void create(UIBOX_INFO* uibox) override
	{
		uibox_set_string(uibox, text, x, y, COL_WHITE, uibox->element_update);
	}
	void update(UIBOX_INFO* uibox) override
	{
		char buffer[4];
		sprintf(buffer, "%i", *input_var);
		std::string var_text(buffer);
		
		for (uint16_t ix = 0; ix < 5; ix++)
		{
			uibox_set_char(uibox, x + ix + (y * uibox->chr_w),
				ix < var_text.size() ? (var_text.c_str())[ix] : 32,
				COL_EMPTY,
				COL_BGUPDATE,
				1);
		}
	}
	bool is_sel() override
	{
		bool _c = *input_var != var;
		if (_c) var = *input_var;
		return _c;
	}
};

struct UIBOX_ELEMENT_VARBOX_S16 : public UIBOX_ELEMENT_MAIN {
	int16_t* input_var = nullptr;
	int16_t var = 0;
	void create(UIBOX_INFO* uibox) override
	{
		uibox_set_string(uibox, text, x, y, COL_WHITE, uibox->element_update);
	}
	void update(UIBOX_INFO* uibox) override
	{
		char buffer[16];
		sprintf(buffer, "%i", *input_var);
		std::string var_text(buffer);

		for (uint16_t ix = 0; ix < 5; ix++)
		{
			uibox_set_char(uibox, x + ix + (y * uibox->chr_w),
				ix < var_text.size() ? (var_text.c_str())[ix] : 32,
				COL_EMPTY,
				COL_BGUPDATE,
				1);
		}
	}
	bool is_sel() override
	{
		bool _c = *input_var != var;
		if (_c) var = *input_var;
		return _c;
	}
};

struct UIBOX_ELEMENT_VARBOX_F : public UIBOX_ELEMENT_MAIN {
	float* input_var = nullptr;
	float var = 0;
	void create(UIBOX_INFO* uibox) override
	{
		uibox_set_string(uibox, text, x, y, COL_WHITE, uibox->element_update);
	}
	void update(UIBOX_INFO* uibox) override
	{
		char buffer[256];
		sprintf(buffer, "%f", *input_var);
		std::string var_text(buffer);

		for (uint16_t ix = 0; ix < 5; ix++)
		{
			uibox_set_char(uibox, x + ix + (y * uibox->chr_w),
				ix < var_text.size() ? (var_text.c_str())[ix] : 32,
				COL_EMPTY,
				COL_BGUPDATE,
				1);
		}
	}
	bool is_sel() override
	{
		bool _c = *input_var != var;
		if (_c) var = *input_var;
		return _c;
	}
};

// BUTTON
struct UIBOX_ELEMENT_BUTTON : public UIBOX_ELEMENT_MAIN {
	uint16_t* input_var = nullptr;
	uint16_t button_var = 0;
	
	void create(UIBOX_INFO* uibox) override
	{
		uibox_set_string(uibox, text, x, y, COL_WHITE, uibox->element_update);
	}
	void set() override
	{
		*input_var = button_var;
	}
	bool is_sel() override
	{
		return *input_var == button_var;
	}
};

struct UIBOX_ELEMENT_BUTTON_U8 : public UIBOX_ELEMENT_MAIN {
	uint8_t* input_var = nullptr;
	uint8_t button_var = 0;

	void create(UIBOX_INFO* uibox) override
	{
		uibox_set_string(uibox, text, x, y, COL_WHITE, uibox->element_update);
	};
	void set() override
	{
		*input_var = button_var;
	}
	bool is_sel() override
	{
		return *input_var == button_var;
	}
};

struct UIBOX_ELEMENT_BUTTON_COLOR : public UIBOX_ELEMENT_MAIN {
	uint8_t* input_var = nullptr;
	uint8_t button_var = 0;

	void create(UIBOX_INFO* uibox) override
	{
		uibox_set_string(uibox, text, x, y, COL_WHITE, uibox->element_update);
	};
	void set() override
	{
		UIBOX_COLOR->update_elements();
		*input_var = button_var;
	}
	bool is_sel() override
	{
		return *input_var == button_var;
	}
};

struct UIBOX_ELEMENT_BUTTON_STRING : public UIBOX_ELEMENT_MAIN {
	std::string* input_var = nullptr;
	std::string button_var = "";

	void create(UIBOX_INFO* uibox) override
	{
		uibox_set_string(uibox, text, x, y, COL_WHITE, uibox->element_update);
	};
	void set() override
	{
		*input_var = button_var;
	}
	bool is_sel() override
	{
		return *input_var == button_var;
	}
};

struct UIBOX_ELEMENT_BUTTON_FILES_GOTO : public UIBOX_ELEMENT_MAIN {
	std::string* input_var = nullptr;
	std::string button_var = "";

	void create(UIBOX_INFO* uibox) override
	{
		uibox_set_string(uibox, text, x, y, COL_WHITE, uibox->element_update);
	};
	void set() override
	{
		if (MOUSEBUTTON_PRESSED_LEFT)
		{
			BRUSH_UPDATE = 0;
			MOUSEBUTTON_LEFT = 0;
			MOUSEBUTTON_PRESSED_LEFT = 0;

			if (*input_var != button_var)
			{
				//std::cout << *input_var << std::endl;
				*input_var = button_var;
				std::filesystem::current_path(button_var);
				//std::cout << *input_var << std::endl;

				int _tx, _ty;
				std::string _tchar;
				for (int i = 0; i < UIBOX_FILE_EXPLORER->element_list.size(); i++)
				{
					_tx = UIBOX_FILE_EXPLORER->element_list[i]->x;
					_ty = UIBOX_FILE_EXPLORER->element_list[i]->y;
					_tchar = UIBOX_FILE_EXPLORER->element_list[i]->text;
					for (uint16_t j = 0; j < _tchar.size(); j++)
					{
						// clearing the previous elements
						uibox_set_char(UIBOX_FILE_EXPLORER, j + (_ty * UIBOX_FILE_EXPLORER->chr_w + _tx), 32, COL_EMPTY, COL_BGUPDATE, 1);
					}
				}

				UIBOX_FILE_EXPLORER->element_update = true;
				UIBOX_FILE_EXPLORER->update = true;

				UIBOX_FILE_EXPLORER->scroll_x = 0;
				UIBOX_FILE_EXPLORER->scroll_y = 0;
				UIBOX_FILE_EXPLORER->scroll_element_list.clear();
				UIBOX_FILE_EXPLORER->scroll_element_list_create = 1;
				UIBOX_FILE_EXPLORER->scroll_element_list_size = 0;

				UIBOX_FILE_EXPLORER->update_elements();
				//uibox_update_file_explorer();
			}
			UPDATE_PATH_FILES();
		}
	}
	bool is_sel() override
	{
		return *input_var == button_var;
	}
};

struct UIBOX_ELEMENT_BUTTON_FILES_LOAD : public UIBOX_ELEMENT_MAIN {
	std::string file;
	std::string path;

	void create(UIBOX_INFO* uibox) override
	{
		uibox_set_string(uibox, text, x, y, COL_WHITE, uibox->element_update);
	}

	void set() override
	{
		if (MOUSEBUTTON_PRESSED_LEFT)
		{
			BRUSH_UPDATE = 0;
			MOUSEBUTTON_LEFT = 0;
			MOUSEBUTTON_PRESSED_LEFT = 0;

			bool _file_exists = 0;

			for (int i = 0; i < FILES.size(); ++i)
			{
				auto _f = FILES[i];
				if ((_f->path + _f->filename) == (this->path + this->file))
				{
					CANVAS_UPDATE = true;
					CURRENT_FILE = i;
					CURRENT_FILE_PTR = _f;
					CURRENT_FRAME = 0;
					CURRENT_FRAME_PTR = _f->frames[0];
					CURRENT_LAYER = 0;
					CURRENT_LAYER_PTR = _f->frames[0]->layers[0];

					refresh_canvas();

					_file_exists = 1;
					break;
				}
			}

			if (!_file_exists)
			{
				SDL_Surface* _surfload;
				std::string _tstr = file.substr(file.size() - 3, 3);
				if (_tstr == "png" || _tstr == "PNG")
				{
					_surfload = IMG_Load((path + file).c_str());
				}
				else if (_tstr == "gif" || _tstr == "GIF") {
					
					//GIF_Image* gif = GIF_LoadImage((path + file).c_str());
					//_surfload = gif->frames[0]->surface;
					_surfload = IMG_Load((path + file).c_str());
				}
				
				SDL_Surface* _surf = SDL_ConvertSurfaceFormat(_surfload, SDL_PIXELFORMAT_RGBA32, 0);

				CANVAS_W = _surf->w;
				CANVAS_H = _surf->h;
				SDL_FreeSurface(_surfload);
				/*while (LAYERS.size() > 1)
				{
					LAYERS.erase(LAYERS.end());
				}
				CURRENT_LAYER = 0;
				CURRENT_LAYER_PTR = LAYERS[CURRENT_LAYER].pixels.get();

				LAYERS[CURRENT_LAYER].texture = SDL_CreateTextureFromSurface(RENDERER, _surf);
				LAYERS[CURRENT_LAYER].pixels = nullptr;
				LAYERS[CURRENT_LAYER].pixels = std::make_unique<COLOR[]>(CANVAS_W * CANVAS_H);*/

				file_add(path, file, CANVAS_W, CANVAS_H);

				CURRENT_LAYER_PTR->texture = SDL_CreateTextureFromSurface(RENDERER, _surf);

				auto layer_data = (CURRENT_LAYER_PTR->pixels);
				for (int i = 0; i < CANVAS_W * CANVAS_H; i++)
				{

					int bpp = _surf->format->BytesPerPixel;
					uint8_t* p = (uint8_t*)_surf->pixels + i * bpp;

					layer_data[i] = COLOR{ p[0], p[1], p[2], p[3] };
				}

				UIBOX_OPEN_FILES->update_elements();
				//uibox_update_open_files();
			}

			// CENTER CANVAS
			CANVAS_X = (WINDOW_W * 0.5f) - (CANVAS_W * 0.5f);
			CANVAS_Y = (WINDOW_H * 0.5f) - (CANVAS_H * 0.5f);
			CANVAS_X_ANIM = CANVAS_X;
			CANVAS_Y_ANIM = CANVAS_Y;
			CANVAS_W_ANIM = CANVAS_W;
			CANVAS_H_ANIM = CANVAS_H;
			CELL_W_ANIM = CELL_W;
			CELL_H_ANIM = CELL_H;
			CANVAS_ZOOM = 1;
		}
	}
};

struct UIBOX_ELEMENT_BUTTON_FILES_OPEN : public UIBOX_ELEMENT_MAIN {
	std::string file;

	void create(UIBOX_INFO* uibox) override
	{
		uibox_set_string(uibox, text, x, y, COL_WHITE, uibox->element_update);
	}

	void set() override
	{
		if (MOUSEBUTTON_PRESSED_LEFT)
		{
			BRUSH_UPDATE = 0;
			MOUSEBUTTON_LEFT = 0;
			MOUSEBUTTON_PRESSED_LEFT = 0;

			for (int i = 0; i < FILES.size(); ++i)
			{
				auto _f = FILES[i];
				if ((_f->filename) == (this->file))
				{
					CANVAS_UPDATE = true;
					CURRENT_FILE = i;
					CURRENT_FILE_PTR = _f;
					CURRENT_FRAME = 0;
					CURRENT_FRAME_PTR = _f->frames[0];
					CURRENT_LAYER = 0;
					CURRENT_LAYER_PTR = _f->frames[0]->layers[0];

					refresh_canvas();
					break;
				}
			}

			// CENTER CANVAS
			CANVAS_X = (WINDOW_W * 0.5f) - (CANVAS_W * 0.5f);
			CANVAS_Y = (WINDOW_H * 0.5f) - (CANVAS_H * 0.5f);
			CANVAS_X_ANIM = CANVAS_X;
			CANVAS_Y_ANIM = CANVAS_Y;
			CANVAS_W_ANIM = CANVAS_W;
			CANVAS_H_ANIM = CANVAS_H;
			CELL_W_ANIM = CELL_W;
			CELL_H_ANIM = CELL_H;
			CANVAS_ZOOM = 1;
		}
	}
};

struct UIBOX_ELEMENT_BUTTON_CUBE_FACE : public UIBOX_ELEMENT_MAIN {
	uint16_t file_n;
	uint16_t faces_n;

	void create(UIBOX_INFO* uibox) override
	{
		uibox_set_string(uibox, text, x, y, COL_WHITE, uibox->element_update);
	}

	void set() override
	{
		if (MOUSEBUTTON_PRESSED_LEFT)
		{
			auto _tn = FILES[CURRENT_FILE];
			CUBE_VIEW_FACES[faces_n]->file = std::move(_tn);
			CUBE_VIEW_FACES[faces_n]->update = 1;
			CUBE_VIEW_UPDATE();
		}
	}
};

// TOGGLE
struct UIBOX_ELEMENT_TOGGLE : public UIBOX_ELEMENT_MAIN {
	bool* input_var = nullptr;
	void create(UIBOX_INFO* uibox) override
	{
		uibox_set_string(uibox, text, x, y, COL_WHITE, uibox->element_update);
	};
	void set() override
	{
		if (MOUSEBUTTON_LEFT && MOUSEBUTTON_PRESSED_LEFT) ELEMENT_TOGGLE_BOOL = !(*input_var);
		*input_var = ELEMENT_TOGGLE_BOOL;// !(*input_var);
	}
	bool is_sel() override
	{
		return *input_var == true;
	}
};

// SLIDER
struct UIBOX_ELEMENT_SLIDER : public UIBOX_ELEMENT_MAIN {
	void create(UIBOX_INFO*) override {}
};

// TEXTINPUT
struct UIBOX_ELEMENT_TEXTINPUT : public UIBOX_ELEMENT_MAIN {
	void create(UIBOX_INFO*) override {}
};

// NUM_INPUT
struct UIBOX_ELEMENT_NUMINPUT : public UIBOX_ELEMENT_MAIN {
	void create(UIBOX_INFO*) override {}
};