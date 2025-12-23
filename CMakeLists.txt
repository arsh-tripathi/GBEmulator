cmake_minimum_required(VERSION 3.7)

project(GBEmulator)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(OpenGL_GL_PREFERENCE GLVND)

find_package(SDL3 REQUIRED)
find_package(OpenGL REQUIRED)

include_directories(${SDL3_INCLUDE_DIRS})
include_directories(inc)

# ---------------------
# Core Library
# ---------------------
file(GLOB_RECURSE CORE_SOURCES 
    src/cpu/*.cpp
    src/memory/*.cpp
)

add_library(GBCore STATIC ${CORE_SOURCES})

target_include_directories(GBCore
    PUBLIC
        inc/cpu
        inc/memory
        inc/utils
)
target_compile_options(GBCore PRIVATE -Wall -Wextra -Werror -Wpedantic)


# ---------------------
# ImGui Library
# ---------------------
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


# ---------------------
# Emulator executable
# ---------------------
file(GLOB_RECURSE EMU_SOURCES
    src/ppu/*.cpp
)

add_executable(GBEmulator 
    src/main.cpp
    ${EMU_SOURCES}
)

target_include_directories(GBEmulator
    PUBLIC
        inc/ppu   
)

target_compile_options(GBEmulator PRIVATE -Wall -Wextra -Wpedantic)

target_link_libraries(GBEmulator 
    PRIVATE
        SDL3::SDL3
        OpenGL::GL 
        imgui
)

# ---------------------
# Testing
# ---------------------
enable_testing()
add_subdirectory(tests)

