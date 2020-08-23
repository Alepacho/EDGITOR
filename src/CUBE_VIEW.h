#pragma once

#ifdef __APPLE__
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <gl/GLU.h>
#else
#include <SDL.h>
#include <SDL_opengl.h>
#include <gl/GLU.h>
#endif

#include "CANVAS.h"
#include <vector>
#include <memory>

extern SDL_Window* WINDOW_CUBE_VIEW;
extern SDL_GLContext CUBE_CONTEXT;

struct CUBE_FACE {
	bool update = 1;
	std::shared_ptr<FILE_INFO> file;
	SDL_Surface* surf;
};
extern std::vector<std::shared_ptr<CUBE_FACE>> CUBE_VIEW_FACES;

extern SDL_Surface* CUBE_VIEW_NET;
extern int CUBE_VIEW_S;

extern void CUBE_VIEW_INIT();
extern void CUBE_VIEW_CREATE();
extern void CUBE_VIEW_UPDATE();
extern void CUBE_VIEW_RENDER();