
#ifdef __APPLE__
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <gl/GLU.h>
#else
#include <SDL.h>
#include <SDL_opengl.h>
#include <gl/GLU.h>
#endif

#include "CUBE_VIEW.h"
#include "CANVAS.h"
#include "SYSTEM.h"

#include <algorithm>
#include <memory>
#include <filesystem>

SDL_Window* WINDOW_CUBE_VIEW;
SDL_GLContext CUBE_CONTEXT;

std::vector<std::shared_ptr<CUBE_FACE>> CUBE_VIEW_FACES;

SDL_Surface* CUBE_VIEW_NET = nullptr;
int CUBE_VIEW_S = -1;

CUBE_FACE CUBE_FILE_FRONT;
CUBE_FACE CUBE_FILE_RIGHT;
CUBE_FACE CUBE_FILE_BACK;
CUBE_FACE CUBE_FILE_LEFT;
CUBE_FACE CUBE_FILE_TOP;
CUBE_FACE CUBE_FILE_BOTTOM;

void CUBE_VIEW_INIT()
{
	static bool _cube_init = 1;

	if (_cube_init) // don't want to accidentally init more than once
	{
		for (int i = 0; i < 6; i++)
		{
			std::shared_ptr<CUBE_FACE> _f = std::make_shared<CUBE_FACE>();

			_f->update = 1;
			_f->file = nullptr;
			_f->surf = nullptr;// SDL_CreateRGBSurfaceWithFormat(0, 1, 1, 32, SDL_PIXELFORMAT_RGBA32);

			CUBE_VIEW_FACES.push_back(std::move(_f));
		}
	}
}

void CUBE_VIEW_CREATE()
{
	static bool _cube_init = 1;

	static float color[8][3] = {
		{1.0, 1.0, 0.0},
		{1.0, 0.0, 0.0},
		{0.0, 0.0, 0.0},
		{0.0, 1.0, 0.0},
		{0.0, 1.0, 1.0},
		{1.0, 1.0, 1.0},
		{1.0, 0.0, 1.0},
		{0.0, 0.0, 1.0}
	};

	static float cubex = 0.0;
	static float cubey = 0.0;
	static float cubez = 0.0;

	static float cube[8][3] = {
		{cubex + 0.5,   cubey + 0.5,    cubez + -0.5},
		{cubex + 0.5,   cubey + -0.5,   cubez + -0.5},
		{cubex + -0.5,  cubey + -0.5,   cubez + -0.5},
		{cubex + -0.5,  cubey + 0.5,    cubez + -0.5},
		{cubex + -0.5,  cubey + 0.5,    cubez + 0.5},
		{cubex + 0.5,   cubey + 0.5,    cubez + 0.5},
		{cubex + 0.5,   cubey + -0.5,   cubez + 0.5},
		{cubex + -0.5,  cubey + -0.5,   cubez + 0.5}
	};

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (CUBE_VIEW_NET != nullptr)
	{
		glEnable(GL_TEXTURE_2D);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, CUBE_VIEW_NET->w, CUBE_VIEW_NET->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, CUBE_VIEW_NET->pixels);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	}
	
	glBegin(GL_QUADS);
	#pragma region CUBE
	// BACK
	glTexCoord2d(1.0 / 6.0 * 2.0, 0.0);
	glVertex3fv(cube[0]);
	glTexCoord2d(1.0 / 6.0 * 2.0, 1.0);
	glVertex3fv(cube[1]);
	glTexCoord2d(1.0 / 6.0 * 3.0, 1.0);
	glVertex3fv(cube[2]);
	glTexCoord2d(1.0 / 6.0 * 3.0, 0.0);
	glVertex3fv(cube[3]);

	// LEFT
	glTexCoord2d(1.0 / 6.0 * 3.0, 0.0);
	glVertex3fv(cube[3]);
	glTexCoord2d(1.0 / 6.0 * 4.0, 0.0);
	glVertex3fv(cube[4]);
	glTexCoord2d(1.0 / 6.0 * 4.0, 1.0);
	glVertex3fv(cube[7]);
	glTexCoord2d(1.0 / 6.0 * 3.0, 1.0);
	glVertex3fv(cube[2]);

	// RIGHT
	glTexCoord2d(1.0 / 6.0 * 2.0, 0.0);
	glVertex3fv(cube[0]);
	glTexCoord2d(1.0 / 6.0, 0.0);
	glVertex3fv(cube[5]);
	glTexCoord2d(1.0 / 6.0, 1.0);
	glVertex3fv(cube[6]);
	glTexCoord2d(1.0 / 6.0 * 2.0, 1.0);
	glVertex3fv(cube[1]);

	// FRONT
	glTexCoord2d(1.0 / 6.0, 0.0);
	glVertex3fv(cube[5]);
	glTexCoord2d(0.0, 0.0);
	glVertex3fv(cube[4]);
	glTexCoord2d(0.0, 1.0);
	glVertex3fv(cube[7]);
	glTexCoord2d(1.0 / 6.0, 1.0);
	glVertex3fv(cube[6]);

	// TOP
	glTexCoord2d(1.0 / 6 * 5.0, 1.0);
	glVertex3fv(cube[5]);
	glTexCoord2d(1.0 / 6.0 * 5.0, 0.0);
	glVertex3fv(cube[0]);
	glTexCoord2d(1.0 / 6.0 * 4.0, 0.0);
	glVertex3fv(cube[3]);
	glTexCoord2d(1.0 / 6.0 * 4.0, 1.0);
	glVertex3fv(cube[4]);

	// BOTTOM
	glTexCoord2d(1.0, 0.0);
	glVertex3fv(cube[6]);
	glTexCoord2d(1.0, 1.0);
	glVertex3fv(cube[1]);
	glTexCoord2d(1.0 / 6.0 * 5.0, 1.0);
	glVertex3fv(cube[2]);
	glTexCoord2d(1.0 / 6.0 * 5.0, 0.0);
	glVertex3fv(cube[7]);
	#pragma endregion
	glEnd();

	glMatrixMode(GL_MODELVIEW);
	if (_cube_init)
	{
		glTranslatef(0.0, 0.0, -2.0);
		glRotatef(30, 1.0, 0.0, 0.0);
	}
	glRotatef(5.0 / 4, 0.0, 1.0, 0.0);

	_cube_init = 0;

}

void CUBE_VIEW_UPDATE()
{
	int _tcw, _tch;
	for (int f = 0; f < 6; f++)
	{
		if (CUBE_VIEW_FACES[f]->update && CUBE_VIEW_FACES[f]->file != nullptr)
		{
			CUBE_VIEW_FACES[f]->update = 0;
			_tcw = CUBE_VIEW_FACES[f]->file->canvas_w;
			_tch = CUBE_VIEW_FACES[f]->file->canvas_h;
			
			if (_tcw != _tch) continue;

			if (CUBE_VIEW_S == -1)
			{
				if (CUBE_VIEW_NET == nullptr) CUBE_VIEW_NET = SDL_CreateRGBSurfaceWithFormat(0, _tcw * 6, _tcw, 32, SDL_PIXELFORMAT_RGBA32);
				CUBE_VIEW_S = _tcw;
			}
			else
			{
				if (CUBE_VIEW_S != _tcw)
				{
					if (CUBE_VIEW_NET != nullptr) SDL_FreeSurface(CUBE_VIEW_NET);
					CUBE_VIEW_NET = nullptr;
					CUBE_VIEW_S = -1;
					break;
				}
			}

			SDL_Surface* _tsurf = SDL_CreateRGBSurfaceWithFormat(0, _tcw, _tch, 32, SDL_PIXELFORMAT_RGBA32);

			// THIS SHOULD BE ITS OWN FUNCTION "FLATTEN IMAGE"
			for (int l = 0; l < CUBE_VIEW_FACES[f]->file->frames[0]->layers.size(); l++)
			{
				auto _layerpixels = CUBE_VIEW_FACES[f]->file->frames[0]->layers[l]->pixels;
				for (int p = 0; p < _tcw * _tch; p++)
				{
					if (!l)
					{
						((COLOR*)_tsurf->pixels)[p] = _layerpixels[p];
					}
					else {
						const COLOR new_col = blend_colors(_layerpixels[p], ((COLOR*)_tsurf->pixels)[p]);
						((COLOR*)_tsurf->pixels)[p] = new_col;
					}
				}
			}
			QUAD _tquad = QUAD::from_xywh(f * _tcw, 0, _tcw, _tch);
			SDL_BlitSurface(_tsurf, NULL, CUBE_VIEW_NET, &(_tquad.to_sdl()));
			//IMG_SavePNG(CUBE_VIEW_NET, (std::filesystem::current_path().string() + PATH_SEP + "cubenet.png").c_str());
			SDL_FreeSurface(_tsurf);
		}
	}
}

void CUBE_VIEW_RENDER()
{
	static int _update = 0;
	static int _update_n = 0;

	if (_update >= 2)
	{
		CUBE_VIEW_FACES[_update_n]->update = 1;
		CUBE_VIEW_UPDATE();
		_update = 0;
		_update_n = ((_update_n + 1) % 6);
	}
	++_update;

	int w, h;
	SDL_GL_MakeCurrent(WINDOW_CUBE_VIEW, CUBE_CONTEXT);
	SDL_GetWindowSize(WINDOW_CUBE_VIEW, &w, &h);
	SDL_SetWindowSize(WINDOW_CUBE_VIEW, (w + h + std::min(w, h)) / 3, (w + h + std::min(w, h)) / 3);
	glViewport(0, 0, w, h);
	CUBE_VIEW_CREATE();
	SDL_GL_SwapWindow(WINDOW_CUBE_VIEW);
}
