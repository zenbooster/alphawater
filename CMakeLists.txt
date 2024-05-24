cmake_minimum_required(VERSION 3.5)
project(alphawater)


SET( CMAKE_MODULE_PATH
  ${CMAKE_CURRENT_SOURCE_DIR}/CMake/Packages )

find_package(OpenGL REQUIRED)
find_package(log4cplus REQUIRED)

add_subdirectory(glfw)
include_directories(glfw/include)
include_directories(glm)
include_directories(glad/include)
include_directories(${LOG4CPLUS_INC})
include_directories(stb)
include_directories(fast-cpp-csv-parser)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -Wextra -std=c++20 -mwindows -static -static-libgcc -static-libstdc++")

set(SOURCE_FILES main.cpp glad/src/glad.c TMyApp.cpp TMyAppWnd.cpp TMPShader.cpp TFrameBufferPair.cpp framebuffer.cpp shader.cpp shaderprogram.cpp shaders.cpp vertexarrayobject.cpp bufferobject.cpp texture.cpp filesystem.cpp common.cpp)
add_executable(alphawater ${SOURCE_FILES})

target_link_libraries(alphawater glfw
                      ${GLFW_LIBRARIES} ${GLAD_LIBRARIES} ${LOG4CPLUS_LIB} ws2_32)