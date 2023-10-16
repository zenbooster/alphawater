cmake_minimum_required(VERSION 3.5)
project(alphawater)

find_package(OpenGL REQUIRED)

add_subdirectory(glfw)
include_directories(glfw/include)
include_directories(glm)
include_directories(glad/include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wextra -std=c++11")

set(SOURCE_FILES main.cpp glad/src/glad.c)
add_executable(alphawater ${SOURCE_FILES})

target_link_libraries(alphawater glfw
                      ${GLFW_LIBRARIES} ${GLAD_LIBRARIES})