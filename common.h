#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <fstream>
#include <sstream>
#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

using namespace std;

struct tostringstream
{
    ostringstream oss;

    template <typename T>
    auto & operator << (const T &t) { oss << t; return *this; }
    operator string () const { return oss.str(); }
    void clear () { oss.str(std::string()); }    
};

#endif