cmake_minimum_required(VERSION 3.7)

project(GBEmulator)

set(CMAKE_CXX_STANDARD 20)
set(OpenGL_GL_PREFERENCE GLVND)

find_package(SDL3 REQUIRED)
find_package(OpenGL REQUIRED)
include_directories(${SDL3_INCLUDE_DIRS})
include_directories(inc)

file(GLOB_RECURSE SOURCES src/*.cpp)

add_executable(GBEmulator ${SOURCES} ${IMGUI_SRC})
target_compile_options(GBEmulator PRIVATE -Wall -Wextra -Wpedantic)

add_library(imgui
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/backends/imgui_impl_sdl3.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui SYSTEM PUBLIC
    external/imgui
    external/imgui/backends
)

target_compile_options(imgui PRIVATE -w)

target_link_libraries(GBEmulator ${SDL3_LIBRARIES} OpenGL::GL imgui)
