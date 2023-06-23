list(
    APPEND
    RESOURCE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/fonts/Roboto-Medium.ttf"
    "${CMAKE_CURRENT_SOURCE_DIR}/sounds/music.wav"
    "${CMAKE_CURRENT_SOURCE_DIR}/sounds/hit.wav"
    "${CMAKE_CURRENT_SOURCE_DIR}/ball/ball.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/bricks/yellow_brick.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/backgrounds/background1.png"
    "${CMAKE_CURRENT_SOURCE_DIR}/platform/platform1.png")

file(COPY ${RESOURCE_FILES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
