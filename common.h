#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <fstream>
#include <sstream>
#include <glad/glad.h>

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/configurator.h>
#include <log4cplus/initializer.h>

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