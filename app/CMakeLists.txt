set(APP_SOURCES
  src/main.cpp
)

add_executable(simupart ${APP_SOURCES})

target_link_libraries(simupart PRIVATE engine)

# Copy shader files next to the executable so the engine can load them at runtime
add_custom_command(TARGET simupart POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/engine/shaders
    $<TARGET_FILE_DIR:simupart>/shaders
)
