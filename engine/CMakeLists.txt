file(GLOB_RECURSE ENGINE_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/*.h
  ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

add_library(engine STATIC ${ENGINE_SOURCES})

# engine must wait for compiled shaders (headers are #included)
add_dependencies(engine compiled_shaders)

target_include_directories(engine
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE
    ${CMAKE_BINARY_DIR}/shaders  # compiled shader .bin.h headers
)

target_link_libraries(engine
  PUBLIC
    glfw
    bgfx
    glm
    EnTT::EnTT
    imgui
    sol2
)
