set(APP_SOURCES
  src/main.cpp
)

add_executable(simupart ${APP_SOURCES})

target_link_libraries(simupart PRIVATE engine)
