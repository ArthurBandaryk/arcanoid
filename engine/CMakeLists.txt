cmake_minimum_required(VERSION 3.22)

project(engine)

include(AddModule)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/shaders")

file(GLOB ENGINE_SOURCES imgui_impl/*.cxx src/*.cxx src/common/*.cxx)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Android")
    add_library(engine SHARED ${ENGINE_SOURCES})
else()
    add_library(engine ${ENGINE_SOURCES})
endif()
target_compile_features(engine PRIVATE cxx_std_17)

target_include_directories(engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
                                         ${CMAKE_CURRENT_SOURCE_DIR}/src/common)

target_include_directories(
    engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/imgui_impl
                   ${CMAKE_CURRENT_SOURCE_DIR}/external/stb_image)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Android")
    target_link_libraries(
        engine
        SDL3::SDL3-shared
        android
        log
        EGL
        GLESv3
        glm::glm
        fmt::fmt)
else()
    target_link_libraries(engine glm::glm fmt::fmt SDL3::SDL3-shared)
endif()

add_module(engine glad ${PROJECT_SOURCE_DIR}/glad)
add_module(engine imgui ${PROJECT_SOURCE_DIR}/external/imgui)
