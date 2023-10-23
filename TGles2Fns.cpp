#include "TGles2Fns.h"

#define SDL_PROC(ret, func, params) ret (APIENTRY *TGles2Fns::func) params;
#include "SDL_gles2funcs.h"
#undef SDL_PROC

void TGles2Fns::load()
{
#ifdef SDL_VIDEO_DRIVER_UIKIT
#define __SDL_NOGETPROCADDR__
#elif defined(SDL_VIDEO_DRIVER_ANDROID)
#define __SDL_NOGETPROCADDR__
#endif

#if defined __SDL_NOGETPROCADDR__
#define SDL_PROC(ret, func, params) TGles2Fns::func = func;
#else
#define SDL_PROC(ret, func, params)                                                            \
    do {                                                                                       \
        TGles2Fns::func = (ret (APIENTRY *) params)SDL_GL_GetProcAddress(#func);                    \
        if (!TGles2Fns::func) {                                                                     \
            throw SDL_SetError("Couldn't load GLES2 function %s: %s", #func, SDL_GetError()); \
        }                                                                                      \
    } while (0);
#endif /* __SDL_NOGETPROCADDR__ */

#include "SDL_gles2funcs.h"
#undef SDL_PROC
}