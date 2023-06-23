cmake_minimum_required(VERSION 3.22)
project(imgui)

# Official CMake doc doesn't recommend to use GLOB. Check this:
# https://cmake.org/cmake/help/latest/command/include_directories.html
list(
    APPEND
    IMGUI_SOURCES
    src/imgui_demo.cpp
    src/imgui_draw.cpp
    src/imgui_tables.cpp
    src/imgui_widgets.cpp
    src/imgui.cpp)

add_library(imgui STATIC ${IMGUI_SOURCES})
target_link_libraries(imgui PRIVATE SDL3::SDL3-shared)

# Prefer to use `target_include_directories()` instead of
# `include_directories()`. Check this:
# https://cmake.org/cmake/help/latest/command/include_directories.html

# We propagate (PUBLIC) `imgui` include directory cause we are going to use it
# not only in engine.
target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
set_property(TARGET imgui PROPERTY POSITION_INDEPENDENT_CODE ON)
