// manpat: really not a fan of this - this should be cmakes job :(
#ifdef __APPLE__
#include <SDL2/SDL.h>
#include <SDL2_image/SDL_image.h>
#include <SDL2_ttf/SDL_ttf.h>
#else
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
//#include "SDL_opengl.h"
//#include <gl/GLU.h>
#endif

#include "UI_CONTROL.h"
#include "VARIABLES.h"
#include "FUNCTIONS.h"
#include "CANVAS.h"
#include "BRUSH.h"
#include "UNDO.h"
#include "COLOR.h"
#include "QUAD.h"
#include "CUBE_VIEW.h"

#include <algorithm>
#include <filesystem>
#include <chrono>

// NOTE(manpat): This is not strictly correct but is the best I can do for now.
// The correct solution would be to use std::filesystem::path everywhere instead of doing path stuff
// manually, but that's a much larger job than I have time for atm
char PATH_SEP = static_cast<char>(std::filesystem::path::preferred_separator);

UIBOX_INFO* UIBOX_TOOLS;
UIBOX_INFO* UIBOX_COLOR;
UIBOX_INFO* UIBOX_BRUSH;
UIBOX_INFO* UIBOX_FILE_EXPLORER;
UIBOX_INFO* UIBOX_OPEN_FILES;
UIBOX_INFO* UIBOX_CANVAS;
UIBOX_INFO* UIBOX_FRAMES_LAYERS;
UIBOX_INFO* UIBOX_CUBE_VIEW;

  //
 //   INITIALISATION   ///////////////////////////////////////////////// ///////  //////   /////    ////     ///      //       /
//

void PRINT(std::string print)
{
	std::cout << print << std::endl;
}

void PRINT(const char* print)
{
	std::cout << print << std::endl;
}

void PRINT(int print)
{
	std::cout << print << std::endl;
}

uint16_t heptrand_seed = (uint16_t)rand();

uint16_t heptrand() {
	heptrand_seed = (0x7777 * heptrand_seed ^ 0x7777);
	return (heptrand_seed >> 0x0007) & 0x7777;
}

uint16_t heptrand(uint16_t seed) {
	heptrand_seed = seed;
	heptrand_seed = (0x7777 * heptrand_seed ^ 0x7777);
	return (heptrand_seed >> 0x0007) & 0x7777;
}


bool INIT_SDL()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
		PRINT("SDL COULD NOT INITIALISE");
		PRINT(SDL_GetError());
		return false;
	}
	else {
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0")) PRINT("WARNING: SDL_HINT_RENDER_SCALE_QUALITY NOT SET");
		if (!SDL_SetHint(SDL_HINT_WINDOWS_DISABLE_THREAD_NAMING, "1")) PRINT("WARNING: SDL_HINT_WINDOWS_DISABLE_THREAD_NAMING NOT SET");
		if (!SDL_SetHint(SDL_HINT_FRAMEBUFFER_ACCELERATION, "opengl")) PRINT("WARNING: SDL_HINT_FRAMEBUFFER_ACCELERATION NOT SET");
		if (!SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl")) PRINT("WARNING: SDL_HINT_RENDER_DRIVER NOT SET");
		if (!SDL_SetHint(SDL_HINT_VIDEO_WINDOW_SHARE_PIXEL_FORMAT, "1")) PRINT("WARNING: SDL_HINT_VIDEO_WINDOW_SHARE_PIXEL_FORMAT NOT SET");
		if (!SDL_SetHint(SDL_HINT_VIDEO_DOUBLE_BUFFER, "1")) PRINT("WARNING: SDL_HINT_VIDEO_DOUBLE_BUFFER NOT SET");
		if (!SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1")) PRINT("WARNING: SDL_HINT_RENDER_VSYNC NOT SET");

		//Initialize PNG loading
		int imgFlags = IMG_INIT_PNG;
		if (!(IMG_Init(imgFlags) & imgFlags)) {
			PRINT("SDL_IMAGE COULD NOT INITIALISE");
			PRINT(SDL_GetError());
			return false;
		}
		else {
			if (TTF_Init() < 0) {
				PRINT("SDL_TTF COULD NOT INITIALISE");
				PRINT(SDL_GetError());
				return false;
			}
		}
	}

	/*if (success)
	{
		file_add("hello", "test");
		PRINT(FILES.size());
		PRINT(FILES[0]->frames.size());
		PRINT(FILES[0]->frames[0]->layers.size());
	}*/

	return true;
}

SDL_Window* INIT_WINDOW()
{
	auto WINDOW = SDL_CreateWindow("EDGITOR", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 2, 2, SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
	SDL_SetWindowFullscreen(WINDOW, SDL_WINDOW_FULLSCREEN_DESKTOP);
	SDL_GetWindowSize(WINDOW, &DISPLAY_W, &DISPLAY_H);
	SDL_SetWindowFullscreen(WINDOW, 0);

	SDL_SetWindowSize(WINDOW, (int)((float)DISPLAY_W * .9), (int)((float)DISPLAY_H * .9)); // make WINDOW 77.77777% of the display
	SDL_GetWindowSize(WINDOW, &WINDOW_W, &WINDOW_H);
	WINDOW_X = ((DISPLAY_W / 2) - (WINDOW_W / 2));
	WINDOW_Y = ((DISPLAY_H / 2) - (WINDOW_H / 2));
	SDL_SetWindowPosition(WINDOW, WINDOW_X, WINDOW_Y);

	SDL_SetWindowMinimumSize(WINDOW, 1168, 900);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	WINDOW_CUBE_VIEW = SDL_CreateWindow("EDGITOR CUBE VIEW", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 640, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_ALWAYS_ON_TOP);

	CUBE_CONTEXT = SDL_GL_CreateContext(WINDOW_CUBE_VIEW);
	SDL_GL_SetSwapInterval(2);
	gluPerspective(77.7, 16 / 9, 0.01, 100.0);
	glEnable(GL_DEPTH_TEST);

	return WINDOW;
}

SDL_Renderer* INIT_RENDERER(SDL_Window* WINDOW)
{
	RENDERER = SDL_CreateRenderer(WINDOW, -1, SDL_RENDERER_ACCELERATED);

	// BRUSH SURFACE
	BRUSH_PIXELS = std::make_unique<COLOR[]>(CANVAS_W * CANVAS_H);
	BRUSH_TEXTURE = SDL_CreateTexture(RENDERER, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, CANVAS_W, CANVAS_H);
	//SDL_UpdateTexture(CANVAS_TEXTURE, nullptr, &CANVAS_PIXELS[0], (sizeof(uint32_t) * CANVAS_W));
	
	// CENTER CANVAS
	CANVAS_X = (WINDOW_W * 0.5f) - (CANVAS_W * 0.5f);
	CANVAS_Y = (WINDOW_H * 0.5f) - (CANVAS_H * 0.5f);
	CANVAS_X_ANIM = CANVAS_X;
	CANVAS_Y_ANIM = CANVAS_Y;
	CANVAS_W_ANIM = CANVAS_W;
	CANVAS_H_ANIM = CANVAS_H;
	CELL_W_ANIM = CELL_W;
	CELL_H_ANIM = CELL_H;

	// DEFAULT LAYER
	//layer_new(RENDERER, 0, 0, 255, SDL_BLENDMODE_BLEND);
	file_add(std::filesystem::current_path().string() + PATH_SEP, "untitled.png", CANVAS_W, CANVAS_H);

	// TERMINAL FONT
	FONT = TTF_OpenFont("resources/FONT.ttf", 16);
	if (FONT == NULL)
	{
		std::cout << std::endl << "COPY THE RESOURCES FOLDER TO THE BUILD FOLDER" << std::endl << std::endl;
		SDL_SetWindowOpacity(WINDOW, 0.0f);
		while (1)
		{
			//
		}
	}
	int _tfw, _tfh;
	TTF_SizeText(FONT, "A", &_tfw, &_tfh);
	FONT_CHRW = (uint16_t)_tfw;
	FONT_CHRH = (uint16_t)_tfh;

	

	// BRUSH

	// ADD DEFAULT BRUSH
	const int _t_bs = 13;
	BRUSH_W = _t_bs;
	static uint8_t BRUSH_POINTGRID[_t_bs * _t_bs] = {};
	
	BRUSH_POINTGRID[6 * _t_bs + 6] = 1;

	brush_new(BRUSH_POINTGRID, _t_bs);
	brush_set(BRUSH_LIST_POS);

	BRUSH_CURSOR_PIXELS = new COLOR[_t_bs * _t_bs];
	BRUSH_CURSOR_PIXELS_CLEAR = new COLOR[_t_bs * _t_bs];

	for (int i = 0; i < (_t_bs * _t_bs); i++)
	{
		if (BRUSH_POINTGRID[i])
		{
			BRUSH_CURSOR_PIXELS[i] = BRUSH_COLOR;
		}
		else
		{
			BRUSH_CURSOR_PIXELS[i] = COLOR{ 0,0,0,0 };
		}
		BRUSH_CURSOR_PIXELS_CLEAR[i] = COLOR{ 0,0,0,0 };
	}

	// UI
	// HUEBAR
	UI_PIXELS_HUEBAR = new COLOR[16 * 360];
	UI_TEXTURE_HUEBAR = SDL_CreateTexture(RENDERER, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, 16, 360);

	for (int16_t H = 0; H < 360; H++)
	{
		float _tr, _tg, _tb;
		lab2rgb(100.0f - (float)H / 360.0f * 100.0f, 74, -104, _tr, _tg, _tb);

		const uint8_t Rs = (uint8_t)floor(_tr);
		const uint8_t Gs = (uint8_t)floor(_tg);
		const uint8_t Bs = (uint8_t)floor(_tb);

		const COLOR color {Rs, Gs, Bs, 0xff};
		std::fill(&UI_PIXELS_HUEBAR[H * 16], &UI_PIXELS_HUEBAR[(H+1) * 16], color);
	}
	SDL_UpdateTexture(UI_TEXTURE_HUEBAR, nullptr, &UI_PIXELS_HUEBAR[0], sizeof(COLOR) * 16);

	// BOXES
	UIBOX_COLOR = uibox_new_color(0, 9999, 272, 400, 1, "COLOUR");
	UIBOX_BRUSH = uibox_new(9999, 9999, 272, 400, 1, "BRUSH");
	UIBOX_CUBE_VIEW = uibox_new((WINDOW_W / 2) - 256, (WINDOW_H / 2) - 128, 512, 256, 1, "CUBE VIEW");

	uibox_add_element_button_cube_face(UIBOX_CUBE_VIEW, 2, 2, 0, 1, "FRONT FACE", 1, 0);
	uibox_add_element_button_cube_face(UIBOX_CUBE_VIEW, 2, 3, 0, 1, "RIGHT FACE", 1, 1);
	uibox_add_element_button_cube_face(UIBOX_CUBE_VIEW, 2, 4, 0, 1, "BACK FACE", 1, 2);
	uibox_add_element_button_cube_face(UIBOX_CUBE_VIEW, 2, 5, 0, 1, "LEFT FACE", 1, 3);
	uibox_add_element_button_cube_face(UIBOX_CUBE_VIEW, 2, 6, 0, 1, "TOP FACE", 1, 4);
	uibox_add_element_button_cube_face(UIBOX_CUBE_VIEW, 2, 7, 0, 1, "BOTTOM FACE", 1, 5);

	//uibox_add_element_var

	//UIBOX_FILE_EXPLORER = uibox_new((WINDOW_W / 2) - 320, (WINDOW_H / 2) - 240, 640, 480, 1, "FILE EXPLORER");
	UIBOX_FILE_EXPLORER = uibox_new_file_explorer(640 + FONT_CHRW*2, 0, 512, 500, 0, "FILE EXPLORER");
	UIBOX_FILE_EXPLORER->snap = 1;
	UIBOX_FILE_EXPLORER->can_scroll = true;
	uibox_shrink(UIBOX_FILE_EXPLORER, 1);
	UIBOX_OPEN_FILES = uibox_new_open_files(128 + FONT_CHRW, 0, 512, 500, 0, "OPEN FILES");
	UIBOX_OPEN_FILES->snap = 1;
	uibox_shrink(UIBOX_OPEN_FILES, 1);

	//UIBOX_FRAMES_LAYERS;// = uibox_new((WINDOW_W / 2) - 256, 9999, 512, 256, 1, "FRAMES & LAYERS");
	UIBOX_FRAMES_LAYERS = uibox_new_frames_layers((WINDOW_W / 2) - 304, 9999, 608, 256, 1, "FRAMES & LAYERS");

	for (int i = 0; i < BRUSH_W * BRUSH_W; i++)
	{
		uibox_add_element_toggle(UIBOX_BRUSH, 4 + ((i % BRUSH_W) * 2), 2 + (i / BRUSH_W), 2, 1,
			((i == (BRUSH_W * BRUSH_W) / 2) ? "><" : "  "), "\xb1\xb1", (bool*)&(BRUSH_LIST[BRUSH_LIST_POS]->alpha[i]));
	}

	// ADD BORDER TO BRUSH UIBOX
	int _tbrush_w = BRUSH_W * 2 + 2;
	for (int j = 0; j < BRUSH_W + 2; j++)
	{
		for (int i = 0; i < _tbrush_w; i++)
		{
			if ((j > 0 && j < BRUSH_W + 1) && (i > 0 && i < _tbrush_w - 1))
			{
				continue;
			}
			uibox_set_char(UIBOX_BRUSH, (j+1) * UIBOX_BRUSH->chr_w + (i+3),
				(j == 0) ? ((i == 0) ? CHAR_LINETL : ((i == _tbrush_w - 1) ? CHAR_LINETR : ((i == _tbrush_w / 2 - 1 || i == _tbrush_w / 2) ? CHAR_LINEHD : CHAR_LINEH))) :
				((j == BRUSH_W + 1) ? ((i == 0) ? CHAR_LINEBL : ((i == _tbrush_w - 1) ? CHAR_LINEBR : ((i == _tbrush_w / 2 - 1 || i == _tbrush_w / 2) ? CHAR_LINEHU : CHAR_LINEH))) :
				((i == 0 && j == BRUSH_W / 2 + 1) ? CHAR_LINEVR2 : ((i == _tbrush_w - 1 && j == BRUSH_W / 2 + 1) ? CHAR_LINEVL2 : CHAR_LINEV))), COL_EMPTY, COL_EMPTY, 1);
		}
	}
	uibox_add_element_textbox(UIBOX_BRUSH, 2, BRUSH_W + 3, "SCATTER X:");
	uibox_add_element_varbox(UIBOX_BRUSH, 13, BRUSH_W + 3, "", &BRUSH_SCATTER_X, 0);
	uibox_add_element_textbox(UIBOX_BRUSH, 2, BRUSH_W + 6, "SCATTER Y:");
	uibox_add_element_varbox(UIBOX_BRUSH, 13, BRUSH_W + 6, "", &BRUSH_SCATTER_Y, 0);
	uibox_add_element_textbox(UIBOX_BRUSH, 2, BRUSH_W + 9, "SCATTER N:");
	uibox_add_element_varbox(UIBOX_BRUSH, 13, BRUSH_W + 9, "", &BRUSH_SCATTER_N, 0);

	std::string _sp = "";
	for (int i = 0; i < 29; i++)
	{
		if (i == 1 || i == 28) _sp = "+"; else _sp = "-";
		uibox_add_element_button(UIBOX_BRUSH, 2 + i, BRUSH_W + 4, 0, 1, _sp, "\xfe", &BRUSH_SCATTER_X, i);
		uibox_add_element_button(UIBOX_BRUSH, 2 + i, BRUSH_W + 7, 0, 1, _sp, "\xfe", &BRUSH_SCATTER_Y, i);
		if (i>0) uibox_add_element_button(UIBOX_BRUSH, 2 + (i-1), BRUSH_W + 10, 0, 1, _sp, "\xfe", &BRUSH_SCATTER_N, i);
	}

	UIBOX_TOOLS = uibox_new(0, 0, 128, 500, 0, "TOOLS");
	UIBOX_TOOLS->snap = 1;
	
	int row = 2;

	uibox_add_element_button(UIBOX_TOOLS, 0, row, -1, 1, "BRUSH", "> BRUSH", &CURRENT_TOOL, TOOL::BRUSH);
	row++;
	uibox_add_element_button(UIBOX_TOOLS, 0, row, -1, 1, "ERASER", "> ERASER", &CURRENT_TOOL, TOOL::ERASER);
	row++;
	uibox_add_element_button(UIBOX_TOOLS, 0, row, -1, 1, "PICKER", "> PICKER", &CURRENT_TOOL, TOOL::PICKER);
	row++;
	uibox_add_element_button(UIBOX_TOOLS, 0, row, -1, 1, "FILL", "> FILL", &CURRENT_TOOL, TOOL::FILL);
	row++;
	uibox_add_element_button(UIBOX_TOOLS, 0, row, -1, 1, "CANVAS", "> CANVAS", &CURRENT_TOOL, TOOL::CANVAS);
	row += 2;

	uibox_add_element_textbox(UIBOX_TOOLS, 2, row, "MOUSE:");
	uibox_add_element_textbox(UIBOX_TOOLS, 2, row+1, "X ");
	uibox_add_element_varbox_s16(UIBOX_TOOLS, 4, row+1, "", &CANVAS_MOUSE_X, 0);
	uibox_add_element_textbox(UIBOX_TOOLS, 2, row+2, "Y ");
	uibox_add_element_varbox_s16(UIBOX_TOOLS, 4, row+2, "", &CANVAS_MOUSE_Y, 0);
	row += 3;

	uibox_add_element_textbox(UIBOX_TOOLS, 2, row, "CELL X ");
	uibox_add_element_varbox_s16(UIBOX_TOOLS, 9, row, "", &CANVAS_MOUSE_CELL_X, 0);
	row++;

	uibox_add_element_textbox(UIBOX_TOOLS, 2, row, "CELL Y ");
	uibox_add_element_varbox_s16(UIBOX_TOOLS, 9, row, "", &CANVAS_MOUSE_CELL_Y, 0);
	row += 2;

	uibox_add_element_textbox(UIBOX_TOOLS, 2, row, "CANVAS:");
	uibox_add_element_textbox(UIBOX_TOOLS, 2, row+1, "W ");
	uibox_add_element_varbox(UIBOX_TOOLS, 4, row+1, "", &CANVAS_W, 0);
	uibox_add_element_textbox(UIBOX_TOOLS, 2, row+2, "H ");
	uibox_add_element_varbox(UIBOX_TOOLS, 4, row+2, "", &CANVAS_H, 0);
	row += 3;

	uibox_add_element_textbox(UIBOX_TOOLS, 2, row, "ZOOM ");
	//uibox_add_element_varbox(UIBOX_TOOLS, 7, row, "", (uint16_t*)(&CANVAS_ZOOM), 0);
	uibox_add_element_varbox_f(UIBOX_TOOLS, 7, row, "", &CANVAS_ZOOM, 0);
	
	// 4-bit chooser
	/*uibox_add_element_textbox(tUIBOX_COLOR, 2, 2, "R:");
	uibox_add_element_varbox_u8(tUIBOX_COLOR, 5, 2, "", &(BRUSH_COLOR.r), 0);

	uibox_add_element_textbox(tUIBOX_COLOR, 2, 6, "G:");
	uibox_add_element_varbox_u8(tUIBOX_COLOR, 5, 6, "", &(BRUSH_COLOR.g), 0);

	uibox_add_element_textbox(tUIBOX_COLOR, 2, 10, "B:");
	uibox_add_element_varbox_u8(tUIBOX_COLOR, 5, 10, "", &(BRUSH_COLOR.b), 0);

	uibox_add_element_textbox(tUIBOX_COLOR, 2, 14, "A:");
	uibox_add_element_varbox_u8(tUIBOX_COLOR, 5, 14, "", &(BRUSH_COLOR.a), 0);

	UIBOX_COLOR = std::move(tUIBOX_COLOR);*/

	//UIBOX_COLOR = std::move(tUIBOX_COLOR);

	for (int i = 0; i < UIBOXES.size(); i++)
	{
		UIBOXES[i]->update_elements();
	}

	SDL_SetCursor(create_system_cursor());

	return RENDERER;
}

SDL_Texture* INIT_FONT()
{
	//static const std::vector<const char*> FONT_CHARS;

	/*for (int i = 0; i < 256; i++)
	{
		SDL_Surface* sur = TTF_RenderUTF8_Solid(FONT, CHAR_BOXTL, SDL_Color{ 255, 255, 255, 255 });
	}*/

	const auto char_map =
	" ☺☻♥♦♣♠•◘○◙♂♀♪♫☼"
	"►◄↕‼¶§▬↨↑↓→←∟↔▲▼"
	" !\"#$%&'()*+,-./"
	"0123456789:;<=>?"
	"@ABCDEFGHIJKLMNO"
	"PQRSTUVWXYZ[\\]^_"
	"`abcdefghijklmno"
	"pqrstuvwxyz{|}~ "
	"ÇüéâäàåçêëèïîìÄÅ"
	"ÉæÆôöòûùÿÖÜ¢£¥₧ƒ"
	"áíóúñÑªº¿⌐¬½¼¡«»"
	"░▒▓│┤╡╢╖╕╣║╗╝╜╛┐"
	"└┴┬├─┼╞╟╚╔╩╦╠═╬╧"
	"╨╤╥╙╘╒╓╫╪┘┌█▄▌▐▀"
	"αßΓπΣσµτΦΘΩδ∞φε∩"
	"≡±≥≤⌠⌡÷≈°∙·√ⁿ²■ ";

	SDL_Surface* sur = TTF_RenderUTF8_Solid(FONT, char_map, SDL_Color{ 255, 255, 255, 255 });
	SDL_Texture* texture = SDL_CreateTextureFromSurface(RENDERER, sur);
	SDL_FreeSurface(sur);

	return texture;
}

void SYSTEM_SHUTDOWN(SDL_Window* WINDOW)
{
	TTF_CloseFont(FONT);
	SDL_DestroyRenderer(RENDERER);
	SDL_DestroyWindow(WINDOW);
	SDL_Quit();
	TTF_Quit();
}


  //
 //   INPUT HANDLING   ///////////////////////////////////////////////// ///////  //////   /////    ////     ///      //       /
//

void SYSTEM_INPUT_UPDATE()
{
	bool _tKEYBOARD_ALT = KEYBOARD_ALT;
	bool _tKEYBOARD_SPACE = KEYBOARD_SPACE;

	MOUSEBUTTON_PRESSED_LEFT = false;
	MOUSEBUTTON_PRESSED_MIDDLE = false;
	MOUSEBUTTON_PRESSED_RIGHT = false;
	KEYBOARD_PRESSED_ALT = false;
	KEYBOARD_PRESSED_CTRL = false;
	KEYBOARD_PRESSED_SHIFT = false;
	KEYBOARD_PRESSED_SPACE = false;
	KEYBOARD_PRESSED_ESC = false;

	MOUSEWHEEL_X = 0;
	MOUSEWHEEL_Y = 0;

	MOUSE_PREVX = MOUSE_X;
	MOUSE_PREVY = MOUSE_Y;

	int DISPLAY_MOUSE_X {}, DISPLAY_MOUSE_Y {};
	SDL_GetGlobalMouseState(&DISPLAY_MOUSE_X, &DISPLAY_MOUSE_Y);
	MOUSE_X = (DISPLAY_MOUSE_X - WINDOW_X);
	MOUSE_Y = (DISPLAY_MOUSE_Y - WINDOW_Y);

	CANVAS_MOUSE_PREVX = CANVAS_MOUSE_X;//((MODE_TILEX) ? ((CANVAS_MOUSE_X + (CELL_W * 10)) % CELL_W) : CANVAS_MOUSE_X);
	CANVAS_MOUSE_PREVY = CANVAS_MOUSE_Y;//((MODE_TILEY) ? ((CANVAS_MOUSE_Y + (CELL_H * 10)) % CELL_H) : CANVAS_MOUSE_Y);
	double t_CANVAS_MOUSE_X = ((double)(((double)MOUSE_X / (double)CANVAS_ZOOM) - ((double)CANVAS_X / (double)CANVAS_ZOOM)));
	double t_CANVAS_MOUSE_Y = ((double)(((double)MOUSE_Y / (double)CANVAS_ZOOM) - ((double)CANVAS_Y / (double)CANVAS_ZOOM)));

	//if (MODE_TILEX) t_CANVAS_MOUSE_X = (((int16_t)floor(t_CANVAS_MOUSE_X) + (CELL_W * 10)) % CELL_W);
	//if (MODE_TILEY) t_CANVAS_MOUSE_Y = (((int16_t)floor(t_CANVAS_MOUSE_Y) + (CELL_H * 10)) % CELL_H);

	CANVAS_MOUSE_X = (int16_t)floor(t_CANVAS_MOUSE_X);
	CANVAS_MOUSE_Y = (int16_t)floor(t_CANVAS_MOUSE_Y);

	CANVAS_MOUSE_CELL_X = (int16_t)floor(t_CANVAS_MOUSE_X / (double)CELL_W);
	CANVAS_MOUSE_CELL_Y = (int16_t)floor(t_CANVAS_MOUSE_Y / (double)CELL_H);

	SDL_Event event {};

	while (SDL_PollEvent(&event))
	{
		if ((SDL_QUIT == event.type) || (SDL_KEYDOWN == event.type && SDL_SCANCODE_ESCAPE == event.key.keysym.scancode))
		{
			QUIT = 1;
			break;
		}

		switch (event.type)
		{
			case SDL_TEXTINPUT:
				if (SDL_strlen(event.text.text) == 0 || event.text.text[0] == '\n') break;
				// add input to text
				if (SDL_strlen(KEY_TEXT) + SDL_strlen(event.text.text) < sizeof(KEY_TEXT))
					SDL_strlcat(KEY_TEXT, event.text.text, sizeof(KEY_TEXT));
				KEY_TEXT_INT = atoi(KEY_TEXT);
				break;

			case SDL_MOUSEBUTTONUP:
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					MOUSEBUTTON_LEFT = false;
					MOUSEBUTTON_PRESSED_LEFT = false;
				}

				if (event.button.button == SDL_BUTTON_MIDDLE)
				{
					MOUSEBUTTON_MIDDLE = false;
					MOUSEBUTTON_PRESSED_MIDDLE = false;
				}

				if (event.button.button == SDL_BUTTON_RIGHT)
				{
					MOUSEBUTTON_RIGHT = false;
					MOUSEBUTTON_PRESSED_RIGHT = false;
				}
				break;

			case SDL_MOUSEBUTTONDOWN:
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					MOUSEBUTTON_LEFT = true;
					MOUSEBUTTON_PRESSED_LEFT = true;
				}

				if (event.button.button == SDL_BUTTON_MIDDLE)
				{
					MOUSEBUTTON_MIDDLE = true;
					MOUSEBUTTON_PRESSED_MIDDLE = true;
				}

				if (event.button.button == SDL_BUTTON_RIGHT)
				{
					MOUSEBUTTON_RIGHT = true;
					MOUSEBUTTON_PRESSED_RIGHT = true;
				}
				break;

			case SDL_MOUSEWHEEL: {
				MOUSEWHEEL_X = (int16_t)event.wheel.x;
				MOUSEWHEEL_Y = (int16_t)event.wheel.y;
				if (UIBOX_IN < 0) {
					float t_CANVAS_ZOOM = CANVAS_ZOOM;
					//CANVAS_ZOOM = clamp(CANVAS_ZOOM + ((float)MOUSEWHEEL_Y * (CANVAS_ZOOM * 0.5f) * 0.5f), 1.0f, 50.0f);
					CANVAS_ZOOM = clamp(CANVAS_ZOOM + (((float)abs(MOUSEWHEEL_Y) +
						truncf(CANVAS_ZOOM / 10.0f) +
						truncf(CANVAS_ZOOM / 20.0f) +
						truncf(CANVAS_ZOOM / 30.0f) +
						truncf(CANVAS_ZOOM / 40.0f) +
						truncf(CANVAS_ZOOM / 50.0f)) * sign((float)MOUSEWHEEL_Y)), 1.0f, 100.0f);

					//CANVAS_ZOOM = floorf((floorf((CANVAS_ZOOM * 0.5f) + 0.5f) * 2.0f) - 0.5f);
					if (t_CANVAS_ZOOM != CANVAS_ZOOM)
					{
						float _mx = (((float)MOUSE_X - (float)CANVAS_X) / (float)t_CANVAS_ZOOM), _my = (((float)MOUSE_Y - (float)CANVAS_Y) / (float)t_CANVAS_ZOOM);

						float _nmx = (((float)MOUSE_X - (float)CANVAS_X) / (float)CANVAS_ZOOM), _nmy = (((float)MOUSE_Y - (float)CANVAS_Y) / (float)CANVAS_ZOOM);
						CANVAS_X += (float)((_nmx - _mx) * CANVAS_ZOOM);
						CANVAS_Y += (float)((_nmy - _my) * CANVAS_ZOOM);
					}
				}
				break;
			}

			case SDL_KEYDOWN: {
				const auto keysym = event.key.keysym;

				if (keysym.sym == SDLK_LALT)
				{
					KEYBOARD_ALT = 1;
					KEYBOARD_PRESSED_ALT = 1;
				} else
				if (keysym.sym == SDLK_LCTRL)
				{
					KEYBOARD_CTRL = 1;
					KEYBOARD_PRESSED_CTRL = 1;
				} else
				if (keysym.sym == SDLK_LSHIFT)
				{
					KEYBOARD_SHIFT = 1;
					KEYBOARD_PRESSED_SHIFT = 1;
				} else
				if (keysym.sym == SDLK_SPACE)
				{
					KEYBOARD_SPACE = 1;
					KEYBOARD_PRESSED_SPACE = 1;
				} else
				if (keysym.sym == SDLK_ESCAPE)
				{
					KEYBOARD_ESC = 1;
					KEYBOARD_PRESSED_ESC = 1;
				}

				if (keysym.mod & KMOD_CTRL) {
					switch (keysym.sym) {
					case SDLK_z: {
						if (keysym.mod & KMOD_SHIFT) {
							// because it's the superior 'redo' shortcut :)
							redo();
						} else {
							undo();
						}
						break;
					}
					case SDLK_y: redo();  break;
					case SDLK_s: {
						SDL_Surface* _tsurf = SDL_CreateRGBSurfaceWithFormat(0, CANVAS_W, CANVAS_H, 32, SDL_PIXELFORMAT_RGBA32);

						auto layer = CURRENT_FRAME_PTR->layers[0] ;
						for (int i = 0; i < CANVAS_W * CANVAS_H; i++)
						{
							((COLOR*)_tsurf->pixels)[i] = layer->pixels[i]; // THERE ISN'T MULTI-LAYER BLENDING YET
						}
						std::string _tpath = CURRENT_FILE_PTR->path + CURRENT_FILE_PTR->filename;// (CURRENT_PATH + CURRENT_FILE);
						IMG_SavePNG(_tsurf, _tpath.c_str());
						SDL_FreeSurface(_tsurf);
						break;
					}
					default: break;
					}
				}
				else if (keysym.mod & KMOD_ALT) {
					switch (keysym.sym) {
					case SDLK_n: {
						CURRENT_FILE_PTR->add_frame(1);
						UIBOX_FRAMES_LAYERS->update_elements();
					}
					}
				}
				else if (keysym.mod & KMOD_SHIFT) {
					switch (keysym.sym) {
					case SDLK_n: {
						CURRENT_FRAME_PTR->add_layer(0, 0, 255, SDL_BLENDMODE_BLEND);
						UIBOX_FRAMES_LAYERS->update_elements();
					}
					}
				}
				else if (keysym.sym == SDLK_BACKSPACE)
				{
					int textlen = SDL_strlen(KEY_TEXT);
					do {
						if (!textlen)
						{
							break;
						}
						if ((KEY_TEXT[textlen - 1] & 0x80) == 0x00)
						{
							/* One byte */
							KEY_TEXT[textlen - 1] = 0x00;
							break;
						}
						if ((KEY_TEXT[textlen - 1] & 0xC0) == 0x80)
						{
							/* Byte from the multibyte sequence */
							KEY_TEXT[textlen - 1] = 0x00;
							textlen--;
						}
						if ((KEY_TEXT[textlen - 1] & 0xC0) == 0xC0)
						{
							/* First byte of multibyte sequence */
							KEY_TEXT[textlen - 1] = 0x00;
							break;
						}
					} while (1);
					KEY_TEXT_INT = atoi(KEY_TEXT);
				}
				else if (keysym.sym == SDLK_b)
				{
					CURRENT_TOOL = TOOL::BRUSH;
					//UIBOX_TOOLS->element_update = 1;
				}
				else if (keysym.sym == SDLK_e)
				{
					CURRENT_TOOL = TOOL::ERASER;
					//UIBOX_TOOLS->element_update = 1;
				}
				else if (keysym.sym == SDLK_p)
				{
					CURRENT_TOOL = TOOL::PICKER;
					//UIBOX_TOOLS->element_update = 1;
				}
				else if (keysym.sym == SDLK_f)
				{
					CURRENT_TOOL = TOOL::FILL;
					//UIBOX_TOOLS->element_update = 1;
				}
				else if (keysym.sym == SDLK_c)
				{
					CURRENT_TOOL = TOOL::CANVAS;
					//UIBOX_TOOLS->element_update = 1;
				}
				else if (keysym.sym == SDLK_UP)
				{
					//CURRENT_LAYER = std::max(0, CURRENT_LAYER - 1);
					if (((int)CURRENT_LAYER - 1) < 0) CURRENT_LAYER = CURRENT_FRAME_PTR->layers.size() - 1; else CURRENT_LAYER--;
					CURRENT_LAYER_PTR = (CURRENT_FRAME_PTR->layers[CURRENT_LAYER]);
					UIBOX_FRAMES_LAYERS->update_elements();

				}
				else if (keysym.sym == SDLK_DOWN)
				{
					//CURRENT_LAYER = std::min((int)(CURRENT_FRAME_PTR->layers.size()-1), CURRENT_LAYER + 1);
					if (++CURRENT_LAYER >= CURRENT_FRAME_PTR->layers.size()) CURRENT_LAYER = 0;
					CURRENT_LAYER_PTR = (CURRENT_FRAME_PTR->layers[CURRENT_LAYER]);
					UIBOX_FRAMES_LAYERS->update_elements();
				}
				else if (keysym.sym == SDLK_LEFT)
				{
					//CURRENT_FRAME = std::max(0, CURRENT_FRAME - 1);
					if (((int)CURRENT_FRAME - 1) < 0) CURRENT_FRAME = CURRENT_FILE_PTR->frames.size() - 1; else CURRENT_FRAME--;
					CURRENT_FRAME_PTR = (CURRENT_FILE_PTR->frames[CURRENT_FRAME]);
					CURRENT_LAYER = clamp(CURRENT_LAYER, 0, CURRENT_FRAME_PTR->layers.size() - 1);
					CURRENT_LAYER_PTR = (CURRENT_FRAME_PTR->layers[CURRENT_LAYER]);
					UIBOX_FRAMES_LAYERS->update_elements();
				}
				else if (keysym.sym == SDLK_RIGHT)
				{
					//CURRENT_FRAME = std::min((int)(CURRENT_FILE_PTR->frames.size() - 1), CURRENT_FRAME + 1);
					if (++CURRENT_FRAME >= CURRENT_FILE_PTR->frames.size()) CURRENT_FRAME = 0;
					CURRENT_FRAME_PTR = (CURRENT_FILE_PTR->frames[CURRENT_FRAME]);
					CURRENT_LAYER = clamp(CURRENT_LAYER, 0, CURRENT_FRAME_PTR->layers.size() - 1);
					CURRENT_LAYER_PTR = (CURRENT_FRAME_PTR->layers[CURRENT_LAYER]);
					UIBOX_FRAMES_LAYERS->update_elements();
				}

				/*
				int16_t tools = uibox_get_uibox_by_title("TOOLS");
				if (tools != -1) {
					int16_t last;
					switch (CURRENT_TOOL) {
						case TOOL::BRUSH: last = uibox_get_element_by_text(tools, "BRUSH"); break;
						case TOOL::ERASER: last = uibox_get_element_by_text(tools, "ERASER"); break;
						case TOOL::FILL: last = uibox_get_element_by_text(tools, "FILL"); break;
						default: last = -1;
					}
					if (last != -1) uibox_update_element(tools, last);
				}*/

				break;
			}

			case SDL_KEYUP: {
				const auto keysym = event.key.keysym;

				if (keysym.sym == SDLK_LALT)
				{
					KEYBOARD_ALT = 0;
					KEYBOARD_PRESSED_ALT = 0;
				}
				else
				if (keysym.sym == SDLK_LCTRL)
				{
					KEYBOARD_CTRL = 0;
					KEYBOARD_PRESSED_CTRL = 0;
				}
				else
				if (keysym.sym == SDLK_LSHIFT)
				{
					KEYBOARD_SHIFT = 0;
					KEYBOARD_PRESSED_SHIFT = 0;
				} else
				if (keysym.sym == SDLK_SPACE)
				{
					KEYBOARD_SPACE = 0;
					KEYBOARD_PRESSED_SPACE = 0;
				} else
				if (keysym.sym == SDLK_ESCAPE)
				{
					KEYBOARD_ESC = 0;
					KEYBOARD_PRESSED_ESC = 0;
				}

				break;
			}
		}
	}

	if (!_tKEYBOARD_ALT && KEYBOARD_ALT && KEYBOARD_PRESSED_ALT && CURRENT_TOOL == TOOL::BRUSH)
	{
		CURRENT_TOOL = TOOL::PICKER;
	}
	else
	if (_tKEYBOARD_ALT && !KEYBOARD_ALT && !KEYBOARD_PRESSED_ALT && CURRENT_TOOL == TOOL::PICKER)
	{
		CURRENT_TOOL = TOOL::BRUSH;
	}

	if (!_tKEYBOARD_SPACE && KEYBOARD_SPACE && KEYBOARD_PRESSED_SPACE && (CURRENT_TOOL == TOOL::BRUSH || CURRENT_TOOL == TOOL::ERASER || CURRENT_TOOL == TOOL::PICKER || CURRENT_TOOL == TOOL::FILL))
	{
		TEMP_CURRENT_TOOL = CURRENT_TOOL;
		CURRENT_TOOL = TOOL::CANVAS;
	}
	else
	if (_tKEYBOARD_SPACE && !KEYBOARD_SPACE && !KEYBOARD_PRESSED_SPACE && CURRENT_TOOL == TOOL::CANVAS)
	{
		CURRENT_TOOL = TEMP_CURRENT_TOOL;
	}

	if (MOUSEBUTTON_MIDDLE)
	{
		if (MOUSEBUTTON_PRESSED_MIDDLE)
		{
			CANVAS_PANX = (float)MOUSE_X - CANVAS_X;
			CANVAS_PANY = (float)MOUSE_Y - CANVAS_Y;
		}
		CANVAS_X = (float)MOUSE_X - CANVAS_PANX;
		CANVAS_Y = (float)MOUSE_Y - CANVAS_PANY;
	}

	if (MOUSEBUTTON_LEFT)
	{
		const bool uibox_claimed_input = SYSTEM_UIBOX_HANDLE_MOUSE_DOWN(MOUSEBUTTON_LEFT, MOUSE_X, MOUSE_Y);
		
		if (!uibox_claimed_input)
		{
			switch (CURRENT_TOOL)
			{
			case TOOL::ERASER:
			case TOOL::BRUSH:
				//if (KEYBOARD_ALT) BRUSH_COLOR = get_pixel_layer(CANVAS_MOUSE_X, CANVAS_MOUSE_Y, CURRENT_LAYER); else

				int16_t mx, my, mpx, mpy;

				mx = abs(abs(CANVAS_MOUSE_X + 160) % 16);
				my = abs(abs(CANVAS_MOUSE_Y + 160) % 16);
				mpx = abs(abs(CANVAS_MOUSE_PREVX + 160) % 16);
				mpy = abs(abs(CANVAS_MOUSE_PREVY + 160) % 16);

				//set_pixel_line(mpx, mpy, mx, my, CURRENT_TOOL ? UNDO_COLOR : BRUSH_COLOR);
				set_pixel_line(CANVAS_MOUSE_PREVX, CANVAS_MOUSE_PREVY, CANVAS_MOUSE_X, CANVAS_MOUSE_Y, CURRENT_TOOL ? UNDO_COLOR : BRUSH_COLOR);
				break;

			case TOOL::PICKER:
				BRUSH_COLOR = get_pixel_layer(CANVAS_MOUSE_X, CANVAS_MOUSE_Y, CURRENT_LAYER);
				//CURRENT_TOOL = TOOL::BRUSH;
				//UIBOX_TOOLS->element_update = 1;
				break;

			case TOOL::FILL:
				//if (MOUSEBUTTON_PRESSED_LEFT)
				floodfill(CANVAS_MOUSE_X, CANVAS_MOUSE_Y, CANVAS_W, CANVAS_H, get_pixel_layer(CANVAS_MOUSE_X, CANVAS_MOUSE_Y, CURRENT_LAYER), BRUSH_COLOR);
				//floodfill_test(CANVAS_MOUSE_X, CANVAS_MOUSE_Y, get_pixel_layer(CANVAS_MOUSE_X, CANVAS_MOUSE_Y, CURRENT_LAYER), BRUSH_COLOR);
				break;

			case TOOL::CANVAS:
				if (MOUSEBUTTON_PRESSED_LEFT)
				{
					CANVAS_PANX = (float)MOUSE_X - CANVAS_X;
					CANVAS_PANY = (float)MOUSE_Y - CANVAS_Y;
				}
				CANVAS_X = (float)MOUSE_X - CANVAS_PANX;
				CANVAS_Y = (float)MOUSE_Y - CANVAS_PANY;
				break;
			}
		}
	}
	else
	{
		SYSTEM_UIBOX_HANDLE_MOUSE_UP();
	}

	if (MOUSEBUTTON_PRESSED_RIGHT)
	{
		// NON-CONTIGUOUS / COLOUR REPLACE
		/*uint32_t _tcol;
		for (int i = 0; i < CANVAS_H; i++)
		{
			for (int j = 0; j < CANVAS_W; j++)
			{
				_tcol = CURRENT_LAYER_PTR[i * CANVAS_W + j];
				if (!_tcol) continue;
				//BRUSH_PIXELS[i * CANVAS_W + j] = CURRENT_LAYER_PTR[i * CANVAS_W + j];
				//BRUSH_PIXELS[i * CANVAS_W + j] = 0xff0000ff;
				set_pixel(j, i, 0xff0000ff);
				BRUSH_UPDATE = true;
				LAYER_UPDATE = 2;
			}
		}*/
		// floodfill((uint16_t)CANVAS_MOUSE_X, (uint16_t)CANVAS_MOUSE_Y, CANVAS_W, CANVAS_H, 0x00000000, 0xff0000ff);
		/*std::thread t1(floodfill, (uint16_t)CANVAS_MOUSE_X-1, (uint16_t)CANVAS_MOUSE_Y, CANVAS_W, CANVAS_H, 0x00000000, 0xff0000ff);
		//std::thread t2(floodfill, (uint16_t)CANVAS_MOUSE_X+1, (uint16_t)CANVAS_MOUSE_Y, CANVAS_W, CANVAS_H, 0x00000000, 0xff0000ff);
		//std::thread t3(floodfill, (uint16_t)CANVAS_MOUSE_X, (uint16_t)CANVAS_MOUSE_Y-1, CANVAS_W, CANVAS_H, 0x00000000, 0xff0000ff);
		//std::thread t4(floodfill, (uint16_t)CANVAS_MOUSE_X, (uint16_t)CANVAS_MOUSE_Y+1, CANVAS_W, CANVAS_H, 0x00000000, 0xff0000ff);
		//t1.join();
		std::cout << t1.joinable() << std::endl;
		std::cout << t1.hardware_concurrency << std::endl;
		t1.join();*/
		//t2.join();
		//t3.join();
		//t4.join();

		//function_undo(1);

	}
}



  //
 //   UPDATING   ///////////////////////////////////////////////// ///////  //////   /////    ////     ///      //       /
//

void SYSTEM_BRUSH_UPDATE()
{
	// UPDATE THE BRUSH TEXTURE PER-CHANGE
			// this is because a complex shape might be drawn in one tick; like floodfill
	if (BRUSH_UPDATE) {
		// make sure the brush_update isn't beyond the canvas
		QUAD const canvas_rect = QUAD::from_wh(CANVAS_W, CANVAS_H);
		QUAD const clipped_brush_update_region = BRUSH_UPDATE_REGION.clip_to(canvas_rect);

		// update the brush texture
		SDL_Rect const brush_update_region_sdl = clipped_brush_update_region.to_sdl();
		int const brush_start_index = clipped_brush_update_region.top * CANVAS_W + clipped_brush_update_region.left;

		SDL_SetTextureBlendMode(BRUSH_TEXTURE, SDL_BLENDMODE_NONE);
		SDL_UpdateTexture(BRUSH_TEXTURE, &brush_update_region_sdl, &BRUSH_PIXELS[brush_start_index], CANVAS_PITCH);

		// the layer updates only when we stop drawing - for performance.
		// so we constantly update the min and max bounds
		LAYER_UPDATE_REGION = LAYER_UPDATE_REGION.include_region(clipped_brush_update_region);

		// reset the brush bounds with every tick
		BRUSH_UPDATE_REGION = QUAD::empty();
	}
}

void SYSTEM_LAYER_UPDATE()
{
	if ((LAYER_UPDATE == 1) && !LAYER_UPDATE_REGION.is_empty())
	{
		UNDO_ENTRY _u{ LAYER_UPDATE_REGION, CURRENT_LAYER, CURRENT_FRAME};

		auto layer_data = (CURRENT_LAYER_PTR->pixels);
		for (auto [_x, _y] : LAYER_UPDATE_REGION) {
			const int _pos = (_y * CANVAS_W + _x);
			const COLOR brush_color = BRUSH_PIXELS[_pos];
			const COLOR dest_color = layer_data[_pos];

			const COLOR empty{ 0, 0, 0, 0 };

			BRUSH_PIXELS[_pos] = empty; // clear the brush pixel

			if (brush_color == empty) // if there's an empty pixel in the brush texture
			{
				_u.set(_x - LAYER_UPDATE_REGION.left, _y - LAYER_UPDATE_REGION.top, dest_color, dest_color);
				continue;
			}

			if (CURRENT_TOOL == 1) // if it's the erase tool
			{
				layer_data[_pos] = empty; // erase the destination pixel
				_u.set(_x - LAYER_UPDATE_REGION.left, _y - LAYER_UPDATE_REGION.top, dest_color, empty);
				continue;
			}

			if (dest_color == empty) // if destination pixel is empty
			{
				layer_data[_pos] = brush_color; // make destination the saved brush pixel
				_u.set(_x - LAYER_UPDATE_REGION.left, _y - LAYER_UPDATE_REGION.top, dest_color, brush_color);
				continue;
			}

			// if it isn't any of those edge cases, we properly mix the colours
			const COLOR new_col = blend_colors(brush_color, dest_color);
			layer_data[_pos] = new_col;
			_u.set(_x - LAYER_UPDATE_REGION.left, _y - LAYER_UPDATE_REGION.top, dest_color, new_col);
		}

		// clear the brush texture (since we made all pixels 0x00000000)
		SDL_Rect const dirty_rect = LAYER_UPDATE_REGION.to_sdl();
		int const dirty_region_start_index = LAYER_UPDATE_REGION.top * CANVAS_W + LAYER_UPDATE_REGION.left;

		SDL_SetTextureBlendMode(BRUSH_TEXTURE, SDL_BLENDMODE_NONE);
		SDL_UpdateTexture(BRUSH_TEXTURE, &dirty_rect, &BRUSH_PIXELS[dirty_region_start_index], CANVAS_PITCH);

		// add the new undo
		push_undo_entry(_u);

		// update the layer we drew to
		SDL_UpdateTexture(CURRENT_LAYER_PTR->texture, &dirty_rect, &layer_data[dirty_region_start_index], CANVAS_PITCH);

		LAYER_UPDATE = 0;
	}
	else
	{
		if (LAYER_UPDATE > 0) LAYER_UPDATE--;

		if (LAYER_UPDATE == 0)
		{
			LAYER_UPDATE_REGION = QUAD::empty();
			LAYER_UPDATE = -1;
		}
	}
}

void SYSTEM_CANVAS_UPDATE()
{
	// CANVAS UPDATE
	if (!CANVAS_UPDATE)
	{
		return;
	}

	if (!UNDO_UPDATE_REGION.is_empty())
	{
		auto const canvas_rect = QUAD::from_wh(CANVAS_W, CANVAS_H);
		auto const sdl_rect = UNDO_UPDATE_REGION.clip_to(canvas_rect).to_sdl();
		auto const layer = CURRENT_LAYER_PTR;
		auto const update_start_index = sdl_rect.y * CANVAS_W + sdl_rect.x;
		UNDO_UPDATE_REGION = QUAD::empty();

		SDL_SetTextureBlendMode(layer->texture, SDL_BLENDMODE_NONE);
		SDL_UpdateTexture(layer->texture, &sdl_rect, &layer->pixels[update_start_index], CANVAS_PITCH);
	}
	else
	{
		for (const auto& layer : CURRENT_FRAME_PTR->layers) {
			SDL_SetTextureBlendMode(layer->texture, SDL_BLENDMODE_NONE);
			SDL_UpdateTexture(layer->texture, nullptr, &layer->pixels[0], CANVAS_PITCH);
		}
	}

	SDL_UpdateTexture(BRUSH_TEXTURE, nullptr, BRUSH_PIXELS.get(), CANVAS_PITCH);
	CANVAS_UPDATE = false;
	UNDO_UPDATE_LAYER = 0;
}

