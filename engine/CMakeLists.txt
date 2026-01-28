file(GLOB_RECURSE ENGINE_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

add_library(engine STATIC ${ENGINE_SOURCES})

target_include_directories(engine
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(engine
  PUBLIC
    glfw
    glm
    EnTT::EnTT
    imgui
    sol2
)
