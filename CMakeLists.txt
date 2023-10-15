cmake_minimum_required(VERSION 3.5)
#set(CMAKE_MAKE_PROGRAM make)
project(alphawater)

find_package(OpenGL REQUIRED)

add_subdirectory(glfw)
include_directories(glfw/include)
include_directories(glm)

# Path to glad directory
set(GLAD_SOURCES_DIR "${PROJECT_SOURCE_DIR}/glad/")
add_subdirectory("${GLAD_SOURCES_DIR}/cmake" glad_cmake)
glad_add_library(glad_gl_core_33 REPRODUCIBLE API gl:core=3.3)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wextra -std=c++11")

set(SOURCE_FILES main.cpp)
add_executable(alphawater ${SOURCE_FILES})

target_link_libraries(alphawater
  PUBLIC
    glad_gl_core_33
    glfw ${GLFW_LIBRARIES} ${OPENGL_LIBRARY}
)