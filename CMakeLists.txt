cmake_minimum_required(VERSION 3.22)

# Project configuration.
project(
    arcanoid
    VERSION 0.1.0
    DESCRIPTION "A simple 2D game based on the simple custom microengine"
    LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message("=== RELEASE MODE ===")
    add_definitions("-DRELEASE")
    add_definitions("-O2")
else()
    message("=== DEBUG MODE ===")
    add_definitions("-DDEBUG")
endif()

# CMake stuff.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")

# Check if `fmt` git submodule is initialized.
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/submodules/fmt)
    message(
        FATAL_ERROR
            "Submodule `fmt` is not initialized. "
            "Make sure you've cloned this repo with '--recurse-submodules' option."
    )
endif()

# Check if `SDL` git submodule is initialized.
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/submodules/SDL)
    message(
        FATAL_ERROR
            "Submodule `SDL` is not initialized. "
            "Make sure you've cloned this repo with '--recurse-submodules' option."
    )
endif()

# Check if `Glm` git submodule is initialized.
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/submodules/glm)
    message(
        FATAL_ERROR
            "Submodule `glm` is not initialized. "
            "Make sure you've cloned this repo with '--recurse-submodules' option."
    )
endif()

# Submodules.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/submodules/fmt
                 ${CMAKE_BINARY_DIR}/fmt)

if(NOT ${CMAKE_SYSTEM_NAME} STREQUAL "Android")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/submodules/SDL
                     ${CMAKE_BINARY_DIR}/SDL)
else()
    add_subdirectory(
        # Specifies the directory for SDL3 root CMakeLists.txt.
        ${CMAKE_CURRENT_SOURCE_DIR}/submodules/SDL
        # Specifies the directory for the build outputs.
        ${CMAKE_CURRENT_BINARY_DIR}/SDL_${ANDROID_ABI})
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/submodules/glm
                 ${CMAKE_BINARY_DIR}/glm)

# Official CMake doc doesn't recommend to use GLOB. Check this:
# https://cmake.org/cmake/help/latest/command/file.html
list(
    APPEND
    APP_SOURCES
    src/component.cxx
    src/entity.cxx
    src/game-system.cxx
    src/game.cxx
    src/coordinator.cxx
    src/main.cxx)

# Game.
if(${CMAKE_SYSTEM_NAME} STREQUAL "Android")
    add_library(arcanoid SHARED ${APP_SOURCES})
else()
    add_executable(arcanoid ${APP_SOURCES})
endif()

target_compile_options(
    arcanoid
    PRIVATE
        "$<$<CXX_COMPILER_ID:Clang,AppleClang,GNU>:-Wall;-Wextra;-Wpedantic;-Werror>"
)
target_compile_features(arcanoid PRIVATE cxx_std_17)

include(AddModule)
add_module(arcanoid engine ${PROJECT_SOURCE_DIR}/engine)

if(WIN32)
    message("===== Copying SDL lib to arcanoid exe folder =====")
    add_custom_command(
        TARGET arcanoid
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:SDL3::SDL3-shared> $<TARGET_FILE_DIR:arcanoid>)
endif()

# Resources.
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/res")
