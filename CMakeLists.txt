cmake_minimum_required(VERSION 3.5)
project(alphawater)

set(SDL_TEST_LIBRARY OFF)
set(SDL_DISABLE_INSTALL ON)
set(SDL_DISABLE_INSTALL_DOCS ON)
set(SDL_SHARED OFF)
set(SDL_STATIC ON)

add_subdirectory(sdl)
include_directories(sdl/include)
include_directories(glm)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wextra -std=c++11")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -Wextra -std=c++11 -mwindows")

set(SOURCE_FILES main.cpp TGles2Fns.cpp shader.cpp shaderprogram.cpp vertexarrayobject.cpp bufferobject.cpp filesystem.cpp)
add_executable(alphawater ${SOURCE_FILES})

target_link_libraries(alphawater SDL3::SDL3-static)