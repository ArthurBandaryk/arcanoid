# Official CMake doc doesn't recommend to use GLOB. Check this:
# https://cmake.org/cmake/help/latest/command/include_directories.html
list(APPEND SHADERS texture.vert texture.frag tex-no-math.vert tex-no-math.frag)
file(COPY ${SHADERS} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
