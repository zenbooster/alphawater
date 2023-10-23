#ifndef TGLES2_FNS
#define TGLES2_FNS
#include "common.h"

struct TGles2Fns
{
#define SDL_PROC(ret, func, params) static ret (APIENTRY *func) params;
#include "SDL_gles2funcs.h"
#undef SDL_PROC
	static void load();
};
#endif